#include "addon_api.h"
#include "tcc_addon.h"
#include "jsbridge_impl.h"
#include <cstring>

// Weak reference so we don't prevent GC
class CompiledModuleWrap : public ADDON_OBJECT_WRAP {
public:
  static void Init(ADDON_INIT_PARAMS);
  static ADDON_METHOD(New);
  static ADDON_METHOD(SetLibPath);
  static ADDON_METHOD(AddIncludePath);
  static ADDON_METHOD(AddLibraryPath);
  static ADDON_METHOD(AddLibrary);
  static ADDON_METHOD(Define);
  static ADDON_METHOD(Undefine);
  static ADDON_METHOD(Compile);
  static ADDON_METHOD(CompileFile);
  static ADDON_METHOD(GetSymbol);
  static ADDON_METHOD(GetFunction);
  static ADDON_METHOD(GetError);
  static ADDON_METHOD(Release);

  static ADDON_PERSISTENT_FUNCTION constructor;

  tinycc::CompiledModule* module_;
  jsbridge::Context* jsctx_;

private:
  CompiledModuleWrap() : module_(NULL), jsctx_(NULL) {}
  ~CompiledModuleWrap() {
    if (module_) {
      delete module_;
      module_ = NULL;
    }
    if (jsctx_) {
      delete jsctx_;
      jsctx_ = NULL;
    }
  }
};

ADDON_PERSISTENT_FUNCTION CompiledModuleWrap::constructor;

void CompiledModuleWrap::Init(ADDON_INIT_PARAMS) {
  ADDON_HANDLE_SCOPE();

  // Constructor template
  auto tpl = ADDON_NEW_CTOR_TEMPLATE_WITH(New);
  ADDON_SET_CLASS_NAME(tpl, "Compiler");
  ADDON_SET_INTERNAL_FIELD_COUNT(tpl, 1);

  // Methods
  ADDON_SET_PROTOTYPE_METHOD(tpl, "setLibPath", SetLibPath);
  ADDON_SET_PROTOTYPE_METHOD(tpl, "addIncludePath", AddIncludePath);
  ADDON_SET_PROTOTYPE_METHOD(tpl, "addLibraryPath", AddLibraryPath);
  ADDON_SET_PROTOTYPE_METHOD(tpl, "addLibrary", AddLibrary);
  ADDON_SET_PROTOTYPE_METHOD(tpl, "define", Define);
  ADDON_SET_PROTOTYPE_METHOD(tpl, "undefine", Undefine);
  ADDON_SET_PROTOTYPE_METHOD(tpl, "compile", Compile);
  ADDON_SET_PROTOTYPE_METHOD(tpl, "compileFile", CompileFile);
  ADDON_SET_PROTOTYPE_METHOD(tpl, "getSymbol", GetSymbol);
  ADDON_SET_PROTOTYPE_METHOD(tpl, "getFunction", GetFunction);
  ADDON_SET_PROTOTYPE_METHOD(tpl, "getError", GetError);
  ADDON_SET_PROTOTYPE_METHOD(tpl, "release", Release);

  ADDON_PERSISTENT_RESET(constructor, ADDON_GET_CTOR_FUNCTION(tpl));
  ADDON_SET(exports, "Compiler", ADDON_GET_CTOR_FUNCTION(tpl));
}

ADDON_METHOD(CompiledModuleWrap::New) {
  ADDON_ENV;
  if (ADDON_IS_CONSTRUCT_CALL()) {
    CompiledModuleWrap* obj = new CompiledModuleWrap();
    obj->module_ = tinycc::createCompiler();
    obj->jsctx_ = new jsbridge::Context();
    obj->Wrap(ADDON_THIS());
    ADDON_RETURN(ADDON_THIS());
  }

  // Invoked as plain function, turn into construct call
  auto cons = ADDON_PERSISTENT_GET(constructor);
  ADDON_RETURN(ADDON_NEW_INSTANCE(cons));
}

ADDON_METHOD(CompiledModuleWrap::SetLibPath) {
  ADDON_ENV;
  CompiledModuleWrap* obj = ADDON_UNWRAP(CompiledModuleWrap, ADDON_HOLDER());

  if (ADDON_ARG_COUNT() < 1 || !ADDON_IS_STRING(ADDON_ARG(0))) {
    ADDON_THROW_TYPE_ERROR("Path must be a string");
    ADDON_VOID_RETURN();
  }

  ADDON_UTF8(path, ADDON_ARG(0));
  obj->module_->setLibPath(ADDON_UTF8_VALUE(path));
  ADDON_VOID_RETURN();
}

ADDON_METHOD(CompiledModuleWrap::AddIncludePath) {
  ADDON_ENV;
  CompiledModuleWrap* obj = ADDON_UNWRAP(CompiledModuleWrap, ADDON_HOLDER());

  if (ADDON_ARG_COUNT() < 1 || !ADDON_IS_STRING(ADDON_ARG(0))) {
    ADDON_THROW_TYPE_ERROR("Path must be a string");
    ADDON_VOID_RETURN();
  }

  ADDON_UTF8(path, ADDON_ARG(0));
  obj->module_->addIncludePath(ADDON_UTF8_VALUE(path));
  ADDON_VOID_RETURN();
}

ADDON_METHOD(CompiledModuleWrap::AddLibraryPath) {
  ADDON_ENV;
  CompiledModuleWrap* obj = ADDON_UNWRAP(CompiledModuleWrap, ADDON_HOLDER());

  if (ADDON_ARG_COUNT() < 1 || !ADDON_IS_STRING(ADDON_ARG(0))) {
    ADDON_THROW_TYPE_ERROR("Path must be a string");
    ADDON_VOID_RETURN();
  }

  ADDON_UTF8(path, ADDON_ARG(0));
  obj->module_->addLibraryPath(ADDON_UTF8_VALUE(path));
  ADDON_VOID_RETURN();
}

ADDON_METHOD(CompiledModuleWrap::AddLibrary) {
  ADDON_ENV;
  CompiledModuleWrap* obj = ADDON_UNWRAP(CompiledModuleWrap, ADDON_HOLDER());

  if (ADDON_ARG_COUNT() < 1 || !ADDON_IS_STRING(ADDON_ARG(0))) {
    ADDON_THROW_TYPE_ERROR("Library name must be a string");
    ADDON_VOID_RETURN();
  }

  ADDON_UTF8(name, ADDON_ARG(0));
  obj->module_->addLibrary(ADDON_UTF8_VALUE(name));
  ADDON_VOID_RETURN();
}

ADDON_METHOD(CompiledModuleWrap::Define) {
  ADDON_ENV;
  CompiledModuleWrap* obj = ADDON_UNWRAP(CompiledModuleWrap, ADDON_HOLDER());

  if (ADDON_ARG_COUNT() < 1 || !ADDON_IS_STRING(ADDON_ARG(0))) {
    ADDON_THROW_TYPE_ERROR("Macro name must be a string");
    ADDON_VOID_RETURN();
  }

  ADDON_UTF8(name, ADDON_ARG(0));
  std::string value = "";

  if (ADDON_ARG_COUNT() >= 2 && ADDON_IS_STRING(ADDON_ARG(1))) {
    ADDON_UTF8(val, ADDON_ARG(1));
    value = ADDON_UTF8_VALUE(val);
  }

  obj->module_->define(ADDON_UTF8_VALUE(name), value);
  ADDON_VOID_RETURN();
}

ADDON_METHOD(CompiledModuleWrap::Undefine) {
  ADDON_ENV;
  CompiledModuleWrap* obj = ADDON_UNWRAP(CompiledModuleWrap, ADDON_HOLDER());

  if (ADDON_ARG_COUNT() < 1 || !ADDON_IS_STRING(ADDON_ARG(0))) {
    ADDON_THROW_TYPE_ERROR("Macro name must be a string");
    ADDON_VOID_RETURN();
  }

  ADDON_UTF8(name, ADDON_ARG(0));
  obj->module_->undefine(ADDON_UTF8_VALUE(name));
  ADDON_VOID_RETURN();
}

ADDON_METHOD(CompiledModuleWrap::Compile) {
  ADDON_ENV;
  CompiledModuleWrap* obj = ADDON_UNWRAP(CompiledModuleWrap, ADDON_HOLDER());

  if (ADDON_ARG_COUNT() < 1 || !ADDON_IS_STRING(ADDON_ARG(0))) {
    ADDON_THROW_TYPE_ERROR("Code must be a string");
    ADDON_VOID_RETURN();
  }

  ADDON_UTF8(code, ADDON_ARG(0));
  bool success = obj->module_->compile(ADDON_UTF8_VALUE(code));

  ADDON_RETURN(ADDON_BOOLEAN(success));
}

ADDON_METHOD(CompiledModuleWrap::CompileFile) {
  ADDON_ENV;
  CompiledModuleWrap* obj = ADDON_UNWRAP(CompiledModuleWrap, ADDON_HOLDER());

  if (ADDON_ARG_COUNT() < 1 || !ADDON_IS_STRING(ADDON_ARG(0))) {
    ADDON_THROW_TYPE_ERROR("Path must be a string");
    ADDON_VOID_RETURN();
  }

  ADDON_UTF8(path, ADDON_ARG(0));
  bool success = obj->module_->compileFile(ADDON_UTF8_VALUE(path));

  ADDON_RETURN(ADDON_BOOLEAN(success));
}

ADDON_METHOD(CompiledModuleWrap::GetSymbol) {
  ADDON_ENV;
  CompiledModuleWrap* obj = ADDON_UNWRAP(CompiledModuleWrap, ADDON_HOLDER());

  if (ADDON_ARG_COUNT() < 1 || !ADDON_IS_STRING(ADDON_ARG(0))) {
    ADDON_THROW_TYPE_ERROR("Symbol name must be a string");
    ADDON_VOID_RETURN();
  }

  ADDON_UTF8(name, ADDON_ARG(0));
  void* sym = obj->module_->getSymbol(ADDON_UTF8_VALUE(name));

  if (sym == NULL) {
    ADDON_RETURN_NULL();
  }

  // Return as number (pointer address)
  ADDON_RETURN(ADDON_NUMBER(reinterpret_cast<uintptr_t>(sym)));
}

// Type enumeration for native function signatures
enum NativeType {
  NTYPE_VOID = 0,
  NTYPE_INT32,
  NTYPE_UINT32,
  NTYPE_INT64,
  NTYPE_UINT64,
  NTYPE_FLOAT,
  NTYPE_DOUBLE,
  NTYPE_POINTER,
  NTYPE_STRING,
  NTYPE_JSVALUE  // Use jsbridge mode
};

// Parse type string to enum
static NativeType parseType(const std::string& typeStr) {
  if (typeStr == "void") return NTYPE_VOID;
  if (typeStr == "int" || typeStr == "int32") return NTYPE_INT32;
  if (typeStr == "uint" || typeStr == "uint32") return NTYPE_UINT32;
  if (typeStr == "int64") return NTYPE_INT64;
  if (typeStr == "uint64") return NTYPE_UINT64;
  if (typeStr == "float") return NTYPE_FLOAT;
  if (typeStr == "double") return NTYPE_DOUBLE;
  if (typeStr == "pointer" || typeStr == "ptr") return NTYPE_POINTER;
  if (typeStr == "string") return NTYPE_STRING;
  if (typeStr == "jsvalue") return NTYPE_JSVALUE;
  return NTYPE_INT32; // Default
}

// Function wrapper for calling native functions from JS
class NativeFunctionWrap : public ADDON_OBJECT_WRAP {
public:
  // Create with jsbridge mode (legacy)
  static ADDON_OBJECT_TYPE Create(void* funcPtr, jsbridge::Context* jsctx, int argCount);
  // Create with native type signature
  static ADDON_OBJECT_TYPE CreateTyped(void* funcPtr, jsbridge::Context* jsctx,
                                       NativeType returnType, const std::vector<NativeType>& argTypes);
  static ADDON_METHOD(Call);

  void* funcPtr_;
  jsbridge::Context* jsctx_;
  int argCount_;
  bool useJsbridge_;  // true = jsbridge mode, false = native types
  NativeType returnType_;
  std::vector<NativeType> argTypes_;

private:
  NativeFunctionWrap()
    : funcPtr_(NULL), jsctx_(NULL), argCount_(0), useJsbridge_(true), returnType_(NTYPE_VOID) {}
};

ADDON_OBJECT_TYPE NativeFunctionWrap::Create(void* funcPtr, jsbridge::Context* jsctx, int argCount) {
  ADDON_ESCAPABLE_SCOPE();

  auto tpl = ADDON_NEW_CTOR_TEMPLATE();
  ADDON_SET_CLASS_NAME(tpl, "NativeFunction");
  ADDON_SET_INTERNAL_FIELD_COUNT(tpl, 1);

  ADDON_SET_PROTOTYPE_METHOD(tpl, "call", Call);

  ADDON_OBJECT_TYPE instance = ADDON_TEMPLATE_NEW_INSTANCE(tpl);

  NativeFunctionWrap* wrap = new NativeFunctionWrap();
  wrap->funcPtr_ = funcPtr;
  wrap->jsctx_ = jsctx;
  wrap->argCount_ = argCount;
  wrap->useJsbridge_ = true;
  wrap->Wrap(instance);

  return ADDON_ESCAPE(instance);
}

ADDON_OBJECT_TYPE NativeFunctionWrap::CreateTyped(void* funcPtr, jsbridge::Context* jsctx,
                                                  NativeType returnType,
                                                  const std::vector<NativeType>& argTypes) {
  ADDON_ESCAPABLE_SCOPE();

  auto tpl = ADDON_NEW_CTOR_TEMPLATE();
  ADDON_SET_CLASS_NAME(tpl, "NativeFunction");
  ADDON_SET_INTERNAL_FIELD_COUNT(tpl, 1);

  ADDON_SET_PROTOTYPE_METHOD(tpl, "call", Call);

  ADDON_OBJECT_TYPE instance = ADDON_TEMPLATE_NEW_INSTANCE(tpl);

  NativeFunctionWrap* wrap = new NativeFunctionWrap();
  wrap->funcPtr_ = funcPtr;
  wrap->jsctx_ = jsctx;
  wrap->argCount_ = static_cast<int>(argTypes.size());
  wrap->useJsbridge_ = false;
  wrap->returnType_ = returnType;
  wrap->argTypes_ = argTypes;
  wrap->Wrap(instance);

  return ADDON_ESCAPE(instance);
}

// Slot size matches register width: 4 bytes on x86, 8 bytes on x64
typedef uintptr_t tcc_slot_t;

// Native type call implementation using cdecl calling convention
// Builds a stack of register-width slots and calls through typed function pointers
static tcc_slot_t callNativeFunction(void* funcPtr, const std::vector<tcc_slot_t>& stack, size_t slotCount) {
  typedef tcc_slot_t (*fn0_t)();
  typedef tcc_slot_t (*fn1_t)(tcc_slot_t);
  typedef tcc_slot_t (*fn2_t)(tcc_slot_t, tcc_slot_t);
  typedef tcc_slot_t (*fn3_t)(tcc_slot_t, tcc_slot_t, tcc_slot_t);
  typedef tcc_slot_t (*fn4_t)(tcc_slot_t, tcc_slot_t, tcc_slot_t, tcc_slot_t);
  typedef tcc_slot_t (*fn5_t)(tcc_slot_t, tcc_slot_t, tcc_slot_t, tcc_slot_t, tcc_slot_t);
  typedef tcc_slot_t (*fn6_t)(tcc_slot_t, tcc_slot_t, tcc_slot_t, tcc_slot_t, tcc_slot_t, tcc_slot_t);
  typedef tcc_slot_t (*fn7_t)(tcc_slot_t, tcc_slot_t, tcc_slot_t, tcc_slot_t, tcc_slot_t, tcc_slot_t, tcc_slot_t);
  typedef tcc_slot_t (*fn8_t)(tcc_slot_t, tcc_slot_t, tcc_slot_t, tcc_slot_t, tcc_slot_t, tcc_slot_t, tcc_slot_t, tcc_slot_t);

  switch (slotCount) {
    case 0: return ((fn0_t)funcPtr)();
    case 1: return ((fn1_t)funcPtr)(stack[0]);
    case 2: return ((fn2_t)funcPtr)(stack[0], stack[1]);
    case 3: return ((fn3_t)funcPtr)(stack[0], stack[1], stack[2]);
    case 4: return ((fn4_t)funcPtr)(stack[0], stack[1], stack[2], stack[3]);
    case 5: return ((fn5_t)funcPtr)(stack[0], stack[1], stack[2], stack[3], stack[4]);
    case 6: return ((fn6_t)funcPtr)(stack[0], stack[1], stack[2], stack[3], stack[4], stack[5]);
    case 7: return ((fn7_t)funcPtr)(stack[0], stack[1], stack[2], stack[3], stack[4], stack[5], stack[6]);
    default: return ((fn8_t)funcPtr)(stack[0], stack[1], stack[2], stack[3], stack[4], stack[5], stack[6], stack[7]);
  }
}

ADDON_METHOD(NativeFunctionWrap::Call) {
  ADDON_ENV;
  NativeFunctionWrap* wrap = ADDON_UNWRAP(NativeFunctionWrap, ADDON_HOLDER());

  if (!wrap->funcPtr_ || !wrap->jsctx_) {
    ADDON_THROW_ERROR("Function not initialized");
    ADDON_VOID_RETURN();
  }

  // Native types mode - convert JS args to C types, call, convert result back
  if (!wrap->useJsbridge_) {
    std::vector<tcc_slot_t> stack;
    std::vector<std::string> stringArgs;  // Keep strings alive

    // Convert each argument based on declared type
    for (size_t i = 0; i < wrap->argTypes_.size(); i++) {
      ADDON_VALUE arg = (i < static_cast<size_t>(ADDON_ARG_COUNT())) ? ADDON_ARG(i) : ADDON_UNDEFINED();

      switch (wrap->argTypes_[i]) {
        case NTYPE_INT32:
        case NTYPE_UINT32:
          stack.push_back(static_cast<tcc_slot_t>(ADDON_TO_INT32_DEFAULT(arg, 0)));
          break;

        case NTYPE_INT64:
        case NTYPE_UINT64: {
          int64_t val = static_cast<int64_t>(ADDON_TO_DOUBLE_DEFAULT(arg, 0.0));
#ifdef _WIN64
          // On x64, 64-bit values fit in a single register-width slot
          stack.push_back(static_cast<tcc_slot_t>(val));
#else
          // On x86, 64-bit values need two 32-bit slots (low, high)
          stack.push_back(static_cast<tcc_slot_t>(val & 0xFFFFFFFF));
          stack.push_back(static_cast<tcc_slot_t>((val >> 32) & 0xFFFFFFFF));
#endif
          break;
        }

        case NTYPE_FLOAT: {
          float f = static_cast<float>(ADDON_TO_DOUBLE_DEFAULT(arg, 0.0));
          tcc_slot_t slot = 0;
          memcpy(&slot, &f, sizeof(float));
          stack.push_back(slot);
          break;
        }

        case NTYPE_DOUBLE: {
          double d = ADDON_TO_DOUBLE_DEFAULT(arg, 0.0);
#ifdef _WIN64
          tcc_slot_t slot = 0;
          memcpy(&slot, &d, sizeof(double));
          stack.push_back(slot);
#else
          uint32_t* parts = reinterpret_cast<uint32_t*>(&d);
          stack.push_back(parts[0]);
          stack.push_back(parts[1]);
#endif
          break;
        }

        case NTYPE_STRING: {
          if (ADDON_IS_STRING(arg)) {
            ADDON_UTF8(str, arg);
            stringArgs.push_back(std::string(ADDON_UTF8_VALUE(str)));
            stack.push_back(reinterpret_cast<tcc_slot_t>(stringArgs.back().c_str()));
          } else {
            stack.push_back(0);
          }
          break;
        }

        case NTYPE_POINTER:
          // Accept number as pointer address
          stack.push_back(static_cast<tcc_slot_t>(
            static_cast<uintptr_t>(ADDON_TO_DOUBLE_DEFAULT(arg, 0.0))));
          break;

        default:
          stack.push_back(0);
          break;
      }
    }

    // Pad stack to 8 slots
    while (stack.size() < 8) {
      stack.push_back(0);
    }

    // Call function
    tcc_slot_t resultRaw = callNativeFunction(wrap->funcPtr_, stack, wrap->argTypes_.size());

    // Convert result based on return type
    switch (wrap->returnType_) {
      case NTYPE_VOID:
        ADDON_RETURN_UNDEFINED();

      case NTYPE_INT32:
        ADDON_RETURN(ADDON_INTEGER(static_cast<int32_t>(resultRaw)));

      case NTYPE_UINT32:
        ADDON_RETURN(ADDON_INTEGER(static_cast<uint32_t>(resultRaw)));

      case NTYPE_FLOAT: {
        float f;
        memcpy(&f, &resultRaw, sizeof(float));
        ADDON_RETURN(ADDON_NUMBER(f));
      }

      case NTYPE_DOUBLE: {
        double d;
        memcpy(&d, &resultRaw, sizeof(double));
        ADDON_RETURN(ADDON_NUMBER(d));
      }

      case NTYPE_POINTER:
        ADDON_RETURN(ADDON_NUMBER(static_cast<double>(resultRaw)));

      case NTYPE_STRING: {
        const char* str = reinterpret_cast<const char*>(resultRaw);
        if (str != NULL) {
          ADDON_RETURN(ADDON_STRING(str));
        }
        ADDON_RETURN_NULL();
      }

      default:
        ADDON_RETURN(ADDON_INTEGER(static_cast<int32_t>(resultRaw)));
    }
  }

  // jsbridge mode (original behavior)
  jsbridge::ContextScope ctxScope(wrap->jsctx_);

  // Convert arguments to jsvalues
  int argc = ADDON_ARG_COUNT();
  std::vector<jsvalue> args(argc);

  for (int i = 0; i < argc; i++) {
    args[i] = wrap->jsctx_->store(ADDON_ARG(i));
  }

  // Call function based on argument count
  jsvalue result = {0};

  switch (argc) {
    case 0: {
      typedef jsvalue (*fn0_t)(jscontext);
      fn0_t fn = reinterpret_cast<fn0_t>(wrap->funcPtr_);
      result = fn(__jscontext);
      break;
    }
    case 1: {
      typedef jsvalue (*fn1_t)(jscontext, jsvalue);
      fn1_t fn = reinterpret_cast<fn1_t>(wrap->funcPtr_);
      result = fn(__jscontext, args[0]);
      break;
    }
    case 2: {
      typedef jsvalue (*fn2_t)(jscontext, jsvalue, jsvalue);
      fn2_t fn = reinterpret_cast<fn2_t>(wrap->funcPtr_);
      result = fn(__jscontext, args[0], args[1]);
      break;
    }
    case 3: {
      typedef jsvalue (*fn3_t)(jscontext, jsvalue, jsvalue, jsvalue);
      fn3_t fn = reinterpret_cast<fn3_t>(wrap->funcPtr_);
      result = fn(__jscontext, args[0], args[1], args[2]);
      break;
    }
    case 4: {
      typedef jsvalue (*fn4_t)(jscontext, jsvalue, jsvalue, jsvalue, jsvalue);
      fn4_t fn = reinterpret_cast<fn4_t>(wrap->funcPtr_);
      result = fn(__jscontext, args[0], args[1], args[2], args[3]);
      break;
    }
    default: {
      // Use variadic version
      typedef jsvalue (*fnn_t)(jscontext, int, jsvalue*);
      fnn_t fn = reinterpret_cast<fnn_t>(wrap->funcPtr_);
      result = fn(__jscontext, argc, args.data());
      break;
    }
  }

  // Release argument refs
  for (int i = 0; i < argc; i++) {
    wrap->jsctx_->release(args[i]);
  }

  // Return result
  ADDON_VALUE jsResult = wrap->jsctx_->retrieve(result);
  wrap->jsctx_->release(result);

  ADDON_RETURN(jsResult);
}

ADDON_METHOD(CompiledModuleWrap::GetFunction) {
  ADDON_ENV;
  CompiledModuleWrap* obj = ADDON_UNWRAP(CompiledModuleWrap, ADDON_HOLDER());

  if (ADDON_ARG_COUNT() < 1 || !ADDON_IS_STRING(ADDON_ARG(0))) {
    ADDON_THROW_TYPE_ERROR("Function name must be a string");
    ADDON_VOID_RETURN();
  }

  ADDON_UTF8(name, ADDON_ARG(0));

  void* funcPtr = obj->module_->getSymbol(ADDON_UTF8_VALUE(name));

  if (funcPtr == NULL) {
    ADDON_THROW_ERROR(obj->module_->getError().c_str());
    ADDON_VOID_RETURN();
  }

  // Check for typed signature: getFunction(name, returnType, [argTypes])
  // Or legacy: getFunction(name, argCount)
  bool isTypedCall = ADDON_ARG_COUNT() >= 2 && ADDON_IS_STRING(ADDON_ARG(1));

  if (isTypedCall) {
    // New API: getFunction(name, returnType, argTypes[])
    ADDON_UTF8(returnTypeStr, ADDON_ARG(1));
    NativeType returnType = parseType(ADDON_UTF8_VALUE(returnTypeStr));

    std::vector<NativeType> argTypes;

    if (ADDON_ARG_COUNT() >= 3 && ADDON_IS_ARRAY(ADDON_ARG(2))) {
      ADDON_ARRAY_TYPE arr = ADDON_CAST_ARRAY(ADDON_ARG(2));
      for (uint32_t i = 0; i < ADDON_LENGTH(arr); i++) {
        ADDON_VALUE item = ADDON_GET_INDEX(arr, i);
        if (ADDON_IS_STRING(item)) {
          ADDON_UTF8(typeStr, item);
          argTypes.push_back(parseType(ADDON_UTF8_VALUE(typeStr)));
        }
      }
    }

    ADDON_OBJECT_TYPE wrapper = NativeFunctionWrap::CreateTyped(funcPtr, obj->jsctx_, returnType, argTypes);
    ADDON_RETURN(wrapper);
  }

  // Legacy API: getFunction(name, argCount) - uses jsbridge mode
  int argCount = 0;
  if (ADDON_ARG_COUNT() >= 2 && ADDON_IS_NUMBER(ADDON_ARG(1))) {
    argCount = ADDON_TO_INT(ADDON_ARG(1));
  }

  ADDON_OBJECT_TYPE wrapper = NativeFunctionWrap::Create(funcPtr, obj->jsctx_, argCount);
  ADDON_RETURN(wrapper);
}

ADDON_METHOD(CompiledModuleWrap::GetError) {
  ADDON_ENV;
  CompiledModuleWrap* obj = ADDON_UNWRAP(CompiledModuleWrap, ADDON_HOLDER());

  const std::string& error = obj->module_->getError();

  if (error.empty()) {
    ADDON_RETURN_NULL();
  }
  ADDON_RETURN(ADDON_STRING(error.c_str()));
}

ADDON_METHOD(CompiledModuleWrap::Release) {
  ADDON_ENV;
  CompiledModuleWrap* obj = ADDON_UNWRAP(CompiledModuleWrap, ADDON_HOLDER());

  if (obj->module_) {
    obj->module_->release();
  }
  if (obj->jsctx_) {
    obj->jsctx_->clear();
  }
  ADDON_VOID_RETURN();
}

// Factory function to create new compiler
ADDON_METHOD(CreateCompiler) {
  ADDON_ENV;
  auto cons = ADDON_PERSISTENT_GET(CompiledModuleWrap::constructor);
  ADDON_RETURN(ADDON_NEW_INSTANCE(cons));
}

// Module initialization
void InitTinyCC(ADDON_INIT_PARAMS) {
  ADDON_OBJECT_TYPE tinycc = ADDON_OBJECT();

  CompiledModuleWrap::Init(tinycc);

  // Factory function
  ADDON_SET_METHOD(tinycc, "create", CreateCompiler);

  ADDON_SET(exports, "tinycc", tinycc);
}
