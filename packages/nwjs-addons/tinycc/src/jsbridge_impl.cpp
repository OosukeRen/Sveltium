#include "jsbridge_impl.h"
#include <cstdarg>
#include <cstring>
#include <cstdlib>

using namespace v8;

// Thread-local storage for current context
#if defined(_MSC_VER)
  __declspec(thread) jsbridge::Context* tls_current_context = NULL;
#else
  __thread jsbridge::Context* tls_current_context = NULL;
#endif

// Global context pointer (exported for C code)
jscontext __jscontext = NULL;

namespace jsbridge {

Context::Context()
  : nextHandle_(1)  // Start at 1, 0 is reserved for null/undefined
{
}

Context::~Context() {
  clear();
}

jsvalue Context::store(Local<Value> value) {
  jsvalue result = { 0 };

  // Handle undefined/null specially
  if (value.IsEmpty() || value->IsUndefined()) {
    return result;
  }

  if (value->IsNull()) {
    result.v = 0xFFFFFFFFFFFFFFFFULL;  // Special marker for null
    return result;
  }

  // Store the value using pointer to avoid copy issues with Persistent
  StoredValue sv;
  sv.persistent = new Persistent<Value>(v8::Isolate::GetCurrent(), value);
  sv.refCount = 1;

  uint64_t handle = nextHandle_++;
  values_[handle] = sv;

  result.v = handle;
  return result;
}

Local<Value> Context::retrieve(jsvalue val) {
  // Handle special cases
  if (val.v == 0) {
    return Nan::Undefined();
  }

  if (val.v == 0xFFFFFFFFFFFFFFFFULL) {
    return Nan::Null();
  }

  std::map<uint64_t, StoredValue>::iterator it = values_.find(val.v);
  if (it == values_.end() || it->second.persistent == NULL) {
    return Nan::Undefined();
  }

  return Nan::New(*(it->second.persistent));
}

void Context::addRef(jsvalue val) {
  if (val.v == 0 || val.v == 0xFFFFFFFFFFFFFFFFULL) {
    return;
  }

  std::map<uint64_t, StoredValue>::iterator it = values_.find(val.v);
  if (it != values_.end()) {
    it->second.refCount++;
  }
}

void Context::release(jsvalue val) {
  if (val.v == 0 || val.v == 0xFFFFFFFFFFFFFFFFULL) {
    return;
  }

  std::map<uint64_t, StoredValue>::iterator it = values_.find(val.v);
  if (it != values_.end()) {
    it->second.refCount--;
    if (it->second.refCount <= 0) {
      if (it->second.persistent != NULL) {
        it->second.persistent->Reset();
        delete it->second.persistent;
        it->second.persistent = NULL;
      }
      values_.erase(it);
    }
  }
}

void Context::clear() {
  std::map<uint64_t, StoredValue>::iterator it;
  for (it = values_.begin(); it != values_.end(); ++it) {
    if (it->second.persistent != NULL) {
      it->second.persistent->Reset();
      delete it->second.persistent;
      it->second.persistent = NULL;
    }
  }
  values_.clear();
  nextHandle_ = 1;
}

Context* Context::current() {
  return tls_current_context;
}

void Context::setCurrent(Context* ctx) {
  tls_current_context = ctx;
  __jscontext = reinterpret_cast<jscontext>(ctx);
}

ContextScope::ContextScope(Context* ctx) {
  previous_ = Context::current();
  Context::setCurrent(ctx);
}

ContextScope::~ContextScope() {
  Context::setCurrent(previous_);
}

} // namespace jsbridge

// ============================================
// C API Implementation
// ============================================

extern "C" {

// Helper to get context
static jsbridge::Context* getCtx(jscontext ctx) {
  return reinterpret_cast<jsbridge::Context*>(ctx);
}

// C -> JS Conversions

jsvalue bool_to_jsvalue(jscontext ctx, bool val) {
  jsbridge::Context* c = getCtx(ctx);
  if (!c) { jsvalue r = {0}; return r; }
  return c->store(Nan::New<Boolean>(val));
}

jsvalue int8_to_jsvalue(jscontext ctx, int8 val) {
  jsbridge::Context* c = getCtx(ctx);
  if (!c) { jsvalue r = {0}; return r; }
  return c->store(Nan::New<Integer>(val));
}

jsvalue uint8_to_jsvalue(jscontext ctx, uint8 val) {
  jsbridge::Context* c = getCtx(ctx);
  if (!c) { jsvalue r = {0}; return r; }
  return c->store(Nan::New<Integer>(val));
}

jsvalue int16_to_jsvalue(jscontext ctx, int16 val) {
  jsbridge::Context* c = getCtx(ctx);
  if (!c) { jsvalue r = {0}; return r; }
  return c->store(Nan::New<Integer>(val));
}

jsvalue uint16_to_jsvalue(jscontext ctx, uint16 val) {
  jsbridge::Context* c = getCtx(ctx);
  if (!c) { jsvalue r = {0}; return r; }
  return c->store(Nan::New<Integer>(val));
}

jsvalue int32_to_jsvalue(jscontext ctx, int32 val) {
  jsbridge::Context* c = getCtx(ctx);
  if (!c) { jsvalue r = {0}; return r; }
  return c->store(Nan::New<Integer>(val));
}

jsvalue uint32_to_jsvalue(jscontext ctx, uint32 val) {
  jsbridge::Context* c = getCtx(ctx);
  if (!c) { jsvalue r = {0}; return r; }
  return c->store(Nan::New<Integer>(static_cast<uint32_t>(val)));
}

jsvalue int64_to_jsvalue(jscontext ctx, int64 val) {
  jsbridge::Context* c = getCtx(ctx);
  if (!c) { jsvalue r = {0}; return r; }
  // V8 3.x doesn't have BigInt, use Number (loses precision for large values)
  return c->store(Nan::New<Number>(static_cast<double>(val)));
}

jsvalue uint64_to_jsvalue(jscontext ctx, uint64 val) {
  jsbridge::Context* c = getCtx(ctx);
  if (!c) { jsvalue r = {0}; return r; }
  return c->store(Nan::New<Number>(static_cast<double>(val)));
}

jsvalue float_to_jsvalue(jscontext ctx, float val) {
  jsbridge::Context* c = getCtx(ctx);
  if (!c) { jsvalue r = {0}; return r; }
  return c->store(Nan::New<Number>(val));
}

jsvalue double_to_jsvalue(jscontext ctx, double val) {
  jsbridge::Context* c = getCtx(ctx);
  if (!c) { jsvalue r = {0}; return r; }
  return c->store(Nan::New<Number>(val));
}

// JS -> C Conversions

bool jsvalue_to_bool(jscontext ctx, jsvalue val) {
  jsbridge::Context* c = getCtx(ctx);
  if (!c) return false;
  Local<Value> v = c->retrieve(val);
  return v->BooleanValue();
}

int8 jsvalue_to_int8(jscontext ctx, jsvalue val) {
  jsbridge::Context* c = getCtx(ctx);
  if (!c) return 0;
  Local<Value> v = c->retrieve(val);
  return static_cast<int8>(Nan::To<int32_t>(v).FromMaybe(0));
}

uint8 jsvalue_to_uint8(jscontext ctx, jsvalue val) {
  jsbridge::Context* c = getCtx(ctx);
  if (!c) return 0;
  Local<Value> v = c->retrieve(val);
  return static_cast<uint8>(Nan::To<uint32_t>(v).FromMaybe(0));
}

int16 jsvalue_to_int16(jscontext ctx, jsvalue val) {
  jsbridge::Context* c = getCtx(ctx);
  if (!c) return 0;
  Local<Value> v = c->retrieve(val);
  return static_cast<int16>(Nan::To<int32_t>(v).FromMaybe(0));
}

uint16 jsvalue_to_uint16(jscontext ctx, jsvalue val) {
  jsbridge::Context* c = getCtx(ctx);
  if (!c) return 0;
  Local<Value> v = c->retrieve(val);
  return static_cast<uint16>(Nan::To<uint32_t>(v).FromMaybe(0));
}

int32 jsvalue_to_int32(jscontext ctx, jsvalue val) {
  jsbridge::Context* c = getCtx(ctx);
  if (!c) return 0;
  Local<Value> v = c->retrieve(val);
  return Nan::To<int32_t>(v).FromMaybe(0);
}

uint32 jsvalue_to_uint32(jscontext ctx, jsvalue val) {
  jsbridge::Context* c = getCtx(ctx);
  if (!c) return 0;
  Local<Value> v = c->retrieve(val);
  return Nan::To<uint32_t>(v).FromMaybe(0);
}

int64 jsvalue_to_int64(jscontext ctx, jsvalue val) {
  jsbridge::Context* c = getCtx(ctx);
  if (!c) return 0;
  Local<Value> v = c->retrieve(val);
  return static_cast<int64>(Nan::To<double>(v).FromMaybe(0.0));
}

uint64 jsvalue_to_uint64(jscontext ctx, jsvalue val) {
  jsbridge::Context* c = getCtx(ctx);
  if (!c) return 0;
  Local<Value> v = c->retrieve(val);
  return static_cast<uint64>(Nan::To<double>(v).FromMaybe(0.0));
}

float jsvalue_to_float(jscontext ctx, jsvalue val) {
  jsbridge::Context* c = getCtx(ctx);
  if (!c) return 0.0f;
  Local<Value> v = c->retrieve(val);
  return static_cast<float>(Nan::To<double>(v).FromMaybe(0.0));
}

double jsvalue_to_double(jscontext ctx, jsvalue val) {
  jsbridge::Context* c = getCtx(ctx);
  if (!c) return 0.0;
  Local<Value> v = c->retrieve(val);
  return Nan::To<double>(v).FromMaybe(0.0);
}

jsvalue jsvalue_to_jsvalue(jscontext ctx, jsvalue val) {
  return val;  // Identity
}

// TypedArray Pointer Access
// Note: V8 3.x TypedArray API differs from modern V8

int8* jsvalue_to_int8_ptr(jscontext ctx, jsvalue val) {
  jsbridge::Context* c = getCtx(ctx);
  if (!c) return NULL;
  Local<Value> v = c->retrieve(val);

  if (!v->IsObject()) return NULL;
  Local<Object> obj = v.As<Object>();

  // Check for Int8Array - V8 3.x style
  if (obj->HasIndexedPropertiesInExternalArrayData()) {
    return static_cast<int8*>(obj->GetIndexedPropertiesExternalArrayData());
  }

  return NULL;
}

uint8* jsvalue_to_uint8_ptr(jscontext ctx, jsvalue val) {
  jsbridge::Context* c = getCtx(ctx);
  if (!c) return NULL;
  Local<Value> v = c->retrieve(val);

  if (!v->IsObject()) return NULL;
  Local<Object> obj = v.As<Object>();

  if (obj->HasIndexedPropertiesInExternalArrayData()) {
    return static_cast<uint8*>(obj->GetIndexedPropertiesExternalArrayData());
  }

  return NULL;
}

int16* jsvalue_to_int16_ptr(jscontext ctx, jsvalue val) {
  jsbridge::Context* c = getCtx(ctx);
  if (!c) return NULL;
  Local<Value> v = c->retrieve(val);

  if (!v->IsObject()) return NULL;
  Local<Object> obj = v.As<Object>();

  if (obj->HasIndexedPropertiesInExternalArrayData()) {
    return static_cast<int16*>(obj->GetIndexedPropertiesExternalArrayData());
  }

  return NULL;
}

uint16* jsvalue_to_uint16_ptr(jscontext ctx, jsvalue val) {
  jsbridge::Context* c = getCtx(ctx);
  if (!c) return NULL;
  Local<Value> v = c->retrieve(val);

  if (!v->IsObject()) return NULL;
  Local<Object> obj = v.As<Object>();

  if (obj->HasIndexedPropertiesInExternalArrayData()) {
    return static_cast<uint16*>(obj->GetIndexedPropertiesExternalArrayData());
  }

  return NULL;
}

int32* jsvalue_to_int32_ptr(jscontext ctx, jsvalue val) {
  jsbridge::Context* c = getCtx(ctx);
  if (!c) return NULL;
  Local<Value> v = c->retrieve(val);

  if (!v->IsObject()) return NULL;
  Local<Object> obj = v.As<Object>();

  if (obj->HasIndexedPropertiesInExternalArrayData()) {
    return static_cast<int32*>(obj->GetIndexedPropertiesExternalArrayData());
  }

  return NULL;
}

uint32* jsvalue_to_uint32_ptr(jscontext ctx, jsvalue val) {
  jsbridge::Context* c = getCtx(ctx);
  if (!c) return NULL;
  Local<Value> v = c->retrieve(val);

  if (!v->IsObject()) return NULL;
  Local<Object> obj = v.As<Object>();

  if (obj->HasIndexedPropertiesInExternalArrayData()) {
    return static_cast<uint32*>(obj->GetIndexedPropertiesExternalArrayData());
  }

  return NULL;
}

int64* jsvalue_to_int64_ptr(jscontext ctx, jsvalue val) {
  // Note: BigInt64Array not available in V8 3.x
  return NULL;
}

uint64* jsvalue_to_uint64_ptr(jscontext ctx, jsvalue val) {
  // Note: BigUint64Array not available in V8 3.x
  return NULL;
}

float* jsvalue_to_float_ptr(jscontext ctx, jsvalue val) {
  jsbridge::Context* c = getCtx(ctx);
  if (!c) return NULL;
  Local<Value> v = c->retrieve(val);

  if (!v->IsObject()) return NULL;
  Local<Object> obj = v.As<Object>();

  if (obj->HasIndexedPropertiesInExternalArrayData()) {
    return static_cast<float*>(obj->GetIndexedPropertiesExternalArrayData());
  }

  return NULL;
}

double* jsvalue_to_double_ptr(jscontext ctx, jsvalue val) {
  jsbridge::Context* c = getCtx(ctx);
  if (!c) return NULL;
  Local<Value> v = c->retrieve(val);

  if (!v->IsObject()) return NULL;
  Local<Object> obj = v.As<Object>();

  if (obj->HasIndexedPropertiesInExternalArrayData()) {
    return static_cast<double*>(obj->GetIndexedPropertiesExternalArrayData());
  }

  return NULL;
}

// Type checking

int _jsvalue_type(jscontext ctx, jsvalue val) {
  jsbridge::Context* c = getCtx(ctx);
  if (!c) return JSVALUE_UNDEFINED;

  if (val.v == 0) return JSVALUE_UNDEFINED;
  if (val.v == 0xFFFFFFFFFFFFFFFFULL) return JSVALUE_NULL;

  Local<Value> v = c->retrieve(val);

  if (v->IsUndefined()) return JSVALUE_UNDEFINED;
  if (v->IsNull()) return JSVALUE_NULL;
  if (v->IsBoolean()) return JSVALUE_BOOLEAN;
  if (v->IsNumber()) return JSVALUE_NUMBER;
  if (v->IsString()) return JSVALUE_STRING;
  if (v->IsArray()) return JSVALUE_ARRAY;
  if (v->IsFunction()) return JSVALUE_FUNCTION;
  if (v->IsDate()) return JSVALUE_DATE;

  // Check for TypedArrays
  if (v->IsObject()) {
    Local<Object> obj = v.As<Object>();
    if (obj->HasIndexedPropertiesInExternalArrayData()) {
      ExternalArrayType type = obj->GetIndexedPropertiesExternalArrayDataType();
      switch (type) {
        case kExternalByteArray: return JSVALUE_INT8_ARRAY;
        case kExternalUnsignedByteArray: return JSVALUE_UINT8_ARRAY;
        case kExternalShortArray: return JSVALUE_INT16_ARRAY;
        case kExternalUnsignedShortArray: return JSVALUE_UINT16_ARRAY;
        case kExternalIntArray: return JSVALUE_INT32_ARRAY;
        case kExternalUnsignedIntArray: return JSVALUE_UINT32_ARRAY;
        case kExternalFloatArray: return JSVALUE_FLOAT32_ARRAY;
        case kExternalDoubleArray: return JSVALUE_FLOAT64_ARRAY;
        default: break;
      }
    }
  }

  if (v->IsObject()) return JSVALUE_OBJECT;

  return JSVALUE_UNDEFINED;
}

// Memory management

void _jsvalue_addref(jscontext ctx, jsvalue val) {
  jsbridge::Context* c = getCtx(ctx);
  if (c) c->addRef(val);
}

void _jsvalue_release(jscontext ctx, jsvalue val) {
  jsbridge::Context* c = getCtx(ctx);
  if (c) c->release(val);
}

// Object creation and manipulation

jsvalue _jsvalue_new(jscontext ctx, const char* fmt, ...) {
  jsbridge::Context* c = getCtx(ctx);
  if (!c || !fmt) {
    jsvalue r = {0};
    return r;
  }

  va_list args;
  va_start(args, fmt);

  jsvalue result = {0};

  if (fmt[0] == '{') {
    // Create empty object
    result = c->store(Nan::New<Object>());
  }
  else if (fmt[0] == '[') {
    // Create empty array
    result = c->store(Nan::New<Array>());
  }
  else if (fmt[0] == 's') {
    // Create string
    const char* str = va_arg(args, const char*);
    if (str) {
      result = c->store(Nan::New<String>(str).ToLocalChecked());
    }
  }
  else if (fmt[0] == 'i') {
    // Create integer
    int32 val = va_arg(args, int32);
    result = c->store(Nan::New<Integer>(val));
  }
  else if (fmt[0] == 'd') {
    // Create double
    double val = va_arg(args, double);
    result = c->store(Nan::New<Number>(val));
  }
  else if (fmt[0] == 'b') {
    // Create boolean
    int val = va_arg(args, int);
    result = c->store(Nan::New<Boolean>(val != 0));
  }
  else if (fmt[0] == 'n') {
    // Create null
    result.v = 0xFFFFFFFFFFFFFFFFULL;
  }
  else if (fmt[0] == 'u') {
    // Create undefined
    result.v = 0;
  }

  va_end(args);
  return result;
}

int _jsvalue_fetch(jscontext ctx, jsvalue val, const char* fmt, ...) {
  jsbridge::Context* c = getCtx(ctx);
  if (!c || !fmt) return -1;

  Local<Value> v = c->retrieve(val);
  if (!v->IsObject()) return -1;

  Local<Object> obj = v.As<Object>();

  va_list args;
  va_start(args, fmt);

  int result = 0;

  // Parse format: "type:property" or ":method(args)"
  const char* colon = strchr(fmt, ':');
  if (!colon) {
    va_end(args);
    return -1;
  }

  char type = fmt[0];
  const char* name = colon + 1;

  // Check if it's a method call
  const char* paren = strchr(name, '(');
  bool isMethod = (paren != NULL);

  if (isMethod) {
    // Method call - not implemented in basic version
    result = -1;
  }
  else if (name[0] == '[') {
    // Array index access: v:[i]
    int index = va_arg(args, int);
    Local<Value> elem = obj->Get(index);

    if (type == 'v') {
      jsvalue* out = va_arg(args, jsvalue*);
      if (out) {
        *out = c->store(elem);
      }
    }
    else if (type == 'i') {
      result = Nan::To<int32_t>(elem).FromMaybe(0);
    }
    else if (type == 'd') {
      // Return as int, caller needs to handle differently for double
      double d = Nan::To<double>(elem).FromMaybe(0.0);
      result = static_cast<int>(d);
    }
  }
  else {
    // Property access
    Local<String> propName = Nan::New<String>(name).ToLocalChecked();
    Local<Value> propVal = obj->Get(propName);

    if (type == 'i') {
      result = Nan::To<int32_t>(propVal).FromMaybe(0);
    }
    else if (type == 'd') {
      double d = Nan::To<double>(propVal).FromMaybe(0.0);
      result = static_cast<int>(d);
    }
    else if (type == 's') {
      char** out = va_arg(args, char**);
      if (out && propVal->IsString()) {
        Nan::Utf8String str(propVal);
        size_t len = str.length();
        *out = static_cast<char*>(malloc(len + 1));
        if (*out) {
          memcpy(*out, *str, len);
          (*out)[len] = '\0';
        }
      }
    }
    else if (type == 'v') {
      jsvalue* out = va_arg(args, jsvalue*);
      if (out) {
        *out = c->store(propVal);
      }
    }
  }

  va_end(args);
  return result;
}

} // extern "C"
