#include "tcc_addon.h"
#include <cstdlib>

// Include jsbridge header to get function declarations
extern "C" {
  #include "../include/jsbridge.h"
}

// Forward declarations for jsbridge functions implemented in jsbridge_impl.cpp
extern "C" {
  // These are defined in jsbridge_impl.cpp
  extern jscontext __jscontext;

  jsvalue bool_to_jsvalue(jscontext ctx, bool val);
  jsvalue int8_to_jsvalue(jscontext ctx, int8 val);
  jsvalue uint8_to_jsvalue(jscontext ctx, uint8 val);
  jsvalue int16_to_jsvalue(jscontext ctx, int16 val);
  jsvalue uint16_to_jsvalue(jscontext ctx, uint16 val);
  jsvalue int32_to_jsvalue(jscontext ctx, int32 val);
  jsvalue uint32_to_jsvalue(jscontext ctx, uint32 val);
  jsvalue int64_to_jsvalue(jscontext ctx, int64 val);
  jsvalue uint64_to_jsvalue(jscontext ctx, uint64 val);
  jsvalue float_to_jsvalue(jscontext ctx, float val);
  jsvalue double_to_jsvalue(jscontext ctx, double val);

  bool   jsvalue_to_bool(jscontext ctx, jsvalue val);
  int8   jsvalue_to_int8(jscontext ctx, jsvalue val);
  uint8  jsvalue_to_uint8(jscontext ctx, jsvalue val);
  int16  jsvalue_to_int16(jscontext ctx, jsvalue val);
  uint16 jsvalue_to_uint16(jscontext ctx, jsvalue val);
  int32  jsvalue_to_int32(jscontext ctx, jsvalue val);
  uint32 jsvalue_to_uint32(jscontext ctx, jsvalue val);
  int64  jsvalue_to_int64(jscontext ctx, jsvalue val);
  uint64 jsvalue_to_uint64(jscontext ctx, jsvalue val);
  float  jsvalue_to_float(jscontext ctx, jsvalue val);
  double jsvalue_to_double(jscontext ctx, jsvalue val);
  jsvalue jsvalue_to_jsvalue(jscontext ctx, jsvalue val);

  int8*   jsvalue_to_int8_ptr(jscontext ctx, jsvalue val);
  uint8*  jsvalue_to_uint8_ptr(jscontext ctx, jsvalue val);
  int16*  jsvalue_to_int16_ptr(jscontext ctx, jsvalue val);
  uint16* jsvalue_to_uint16_ptr(jscontext ctx, jsvalue val);
  int32*  jsvalue_to_int32_ptr(jscontext ctx, jsvalue val);
  uint32* jsvalue_to_uint32_ptr(jscontext ctx, jsvalue val);
  int64*  jsvalue_to_int64_ptr(jscontext ctx, jsvalue val);
  uint64* jsvalue_to_uint64_ptr(jscontext ctx, jsvalue val);
  float*  jsvalue_to_float_ptr(jscontext ctx, jsvalue val);
  double* jsvalue_to_double_ptr(jscontext ctx, jsvalue val);

  jsvalue _jsvalue_new(jscontext ctx, const char* fmt, ...);
  int _jsvalue_fetch(jscontext ctx, jsvalue val, const char* fmt, ...);
  int _jsvalue_type(jscontext ctx, jsvalue val);
  void _jsvalue_addref(jscontext ctx, jsvalue val);
  void _jsvalue_release(jscontext ctx, jsvalue val);
}

namespace tinycc {

// Register all jsbridge symbols with TinyCC so compiled code can use them
static void registerJSBridgeSymbols(TCCState* state) {
  if (state == NULL) {
    return;
  }

  // Global context pointer
  tcc_add_symbol(state, "__jscontext", &__jscontext);

  // C -> JS conversions
  tcc_add_symbol(state, "bool_to_jsvalue", (void*)bool_to_jsvalue);
  tcc_add_symbol(state, "int8_to_jsvalue", (void*)int8_to_jsvalue);
  tcc_add_symbol(state, "uint8_to_jsvalue", (void*)uint8_to_jsvalue);
  tcc_add_symbol(state, "int16_to_jsvalue", (void*)int16_to_jsvalue);
  tcc_add_symbol(state, "uint16_to_jsvalue", (void*)uint16_to_jsvalue);
  tcc_add_symbol(state, "int32_to_jsvalue", (void*)int32_to_jsvalue);
  tcc_add_symbol(state, "uint32_to_jsvalue", (void*)uint32_to_jsvalue);
  tcc_add_symbol(state, "int64_to_jsvalue", (void*)int64_to_jsvalue);
  tcc_add_symbol(state, "uint64_to_jsvalue", (void*)uint64_to_jsvalue);
  tcc_add_symbol(state, "float_to_jsvalue", (void*)float_to_jsvalue);
  tcc_add_symbol(state, "double_to_jsvalue", (void*)double_to_jsvalue);

  // JS -> C conversions
  tcc_add_symbol(state, "jsvalue_to_bool", (void*)jsvalue_to_bool);
  tcc_add_symbol(state, "jsvalue_to_int8", (void*)jsvalue_to_int8);
  tcc_add_symbol(state, "jsvalue_to_uint8", (void*)jsvalue_to_uint8);
  tcc_add_symbol(state, "jsvalue_to_int16", (void*)jsvalue_to_int16);
  tcc_add_symbol(state, "jsvalue_to_uint16", (void*)jsvalue_to_uint16);
  tcc_add_symbol(state, "jsvalue_to_int32", (void*)jsvalue_to_int32);
  tcc_add_symbol(state, "jsvalue_to_uint32", (void*)jsvalue_to_uint32);
  tcc_add_symbol(state, "jsvalue_to_int64", (void*)jsvalue_to_int64);
  tcc_add_symbol(state, "jsvalue_to_uint64", (void*)jsvalue_to_uint64);
  tcc_add_symbol(state, "jsvalue_to_float", (void*)jsvalue_to_float);
  tcc_add_symbol(state, "jsvalue_to_double", (void*)jsvalue_to_double);
  tcc_add_symbol(state, "jsvalue_to_jsvalue", (void*)jsvalue_to_jsvalue);

  // TypedArray pointers
  tcc_add_symbol(state, "jsvalue_to_int8_ptr", (void*)jsvalue_to_int8_ptr);
  tcc_add_symbol(state, "jsvalue_to_uint8_ptr", (void*)jsvalue_to_uint8_ptr);
  tcc_add_symbol(state, "jsvalue_to_int16_ptr", (void*)jsvalue_to_int16_ptr);
  tcc_add_symbol(state, "jsvalue_to_uint16_ptr", (void*)jsvalue_to_uint16_ptr);
  tcc_add_symbol(state, "jsvalue_to_int32_ptr", (void*)jsvalue_to_int32_ptr);
  tcc_add_symbol(state, "jsvalue_to_uint32_ptr", (void*)jsvalue_to_uint32_ptr);
  tcc_add_symbol(state, "jsvalue_to_int64_ptr", (void*)jsvalue_to_int64_ptr);
  tcc_add_symbol(state, "jsvalue_to_uint64_ptr", (void*)jsvalue_to_uint64_ptr);
  tcc_add_symbol(state, "jsvalue_to_float_ptr", (void*)jsvalue_to_float_ptr);
  tcc_add_symbol(state, "jsvalue_to_double_ptr", (void*)jsvalue_to_double_ptr);

  // Object/array manipulation
  tcc_add_symbol(state, "_jsvalue_new", (void*)_jsvalue_new);
  tcc_add_symbol(state, "_jsvalue_fetch", (void*)_jsvalue_fetch);
  tcc_add_symbol(state, "_jsvalue_type", (void*)_jsvalue_type);
  tcc_add_symbol(state, "_jsvalue_addref", (void*)_jsvalue_addref);
  tcc_add_symbol(state, "_jsvalue_release", (void*)_jsvalue_release);
}

CompiledModule::CompiledModule()
  : state_(NULL)
  , compiled_(false)
  , relocated_(false)
{
  state_ = tcc_new();
  if (state_ != NULL) {
    // Set error handler
    tcc_set_error_func(state_, this, errorCallback);

    // Set output type to memory (in-memory execution)
    tcc_set_output_type(state_, TCC_OUTPUT_MEMORY);

    // Register jsbridge symbols so compiled code can use them
    registerJSBridgeSymbols(state_);
  }
}

CompiledModule::~CompiledModule() {
  release();
}

void CompiledModule::errorCallback(void* opaque, const char* msg) {
  CompiledModule* self = static_cast<CompiledModule*>(opaque);
  if (self != NULL && msg != NULL) {
    if (!self->lastError_.empty()) {
      self->lastError_ += "\n";
    }
    self->lastError_ += msg;
  }
}

void CompiledModule::setLibPath(const std::string& path) {
  if (state_ != NULL) {
    tcc_set_lib_path(state_, path.c_str());
  }
}

void CompiledModule::addIncludePath(const std::string& path) {
  if (state_ != NULL) {
    tcc_add_include_path(state_, path.c_str());
  }
}

void CompiledModule::addLibraryPath(const std::string& path) {
  if (state_ != NULL) {
    tcc_add_library_path(state_, path.c_str());
  }
}

void CompiledModule::addLibrary(const std::string& name) {
  if (state_ != NULL) {
    tcc_add_library(state_, name.c_str());
  }
}

void CompiledModule::define(const std::string& name, const std::string& value) {
  if (state_ != NULL) {
    const char* valPtr = value.empty() ? NULL : value.c_str();
    tcc_define_symbol(state_, name.c_str(), valPtr);
  }
}

void CompiledModule::undefine(const std::string& name) {
  if (state_ != NULL) {
    tcc_undefine_symbol(state_, name.c_str());
  }
}

bool CompiledModule::compile(const std::string& code) {
  if (state_ == NULL) {
    lastError_ = "TCC state not initialized";
    return false;
  }

  if (compiled_) {
    lastError_ = "Already compiled - create new instance";
    return false;
  }

  lastError_.clear();

  int result = tcc_compile_string(state_, code.c_str());
  compiled_ = (result == 0);

  if (!compiled_ && lastError_.empty()) {
    lastError_ = "Compilation failed";
  }

  return compiled_;
}

bool CompiledModule::compileFile(const std::string& path) {
  if (state_ == NULL) {
    lastError_ = "TCC state not initialized";
    return false;
  }

  if (compiled_) {
    lastError_ = "Already compiled - create new instance";
    return false;
  }

  lastError_.clear();

  int result = tcc_add_file(state_, path.c_str());
  compiled_ = (result == 0);

  if (!compiled_ && lastError_.empty()) {
    lastError_ = "Failed to compile file: " + path;
  }

  return compiled_;
}

bool CompiledModule::relocate() {
  if (state_ == NULL) {
    lastError_ = "TCC state not initialized";
    return false;
  }

  if (!compiled_) {
    lastError_ = "Must compile before relocating";
    return false;
  }

  if (relocated_) {
    return true;  // Already relocated
  }

  lastError_.clear();

  // TCC_RELOCATE_AUTO lets TCC allocate memory
  int result = tcc_relocate(state_, TCC_RELOCATE_AUTO);
  relocated_ = (result == 0);

  if (!relocated_ && lastError_.empty()) {
    lastError_ = "Relocation failed";
  }

  return relocated_;
}

void* CompiledModule::getSymbol(const std::string& name) {
  if (state_ == NULL) {
    lastError_ = "TCC state not initialized";
    return NULL;
  }

  if (!relocated_) {
    // Try to relocate first
    if (!relocate()) {
      return NULL;
    }
  }

  void* symbol = tcc_get_symbol(state_, name.c_str());

  if (symbol == NULL) {
    lastError_ = "Symbol not found: " + name;
  }

  return symbol;
}

void CompiledModule::release() {
  if (state_ != NULL) {
    tcc_delete(state_);
    state_ = NULL;
  }
  compiled_ = false;
  relocated_ = false;
}

CompiledModule* createCompiler() {
  return new CompiledModule();
}

} // namespace tinycc
