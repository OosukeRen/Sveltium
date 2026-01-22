#include "dll_loader.h"
#include <shlwapi.h>

namespace calldll {

DLLHandle::DLLHandle(const std::string& path, bool isSystem)
  : path_(path)
  , handle_(NULL)
{
  if (isSystem) {
    // For system DLLs, just use the name (Windows will search system dirs)
    // Append .dll if not present
    std::string dllName = path;
    if (dllName.length() < 4 ||
        _stricmp(dllName.substr(dllName.length() - 4).c_str(), ".dll") != 0) {
      dllName += ".dll";
    }
    handle_ = LoadLibraryA(dllName.c_str());
  } else {
    // Full path provided
    handle_ = LoadLibraryA(path.c_str());
  }

  if (handle_ == NULL) {
    setErrorFromWin32();
  }
}

DLLHandle::~DLLHandle() {
  close();
}

DLLHandle::DLLHandle(const DLLHandle& other) {
  // Copy is not allowed but we need to define it
  path_ = other.path_;
  handle_ = NULL;
  lastError_ = "Copy not allowed";
}

DLLHandle& DLLHandle::operator=(const DLLHandle& other) {
  // Assignment is not allowed
  if (this != &other) {
    close();
    path_ = other.path_;
    handle_ = NULL;
    lastError_ = "Assignment not allowed";
  }
  return *this;
}

void* DLLHandle::getFunction(const std::string& name) {
  if (handle_ == NULL) {
    lastError_ = "DLL not loaded";
    return NULL;
  }

  void* proc = reinterpret_cast<void*>(GetProcAddress(handle_, name.c_str()));

  if (proc == NULL) {
    setErrorFromWin32();
  }

  return proc;
}

void* DLLHandle::getFunctionByOrdinal(int ordinal) {
  if (handle_ == NULL) {
    lastError_ = "DLL not loaded";
    return NULL;
  }

  // MAKEINTRESOURCE converts ordinal to string format GetProcAddress expects
  void* proc = reinterpret_cast<void*>(
    GetProcAddress(handle_, MAKEINTRESOURCEA(ordinal))
  );

  if (proc == NULL) {
    setErrorFromWin32();
  }

  return proc;
}

void* DLLHandle::getSymbol(const std::string& name) {
  // For DLLs, symbol lookup is same as function lookup
  return getFunction(name);
}

void DLLHandle::close() {
  if (handle_ != NULL) {
    FreeLibrary(handle_);
    handle_ = NULL;
  }
}

void DLLHandle::setErrorFromWin32() {
  DWORD error = GetLastError();

  char buffer[256];
  DWORD len = FormatMessageA(
    FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
    NULL,
    error,
    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
    buffer,
    sizeof(buffer),
    NULL
  );

  if (len > 0) {
    // Remove trailing newline
    while (len > 0 && (buffer[len-1] == '\n' || buffer[len-1] == '\r')) {
      buffer[--len] = '\0';
    }
    lastError_ = buffer;
  } else {
    char numBuf[32];
    sprintf(numBuf, "Error code %lu", error);
    lastError_ = numBuf;
  }
}

} // namespace calldll
