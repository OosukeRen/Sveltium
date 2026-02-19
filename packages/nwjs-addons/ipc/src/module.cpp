#include "addon_api.h"
#include "ipc.h"
#include <map>

using namespace ipc;

// Store channels by ID
static std::map<uint32_t, Channel*> channels;
static uint32_t nextChannelId = 1;

ADDON_METHOD(IsProcessRunning) {
  ADDON_ENV;
  if (ADDON_ARG_COUNT() < 1 || !ADDON_IS_NUMBER(ADDON_ARG(0))) {
    ADDON_THROW_TYPE_ERROR("Argument must be a number (process ID)");
    ADDON_VOID_RETURN();
  }

  uint32_t pid = ADDON_TO_UINT32(ADDON_ARG(0));
  bool running = isProcessRunning(static_cast<DWORD>(pid));
  ADDON_RETURN(ADDON_BOOL(running));
}

ADDON_METHOD(GenerateChannelName) {
  ADDON_ENV;
  std::string name = generateChannelName();
  ADDON_RETURN(ADDON_STRING(name));
}

ADDON_METHOD(CreateChannel) {
  ADDON_ENV;
  if (ADDON_ARG_COUNT() < 2 || !ADDON_IS_STRING(ADDON_ARG(0)) || !ADDON_IS_BOOLEAN(ADDON_ARG(1))) {
    ADDON_THROW_TYPE_ERROR("Arguments: (name: string, isServer: boolean)");
    ADDON_VOID_RETURN();
  }

  ADDON_UTF8(name, ADDON_ARG(0));
  bool isServer = ADDON_TO_BOOL(ADDON_ARG(1));

  Channel* channel = new Channel(std::string(ADDON_UTF8_VALUE(name)), isServer);
  uint32_t id = nextChannelId++;
  channels[id] = channel;

  ADDON_RETURN(ADDON_UINT(id));
}

ADDON_METHOD(ChannelConnect) {
  ADDON_ENV;
  if (ADDON_ARG_COUNT() < 1 || !ADDON_IS_NUMBER(ADDON_ARG(0))) {
    ADDON_THROW_TYPE_ERROR("Argument must be a channel ID");
    ADDON_VOID_RETURN();
  }

  uint32_t id = ADDON_TO_UINT32(ADDON_ARG(0));

  std::map<uint32_t, Channel*>::iterator it = channels.find(id);
  if (it == channels.end()) {
    ADDON_RETURN(ADDON_BOOL(false));
  }

  bool success = it->second->connect();
  ADDON_RETURN(ADDON_BOOL(success));
}

ADDON_METHOD(ChannelSend) {
  ADDON_ENV;
  if (ADDON_ARG_COUNT() < 2 || !ADDON_IS_NUMBER(ADDON_ARG(0))) {
    ADDON_THROW_TYPE_ERROR("Arguments: (channelId: number, data: string|Buffer)");
    ADDON_VOID_RETURN();
  }

  uint32_t id = ADDON_TO_UINT32(ADDON_ARG(0));

  std::map<uint32_t, Channel*>::iterator it = channels.find(id);
  if (it == channels.end()) {
    ADDON_RETURN(ADDON_BOOL(false));
  }

  bool success = false;

  if (ADDON_IS_STRING(ADDON_ARG(1))) {
    ADDON_UTF8(str, ADDON_ARG(1));
    success = it->second->send(ADDON_UTF8_VALUE(str), ADDON_UTF8_LENGTH(str));
  } else if (ADDON_BUFFER_IS(ADDON_ARG(1))) {
    char* data = ADDON_BUFFER_DATA(ADDON_ARG(1));
    size_t length = ADDON_BUFFER_LENGTH(ADDON_ARG(1));
    success = it->second->send(data, length);
  }

  ADDON_RETURN(ADDON_BOOL(success));
}

ADDON_METHOD(ChannelReceive) {
  ADDON_ENV;
  if (ADDON_ARG_COUNT() < 1 || !ADDON_IS_NUMBER(ADDON_ARG(0))) {
    ADDON_THROW_TYPE_ERROR("Argument must be a channel ID");
    ADDON_VOID_RETURN();
  }

  uint32_t id = ADDON_TO_UINT32(ADDON_ARG(0));

  std::map<uint32_t, Channel*>::iterator it = channels.find(id);
  if (it == channels.end()) {
    ADDON_RETURN_NULL();
  }

  std::string data = it->second->receive();

  if (data.empty()) {
    ADDON_RETURN_NULL();
  }
  ADDON_RETURN(ADDON_COPY_BUFFER(data.c_str(), data.size()));
}

ADDON_METHOD(ChannelClose) {
  ADDON_ENV;
  if (ADDON_ARG_COUNT() < 1 || !ADDON_IS_NUMBER(ADDON_ARG(0))) {
    ADDON_THROW_TYPE_ERROR("Argument must be a channel ID");
    ADDON_VOID_RETURN();
  }

  uint32_t id = ADDON_TO_UINT32(ADDON_ARG(0));

  std::map<uint32_t, Channel*>::iterator it = channels.find(id);
  if (it != channels.end()) {
    it->second->close();
    delete it->second;
    channels.erase(it);
  }
  ADDON_VOID_RETURN();
}

ADDON_METHOD(ChannelIsConnected) {
  ADDON_ENV;
  if (ADDON_ARG_COUNT() < 1 || !ADDON_IS_NUMBER(ADDON_ARG(0))) {
    ADDON_THROW_TYPE_ERROR("Argument must be a channel ID");
    ADDON_VOID_RETURN();
  }

  uint32_t id = ADDON_TO_UINT32(ADDON_ARG(0));

  std::map<uint32_t, Channel*>::iterator it = channels.find(id);
  if (it == channels.end()) {
    ADDON_RETURN(ADDON_BOOL(false));
  }

  ADDON_RETURN(ADDON_BOOL(it->second->isConnected()));
}

ADDON_METHOD(ChannelIsServer) {
  ADDON_ENV;
  if (ADDON_ARG_COUNT() < 1 || !ADDON_IS_NUMBER(ADDON_ARG(0))) {
    ADDON_THROW_TYPE_ERROR("Argument must be a channel ID");
    ADDON_VOID_RETURN();
  }

  uint32_t id = ADDON_TO_UINT32(ADDON_ARG(0));

  std::map<uint32_t, Channel*>::iterator it = channels.find(id);
  if (it == channels.end()) {
    ADDON_RETURN(ADDON_BOOL(false));
  }

  ADDON_RETURN(ADDON_BOOL(it->second->isServer()));
}

void InitIPC(ADDON_INIT_PARAMS) {
  ADDON_EXPORT_FUNCTION(exports, "ipcIsProcessRunning", IsProcessRunning);
  ADDON_EXPORT_FUNCTION(exports, "ipcGenerateChannelName", GenerateChannelName);
  ADDON_EXPORT_FUNCTION(exports, "ipcCreateChannel", CreateChannel);
  ADDON_EXPORT_FUNCTION(exports, "ipcChannelConnect", ChannelConnect);
  ADDON_EXPORT_FUNCTION(exports, "ipcChannelSend", ChannelSend);
  ADDON_EXPORT_FUNCTION(exports, "ipcChannelReceive", ChannelReceive);
  ADDON_EXPORT_FUNCTION(exports, "ipcChannelClose", ChannelClose);
  ADDON_EXPORT_FUNCTION(exports, "ipcChannelIsConnected", ChannelIsConnected);
  ADDON_EXPORT_FUNCTION(exports, "ipcChannelIsServer", ChannelIsServer);
}
