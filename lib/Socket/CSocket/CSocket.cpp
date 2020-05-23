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

#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>

namespace {
class SocketImp: public bcl::Socket<std::string> {
public:
  SocketImp(bcl::net::SocketT ConnectionFD, bcl::net::Connection &Connection,
      std::size_t BufferSize, const bcl::net::SocketStatusHandler &on)
     : mConnectionFD(ConnectionFD)
     , mConnection(Connection)
     , mBufferSize(BufferSize)
     , mOn(on) {}

  void send(const std::string &Message) const override {
    if (write(mConnectionFD, Message.c_str(), Message.length()) < 0) {
      mOn(bcl::net::SocketStatus::SendError, mConnection);
      closeSocket(false);
    }
    mOn(bcl::net::SocketStatus::Send, mConnection);
  }

  void receive(const ReceiveCallback &F) const override {
    mReceiveCallbacks.push_back(F);
  }

  void closed(const ClosedCallback &F) const override {
    mClosedCallbacks.push_back(F);
  }

  int run() {
    bcl::createServer(this);
    auto Buffer = new char[mBufferSize + 1];
    for (;;) {
      auto ReceivedSize = recv(mConnectionFD, Buffer, mBufferSize, 0);
      if (ReceivedSize == 0) {
        closeSocket(true);
        return 0;
      }
      if (ReceivedSize < 0) {
        mOn(bcl::net::SocketStatus::ReceiveError, mConnection);
        closeSocket(false);
        return 1;
      }
      Buffer[ReceivedSize] = '\0';
      mOn(bcl::net::SocketStatus::Receive, mConnection);
      for (auto &Callback : mReceiveCallbacks)
        Callback(Buffer);
      if (mClosed)
        return 1;
    }
  }

 private:
  void closeSocket(bool IsOk) const {
    mClosed = true;
    if (close(mConnectionFD) < 0) {
      IsOk = false;
      mOn(bcl::net::SocketStatus::CloseError, mConnection);
    } else {
      mOn(bcl::net::SocketStatus::Close, mConnection);
    }
    for (auto &Callback : mClosedCallbacks)
      Callback(IsOk);
  }

  bcl::net::SocketT mConnectionFD;
  bcl::net::Connection mConnection;
  std::size_t mBufferSize;
  bcl::net::SocketStatusHandler mOn;
  mutable std::vector<ReceiveCallback> mReceiveCallbacks;
  mutable std::vector<ClosedCallback> mClosedCallbacks;
  mutable bool mClosed;
};
}

void bcl::net::startServer(const net::AddressT &Address, net::PortT PortNo,
    std::size_t ConnectionMaxNumber, const net::SocketStatusHandler &on,
    std::size_t BufferSize) {
  net::Connection Connection(Address, PortNo);
  auto closeSocket = [&on](net::SocketT SocketFD, const net::Connection &C) {
    if (close(SocketFD) < 0)
      on(net::SocketStatus::CloseError, C);
    else
      on(net::SocketStatus::Close, C);
  };
  auto SocketFD = socket(AF_INET, SOCK_STREAM, 0);
  if (SocketFD < 0) {
    on(net::SocketStatus::CreateError, Connection);
    return;
  }
  int Opt = 1;
  // Use it to enable binding while socket is in TIME_WAIT state.
  if (setsockopt(SocketFD, SOL_SOCKET, SO_REUSEADDR, &Opt, sizeof(Opt)) < 0) {
    on(net::SocketStatus::OptionError, Connection);
    closeSocket(SocketFD, Connection);
    return;
  }
  sockaddr_in ServerAddr;
  ServerAddr.sin_family = AF_INET;
  ServerAddr.sin_port = htons(PortNo);
  if (Address.empty()) {
    ServerAddr.sin_addr.s_addr = INADDR_ANY;
  } else {
    auto Host = gethostbyname(Address.c_str());
    if (!Host) {
      on(net::SocketStatus::HostnameError, Connection);
      closeSocket(SocketFD, Connection);
      return;
    }
    memmove(&ServerAddr.sin_addr.s_addr,
      Host->h_addr_list[0], Host->h_length);
  }
  if (bind(SocketFD, (sockaddr *)&ServerAddr, sizeof(ServerAddr)) < 0) {
    on(net::SocketStatus::BindError, Connection);
    closeSocket(SocketFD, Connection);
    return;
  }
  if (listen(SocketFD, 5)) {
    on(net::SocketStatus::ListenError, Connection);
    closeSocket(SocketFD, Connection);
    return;
  }
  on(net::SocketStatus::Listen, Connection);
  std::unordered_map<net::SocketT, std::future<int>> ActiveSockets;
  if (ConnectionMaxNumber == 0 || ConnectionMaxNumber > ActiveSockets.max_size())
    ConnectionMaxNumber = ActiveSockets.max_size();
  auto engine =
    [BufferSize, &on, closeSocket](
        net::SocketT ConnectionFD, net::Connection &&C) -> int {
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
    net::SocketT ConnectionFD =
      accept(SocketFD, (sockaddr *)&ClientAddr, &ClientAddrLength);
    if (ConnectionFD < 0) {
      on(net::SocketStatus::AcceptError, Connection);
      continue;
    }
    sockaddr_in ActualServerAddr;
    socklen_t ActualServerAddrLength;
    if (getsockname(ConnectionFD,
         (sockaddr *)&ActualServerAddr, &ActualServerAddrLength) < 0) {
      on(net::SocketStatus::ServerAddressError, Connection);
      closeSocket(ConnectionFD, Connection);
      continue;
    }
    net::Connection NewConnection(
      inet_ntoa(ActualServerAddr.sin_addr), PortNo,
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
  closeSocket(SocketFD, Connection);
}
