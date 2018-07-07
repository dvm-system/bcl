//===--- Socket.h -------------- Socket Abstraction -------------*- C++ -*-===//
//
//                       Base Construction Library (BCL)
//
// Copyright 2018 Nikita Kataev
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
// This file defines interface to connect different client and server through
// a network. Client must implements the Socket abstract class, and server
// must implements createServer() method. After that client creates and server
// using this implementation and passes created socket as a parameter.
//
//===----------------------------------------------------------------------===//
#ifndef BCL_SOCKET_H
#define BCL_SOCKET_H

#include <functional>
#include <string>

#if defined _WIN32 || defined _CYGWIN
#  ifdef BCL_EXPORTING
#    ifdef __GNUC__
#      define BCL_DECLSPEC __attribute__ ((dllexport))
#    else
#      define BCL_DECLSPEC __declspec(dllexport)
#    endif
#  else
#    ifdef __GNUC__
#      define BCL_DECLSPEC __attribute__ (dllimport))
#    else
#      define BCL_DECLSPEC __declspec(dllimport)
#    endif
#  endif
#else
#  if __GNUC__ > 4
#    define BCL_DECLSPEC __attribute__ ((visibility ("default")))
#  else
#    define BCL_DECLSPEC
#  endif
#endif

namespace bcl {
/// Interface to connect different entities in a network.
template<class MessageTy_ = std::string>
struct Socket {
  /// Message representation.
  typedef MessageTy_ MessageTy;

  /// This represents a prototype of listeners which are invoked when some data
  /// are received.
  typedef std::function<void(const MessageTy &)> ReceiveCallback;

  /// \brief This represents a prototype of listeners which are invoked when
  /// the socket become closed.
  ///
  /// Access to the socket inside this callback leads to undefined behavior.
  typedef std::function<void(bool)> ClosedCallback;

  /// Destructor.
  virtual ~Socket() {}

  /// Sends a message.
  virtual void send(const MessageTy &Message) const = 0;

  /// Adds the listener function to the end of array of listeners, which are
  /// invoked when some data are received.
  virtual void receive(const ReceiveCallback &F) const = 0;

  /// Adds the listener function to the end of array of listeners, which are
  /// invoked when the socket become closed.
  virtual void closed(const ClosedCallback &F) const = 0;
};

/// Run a server and use a specified socket to provide communications.
template<class MessageTy> BCL_DECLSPEC
void createServer(const Socket<MessageTy> *S);
}

#endif//BCL_SOCKET_H
