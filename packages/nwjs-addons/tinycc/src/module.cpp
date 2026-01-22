#include <nan.h>
#include "tcc_addon.h"
#include "jsbridge_impl.h"

using namespace v8;

// Weak reference so we don't prevent GC
class CompiledModuleWrap : public Nan::ObjectWrap {
public:
  static void Init(Local<Object> exports);
  static NAN_METHOD(New);
  static NAN_METHOD(SetLibPath);
  static NAN_METHOD(AddIncludePath);
  static NAN_METHOD(AddLibraryPath);
  static NAN_METHOD(AddLibrary);
  static NAN_METHOD(Define);
  static NAN_METHOD(Undefine);
  static NAN_METHOD(Compile);
  static NAN_METHOD(CompileFile);
  static NAN_METHOD(GetSymbol);
  static NAN_METHOD(GetFunction);
  static NAN_METHOD(GetError);
  static NAN_METHOD(Release);

  static Nan::Persistent<Function> constructor;

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

Nan::Persistent<Function> CompiledModuleWrap::constructor;

void CompiledModuleWrap::Init(Local<Object> exports) {
  Nan::HandleScope scope;

  // Constructor template
  Local<FunctionTemplate> tpl = Nan::New<FunctionTemplate>(New);
  tpl->SetClassName(Nan::New("Compiler").ToLocalChecked());
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  // Methods
  Nan::SetPrototypeMethod(tpl, "setLibPath", SetLibPath);
  Nan::SetPrototypeMethod(tpl, "addIncludePath", AddIncludePath);
  Nan::SetPrototypeMethod(tpl, "addLibraryPath", AddLibraryPath);
  Nan::SetPrototypeMethod(tpl, "addLibrary", AddLibrary);
  Nan::SetPrototypeMethod(tpl, "define", Define);
  Nan::SetPrototypeMethod(tpl, "undefine", Undefine);
  Nan::SetPrototypeMethod(tpl, "compile", Compile);
  Nan::SetPrototypeMethod(tpl, "compileFile", CompileFile);
  Nan::SetPrototypeMethod(tpl, "getSymbol", GetSymbol);
  Nan::SetPrototypeMethod(tpl, "getFunction", GetFunction);
  Nan::SetPrototypeMethod(tpl, "getError", GetError);
  Nan::SetPrototypeMethod(tpl, "release", Release);

  constructor.Reset(tpl->GetFunction());
  exports->Set(Nan::New("Compiler").ToLocalChecked(), tpl->GetFunction());
}

NAN_METHOD(CompiledModuleWrap::New) {
  if (info.IsConstructCall()) {
    CompiledModuleWrap* obj = new CompiledModuleWrap();
    obj->module_ = tinycc::createCompiler();
    obj->jsctx_ = new jsbridge::Context();
    obj->Wrap(info.This());
    info.GetReturnValue().Set(info.This());
  } else {
    // Invoked as plain function, turn into construct call
    Local<Function> cons = Nan::New(constructor);
    info.GetReturnValue().Set(Nan::NewInstance(cons).ToLocalChecked());
  }
}

NAN_METHOD(CompiledModuleWrap::SetLibPath) {
  CompiledModuleWrap* obj = Nan::ObjectWrap::Unwrap<CompiledModuleWrap>(info.Holder());

  if (info.Length() < 1 || !info[0]->IsString()) {
    Nan::ThrowTypeError("Path must be a string");
    return;
  }

  Nan::Utf8String path(info[0]);
  obj->module_->setLibPath(*path);
}

NAN_METHOD(CompiledModuleWrap::AddIncludePath) {
  CompiledModuleWrap* obj = Nan::ObjectWrap::Unwrap<CompiledModuleWrap>(info.Holder());

  if (info.Length() < 1 || !info[0]->IsString()) {
    Nan::ThrowTypeError("Path must be a string");
    return;
  }

  Nan::Utf8String path(info[0]);
  obj->module_->addIncludePath(*path);
}

NAN_METHOD(CompiledModuleWrap::AddLibraryPath) {
  CompiledModuleWrap* obj = Nan::ObjectWrap::Unwrap<CompiledModuleWrap>(info.Holder());

  if (info.Length() < 1 || !info[0]->IsString()) {
    Nan::ThrowTypeError("Path must be a string");
    return;
  }

  Nan::Utf8String path(info[0]);
  obj->module_->addLibraryPath(*path);
}

NAN_METHOD(CompiledModuleWrap::AddLibrary) {
  CompiledModuleWrap* obj = Nan::ObjectWrap::Unwrap<CompiledModuleWrap>(info.Holder());

  if (info.Length() < 1 || !info[0]->IsString()) {
    Nan::ThrowTypeError("Library name must be a string");
    return;
  }

  Nan::Utf8String name(info[0]);
  obj->module_->addLibrary(*name);
}

NAN_METHOD(CompiledModuleWrap::Define) {
  CompiledModuleWrap* obj = Nan::ObjectWrap::Unwrap<CompiledModuleWrap>(info.Holder());

  if (info.Length() < 1 || !info[0]->IsString()) {
    Nan::ThrowTypeError("Macro name must be a string");
    return;
  }

  Nan::Utf8String name(info[0]);
  std::string value = "";

  if (info.Length() >= 2 && info[1]->IsString()) {
    Nan::Utf8String val(info[1]);
    value = *val;
  }

  obj->module_->define(*name, value);
}

NAN_METHOD(CompiledModuleWrap::Undefine) {
  CompiledModuleWrap* obj = Nan::ObjectWrap::Unwrap<CompiledModuleWrap>(info.Holder());

  if (info.Length() < 1 || !info[0]->IsString()) {
    Nan::ThrowTypeError("Macro name must be a string");
    return;
  }

  Nan::Utf8String name(info[0]);
  obj->module_->undefine(*name);
}

NAN_METHOD(CompiledModuleWrap::Compile) {
  CompiledModuleWrap* obj = Nan::ObjectWrap::Unwrap<CompiledModuleWrap>(info.Holder());

  if (info.Length() < 1 || !info[0]->IsString()) {
    Nan::ThrowTypeError("Code must be a string");
    return;
  }

  Nan::Utf8String code(info[0]);
  bool success = obj->module_->compile(*code);

  info.GetReturnValue().Set(Nan::New<Boolean>(success));
}

NAN_METHOD(CompiledModuleWrap::CompileFile) {
  CompiledModuleWrap* obj = Nan::ObjectWrap::Unwrap<CompiledModuleWrap>(info.Holder());

  if (info.Length() < 1 || !info[0]->IsString()) {
    Nan::ThrowTypeError("Path must be a string");
    return;
  }

  Nan::Utf8String path(info[0]);
  bool success = obj->module_->compileFile(*path);

  info.GetReturnValue().Set(Nan::New<Boolean>(success));
}

NAN_METHOD(CompiledModuleWrap::GetSymbol) {
  CompiledModuleWrap* obj = Nan::ObjectWrap::Unwrap<CompiledModuleWrap>(info.Holder());

  if (info.Length() < 1 || !info[0]->IsString()) {
    Nan::ThrowTypeError("Symbol name must be a string");
    return;
  }

  Nan::Utf8String name(info[0]);
  void* sym = obj->module_->getSymbol(*name);

  if (sym == NULL) {
    info.GetReturnValue().Set(Nan::Null());
    return;
  }

  // Return as number (pointer address)
  info.GetReturnValue().Set(Nan::New<Number>(reinterpret_cast<uintptr_t>(sym)));
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
class NativeFunctionWrap : public Nan::ObjectWrap {
public:
  // Create with jsbridge mode (legacy)
  static Local<Object> Create(void* funcPtr, jsbridge::Context* jsctx, int argCount);
  // Create with native type signature
  static Local<Object> CreateTyped(void* funcPtr, jsbridge::Context* jsctx,
                                   NativeType returnType, const std::vector<NativeType>& argTypes);
  static NAN_METHOD(Call);

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

Local<Object> NativeFunctionWrap::Create(void* funcPtr, jsbridge::Context* jsctx, int argCount) {
  Nan::EscapableHandleScope scope;

  Local<FunctionTemplate> tpl = Nan::New<FunctionTemplate>();
  tpl->SetClassName(Nan::New("NativeFunction").ToLocalChecked());
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  Nan::SetPrototypeMethod(tpl, "call", Call);

  Local<Object> instance = tpl->InstanceTemplate()->NewInstance();

  NativeFunctionWrap* wrap = new NativeFunctionWrap();
  wrap->funcPtr_ = funcPtr;
  wrap->jsctx_ = jsctx;
  wrap->argCount_ = argCount;
  wrap->useJsbridge_ = true;
  wrap->Wrap(instance);

  return scope.Escape(instance);
}

Local<Object> NativeFunctionWrap::CreateTyped(void* funcPtr, jsbridge::Context* jsctx,
                                              NativeType returnType,
                                              const std::vector<NativeType>& argTypes) {
  Nan::EscapableHandleScope scope;

  Local<FunctionTemplate> tpl = Nan::New<FunctionTemplate>();
  tpl->SetClassName(Nan::New("NativeFunction").ToLocalChecked());
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  Nan::SetPrototypeMethod(tpl, "call", Call);

  Local<Object> instance = tpl->InstanceTemplate()->NewInstance();

  NativeFunctionWrap* wrap = new NativeFunctionWrap();
  wrap->funcPtr_ = funcPtr;
  wrap->jsctx_ = jsctx;
  wrap->argCount_ = static_cast<int>(argTypes.size());
  wrap->useJsbridge_ = false;
  wrap->returnType_ = returnType;
  wrap->argTypes_ = argTypes;
  wrap->Wrap(instance);

  return scope.Escape(instance);
}

// Native type call implementation using cdecl calling convention
// Builds a stack of uint32 slots and calls through typed function pointers
static uint32_t callNativeFunction(void* funcPtr, const std::vector<uint32_t>& stack, size_t slotCount) {
  // Cdecl function pointer types
  typedef uint32_t (*fn0_t)();
  typedef uint32_t (*fn1_t)(uint32_t);
  typedef uint32_t (*fn2_t)(uint32_t, uint32_t);
  typedef uint32_t (*fn3_t)(uint32_t, uint32_t, uint32_t);
  typedef uint32_t (*fn4_t)(uint32_t, uint32_t, uint32_t, uint32_t);
  typedef uint32_t (*fn5_t)(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
  typedef uint32_t (*fn6_t)(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
  typedef uint32_t (*fn7_t)(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
  typedef uint32_t (*fn8_t)(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);

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

NAN_METHOD(NativeFunctionWrap::Call) {
  NativeFunctionWrap* wrap = Nan::ObjectWrap::Unwrap<NativeFunctionWrap>(info.Holder());

  if (!wrap->funcPtr_ || !wrap->jsctx_) {
    Nan::ThrowError("Function not initialized");
    return;
  }

  // Native types mode - convert JS args to C types, call, convert result back
  if (!wrap->useJsbridge_) {
    std::vector<uint32_t> stack;
    std::vector<std::string> stringArgs;  // Keep strings alive

    // Convert each argument based on declared type
    for (size_t i = 0; i < wrap->argTypes_.size(); i++) {
      Local<Value> arg = (i < static_cast<size_t>(info.Length())) ? info[i] : Nan::Undefined();

      switch (wrap->argTypes_[i]) {
        case NTYPE_INT32:
        case NTYPE_UINT32:
          stack.push_back(static_cast<uint32_t>(Nan::To<int32_t>(arg).FromMaybe(0)));
          break;

        case NTYPE_INT64:
        case NTYPE_UINT64: {
          // 64-bit: push as two 32-bit slots (low, high)
          int64_t val = static_cast<int64_t>(Nan::To<double>(arg).FromMaybe(0.0));
          stack.push_back(static_cast<uint32_t>(val & 0xFFFFFFFF));
          stack.push_back(static_cast<uint32_t>((val >> 32) & 0xFFFFFFFF));
          break;
        }

        case NTYPE_FLOAT: {
          float f = static_cast<float>(Nan::To<double>(arg).FromMaybe(0.0));
          stack.push_back(*reinterpret_cast<uint32_t*>(&f));
          break;
        }

        case NTYPE_DOUBLE: {
          double d = Nan::To<double>(arg).FromMaybe(0.0);
          uint32_t* parts = reinterpret_cast<uint32_t*>(&d);
          stack.push_back(parts[0]);
          stack.push_back(parts[1]);
          break;
        }

        case NTYPE_STRING: {
          if (arg->IsString()) {
            Nan::Utf8String str(arg);
            stringArgs.push_back(std::string(*str));
            stack.push_back(reinterpret_cast<uint32_t>(stringArgs.back().c_str()));
          } else {
            stack.push_back(0);
          }
          break;
        }

        case NTYPE_POINTER:
          // Accept number as pointer
          stack.push_back(static_cast<uint32_t>(Nan::To<uint32_t>(arg).FromMaybe(0)));
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
    uint32_t resultLow = callNativeFunction(wrap->funcPtr_, stack, wrap->argTypes_.size());

    // Convert result based on return type
    switch (wrap->returnType_) {
      case NTYPE_VOID:
        info.GetReturnValue().SetUndefined();
        break;

      case NTYPE_INT32:
        info.GetReturnValue().Set(Nan::New<Integer>(static_cast<int32_t>(resultLow)));
        break;

      case NTYPE_UINT32:
        info.GetReturnValue().Set(Nan::New<Integer>(resultLow));
        break;

      case NTYPE_FLOAT: {
        float f = *reinterpret_cast<float*>(&resultLow);
        info.GetReturnValue().Set(Nan::New<Number>(f));
        break;
      }

      case NTYPE_DOUBLE: {
        // Note: double return from x86 cdecl is in FPU ST(0), not EAX:EDX
        // This simplified version treats it as 32-bit
        info.GetReturnValue().Set(Nan::New<Number>(static_cast<double>(resultLow)));
        break;
      }

      case NTYPE_POINTER:
        info.GetReturnValue().Set(Nan::New<Number>(resultLow));
        break;

      case NTYPE_STRING: {
        const char* str = reinterpret_cast<const char*>(resultLow);
        if (str != NULL) {
          info.GetReturnValue().Set(Nan::New<String>(str).ToLocalChecked());
        } else {
          info.GetReturnValue().SetNull();
        }
        break;
      }

      default:
        info.GetReturnValue().Set(Nan::New<Integer>(static_cast<int32_t>(resultLow)));
        break;
    }

    return;
  }

  // jsbridge mode (original behavior)
  jsbridge::ContextScope ctxScope(wrap->jsctx_);

  // Convert arguments to jsvalues
  int argc = info.Length();
  std::vector<jsvalue> args(argc);

  for (int i = 0; i < argc; i++) {
    args[i] = wrap->jsctx_->store(info[i]);
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
  Local<Value> jsResult = wrap->jsctx_->retrieve(result);
  wrap->jsctx_->release(result);

  info.GetReturnValue().Set(jsResult);
}

NAN_METHOD(CompiledModuleWrap::GetFunction) {
  CompiledModuleWrap* obj = Nan::ObjectWrap::Unwrap<CompiledModuleWrap>(info.Holder());

  if (info.Length() < 1 || !info[0]->IsString()) {
    Nan::ThrowTypeError("Function name must be a string");
    return;
  }

  Nan::Utf8String name(info[0]);

  void* funcPtr = obj->module_->getSymbol(*name);

  if (funcPtr == NULL) {
    Nan::ThrowError(obj->module_->getError().c_str());
    return;
  }

  // Check for typed signature: getFunction(name, returnType, [argTypes])
  // Or legacy: getFunction(name, argCount)
  bool isTypedCall = info.Length() >= 2 && info[1]->IsString();

  if (isTypedCall) {
    // New API: getFunction(name, returnType, argTypes[])
    Nan::Utf8String returnTypeStr(info[1]);
    NativeType returnType = parseType(*returnTypeStr);

    std::vector<NativeType> argTypes;

    if (info.Length() >= 3 && info[2]->IsArray()) {
      Local<Array> arr = Local<Array>::Cast(info[2]);
      for (uint32_t i = 0; i < arr->Length(); i++) {
        Local<Value> item = Nan::Get(arr, i).ToLocalChecked();
        if (item->IsString()) {
          Nan::Utf8String typeStr(item);
          argTypes.push_back(parseType(*typeStr));
        }
      }
    }

    Local<Object> wrapper = NativeFunctionWrap::CreateTyped(funcPtr, obj->jsctx_, returnType, argTypes);
    info.GetReturnValue().Set(wrapper);
  } else {
    // Legacy API: getFunction(name, argCount) - uses jsbridge mode
    int argCount = 0;
    if (info.Length() >= 2 && info[1]->IsNumber()) {
      argCount = Nan::To<int>(info[1]).FromJust();
    }

    Local<Object> wrapper = NativeFunctionWrap::Create(funcPtr, obj->jsctx_, argCount);
    info.GetReturnValue().Set(wrapper);
  }
}

NAN_METHOD(CompiledModuleWrap::GetError) {
  CompiledModuleWrap* obj = Nan::ObjectWrap::Unwrap<CompiledModuleWrap>(info.Holder());

  const std::string& error = obj->module_->getError();

  if (error.empty()) {
    info.GetReturnValue().Set(Nan::Null());
  } else {
    info.GetReturnValue().Set(Nan::New<String>(error.c_str()).ToLocalChecked());
  }
}

NAN_METHOD(CompiledModuleWrap::Release) {
  CompiledModuleWrap* obj = Nan::ObjectWrap::Unwrap<CompiledModuleWrap>(info.Holder());

  if (obj->module_) {
    obj->module_->release();
  }
  if (obj->jsctx_) {
    obj->jsctx_->clear();
  }
}

// Factory function to create new compiler
NAN_METHOD(CreateCompiler) {
  Local<Function> cons = Nan::New(CompiledModuleWrap::constructor);
  info.GetReturnValue().Set(Nan::NewInstance(cons).ToLocalChecked());
}

// Module initialization
void InitTinyCC(Local<Object> exports) {
  Local<Object> tinycc = Nan::New<Object>();

  CompiledModuleWrap::Init(tinycc);

  // Factory function
  Nan::SetMethod(tinycc, "create", CreateCompiler);

  exports->Set(Nan::New("tinycc").ToLocalChecked(), tinycc);
}
