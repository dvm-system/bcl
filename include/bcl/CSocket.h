//===-- CSocket.h ------------ Socket Implementation -----------*- C++ -*-===//
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

#include <bcl/bcl-config.h>
// Disable this header if CSocket library is not available.
#ifndef BCL_C_SOCKET
# define BCL_C_SOCKET_H
#endif

#ifndef BCL_C_SOCKET_H
#define BCL_C_SOCKET_H

#include <bcl/Socket.h>
#include <functional>

namespace bcl {
namespace net {

using PortT = int;
using AddressT = std::string;

enum class SocketStatus : uint8_t {
  UnknownError,
  InitializeError,
  HostnameError,
  ServerAddressError,
  CreateError,
  OptionError,
  BindError,
  ListenError,
  AcceptError,
  ReceiveError,
  SendError,
  CloseError,
  Listen,
  Accept,
  Receive,
  Send,
  Close,
};

/// Description of a network connection. If connection is active then
/// addresses of a server and a client are both available.
class Connection {
public:
  Connection(const AddressT &ServerAddress, PortT ServerPort)
    : mServerAddress(ServerAddress), mServerPort(ServerPort) {}

  Connection(
      const AddressT &ServerAddress, PortT ServerPort,
      const AddressT &ClientAddress, PortT ClientPort)
    : mServerAddress(ServerAddress), mServerPort(ServerPort)
    , mClientAddress(ClientAddress), mClientPort(ClientPort) {}

  AddressT getServerAddress() const noexcept { return mServerAddress; }
  PortT getServerPort() const noexcept { return mServerPort; }

  AddressT getClientAddress() const noexcept { return mClientAddress; }
  PortT getClientPort() const noexcept { return mClientPort; }

  bool isActive() const noexcept { return mServerPort > 0 && mClientPort > 0; }

private:
  AddressT mServerAddress;
  PortT mServerPort = 0;
  AddressT mClientAddress;
  PortT mClientPort = 0;
};

using SocketStatusHandler = std::function<void(SocketStatus, const Connection &)>;

/// Start server which is listening for connection.
///
/// If new connection is established a separate thread is launched to maintain
/// communcation.
/// \param [in] Address Host name or an IPv4 address in standard dot notation.
/// \param [in] PortNo Server port number.
/// \param [in] ConnectionMaxNumber Maximum number of connections which can
///             be active at the same time. Note, that an actual number of
///             connections cannot exceed a maximum number of sockets
///             that cannot be opened simultaneously.
///             Use 0 to disable connection limits
/// \param [in] on Handler which will be invoked to process any event.
///             All possible events are listed in bcl::net::SocketStatus.
/// \param [in] BufferSize Size of a buffer to store received chunks of data.
void startServer(const net::AddressT &Address, net::PortT PortNo,
    std::size_t ConnectionMaxNumber = 0,
    const net::SocketStatusHandler &on =
      [](net::SocketStatus, const net::Connection &){},
    std::size_t BufferSize = 65535);
}
}
#endif//BCL_C_SOCKET_H

