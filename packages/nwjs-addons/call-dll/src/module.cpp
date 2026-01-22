#include <nan.h>
#include "dll_loader.h"
#include "function_call.h"

using namespace v8;
using namespace calldll;

// Type string to ArgType mapping
static ArgType parseType(const std::string& typeStr) {
  if (typeStr == "void") return TYPE_VOID;
  if (typeStr == "bool") return TYPE_BOOL;
  if (typeStr == "int8") return TYPE_INT8;
  if (typeStr == "uint8") return TYPE_UINT8;
  if (typeStr == "int16") return TYPE_INT16;
  if (typeStr == "uint16") return TYPE_UINT16;
  if (typeStr == "int32") return TYPE_INT32;
  if (typeStr == "uint32") return TYPE_UINT32;
  if (typeStr == "int64") return TYPE_INT64;
  if (typeStr == "uint64") return TYPE_UINT64;
  if (typeStr == "float") return TYPE_FLOAT;
  if (typeStr == "double") return TYPE_DOUBLE;
  if (typeStr == "pointer") return TYPE_POINTER;
  if (typeStr == "string") return TYPE_STRING;
  if (typeStr == "wstring") return TYPE_WSTRING;
  if (typeStr == "buffer") return TYPE_BUFFER;
  return TYPE_VOID;
}

// DLL Handle wrapper
class DLLHandleWrap : public Nan::ObjectWrap {
public:
  static void Init(Local<Object> exports);
  static NAN_METHOD(Load);
  static NAN_METHOD(LoadSystem);
  static NAN_METHOD(GetFunction);
  static NAN_METHOD(GetSymbol);
  static NAN_METHOD(Close);
  static NAN_METHOD(GetPath);
  static NAN_METHOD(GetError);

  static Nan::Persistent<Function> constructor;

  DLLHandle* handle_;

private:
  DLLHandleWrap() : handle_(NULL) {}
  ~DLLHandleWrap() {
    if (handle_) {
      delete handle_;
      handle_ = NULL;
    }
  }
};

Nan::Persistent<Function> DLLHandleWrap::constructor;

// DLL Function wrapper
class DLLFunctionWrap : public Nan::ObjectWrap {
public:
  static Local<Object> Create(DLLFunction* func);
  static NAN_METHOD(Call);
  static NAN_METHOD(GetPointer);

  DLLFunction* func_;

private:
  DLLFunctionWrap() : func_(NULL) {}
  ~DLLFunctionWrap() {
    if (func_) {
      delete func_;
      func_ = NULL;
    }
  }
};

void DLLHandleWrap::Init(Local<Object> exports) {
  Nan::HandleScope scope;

  // DLLHandle constructor
  Local<FunctionTemplate> tpl = Nan::New<FunctionTemplate>();
  tpl->SetClassName(Nan::New("DLLHandle").ToLocalChecked());
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  Nan::SetPrototypeMethod(tpl, "getFunction", GetFunction);
  Nan::SetPrototypeMethod(tpl, "getSymbol", GetSymbol);
  Nan::SetPrototypeMethod(tpl, "close", Close);
  Nan::SetPrototypeMethod(tpl, "getPath", GetPath);
  Nan::SetPrototypeMethod(tpl, "getError", GetError);

  constructor.Reset(tpl->GetFunction());

  // Module level functions
  Nan::SetMethod(exports, "load", Load);
  Nan::SetMethod(exports, "loadSystem", LoadSystem);
}

NAN_METHOD(DLLHandleWrap::Load) {
  if (info.Length() < 1 || !info[0]->IsString()) {
    Nan::ThrowTypeError("Path must be a string");
    return;
  }

  Nan::Utf8String path(info[0]);

  DLLHandle* handle = new DLLHandle(*path, false);

  if (!handle->isLoaded()) {
    std::string err = handle->getError();
    delete handle;
    Nan::ThrowError(err.c_str());
    return;
  }

  Local<Function> cons = Nan::New(constructor);
  Local<Object> instance = Nan::NewInstance(cons).ToLocalChecked();

  DLLHandleWrap* wrap = new DLLHandleWrap();
  wrap->handle_ = handle;
  wrap->Wrap(instance);

  info.GetReturnValue().Set(instance);
}

NAN_METHOD(DLLHandleWrap::LoadSystem) {
  if (info.Length() < 1 || !info[0]->IsString()) {
    Nan::ThrowTypeError("DLL name must be a string");
    return;
  }

  Nan::Utf8String name(info[0]);

  DLLHandle* handle = new DLLHandle(*name, true);

  if (!handle->isLoaded()) {
    std::string err = handle->getError();
    delete handle;
    Nan::ThrowError(err.c_str());
    return;
  }

  Local<Function> cons = Nan::New(constructor);
  Local<Object> instance = Nan::NewInstance(cons).ToLocalChecked();

  DLLHandleWrap* wrap = new DLLHandleWrap();
  wrap->handle_ = handle;
  wrap->Wrap(instance);

  info.GetReturnValue().Set(instance);
}

NAN_METHOD(DLLHandleWrap::GetFunction) {
  DLLHandleWrap* wrap = Nan::ObjectWrap::Unwrap<DLLHandleWrap>(info.Holder());

  if (!wrap->handle_ || !wrap->handle_->isLoaded()) {
    Nan::ThrowError("DLL not loaded");
    return;
  }

  // Arguments: name, returnType, argTypes[], options?
  if (info.Length() < 3) {
    Nan::ThrowTypeError("Expected: name, returnType, argTypes[]");
    return;
  }

  if (!info[0]->IsString()) {
    Nan::ThrowTypeError("Function name must be a string");
    return;
  }

  if (!info[1]->IsString()) {
    Nan::ThrowTypeError("Return type must be a string");
    return;
  }

  if (!info[2]->IsArray()) {
    Nan::ThrowTypeError("Argument types must be an array");
    return;
  }

  Nan::Utf8String funcName(info[0]);
  Nan::Utf8String returnTypeStr(info[1]);
  Local<Array> argTypesArr = info[2].As<Array>();

  // Parse return type
  ArgType returnType = parseType(*returnTypeStr);

  // Parse argument types
  std::vector<ArgType> argTypes;
  for (uint32_t i = 0; i < argTypesArr->Length(); i++) {
    Local<Value> elem = argTypesArr->Get(i);
    if (elem->IsString()) {
      Nan::Utf8String typeStr(elem);
      argTypes.push_back(parseType(*typeStr));
    }
  }

  // Parse options
  CallConvention convention = CALL_CDECL;
  if (info.Length() >= 4 && info[3]->IsObject()) {
    Local<Object> opts = info[3].As<Object>();
    Local<Value> convVal = opts->Get(Nan::New("callConvention").ToLocalChecked());
    if (convVal->IsString()) {
      Nan::Utf8String convStr(convVal);
      if (strcmp(*convStr, "stdcall") == 0) {
        convention = CALL_STDCALL;
      } else if (strcmp(*convStr, "fastcall") == 0) {
        convention = CALL_FASTCALL;
      }
    }
  }

  // Get function pointer
  void* funcPtr = wrap->handle_->getFunction(*funcName);

  if (funcPtr == NULL) {
    Nan::ThrowError(wrap->handle_->getError().c_str());
    return;
  }

  // Create DLLFunction
  DLLFunction* func = new DLLFunction(funcPtr, returnType, argTypes, convention);
  Local<Object> funcObj = DLLFunctionWrap::Create(func);

  info.GetReturnValue().Set(funcObj);
}

NAN_METHOD(DLLHandleWrap::GetSymbol) {
  DLLHandleWrap* wrap = Nan::ObjectWrap::Unwrap<DLLHandleWrap>(info.Holder());

  if (!wrap->handle_ || !wrap->handle_->isLoaded()) {
    Nan::ThrowError("DLL not loaded");
    return;
  }

  if (info.Length() < 1 || !info[0]->IsString()) {
    Nan::ThrowTypeError("Symbol name must be a string");
    return;
  }

  Nan::Utf8String name(info[0]);
  void* sym = wrap->handle_->getSymbol(*name);

  if (sym == NULL) {
    info.GetReturnValue().Set(Nan::Null());
  } else {
    info.GetReturnValue().Set(Nan::New<Number>(reinterpret_cast<uintptr_t>(sym)));
  }
}

NAN_METHOD(DLLHandleWrap::Close) {
  DLLHandleWrap* wrap = Nan::ObjectWrap::Unwrap<DLLHandleWrap>(info.Holder());

  if (wrap->handle_) {
    wrap->handle_->close();
  }
}

NAN_METHOD(DLLHandleWrap::GetPath) {
  DLLHandleWrap* wrap = Nan::ObjectWrap::Unwrap<DLLHandleWrap>(info.Holder());

  if (wrap->handle_) {
    info.GetReturnValue().Set(
      Nan::New<String>(wrap->handle_->path().c_str()).ToLocalChecked()
    );
  } else {
    info.GetReturnValue().Set(Nan::Null());
  }
}

NAN_METHOD(DLLHandleWrap::GetError) {
  DLLHandleWrap* wrap = Nan::ObjectWrap::Unwrap<DLLHandleWrap>(info.Holder());

  if (wrap->handle_) {
    const std::string& err = wrap->handle_->getError();
    if (err.empty()) {
      info.GetReturnValue().Set(Nan::Null());
    } else {
      info.GetReturnValue().Set(Nan::New<String>(err.c_str()).ToLocalChecked());
    }
  } else {
    info.GetReturnValue().Set(Nan::Null());
  }
}

// DLLFunction wrapper implementation

Local<Object> DLLFunctionWrap::Create(DLLFunction* func) {
  Nan::EscapableHandleScope scope;

  Local<FunctionTemplate> tpl = Nan::New<FunctionTemplate>();
  tpl->SetClassName(Nan::New("DLLFunction").ToLocalChecked());
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  Nan::SetPrototypeMethod(tpl, "call", Call);
  Nan::SetPrototypeMethod(tpl, "getPointer", GetPointer);

  Local<Object> instance = Nan::NewInstance(tpl->GetFunction()).ToLocalChecked();

  DLLFunctionWrap* wrap = new DLLFunctionWrap();
  wrap->func_ = func;
  wrap->Wrap(instance);

  return scope.Escape(instance);
}

NAN_METHOD(DLLFunctionWrap::Call) {
  DLLFunctionWrap* wrap = Nan::ObjectWrap::Unwrap<DLLFunctionWrap>(info.Holder());

  if (!wrap->func_) {
    Nan::ThrowError("Function not initialized");
    return;
  }

  // Convert JS arguments to FunctionArgs
  std::vector<FunctionArg> args;

  for (int i = 0; i < info.Length(); i++) {
    FunctionArg arg;

    Local<Value> val = info[i];

    if (val->IsNull() || val->IsUndefined()) {
      arg.type = TYPE_POINTER;
      arg.value.ptrVal = NULL;
    }
    else if (val->IsBoolean()) {
      arg.type = TYPE_BOOL;
      arg.value.boolVal = val->BooleanValue();
    }
    else if (val->IsNumber()) {
      double num = Nan::To<double>(val).FromJust();
      // Check if it's an integer
      if (num == static_cast<double>(static_cast<int32_t>(num))) {
        arg.type = TYPE_INT32;
        arg.value.int32Val = static_cast<int32_t>(num);
      } else {
        arg.type = TYPE_DOUBLE;
        arg.value.doubleVal = num;
      }
    }
    else if (val->IsString()) {
      arg.type = TYPE_STRING;
      Nan::Utf8String str(val);
      arg.strValue = *str;
      arg.value.ptrVal = const_cast<char*>(arg.strValue.c_str());
    }
    else if (val->IsObject()) {
      Local<Object> obj = val.As<Object>();

      // Check for Buffer
      if (node::Buffer::HasInstance(obj)) {
        arg.type = TYPE_BUFFER;
        arg.value.ptrVal = node::Buffer::Data(obj);
        arg.bufferSize = node::Buffer::Length(obj);
      }
      // Check for TypedArray
      else if (obj->HasIndexedPropertiesInExternalArrayData()) {
        arg.type = TYPE_POINTER;
        arg.value.ptrVal = obj->GetIndexedPropertiesExternalArrayData();
      }
      else {
        arg.type = TYPE_POINTER;
        arg.value.ptrVal = NULL;
      }
    }
    else {
      arg.type = TYPE_POINTER;
      arg.value.ptrVal = NULL;
    }

    args.push_back(arg);
  }

  // Call the function
  FunctionArg result = wrap->func_->call(args);

  // Convert result to JS value
  switch (result.type) {
    case TYPE_VOID:
      info.GetReturnValue().Set(Nan::Undefined());
      break;

    case TYPE_BOOL:
      info.GetReturnValue().Set(Nan::New<Boolean>(result.value.boolVal));
      break;

    case TYPE_INT8:
    case TYPE_INT16:
    case TYPE_INT32:
      info.GetReturnValue().Set(Nan::New<Integer>(result.value.int32Val));
      break;

    case TYPE_UINT8:
    case TYPE_UINT16:
    case TYPE_UINT32:
      info.GetReturnValue().Set(Nan::New<Integer>(static_cast<int32_t>(result.value.uint32Val)));
      break;

    case TYPE_INT64:
    case TYPE_UINT64:
      // Use Number (may lose precision for large values)
      info.GetReturnValue().Set(Nan::New<Number>(static_cast<double>(result.value.int64Val)));
      break;

    case TYPE_FLOAT:
      info.GetReturnValue().Set(Nan::New<Number>(result.value.floatVal));
      break;

    case TYPE_DOUBLE:
      info.GetReturnValue().Set(Nan::New<Number>(result.value.doubleVal));
      break;

    case TYPE_POINTER:
    case TYPE_STRING:
    case TYPE_WSTRING:
    case TYPE_BUFFER:
      if (result.value.ptrVal == NULL) {
        info.GetReturnValue().Set(Nan::Null());
      } else {
        info.GetReturnValue().Set(
          Nan::New<Number>(reinterpret_cast<uintptr_t>(result.value.ptrVal))
        );
      }
      break;

    default:
      info.GetReturnValue().Set(Nan::Undefined());
      break;
  }
}

NAN_METHOD(DLLFunctionWrap::GetPointer) {
  DLLFunctionWrap* wrap = Nan::ObjectWrap::Unwrap<DLLFunctionWrap>(info.Holder());

  if (wrap->func_) {
    info.GetReturnValue().Set(
      Nan::New<Number>(reinterpret_cast<uintptr_t>(wrap->func_->pointer()))
    );
  } else {
    info.GetReturnValue().Set(Nan::New<Number>(0));
  }
}

// Memory allocation helpers
NAN_METHOD(AllocMemory) {
  if (info.Length() < 1 || !info[0]->IsNumber()) {
    Nan::ThrowTypeError("Size must be a number");
    return;
  }

  size_t size = static_cast<size_t>(Nan::To<uint32_t>(info[0]).FromJust());
  void* ptr = malloc(size);

  if (ptr == NULL) {
    Nan::ThrowError("Memory allocation failed");
    return;
  }

  memset(ptr, 0, size);

  info.GetReturnValue().Set(Nan::New<Number>(reinterpret_cast<uintptr_t>(ptr)));
}

NAN_METHOD(FreeMemory) {
  if (info.Length() < 1 || !info[0]->IsNumber()) {
    Nan::ThrowTypeError("Pointer must be a number");
    return;
  }

  uintptr_t addr = static_cast<uintptr_t>(Nan::To<double>(info[0]).FromJust());
  void* ptr = reinterpret_cast<void*>(addr);

  if (ptr != NULL) {
    free(ptr);
  }
}

NAN_METHOD(ReadInt32) {
  if (info.Length() < 1 || !info[0]->IsNumber()) {
    Nan::ThrowTypeError("Pointer must be a number");
    return;
  }

  uintptr_t addr = static_cast<uintptr_t>(Nan::To<double>(info[0]).FromJust());

  int offset = 0;
  if (info.Length() >= 2 && info[1]->IsNumber()) {
    offset = Nan::To<int>(info[1]).FromJust();
  }

  int32_t* ptr = reinterpret_cast<int32_t*>(addr + offset);
  info.GetReturnValue().Set(Nan::New<Integer>(*ptr));
}

NAN_METHOD(WriteInt32) {
  if (info.Length() < 2) {
    Nan::ThrowTypeError("Expected: pointer, value[, offset]");
    return;
  }

  uintptr_t addr = static_cast<uintptr_t>(Nan::To<double>(info[0]).FromJust());
  int32_t value = Nan::To<int32_t>(info[1]).FromJust();

  int offset = 0;
  if (info.Length() >= 3 && info[2]->IsNumber()) {
    offset = Nan::To<int>(info[2]).FromJust();
  }

  int32_t* ptr = reinterpret_cast<int32_t*>(addr + offset);
  *ptr = value;
}

// Module initialization
void InitCallDLL(Local<Object> exports) {
  Local<Object> calldll = Nan::New<Object>();

  DLLHandleWrap::Init(calldll);

  // Memory functions
  Nan::SetMethod(calldll, "alloc", AllocMemory);
  Nan::SetMethod(calldll, "free", FreeMemory);
  Nan::SetMethod(calldll, "readInt32", ReadInt32);
  Nan::SetMethod(calldll, "writeInt32", WriteInt32);

  // Type constants
  Local<Object> types = Nan::New<Object>();
  types->Set(Nan::New("void").ToLocalChecked(), Nan::New("void").ToLocalChecked());
  types->Set(Nan::New("bool").ToLocalChecked(), Nan::New("bool").ToLocalChecked());
  types->Set(Nan::New("int8").ToLocalChecked(), Nan::New("int8").ToLocalChecked());
  types->Set(Nan::New("uint8").ToLocalChecked(), Nan::New("uint8").ToLocalChecked());
  types->Set(Nan::New("int16").ToLocalChecked(), Nan::New("int16").ToLocalChecked());
  types->Set(Nan::New("uint16").ToLocalChecked(), Nan::New("uint16").ToLocalChecked());
  types->Set(Nan::New("int32").ToLocalChecked(), Nan::New("int32").ToLocalChecked());
  types->Set(Nan::New("uint32").ToLocalChecked(), Nan::New("uint32").ToLocalChecked());
  types->Set(Nan::New("int64").ToLocalChecked(), Nan::New("int64").ToLocalChecked());
  types->Set(Nan::New("uint64").ToLocalChecked(), Nan::New("uint64").ToLocalChecked());
  types->Set(Nan::New("float").ToLocalChecked(), Nan::New("float").ToLocalChecked());
  types->Set(Nan::New("double").ToLocalChecked(), Nan::New("double").ToLocalChecked());
  types->Set(Nan::New("pointer").ToLocalChecked(), Nan::New("pointer").ToLocalChecked());
  types->Set(Nan::New("string").ToLocalChecked(), Nan::New("string").ToLocalChecked());
  types->Set(Nan::New("wstring").ToLocalChecked(), Nan::New("wstring").ToLocalChecked());
  types->Set(Nan::New("buffer").ToLocalChecked(), Nan::New("buffer").ToLocalChecked());

  calldll->Set(Nan::New("types").ToLocalChecked(), types);

  exports->Set(Nan::New("calldll").ToLocalChecked(), calldll);
}
