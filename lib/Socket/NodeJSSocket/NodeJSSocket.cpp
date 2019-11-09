//===- NodeJSSocket.cpp -- Node.js net.Socket Wrapper -----------*- C++ -*-===//
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
// This file implements a socket interface by using the net.Socket
// functionality. So this proposes Node.js Addon written on C++ which is
// useful to exchange messages between JavaScript and C++ applications.
// A C++ application must implement createServer() function from Socket.h and
// will be treated as a server which is run when JS application call
// startServer() function for a connection obtained after connect() call.
// After that application must be linked with this addon.
//
// Usage:
//  const connect = module.require('./bclSocket');
//
//  var server = net.createServer((socket) => {
//    let connection = connect(
//      (data) => {socket.write(data)},
//      (event, callback) => {socket.on(event, callback)});
//    connection.startServer();
//  });
//
// Now it is possible to use general 'net' module possibilities to exchange
// string messages between client (JS application) and server (C++ application).
//===----------------------------------------------------------------------===//

#include <node.h>
#include <node_object_wrap.h>
#include <functional>
#include <memory>
#include <string>
#include <stack>
#include "bcl/Socket.h"
#include "bcl/utility.h"

using namespace v8;
using namespace bcl;

namespace {
/// This implements a socket interface by using the net.Socket functionality.
class SocketImp: public Socket<std::string> {
public:
  /// \brief Create a wrapper of net.Socket.
  ///
  /// \param [in] I Isolated instance of the V8 engine.
  /// \param [in] Send A function which sends data to a socket, for example
  /// (data) => {socket.write(data)}
  /// \param [in] On A function which adds listeners functions to socket events,
  /// for example (event, callback) => {socket.on(event, callback)}
  SocketImp(Isolate *I, Local<Function> Send, Local<Function> On) :
    mIsolate(I), mSend(I, Send), mOn(I, On) {}

  /// Send a message.
  void send(const std::string &Message) const override {
    const unsigned Argc = 1;
    Local<Value> Argv[Argc] = {
        String::NewFromUtf8(mIsolate, Message.c_str()).ToLocalChecked()};
    Local<Context> Ctx = mIsolate->GetCurrentContext();
    Local<Function>::New(mIsolate, mSend)->Call(Ctx, Null(mIsolate), Argc, Argv);
  }

  /// Add the listener function to the end of array of listeners, which are
  /// invoked when some data are received.
  void receive(const ReceiveCallback &F) const override {
    mReceiveCallbacks.push(bcl::make_unique<ReceiveCallback>(F));
    Local<FunctionTemplate> Tpl =
      FunctionTemplate::New(mIsolate, receiveWrapper,
        External::New(mIsolate, mReceiveCallbacks.top().get()));
    Local<Context> Ctx = mIsolate->GetCurrentContext();
    Local<Function> JSF = Tpl->GetFunction(Ctx).ToLocalChecked();
    const unsigned Argc = 2;
    Local<Value> Argv[Argc] = {
        String::NewFromUtf8(mIsolate, "data").ToLocalChecked(), JSF};
    Local<Function>::New(mIsolate, mOn)->Call(Ctx, Null(mIsolate), Argc, Argv);
  }

  /// Add the listener function to the end of array of listeners, which are
  /// invoked when some data are received.
  void closed(const ClosedCallback &F) const override {
    mClosedCallbacks.push(bcl::make_unique<ClosedCallback>(F));
    Local<FunctionTemplate> Tpl =
      FunctionTemplate::New(mIsolate, closedWrapper,
        External::New(mIsolate, mClosedCallbacks.top().get()));
    Local<Context> Ctx = mIsolate->GetCurrentContext();
    Local<Function> JSF = Tpl->GetFunction(Ctx).ToLocalChecked();
    const unsigned Argc = 2;
    Local<Value> Argv[Argc] = {
        String::NewFromUtf8(mIsolate, "close").ToLocalChecked(), JSF};
    Local<Function>::New(mIsolate, mOn)->Call(Ctx, Null(mIsolate), Argc, Argv);
  }

private:
  /// This is a wrapper of a listener specified by a user, this is binded
  /// to a 'data' event when receive() is called.
  static void receiveWrapper(const FunctionCallbackInfo<Value> &Args) {
    Isolate *CurrIsolate  = Args.GetIsolate();
    if (Args.Length() < 1 || !Args[0]->IsUint8Array()) {
      CurrIsolate->ThrowException(Exception::TypeError(
          String::NewFromUtf8(CurrIsolate, "Unsupported type of received data!")
              .ToLocalChecked()));
      return;
    }
    auto Data = Local<Uint8Array>::Cast(Args[0]);
    auto DataUtf8 = new char [Data->ByteLength() + 1];
    Data->CopyContents(DataUtf8, Data->ByteLength());
    DataUtf8[Data->ByteLength()] = '\0';
    void *UserFunc = Local<External>::Cast(Args.Data())->Value();
    auto F = static_cast<ReceiveCallback *>(UserFunc);
    (*F)(std::string(DataUtf8));
  }

  /// This is a wrapper of a listener specified by a user, this is binded
  /// to a 'close' event when closed() is called.
  static void closedWrapper(const FunctionCallbackInfo<Value> &Args) {
    Isolate *CurrIsolate  = Args.GetIsolate();
    if (Args.Length() < 1 || !Args[0]->IsBoolean()) {
      CurrIsolate->ThrowException(Exception::TypeError(
        String::NewFromUtf8(CurrIsolate,
         "Unsupported type of 'had_error' parameter!").ToLocalChecked()));
      return;
    }
    auto HadError = Local<Boolean>::Cast(Args[0]);
    void *UserFunc = Local<External>::Cast(Args.Data())->Value();
    auto F = static_cast<ClosedCallback *>(UserFunc);
    (*F)(HadError->Value());
  }

  Isolate *mIsolate;
  Persistent<Function> mSend;
  Persistent<Function> mOn;
  mutable std::stack<std::unique_ptr<ReceiveCallback>> mReceiveCallbacks;
  mutable std::stack<std::unique_ptr<ClosedCallback>> mClosedCallbacks;
};

/// This represents client/server connection, server will be started when
/// connection is created.
class Connection : public node::ObjectWrap {
  typedef Socket<std::string> SocketTy;

public:
  /// Perform initialization to propose creation of new instances of an object.
  static void init(Isolate* I) {
    // Prepare constructor template
    Local<FunctionTemplate> Tpl = FunctionTemplate::New(I, New);
    Tpl->SetClassName(String::NewFromUtf8(I, "Connection").ToLocalChecked());
    Tpl->InstanceTemplate()->SetInternalFieldCount(1);
    NODE_SET_PROTOTYPE_METHOD(Tpl, "startServer", startServer);
    Local<Context> Ctx = I->GetCurrentContext();
    mCtor.Reset(I, Tpl->GetFunction(Ctx).ToLocalChecked());
  }

  /// Create new instance of this object.
  /// \param [in] Args Parameters must proposes two functions:
  /// - A function which sends data to a socket, for example
  /// (data) => {socket.write(data)}.
  /// - A function which adds listeners functions to socket events, for example
  /// (event, callback) => {socket.on(event, callback)}
  static void newInstance(const FunctionCallbackInfo<Value>& Args) {
    auto *CurrIsolate = Args.GetIsolate();
    const unsigned Argc = 2;
    Local<Value> Argv[Argc] = { Args[0], Args[1] };
    Local<Function> Ctor = Local<Function>::New(CurrIsolate, mCtor);
    Local<Context> Ctx = CurrIsolate->GetCurrentContext();
    Local<Object> Instance =
      Ctor->NewInstance(Ctx, Argc, Argv).ToLocalChecked();
    Args.GetReturnValue().Set(Instance);
  }

private:
  /// Constructor.
  explicit Connection(const SocketTy *S) : mSocket(S) {}

  /// Destructor.
  ~Connection() { delete mSocket; }

  /// Start execution of a server.
  static void startServer(const FunctionCallbackInfo<Value>& Args) {
    auto* C = ObjectWrap::Unwrap<Connection>(Args.Holder());
    C->mSocket->closed([C](bool){ C->Unref(); });
    createServer(C->mSocket);
  }

  /// Create new instance of this object.
  /// \param [in] Args Parameters must proposes two functions:
  /// - A function which sends data to a socket, for example
  /// (data) => {socket.write(data)}.
  /// - A function which adds listeners functions to socket events, for example
  /// (event, callback) => {socket.on(event, callback)}
  static void New(const FunctionCallbackInfo<Value>& Args) {
    if (Args.IsConstructCall()) {
      auto *CurrIsolate  = Args.GetIsolate();
      if (Args.Length() < 2) {
      CurrIsolate->ThrowException(Exception::TypeError(
        String::NewFromUtf8(CurrIsolate,
          "Too few parameters, two parameters expected!").ToLocalChecked()));
      return;
      }
      if (!Args[0]->IsFunction() || !Args[1]->IsFunction()) {
        CurrIsolate->ThrowException(Exception::TypeError(
          String::NewFromUtf8(CurrIsolate,
            "The first and the second parameters must be functions!")
          .ToLocalChecked()));
        return;
      }
      auto S = new SocketImp(CurrIsolate,
        Local<Function>::Cast(Args[0]),
        Local<Function>::Cast(Args[1]));
      auto *C = new Connection(S);
      C->Wrap(Args.This());
      C->Ref();
      Args.GetReturnValue().Set(Args.This());
    } else {
      newInstance(Args);
    }
  }

  static Persistent<Function> mCtor;
  const SocketTy *mSocket;
};

Persistent<Function> Connection::mCtor;

void connect(const FunctionCallbackInfo<Value>& Args) {
  Connection::newInstance(Args);
}

void init(Local<Object> Exports, Local<Object> Module) {
  Connection::init(Exports->GetIsolate());
  NODE_SET_METHOD(Module, "exports", connect);
}

NODE_MODULE(bclSocket, init)
}

