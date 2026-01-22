#include <nan.h>
#include "ipc.h"
#include <map>

using namespace v8;
using namespace ipc;

// Store channels by ID
static std::map<uint32_t, Channel*> channels;
static uint32_t nextChannelId = 1;

NAN_METHOD(IsProcessRunning) {
  if (info.Length() < 1 || !info[0]->IsNumber()) {
    Nan::ThrowTypeError("Argument must be a number (process ID)");
    return;
  }

  uint32_t pid = Nan::To<uint32_t>(info[0]).FromJust();
  bool running = isProcessRunning(static_cast<DWORD>(pid));
  info.GetReturnValue().Set(Nan::New(running));
}

NAN_METHOD(GenerateChannelName) {
  std::string name = generateChannelName();
  info.GetReturnValue().Set(Nan::New(name).ToLocalChecked());
}

NAN_METHOD(CreateChannel) {
  if (info.Length() < 2 || !info[0]->IsString() || !info[1]->IsBoolean()) {
    Nan::ThrowTypeError("Arguments: (name: string, isServer: boolean)");
    return;
  }

  Nan::Utf8String name(info[0]);
  bool isServer = Nan::To<bool>(info[1]).FromJust();

  Channel* channel = new Channel(std::string(*name), isServer);
  uint32_t id = nextChannelId++;
  channels[id] = channel;

  info.GetReturnValue().Set(Nan::New(id));
}

NAN_METHOD(ChannelConnect) {
  if (info.Length() < 1 || !info[0]->IsNumber()) {
    Nan::ThrowTypeError("Argument must be a channel ID");
    return;
  }

  uint32_t id = Nan::To<uint32_t>(info[0]).FromJust();

  std::map<uint32_t, Channel*>::iterator it = channels.find(id);
  if (it == channels.end()) {
    info.GetReturnValue().Set(Nan::New(false));
    return;
  }

  bool success = it->second->connect();
  info.GetReturnValue().Set(Nan::New(success));
}

NAN_METHOD(ChannelSend) {
  if (info.Length() < 2 || !info[0]->IsNumber()) {
    Nan::ThrowTypeError("Arguments: (channelId: number, data: string|Buffer)");
    return;
  }

  uint32_t id = Nan::To<uint32_t>(info[0]).FromJust();

  std::map<uint32_t, Channel*>::iterator it = channels.find(id);
  if (it == channels.end()) {
    info.GetReturnValue().Set(Nan::New(false));
    return;
  }

  bool success = false;

  if (info[1]->IsString()) {
    Nan::Utf8String str(info[1]);
    success = it->second->send(*str, str.length());
  } else if (node::Buffer::HasInstance(info[1])) {
    char* data = node::Buffer::Data(info[1]);
    size_t length = node::Buffer::Length(info[1]);
    success = it->second->send(data, length);
  }

  info.GetReturnValue().Set(Nan::New(success));
}

NAN_METHOD(ChannelReceive) {
  if (info.Length() < 1 || !info[0]->IsNumber()) {
    Nan::ThrowTypeError("Argument must be a channel ID");
    return;
  }

  uint32_t id = Nan::To<uint32_t>(info[0]).FromJust();

  std::map<uint32_t, Channel*>::iterator it = channels.find(id);
  if (it == channels.end()) {
    info.GetReturnValue().SetNull();
    return;
  }

  std::string data = it->second->receive();

  if (data.empty()) {
    info.GetReturnValue().SetNull();
  } else {
    info.GetReturnValue().Set(Nan::CopyBuffer(data.c_str(), data.size()).ToLocalChecked());
  }
}

NAN_METHOD(ChannelClose) {
  if (info.Length() < 1 || !info[0]->IsNumber()) {
    Nan::ThrowTypeError("Argument must be a channel ID");
    return;
  }

  uint32_t id = Nan::To<uint32_t>(info[0]).FromJust();

  std::map<uint32_t, Channel*>::iterator it = channels.find(id);
  if (it != channels.end()) {
    it->second->close();
    delete it->second;
    channels.erase(it);
  }
}

NAN_METHOD(ChannelIsConnected) {
  if (info.Length() < 1 || !info[0]->IsNumber()) {
    Nan::ThrowTypeError("Argument must be a channel ID");
    return;
  }

  uint32_t id = Nan::To<uint32_t>(info[0]).FromJust();

  std::map<uint32_t, Channel*>::iterator it = channels.find(id);
  if (it == channels.end()) {
    info.GetReturnValue().Set(Nan::New(false));
    return;
  }

  info.GetReturnValue().Set(Nan::New(it->second->isConnected()));
}

NAN_METHOD(ChannelIsServer) {
  if (info.Length() < 1 || !info[0]->IsNumber()) {
    Nan::ThrowTypeError("Argument must be a channel ID");
    return;
  }

  uint32_t id = Nan::To<uint32_t>(info[0]).FromJust();

  std::map<uint32_t, Channel*>::iterator it = channels.find(id);
  if (it == channels.end()) {
    info.GetReturnValue().Set(Nan::New(false));
    return;
  }

  info.GetReturnValue().Set(Nan::New(it->second->isServer()));
}

void InitIPC(Local<Object> exports) {
  Nan::Set(exports, Nan::New("ipcIsProcessRunning").ToLocalChecked(),
    Nan::GetFunction(Nan::New<FunctionTemplate>(IsProcessRunning)).ToLocalChecked());

  Nan::Set(exports, Nan::New("ipcGenerateChannelName").ToLocalChecked(),
    Nan::GetFunction(Nan::New<FunctionTemplate>(GenerateChannelName)).ToLocalChecked());

  Nan::Set(exports, Nan::New("ipcCreateChannel").ToLocalChecked(),
    Nan::GetFunction(Nan::New<FunctionTemplate>(CreateChannel)).ToLocalChecked());

  Nan::Set(exports, Nan::New("ipcChannelConnect").ToLocalChecked(),
    Nan::GetFunction(Nan::New<FunctionTemplate>(ChannelConnect)).ToLocalChecked());

  Nan::Set(exports, Nan::New("ipcChannelSend").ToLocalChecked(),
    Nan::GetFunction(Nan::New<FunctionTemplate>(ChannelSend)).ToLocalChecked());

  Nan::Set(exports, Nan::New("ipcChannelReceive").ToLocalChecked(),
    Nan::GetFunction(Nan::New<FunctionTemplate>(ChannelReceive)).ToLocalChecked());

  Nan::Set(exports, Nan::New("ipcChannelClose").ToLocalChecked(),
    Nan::GetFunction(Nan::New<FunctionTemplate>(ChannelClose)).ToLocalChecked());

  Nan::Set(exports, Nan::New("ipcChannelIsConnected").ToLocalChecked(),
    Nan::GetFunction(Nan::New<FunctionTemplate>(ChannelIsConnected)).ToLocalChecked());

  Nan::Set(exports, Nan::New("ipcChannelIsServer").ToLocalChecked(),
    Nan::GetFunction(Nan::New<FunctionTemplate>(ChannelIsServer)).ToLocalChecked());
}
