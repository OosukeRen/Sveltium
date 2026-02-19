#include "addon_api.h"
#include "dll_loader.h"
#include "function_call.h"

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
class DLLHandleWrap : public ADDON_OBJECT_WRAP {
public:
  static void Init(ADDON_INIT_PARAMS);
  static ADDON_METHOD(Load);
  static ADDON_METHOD(LoadSystem);
  static ADDON_METHOD(GetFunction);
  static ADDON_METHOD(GetSymbol);
  static ADDON_METHOD(Close);
  static ADDON_METHOD(GetPath);
  static ADDON_METHOD(GetError);

  static ADDON_PERSISTENT_FUNCTION constructor;

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

ADDON_PERSISTENT_FUNCTION DLLHandleWrap::constructor;

// DLL Function wrapper
class DLLFunctionWrap : public ADDON_OBJECT_WRAP {
public:
  static ADDON_OBJECT_TYPE Create(DLLFunction* func);
  static ADDON_METHOD(Call);
  static ADDON_METHOD(GetPointer);

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

void DLLHandleWrap::Init(ADDON_INIT_PARAMS) {
  ADDON_HANDLE_SCOPE();

  // DLLHandle constructor
  auto tpl = ADDON_NEW_CTOR_TEMPLATE();
  ADDON_SET_CLASS_NAME(tpl, "DLLHandle");
  ADDON_SET_INTERNAL_FIELD_COUNT(tpl, 1);

  ADDON_SET_PROTOTYPE_METHOD(tpl, "getFunction", GetFunction);
  ADDON_SET_PROTOTYPE_METHOD(tpl, "getSymbol", GetSymbol);
  ADDON_SET_PROTOTYPE_METHOD(tpl, "close", Close);
  ADDON_SET_PROTOTYPE_METHOD(tpl, "getPath", GetPath);
  ADDON_SET_PROTOTYPE_METHOD(tpl, "getError", GetError);

  ADDON_PERSISTENT_RESET(constructor, ADDON_GET_CTOR_FUNCTION(tpl));

  // Module level functions
  ADDON_SET_METHOD(exports, "load", Load);
  ADDON_SET_METHOD(exports, "loadSystem", LoadSystem);
}

ADDON_METHOD(DLLHandleWrap::Load) {
  ADDON_ENV;
  if (ADDON_ARG_COUNT() < 1 || !ADDON_IS_STRING(ADDON_ARG(0))) {
    ADDON_THROW_TYPE_ERROR("Path must be a string");
    ADDON_VOID_RETURN();
  }

  ADDON_UTF8(path, ADDON_ARG(0));

  DLLHandle* handle = new DLLHandle(ADDON_UTF8_VALUE(path), false);

  if (!handle->isLoaded()) {
    std::string err = handle->getError();
    delete handle;
    ADDON_THROW_ERROR(err.c_str());
    ADDON_VOID_RETURN();
  }

  auto cons = ADDON_PERSISTENT_GET(constructor);
  ADDON_OBJECT_TYPE instance = ADDON_NEW_INSTANCE(cons);

  DLLHandleWrap* wrap = new DLLHandleWrap();
  wrap->handle_ = handle;
  wrap->Wrap(instance);

  ADDON_RETURN(instance);
}

ADDON_METHOD(DLLHandleWrap::LoadSystem) {
  ADDON_ENV;
  if (ADDON_ARG_COUNT() < 1 || !ADDON_IS_STRING(ADDON_ARG(0))) {
    ADDON_THROW_TYPE_ERROR("DLL name must be a string");
    ADDON_VOID_RETURN();
  }

  ADDON_UTF8(name, ADDON_ARG(0));

  DLLHandle* handle = new DLLHandle(ADDON_UTF8_VALUE(name), true);

  if (!handle->isLoaded()) {
    std::string err = handle->getError();
    delete handle;
    ADDON_THROW_ERROR(err.c_str());
    ADDON_VOID_RETURN();
  }

  auto cons = ADDON_PERSISTENT_GET(constructor);
  ADDON_OBJECT_TYPE instance = ADDON_NEW_INSTANCE(cons);

  DLLHandleWrap* wrap = new DLLHandleWrap();
  wrap->handle_ = handle;
  wrap->Wrap(instance);

  ADDON_RETURN(instance);
}

ADDON_METHOD(DLLHandleWrap::GetFunction) {
  ADDON_ENV;
  DLLHandleWrap* wrap = ADDON_UNWRAP(DLLHandleWrap, ADDON_HOLDER());

  if (!wrap->handle_ || !wrap->handle_->isLoaded()) {
    ADDON_THROW_ERROR("DLL not loaded");
    ADDON_VOID_RETURN();
  }

  // Arguments: name, returnType, argTypes[], options?
  if (ADDON_ARG_COUNT() < 3) {
    ADDON_THROW_TYPE_ERROR("Expected: name, returnType, argTypes[]");
    ADDON_VOID_RETURN();
  }

  if (!ADDON_IS_STRING(ADDON_ARG(0))) {
    ADDON_THROW_TYPE_ERROR("Function name must be a string");
    ADDON_VOID_RETURN();
  }

  if (!ADDON_IS_STRING(ADDON_ARG(1))) {
    ADDON_THROW_TYPE_ERROR("Return type must be a string");
    ADDON_VOID_RETURN();
  }

  if (!ADDON_IS_ARRAY(ADDON_ARG(2))) {
    ADDON_THROW_TYPE_ERROR("Argument types must be an array");
    ADDON_VOID_RETURN();
  }

  ADDON_UTF8(funcName, ADDON_ARG(0));
  ADDON_UTF8(returnTypeStr, ADDON_ARG(1));
  ADDON_ARRAY_TYPE argTypesArr = ADDON_AS_ARRAY(ADDON_ARG(2));

  // Parse return type
  ArgType returnType = parseType(ADDON_UTF8_VALUE(returnTypeStr));

  // Parse argument types
  std::vector<ArgType> argTypes;
  for (uint32_t i = 0; i < ADDON_LENGTH(argTypesArr); i++) {
    ADDON_VALUE elem = ADDON_GET_INDEX(argTypesArr, i);
    if (ADDON_IS_STRING(elem)) {
      ADDON_UTF8(typeStr, elem);
      argTypes.push_back(parseType(ADDON_UTF8_VALUE(typeStr)));
    }
  }

  // Parse options
  CallConvention convention = CALL_CDECL;
  if (ADDON_ARG_COUNT() >= 4 && ADDON_IS_OBJECT(ADDON_ARG(3))) {
    ADDON_OBJECT_TYPE opts = ADDON_AS_OBJECT(ADDON_ARG(3));
    ADDON_VALUE convVal = ADDON_GET(opts, "callConvention");
    if (ADDON_IS_STRING(convVal)) {
      ADDON_UTF8(convStr, convVal);
      if (strcmp(ADDON_UTF8_VALUE(convStr), "stdcall") == 0) {
        convention = CALL_STDCALL;
      } else if (strcmp(ADDON_UTF8_VALUE(convStr), "fastcall") == 0) {
        convention = CALL_FASTCALL;
      }
    }
  }

  // Get function pointer
  void* funcPtr = wrap->handle_->getFunction(ADDON_UTF8_VALUE(funcName));

  if (funcPtr == NULL) {
    ADDON_THROW_ERROR(wrap->handle_->getError().c_str());
    ADDON_VOID_RETURN();
  }

  // Create DLLFunction
  DLLFunction* func = new DLLFunction(funcPtr, returnType, argTypes, convention);
  ADDON_OBJECT_TYPE funcObj = DLLFunctionWrap::Create(func);

  ADDON_RETURN(funcObj);
}

ADDON_METHOD(DLLHandleWrap::GetSymbol) {
  ADDON_ENV;
  DLLHandleWrap* wrap = ADDON_UNWRAP(DLLHandleWrap, ADDON_HOLDER());

  if (!wrap->handle_ || !wrap->handle_->isLoaded()) {
    ADDON_THROW_ERROR("DLL not loaded");
    ADDON_VOID_RETURN();
  }

  if (ADDON_ARG_COUNT() < 1 || !ADDON_IS_STRING(ADDON_ARG(0))) {
    ADDON_THROW_TYPE_ERROR("Symbol name must be a string");
    ADDON_VOID_RETURN();
  }

  ADDON_UTF8(name, ADDON_ARG(0));
  void* sym = wrap->handle_->getSymbol(ADDON_UTF8_VALUE(name));

  if (sym == NULL) {
    ADDON_RETURN_NULL();
  }
  ADDON_RETURN(ADDON_NUMBER(reinterpret_cast<uintptr_t>(sym)));
}

ADDON_METHOD(DLLHandleWrap::Close) {
  ADDON_ENV;
  DLLHandleWrap* wrap = ADDON_UNWRAP(DLLHandleWrap, ADDON_HOLDER());

  if (wrap->handle_) {
    wrap->handle_->close();
  }
  ADDON_VOID_RETURN();
}

ADDON_METHOD(DLLHandleWrap::GetPath) {
  ADDON_ENV;
  DLLHandleWrap* wrap = ADDON_UNWRAP(DLLHandleWrap, ADDON_HOLDER());

  if (wrap->handle_) {
    ADDON_RETURN(ADDON_STRING(wrap->handle_->path().c_str()));
  }
  ADDON_RETURN_NULL();
}

ADDON_METHOD(DLLHandleWrap::GetError) {
  ADDON_ENV;
  DLLHandleWrap* wrap = ADDON_UNWRAP(DLLHandleWrap, ADDON_HOLDER());

  if (wrap->handle_) {
    const std::string& err = wrap->handle_->getError();
    if (!err.empty()) {
      ADDON_RETURN(ADDON_STRING(err.c_str()));
    }
  }
  ADDON_RETURN_NULL();
}

// DLLFunction wrapper implementation

ADDON_OBJECT_TYPE DLLFunctionWrap::Create(DLLFunction* func) {
  ADDON_ESCAPABLE_SCOPE();

  auto tpl = ADDON_NEW_CTOR_TEMPLATE();
  ADDON_SET_CLASS_NAME(tpl, "DLLFunction");
  ADDON_SET_INTERNAL_FIELD_COUNT(tpl, 1);

  ADDON_SET_PROTOTYPE_METHOD(tpl, "call", Call);
  ADDON_SET_PROTOTYPE_METHOD(tpl, "getPointer", GetPointer);

  ADDON_OBJECT_TYPE instance = ADDON_NEW_INSTANCE(ADDON_GET_CTOR_FUNCTION(tpl));

  DLLFunctionWrap* wrap = new DLLFunctionWrap();
  wrap->func_ = func;
  wrap->Wrap(instance);

  return ADDON_ESCAPE(instance);
}

ADDON_METHOD(DLLFunctionWrap::Call) {
  ADDON_ENV;
  DLLFunctionWrap* wrap = ADDON_UNWRAP(DLLFunctionWrap, ADDON_HOLDER());

  if (!wrap->func_) {
    ADDON_THROW_ERROR("Function not initialized");
    ADDON_VOID_RETURN();
  }

  // Convert JS arguments to FunctionArgs
  std::vector<FunctionArg> args;

  for (int i = 0; i < ADDON_ARG_COUNT(); i++) {
    FunctionArg arg;

    ADDON_VALUE val = ADDON_ARG(i);

    if (ADDON_IS_NULL(val) || ADDON_IS_UNDEFINED(val)) {
      arg.type = TYPE_POINTER;
      arg.value.ptrVal = NULL;
    }
    else if (ADDON_IS_BOOLEAN(val)) {
      arg.type = TYPE_BOOL;
      arg.value.boolVal = ADDON_BOOL_VALUE(val);
    }
    else if (ADDON_IS_NUMBER(val)) {
      double num = ADDON_TO_DOUBLE(val);
      // Check if it's an integer
      if (num == static_cast<double>(static_cast<int32_t>(num))) {
        arg.type = TYPE_INT32;
        arg.value.int32Val = static_cast<int32_t>(num);
      } else {
        arg.type = TYPE_DOUBLE;
        arg.value.doubleVal = num;
      }
    }
    else if (ADDON_IS_STRING(val)) {
      arg.type = TYPE_STRING;
      ADDON_UTF8(str, val);
      arg.strValue = ADDON_UTF8_VALUE(str);
      arg.value.ptrVal = const_cast<char*>(arg.strValue.c_str());
    }
    else if (ADDON_IS_OBJECT(val)) {
      ADDON_OBJECT_TYPE obj = ADDON_AS_OBJECT(val);

      // Check for Buffer
      if (ADDON_BUFFER_IS(obj)) {
        arg.type = TYPE_BUFFER;
        arg.value.ptrVal = ADDON_BUFFER_DATA(obj);
        arg.bufferSize = ADDON_BUFFER_LENGTH(obj);
      }
      // Check for TypedArray
      else if (ADDON_IS_TYPEDARRAY(obj)) {
        arg.type = TYPE_POINTER;
        arg.value.ptrVal = ADDON_GET_TYPEDARRAY_DATA(obj);
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
      ADDON_RETURN_UNDEFINED();

    case TYPE_BOOL:
      ADDON_RETURN(ADDON_BOOLEAN(result.value.boolVal));

    case TYPE_INT8:
    case TYPE_INT16:
    case TYPE_INT32:
      ADDON_RETURN(ADDON_INTEGER(result.value.int32Val));

    case TYPE_UINT8:
    case TYPE_UINT16:
    case TYPE_UINT32:
      ADDON_RETURN(ADDON_INTEGER(static_cast<int32_t>(result.value.uint32Val)));

    case TYPE_INT64:
    case TYPE_UINT64:
      // Use Number (may lose precision for large values)
      ADDON_RETURN(ADDON_NUMBER(result.value.int64Val));

    case TYPE_FLOAT:
      ADDON_RETURN(ADDON_NUMBER(result.value.floatVal));

    case TYPE_DOUBLE:
      ADDON_RETURN(ADDON_NUMBER(result.value.doubleVal));

    case TYPE_POINTER:
    case TYPE_STRING:
    case TYPE_WSTRING:
    case TYPE_BUFFER:
      if (result.value.ptrVal == NULL) {
        ADDON_RETURN_NULL();
      }
      ADDON_RETURN(ADDON_NUMBER(reinterpret_cast<uintptr_t>(result.value.ptrVal)));

    default:
      ADDON_RETURN_UNDEFINED();
  }
}

ADDON_METHOD(DLLFunctionWrap::GetPointer) {
  ADDON_ENV;
  DLLFunctionWrap* wrap = ADDON_UNWRAP(DLLFunctionWrap, ADDON_HOLDER());

  if (wrap->func_) {
    ADDON_RETURN(ADDON_NUMBER(reinterpret_cast<uintptr_t>(wrap->func_->pointer())));
  }
  ADDON_RETURN(ADDON_NUMBER(0));
}

// Memory allocation helpers
ADDON_METHOD(AllocMemory) {
  ADDON_ENV;
  if (ADDON_ARG_COUNT() < 1 || !ADDON_IS_NUMBER(ADDON_ARG(0))) {
    ADDON_THROW_TYPE_ERROR("Size must be a number");
    ADDON_VOID_RETURN();
  }

  size_t size = static_cast<size_t>(ADDON_TO_UINT32(ADDON_ARG(0)));
  void* ptr = malloc(size);

  if (ptr == NULL) {
    ADDON_THROW_ERROR("Memory allocation failed");
    ADDON_VOID_RETURN();
  }

  memset(ptr, 0, size);

  ADDON_RETURN(ADDON_NUMBER(reinterpret_cast<uintptr_t>(ptr)));
}

ADDON_METHOD(FreeMemory) {
  ADDON_ENV;
  if (ADDON_ARG_COUNT() < 1 || !ADDON_IS_NUMBER(ADDON_ARG(0))) {
    ADDON_THROW_TYPE_ERROR("Pointer must be a number");
    ADDON_VOID_RETURN();
  }

  uintptr_t addr = static_cast<uintptr_t>(ADDON_TO_DOUBLE(ADDON_ARG(0)));
  void* ptr = reinterpret_cast<void*>(addr);

  if (ptr != NULL) {
    free(ptr);
  }
  ADDON_VOID_RETURN();
}

ADDON_METHOD(ReadInt32) {
  ADDON_ENV;
  if (ADDON_ARG_COUNT() < 1 || !ADDON_IS_NUMBER(ADDON_ARG(0))) {
    ADDON_THROW_TYPE_ERROR("Pointer must be a number");
    ADDON_VOID_RETURN();
  }

  uintptr_t addr = static_cast<uintptr_t>(ADDON_TO_DOUBLE(ADDON_ARG(0)));

  int offset = 0;
  if (ADDON_ARG_COUNT() >= 2 && ADDON_IS_NUMBER(ADDON_ARG(1))) {
    offset = ADDON_TO_INT(ADDON_ARG(1));
  }

  int32_t* ptr = reinterpret_cast<int32_t*>(addr + offset);
  ADDON_RETURN(ADDON_INTEGER(*ptr));
}

ADDON_METHOD(WriteInt32) {
  ADDON_ENV;
  if (ADDON_ARG_COUNT() < 2) {
    ADDON_THROW_TYPE_ERROR("Expected: pointer, value[, offset]");
    ADDON_VOID_RETURN();
  }

  uintptr_t addr = static_cast<uintptr_t>(ADDON_TO_DOUBLE(ADDON_ARG(0)));
  int32_t value = ADDON_TO_INT32(ADDON_ARG(1));

  int offset = 0;
  if (ADDON_ARG_COUNT() >= 3 && ADDON_IS_NUMBER(ADDON_ARG(2))) {
    offset = ADDON_TO_INT(ADDON_ARG(2));
  }

  int32_t* ptr = reinterpret_cast<int32_t*>(addr + offset);
  *ptr = value;
  ADDON_VOID_RETURN();
}

// Module initialization
void InitCallDLL(ADDON_INIT_PARAMS) {
  ADDON_OBJECT_TYPE calldll = ADDON_OBJECT();

  DLLHandleWrap::Init(calldll);

  // Memory functions
  ADDON_SET_METHOD(calldll, "alloc", AllocMemory);
  ADDON_SET_METHOD(calldll, "free", FreeMemory);
  ADDON_SET_METHOD(calldll, "readInt32", ReadInt32);
  ADDON_SET_METHOD(calldll, "writeInt32", WriteInt32);

  // Type constants
  ADDON_OBJECT_TYPE types = ADDON_OBJECT();
  ADDON_SET(types, "void", ADDON_STRING("void"));
  ADDON_SET(types, "bool", ADDON_STRING("bool"));
  ADDON_SET(types, "int8", ADDON_STRING("int8"));
  ADDON_SET(types, "uint8", ADDON_STRING("uint8"));
  ADDON_SET(types, "int16", ADDON_STRING("int16"));
  ADDON_SET(types, "uint16", ADDON_STRING("uint16"));
  ADDON_SET(types, "int32", ADDON_STRING("int32"));
  ADDON_SET(types, "uint32", ADDON_STRING("uint32"));
  ADDON_SET(types, "int64", ADDON_STRING("int64"));
  ADDON_SET(types, "uint64", ADDON_STRING("uint64"));
  ADDON_SET(types, "float", ADDON_STRING("float"));
  ADDON_SET(types, "double", ADDON_STRING("double"));
  ADDON_SET(types, "pointer", ADDON_STRING("pointer"));
  ADDON_SET(types, "string", ADDON_STRING("string"));
  ADDON_SET(types, "wstring", ADDON_STRING("wstring"));
  ADDON_SET(types, "buffer", ADDON_STRING("buffer"));

  ADDON_SET(calldll, "types", types);

  ADDON_SET(exports, "calldll", calldll);
}
