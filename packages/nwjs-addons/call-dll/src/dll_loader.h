#ifndef DLL_LOADER_H
#define DLL_LOADER_H

#include <string>
#include <windows.h>

namespace calldll {

/**
 * DLL Handle wrapper
 * Manages loading/unloading of DLLs
 */
class DLLHandle {
public:
  /**
   * Load DLL from path
   * @param path Full path to DLL or system DLL name
   * @param isSystem If true, search system directories
   */
  DLLHandle(const std::string& path, bool isSystem);
  ~DLLHandle();

  // Prevent copying
  DLLHandle(const DLLHandle&);
  DLLHandle& operator=(const DLLHandle&);

  /**
   * Check if DLL loaded successfully
   */
  bool isLoaded() const { return handle_ != NULL; }

  /**
   * Get path used to load DLL
   */
  const std::string& path() const { return path_; }

  /**
   * Get native handle
   */
  HMODULE handle() const { return handle_; }

  /**
   * Get function by name
   * @param name Function name
   * @returns Function pointer or NULL
   */
  void* getFunction(const std::string& name);

  /**
   * Get function by ordinal
   * @param ordinal Ordinal number
   * @returns Function pointer or NULL
   */
  void* getFunctionByOrdinal(int ordinal);

  /**
   * Get exported symbol address
   * @param name Symbol name
   * @returns Symbol address or NULL
   */
  void* getSymbol(const std::string& name);

  /**
   * Close and unload DLL
   */
  void close();

  /**
   * Get last error message
   */
  const std::string& getError() const { return lastError_; }

private:
  std::string path_;
  HMODULE handle_;
  std::string lastError_;

  void setErrorFromWin32();
};

} // namespace calldll

#endif // DLL_LOADER_H
