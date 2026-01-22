#ifndef TCC_ADDON_H
#define TCC_ADDON_H

#include <string>
#include <vector>
#include <map>

// TinyCC header - extern "C" for C linkage
extern "C" {
  #include "libtcc.h"
}

namespace tinycc {

class CompiledModule {
public:
  CompiledModule();
  ~CompiledModule();

  // Configure compilation
  void setLibPath(const std::string& path);
  void addIncludePath(const std::string& path);
  void addLibraryPath(const std::string& path);
  void addLibrary(const std::string& name);
  void define(const std::string& name, const std::string& value);
  void undefine(const std::string& name);

  // Compile code or file
  bool compile(const std::string& code);
  bool compileFile(const std::string& path);

  // After compilation, relocate to execute
  bool relocate();

  // Get symbol after relocation
  void* getSymbol(const std::string& name);

  // Error handling
  const std::string& getError() const { return lastError_; }
  bool isCompiled() const { return compiled_; }
  bool isRelocated() const { return relocated_; }

  // Cleanup
  void release();

private:
  static void errorCallback(void* opaque, const char* msg);

  TCCState* state_;
  std::string lastError_;
  bool compiled_;
  bool relocated_;
};

// Factory function to create a new compiler instance
CompiledModule* createCompiler();

} // namespace tinycc

#endif // TCC_ADDON_H
