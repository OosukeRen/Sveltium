#ifndef IPC_H
#define IPC_H

#include <string>
#include <windows.h>

namespace ipc {

// Check if a process is running
bool isProcessRunning(DWORD pid);

// Generate unique channel name
std::string generateChannelName();

// Named pipe channel
class Channel {
public:
  Channel(const std::string& name, bool isServer);
  ~Channel();

  bool connect();
  bool send(const char* data, size_t length);
  std::string receive();
  void close();

  bool isConnected() const { return connected_; }
  bool isServer() const { return isServer_; }
  const std::string& name() const { return name_; }

private:
  std::string name_;
  std::string pipeName_;
  bool isServer_;
  bool connected_;
  HANDLE pipe_;
};

} // namespace ipc

#endif // IPC_H
