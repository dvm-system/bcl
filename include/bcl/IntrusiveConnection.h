//===- IntrusiveConnection.h -- Intrusive Connection ------------*- C++ -*-===//
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
// This file proposes intrusive connection between client and server.
// Connection is intrusive because a server sends data to a client only when
// it receive a request and a new request can be received only when the
// previous one has been responded. It looks like this:
//                          Client           Server
//                            Request ->
//                                    <- Response
//                            Request ->
//                                    <- Response
//
// There are two main functions:
// - IntrusiveConnection::connect() starts connection;
// - IntrusiveConnection::answer() answers a request.
// Note that InstrusiveConnection can not be copied only move operations are
// supported. This is due to the fact that when InstrusiveConnection is
// destructed a connection will be automatically closed and all subsequent
// requests is going to be rejected (client will receive
// InstrusiveConnection::Reject response).
//
// Usage:
//  void foo(bcl::IntrusiveConnection C) {
//    do {
//      C.answer([](const std::string &Request) {
//         /* Answer a request */
//      });
//    } while (/*Exit if any requests are expected.*/);
//  }
//
//  template<>
//  void bcl::createServer<std::string>(const Socket<std::string> *S) {
//    IntrusiveConnection::connect(S, '\0', foo);
//}
//
//===----------------------------------------------------------------------===//

#ifndef BCL_INTRUSIVE_CONNECTION_H
#define BCL_INTRUSIVE_CONNECTION_H

#include "Socket.h"
#include <condition_variable>
#include <mutex>
#include <queue>
#include <sstream>
#include <thread>

namespace bcl {
/// Intrusive connection between client and server (look at the file beginning
/// for details.
class IntrusiveConnection {
  struct Status {
    std::mutex mReceiveMutex, mSendMutex;
    std::condition_variable mReceiveEvent, mSendEvent;
    std::string mReceiveData, mSendData;
    bool mIsReceive = false, mIsSend = false, mClose = false, mReject = false;
  };

  explicit IntrusiveConnection(Status *I) : mStatus(I) {}

public:
  /// This response is going to be set if connection is closed but request
  /// has been received.
  struct Reject {
    operator const std::string & () {
      static const std::string Code = "REJECT";
      return Code;
    }
  };

  IntrusiveConnection(const IntrusiveConnection &) = delete;
  IntrusiveConnection & operator=(const IntrusiveConnection &) = delete;

  /// Move constructor.
  IntrusiveConnection(IntrusiveConnection &&C) : mStatus(C.mStatus) {
    C.mStatus = nullptr;
  }

  /// Move assignment.
  IntrusiveConnection & operator=(IntrusiveConnection &&C) {
    if (this == &C)
      return *this;
    mStatus = C.mStatus;
    C.mStatus = nullptr;
  }

  /// Destructor.
  ~IntrusiveConnection() {
    if (!mStatus)
      return;
    {
      std::unique_lock<std::mutex> Lock(mStatus->mSendMutex);
      mStatus->mReject = true;
    }
    mStatus->mSendEvent.notify_one();
  }

  /// \brief Receives a request, evaluates it and sends response.
  ///
  /// \param [in] F A function with prototype std::string(const std::string &).
  /// A parameter is a request and a return value is a response.
  /// \return true on success, otherwise returns false. If this function
  /// returns false then function 'F' has not been invoked, connection
  /// is broken and socket is closed.
  template<class Function> bool answer(Function F) {
    if (!mStatus)
      return false;
    std::string Data;
    bool Close = false;
    {
      std::unique_lock<std::mutex> Lock(mStatus->mReceiveMutex);
      mStatus->mReceiveEvent.wait(Lock,
        [this] { return mStatus->mIsReceive || mStatus->mClose; });
      Close = mStatus->mClose;
      Data = mStatus->mReceiveData;
      mStatus->mReceiveData.clear();
      mStatus->mIsReceive = false;
    }
    if (Close) {
      {
        std::unique_lock<std::mutex> Lock(mStatus->mSendMutex);
        mStatus->mReject = true;
      }
      mStatus->mSendEvent.notify_one();
      mStatus = nullptr;
      return false;
    }
    Data = F(Data);
    {
      std::unique_lock<std::mutex> Lock(mStatus->mSendMutex);
      mStatus->mSendData = Data;
      mStatus->mIsSend = true;
    }
    mStatus->mSendEvent.notify_one();
    return true;
  }

  /// \brief Starts connection.
  ///
  /// \param [in] S Socket which provides connection.
  /// \param [in] Delimiter This separates different requests.
  /// \param [in, out] F Implementation of a server with prototype
  /// void(IntrusiveConnection, ArgsTy...)
  /// \param [in, out] Args Arguments which will be passed to F when server
  /// is be started.
  ///
  /// Sometimes multiple requests may be joined. If a client send:
  /// 'foo' then 'bar', the server may receive two different requests 'foo' then
  /// 'bar' or a single request 'foobar'. To avoid the last case the Delimiter
  /// parameter can be used.
  /// For example, if it is known that a client uses '$' to separate different
  /// requests ('foo$' then 'bar$'), it is possible to set Delimiter parameter
  /// to '$'. Note that the delimiter also used when server sends responses
  /// (even for REJECT response), so it will send 'response to foo$' then
  /// 'response to bar$'. To avoid use of delimiters set Delimiter parameter
  /// to '\0'.
  template<class Function, class... ArgsTy>
  static void connect(const bcl::Socket<std::string> *S,
      char Delimiter, Function &&F, ArgsTy&&... Args) {
    auto CS = new Status;
    std::thread Engine(std::forward<Function>(F),
      IntrusiveConnection(CS), std::forward<ArgsTy>(Args)...);
    Engine.detach();
    auto *Reject = new bool(false);
    S->closed([Reject, CS](bool) {
      if (!*Reject) {
        {
          std::unique_lock<std::mutex> Lock(CS->mReceiveMutex);
          CS->mClose = true;
        }
        CS->mReceiveEvent.notify_one();
        {
          std::unique_lock<std::mutex> Lock(CS->mSendMutex);
          CS->mSendEvent.wait(Lock, [CS] { return CS->mReject; });
        }
        delete CS;
      }
      delete Reject;
    });
    // It is not possible to store socket S in connection status CS, because
    // when CS will be deleted it is necessary to access S to send REJECT
    // response.
    S->receive([Reject, Delimiter, S, CS](const std::string &Request) {
      if (*Reject) {
        S->send(std::string(IntrusiveConnection::Reject()) + Delimiter);
        return;
      }
      std::queue<std::string> Buffer;
      std::stringstream IS(Request);
      std::string Item;
      while (std::getline(IS, Item, Delimiter) && !IS.eof())
        Buffer.push(Item);
      // If there is no delimiter after last not empty string than this
      // substring will be treated as a separate request.
      // The last empty substring will be ignored.
      if (!Item.empty())
        Buffer.push(Item);
      while (!Buffer.empty()) {
        {
          std::unique_lock<std::mutex> Lock(CS->mReceiveMutex);
          CS->mIsReceive = true;
          CS->mReceiveData = Buffer.front();
        }
        CS->mReceiveEvent.notify_one();
        Buffer.pop();
        std::string Data;
        {
          std::unique_lock<std::mutex> Lock(CS->mSendMutex);
          CS->mSendEvent.wait(Lock,
            [CS] { return CS->mIsSend || CS->mReject; });
          // If CS->mReject is true, it is possible that there are some data,
          // that have not been sent yet, so check it.
          Data = CS->mIsSend ? CS->mSendData : IntrusiveConnection::Reject();
          *Reject = CS->mReject;
          CS->mSendData.clear();
          CS->mIsSend = false;;
        }
        S->send(Data + Delimiter);
        if (*Reject) {
          delete CS;
          break;
        }
      }
    });
  }

private:
  Status *mStatus;
};
}

#endif//BCL_INTRUSIVE_CONNECTION_H
