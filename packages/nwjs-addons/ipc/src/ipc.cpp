#include "ipc.h"
#include <objbase.h>
#include <cstdio>

namespace ipc {

bool isProcessRunning(DWORD pid) {
  HANDLE process = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);

  if (process == NULL) {
    // Try with less privileges (XP compatibility)
    process = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pid);
    if (process == NULL) {
      return false;
    }
  }

  DWORD exitCode = 0;
  BOOL result = GetExitCodeProcess(process, &exitCode);
  CloseHandle(process);

  return result && exitCode == STILL_ACTIVE;
}

std::string generateChannelName() {
  GUID guid;
  CoCreateGuid(&guid);

  char buffer[64];
  sprintf(buffer,
    "%08lx-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x",
    guid.Data1, guid.Data2, guid.Data3,
    guid.Data4[0], guid.Data4[1], guid.Data4[2], guid.Data4[3],
    guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]);

  return std::string(buffer);
}

Channel::Channel(const std::string& name, bool isServer)
  : name_(name)
  , isServer_(isServer)
  , connected_(false)
  , pipe_(INVALID_HANDLE_VALUE)
{
  pipeName_ = "\\\\.\\pipe\\" + name;
}

Channel::~Channel() {
  close();
}

bool Channel::connect() {
  if (isServer_) {
    // Create named pipe server
    pipe_ = CreateNamedPipeA(
      pipeName_.c_str(),
      PIPE_ACCESS_DUPLEX,
      PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
      1,                    // Max instances
      4096,                 // Out buffer
      4096,                 // In buffer
      0,                    // Default timeout
      NULL                  // Security
    );

    if (pipe_ == INVALID_HANDLE_VALUE) {
      return false;
    }

    // Wait for client connection (blocking)
    BOOL result = ConnectNamedPipe(pipe_, NULL);
    if (!result && GetLastError() != ERROR_PIPE_CONNECTED) {
      CloseHandle(pipe_);
      pipe_ = INVALID_HANDLE_VALUE;
      return false;
    }

    connected_ = true;
  } else {
    // Connect as client
    pipe_ = CreateFileA(
      pipeName_.c_str(),
      GENERIC_READ | GENERIC_WRITE,
      0,
      NULL,
      OPEN_EXISTING,
      0,
      NULL
    );

    if (pipe_ == INVALID_HANDLE_VALUE) {
      return false;
    }

    // Set message mode
    DWORD mode = PIPE_READMODE_MESSAGE;
    SetNamedPipeHandleState(pipe_, &mode, NULL, NULL);

    connected_ = true;
  }

  return true;
}

bool Channel::send(const char* data, size_t length) {
  if (!connected_ || pipe_ == INVALID_HANDLE_VALUE) {
    return false;
  }

  DWORD written = 0;
  BOOL success = WriteFile(pipe_, data, static_cast<DWORD>(length), &written, NULL);

  return success && written == length;
}

std::string Channel::receive() {
  std::string result;

  if (!connected_ || pipe_ == INVALID_HANDLE_VALUE) {
    return result;
  }

  char buffer[4096];
  DWORD bytesRead = 0;

  BOOL success = ReadFile(pipe_, buffer, sizeof(buffer), &bytesRead, NULL);

  if (success && bytesRead > 0) {
    result.assign(buffer, bytesRead);
  }

  return result;
}

void Channel::close() {
  connected_ = false;

  if (pipe_ != INVALID_HANDLE_VALUE) {
    if (isServer_) {
      DisconnectNamedPipe(pipe_);
    }
    CloseHandle(pipe_);
    pipe_ = INVALID_HANDLE_VALUE;
  }
}

} // namespace ipc
