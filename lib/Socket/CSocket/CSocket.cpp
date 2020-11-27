//===-- CSocket.cpp ---------- Socket Implementation -----------*- C++ -*-===//
//
//                       Base Construction Library (BCL)
//
// Copyright 2020 Nikita Kataev
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
//===----------------------------------------------------------------------===//
//
// This file implements a network server, which is based on C sockets,
// to listen incoming connections from clients. This network server relies on
// createServer() from bcl/Socket.h to process client's requests.
// Multiple clinets can be connected at the same time.
// A separate thread is used to maintain each connection.
//
//===----------------------------------------------------------------------===//

#include <bcl/CSocket.h>
#include <bcl/utility.h>
#include <cassert>
#include <csignal>
#include <chrono>
#include <future>
#include <string>
#include <thread>
#include <vector>
#include <unordered_map>

#if _WIN32
# include <ws2tcpip.h>
# include <winsock2.h>
#else
# include <arpa/inet.h>
# include <sys/socket.h>
# include <netdb.h>
# include <netinet/in.h>
# include <unistd.h>
#endif

using namespace bcl;

#ifdef _WIN32
using SocketT = SOCKET;

struct AddressInfoT : private bcl::Uncopyable {
  ~AddressInfoT() {
    if (Address)
      freeaddrinfo(Address);
  }
  addrinfo *Address = nullptr;
};

static inline bool initialize() {
  WSADATA WsaData;
  return WSAStartup(MAKEWORD(2, 2), &WsaData) == 0;
}

static inline void finalize() { WSACleanup(); }

static inline bool closeSocket(SocketT S) {
  return closesocket(S) != SOCKET_ERROR;
}

static inline bool getAddressInfo(int Domain, int Type, int Protocol,
    const net::AddressT &Address, net::PortT PortNo, AddressInfoT &Result) {
  addrinfo Hints;
  std::memset(&Hints, 0, sizeof(Hints));
  Hints.ai_family = Domain;
  Hints.ai_socktype = Type;
  Hints.ai_protocol = Protocol;
  Hints.ai_flags = AI_PASSIVE;
  return (getaddrinfo(!Address.empty() ? Address.c_str() : nullptr,
                      std::to_string(PortNo).c_str(), &Hints,
                      &Result.Address) == 0)
             ? true
             : false;
}

static inline std::pair<SocketT, bool>
createSocket(const AddressInfoT &AddressInfo) {
  auto S =
      socket(AddressInfo.Address->ai_family, AddressInfo.Address->ai_socktype,
             AddressInfo.Address->ai_protocol);
  if (S != INVALID_SOCKET)
    return std::make_pair(S, true);
  return std::make_pair(INVALID_SOCKET, false);
}

static inline bool setSocketOptions(SocketT S) {
  char Opt = 1;
  // Use it to enable binding while socket is in TIME_WAIT state.
  return setsockopt(S, SOL_SOCKET, SO_REUSEADDR, &Opt, sizeof(Opt)) >= 0;
}

static inline bool bindSocket(SocketT S, const AddressInfoT &AddressInfo) {
  return bind(S, AddressInfo.Address->ai_addr,
              (int)AddressInfo.Address->ai_addrlen) != SOCKET_ERROR;
}

static inline bool sendData(SocketT S, const std::string &Message) {
  return send(S, Message.c_str(), Message.length(), 0) != SOCKET_ERROR;
}

static inline std::pair<std::size_t, bool> receiveData(SocketT S,
    char *Buffer, std::size_t BufferSize) {
  auto ReceivedSize = recv(S, Buffer, BufferSize, 0);
  if (ReceivedSize == SOCKET_ERROR)
    return std::make_pair(0, false);
  return std::make_pair(ReceivedSize, true);
}
#else
using SocketT = int;

struct AddressInfoT {
  int Domain;
  int Type;
  int Protocol;
  sockaddr_in Address;
};

static inline bool initialize() noexcept { return true; }
static inline void finalize() noexcept {}

static inline bool closeSocket(SocketT S) { return close(S) >= 0; }

static inline bool getAddressInfo(int Domain, int Type, int Protocol,
    const net::AddressT &Address, net::PortT PortNo,
    AddressInfoT &Result) noexcept {
  Result.Domain = Domain;
  Result.Type = Type;
  Result.Protocol = Protocol;
  Result.Address.sin_family = AF_INET;
  Result.Address.sin_port = htons(PortNo);
  if (Address.empty()) {
    Result.Address.sin_addr.s_addr = INADDR_ANY;
  } else {
    auto Host = gethostbyname(Address.c_str());
    if (!Host)
      return false;
    memmove(&Result.Address.sin_addr.s_addr,
      Host->h_addr_list[0], Host->h_length);
  }
  return true;
}

static inline std::pair<SocketT, bool>
createSocket(const AddressInfoT &AddressInfo) {
  auto SocketFD =
      socket(AddressInfo.Domain, AddressInfo.Type, AddressInfo.Protocol);
  return std::make_pair(SocketFD, SocketFD >= 0);
}

static inline bool setSocketOptions(SocketT S) {
  int Opt = 1;
  // Use it to enable binding while socket is in TIME_WAIT state.
  return setsockopt(S, SOL_SOCKET, SO_REUSEADDR, &Opt, sizeof(Opt)) >= 0;
}

static inline bool bindSocket(SocketT S, const AddressInfoT &AddressInfo) {
  return bind(S, (sockaddr *)&AddressInfo.Address,
              sizeof(AddressInfo.Address)) >= 0;
}

static inline bool sendData(SocketT S, const std::string &Message) {
  return send(S, Message.c_str(), Message.length(), 0) >= 0;
}

static inline std::pair<std::size_t, bool> receiveData(SocketT S,
    char *Buffer, std::size_t BufferSize) {
  auto ReceivedSize = recv(S, Buffer, BufferSize, 0);
  if (ReceivedSize < 0)
    return std::make_pair(0, false);
  return std::make_pair(ReceivedSize, true);
}
#endif

namespace {
class SocketImp: public bcl::Socket<std::string> {
  enum class State : uint8_t {
    Open,
    OnClose,
    Closed,
  };
public:
  SocketImp(SocketT ConnectionFD, bcl::net::Connection &Connection,
      std::size_t BufferSize, const bcl::net::SocketStatusHandler &on)
     : mConnectionFD(ConnectionFD)
     , mConnection(Connection)
     , mBufferSize(BufferSize)
     , mOn(on) {}

  void send(const std::string &Message) const override {
    if (!sendData(mConnectionFD, Message)) {
      mOn(bcl::net::SocketStatus::SendError, mConnection);
      mState = State::OnClose;
    } else {
      mOn(bcl::net::SocketStatus::Send, mConnection);
    }
  }

  void receive(const ReceiveCallback &F) const override {
    mReceiveCallbacks.push_back(F);
  }

  void closed(const ClosedCallback &F) const override {
    mClosedCallbacks.push_back(F);
  }

  int run() {
    bcl::createServer(this);
    auto Buffer = bcl::make_unique<char[]>(mBufferSize + 1);
    for (;;) {
      auto ReceivedInfo = receiveData(mConnectionFD, Buffer.get(), mBufferSize);
      if (!ReceivedInfo.second) {
        mOn(bcl::net::SocketStatus::ReceiveError, mConnection);
        closeSocket(false);
        return 1;
      }
      if (ReceivedInfo.first == 0) {
        closeSocket(true);
        return 0;
      }
      Buffer.get()[ReceivedInfo.first] = '\0';
      mOn(bcl::net::SocketStatus::Receive, mConnection);
      for (auto &Callback : mReceiveCallbacks)
        Callback(std::string(Buffer.get()));
      if (mState == State::OnClose) {
        closeSocket(false);
        return 1;
      }
    }
  }
  std::string Data;
 private:
  void closeSocket(bool IsOk) const {
    mState = State::Closed;
    if (!::closeSocket(mConnectionFD)) {
      IsOk = false;
      mOn(bcl::net::SocketStatus::CloseError, mConnection);
    } else {
      mOn(bcl::net::SocketStatus::Close, mConnection);
    }
    for (auto &Callback : mClosedCallbacks)
      Callback(IsOk);
  }

  SocketT mConnectionFD;
  bcl::net::Connection mConnection;
  std::size_t mBufferSize;
  bcl::net::SocketStatusHandler mOn;
  mutable std::vector<ReceiveCallback> mReceiveCallbacks;
  mutable std::vector<ClosedCallback> mClosedCallbacks;
  mutable State mState = State::Open;
};
}

void bcl::net::startServer(const net::AddressT &Address, net::PortT PortNo,
    std::size_t ConnectionMaxNumber, const net::SocketStatusHandler &on,
    std::size_t BufferSize) {
  net::Connection PreConnection(Address, PortNo);
  if (!initialize()) {
    on(net::SocketStatus::InitializeError, PreConnection);
    return;
  }
  auto closeAndLog = [&on](SocketT SocketFD, const net::Connection &C) {
    if (!closeSocket(SocketFD))
      on(net::SocketStatus::CloseError, C);
    else
      on(net::SocketStatus::Close, C);
  };
  SocketT SocketFD;
  {
    // Start scope here to early destroy AddressInfo.
    AddressInfoT AddressInfo;
    if (!getAddressInfo(AF_INET, SOCK_STREAM, 0, Address, PortNo,
                        AddressInfo)) {
      on(net::SocketStatus::HostnameError, PreConnection);
      finalize();
      return;
    }
    auto SocketInfo = createSocket(AddressInfo);
    if (!SocketInfo.second) {
      on(net::SocketStatus::CreateError, PreConnection);
      finalize();
      return;
    }
    SocketFD = SocketInfo.first;
    // Use it to enable binding while socket is in TIME_WAIT state.
    if (!setSocketOptions(SocketFD)) {
      on(net::SocketStatus::OptionError, PreConnection);
      closeAndLog(SocketFD, PreConnection);
      finalize();
      return;
    }
    if (!bindSocket(SocketFD, AddressInfo)) {
      on(net::SocketStatus::BindError, PreConnection);
      closeAndLog(SocketFD, PreConnection);
      finalize();
      return;
    }
  }
  sockaddr_in ServerAddr;
  socklen_t ServerAddrLength = sizeof(ServerAddr);
  if (getsockname(SocketFD,
        (sockaddr *)&ServerAddr, &ServerAddrLength) != 0) {
    on(net::SocketStatus::ServerAddressError, PreConnection);
    closeAndLog(SocketFD, PreConnection);
    finalize();
    return;
  }
  net::Connection Connection(
    inet_ntoa(ServerAddr.sin_addr), ntohs(ServerAddr.sin_port));
  if (listen(SocketFD, 5)) {
    on(net::SocketStatus::ListenError, Connection);
    closeAndLog(SocketFD, Connection);
    finalize();
    return;
  }
  on(net::SocketStatus::Listen, Connection);
  std::unordered_map<SocketT, std::future<int>> ActiveSockets;
  if (ConnectionMaxNumber == 0 || ConnectionMaxNumber > ActiveSockets.max_size())
    ConnectionMaxNumber = ActiveSockets.max_size();
  auto engine =
    [BufferSize, &on, closeAndLog](
        SocketT ConnectionFD, net::Connection &&C) -> int {
      auto Engine = bcl::make_unique<SocketImp>(ConnectionFD, C, BufferSize, on);
      return Engine->run();
    };
  auto sanitizeConnections = [&ActiveSockets]() {
    auto Size = ActiveSockets.size();
    for (auto I = ActiveSockets.begin(), EI = ActiveSockets.end(); I != EI;) {
      auto Status = I->second.wait_for(std::chrono::seconds(0));
      if (Status == std::future_status::ready)
        I = ActiveSockets.erase(I);
      else
        ++I;
    }
    return Size != ActiveSockets.size();
  };
  for(;;) {
    if (ActiveSockets.size() == ConnectionMaxNumber) {
      while (!sanitizeConnections())
        std::this_thread::sleep_for(std::chrono::seconds(5));
    } else if (ActiveSockets.size() > ConnectionMaxNumber / 2) {
      sanitizeConnections();
    }
    sockaddr_in ClientAddr;
    socklen_t ClientAddrLength = sizeof(ClientAddr);
    SocketT ConnectionFD =
      accept(SocketFD, (sockaddr *)&ClientAddr, &ClientAddrLength);
    if (ConnectionFD < 0) {
      on(net::SocketStatus::AcceptError, Connection);
      continue;
    }
    sockaddr_in ActualServerAddr;
    socklen_t ActualServerAddrLength = sizeof(ActualServerAddr);
    if (getsockname(ConnectionFD,
         (sockaddr *)&ActualServerAddr, &ActualServerAddrLength) != 0) {
      on(net::SocketStatus::ServerAddressError, Connection);
      closeAndLog(ConnectionFD, Connection);
      continue;
    }
    net::Connection NewConnection(
      inet_ntoa(ActualServerAddr.sin_addr), ntohs(ActualServerAddr.sin_port),
      inet_ntoa(ClientAddr.sin_addr), ntohs(ClientAddr.sin_port));
    on(net::SocketStatus::Accept, NewConnection);
    auto EngineItr = ActiveSockets.emplace(std::piecewise_construct,
      std::forward_as_tuple(ConnectionFD),
      std::forward_as_tuple()).first;
    assert(!EngineItr->second.valid() ||
           EngineItr->second.wait_for(std::chrono::seconds(0))
             == std::future_status::ready && "Unable to join thread!");
    EngineItr->second = std::async(std::launch::async,
      engine, ConnectionFD, std::move(NewConnection));
  }
  closeAndLog(SocketFD, Connection);
  finalize();
}
