#include "jsbridge_impl.h"
#include <cstdarg>
#include <cstring>
#include <cstdlib>

#ifndef USE_NAPI
using namespace v8;
#endif

// Thread-local storage for current context
#if defined(_MSC_VER)
  __declspec(thread) jsbridge::Context* tls_current_context = NULL;
#else
  __thread jsbridge::Context* tls_current_context = NULL;
#endif

// Global context pointer (exported for C code)
jscontext __jscontext = NULL;

// ============================================
// Special jsvalue markers
// ============================================
static const uint64_t JSVALUE_HANDLE_UNDEFINED = 0;
static const uint64_t JSVALUE_HANDLE_NULL = 0xFFFFFFFFFFFFFFFFULL;

namespace jsbridge {

Context::Context()
  : nextHandle_(1)  // Start at 1, 0 is reserved for undefined
{
}

Context::~Context() {
  clear();
}

jsvalue Context::store(ADDON_VALUE value) {
  jsvalue result = { 0 };

#ifdef USE_NAPI
  napi_env env = addon_detail::tls_env();

  // Handle undefined/null specially — no reference needed
  napi_valuetype type;
  napi_typeof(env, value, &type);

  if (type == napi_undefined) {
    return result;
  }
  if (type == napi_null) {
    result.v = JSVALUE_HANDLE_NULL;
    return result;
  }

  // Create a strong reference to prevent GC
  StoredValue sv;
  napi_create_reference(env, value, 1, &sv.ref);
  sv.refCount = 1;
#else
  // Handle undefined/null specially
  if (value.IsEmpty() || value->IsUndefined()) {
    return result;
  }
  if (value->IsNull()) {
    result.v = JSVALUE_HANDLE_NULL;
    return result;
  }

  // Store via heap-allocated Persistent to avoid copy issues
  StoredValue sv;
  sv.persistent = new Persistent<Value>(v8::Isolate::GetCurrent(), value);
  sv.refCount = 1;
#endif

  uint64_t handle = nextHandle_++;
  values_[handle] = sv;

  result.v = handle;
  return result;
}

ADDON_VALUE Context::retrieve(jsvalue val) {
  if (val.v == JSVALUE_HANDLE_UNDEFINED) {
#ifdef USE_NAPI
    return addon_detail::env().Undefined();
#else
    return Nan::Undefined();
#endif
  }

  if (val.v == JSVALUE_HANDLE_NULL) {
#ifdef USE_NAPI
    return addon_detail::env().Null();
#else
    return Nan::Null();
#endif
  }

  std::map<uint64_t, StoredValue>::iterator it = values_.find(val.v);

#ifdef USE_NAPI
  if (it == values_.end() || it->second.ref == NULL) {
    return addon_detail::env().Undefined();
  }

  napi_value result;
  napi_get_reference_value(addon_detail::tls_env(), it->second.ref, &result);
  return Napi::Value(addon_detail::env(), result);
#else
  if (it == values_.end() || it->second.persistent == NULL) {
    return Nan::Undefined();
  }

  return Nan::New(*(it->second.persistent));
#endif
}

void Context::addRef(jsvalue val) {
  if (val.v == JSVALUE_HANDLE_UNDEFINED || val.v == JSVALUE_HANDLE_NULL) {
    return;
  }

  std::map<uint64_t, StoredValue>::iterator it = values_.find(val.v);
  if (it != values_.end()) {
    it->second.refCount++;
  }
}

void Context::release(jsvalue val) {
  if (val.v == JSVALUE_HANDLE_UNDEFINED || val.v == JSVALUE_HANDLE_NULL) {
    return;
  }

  std::map<uint64_t, StoredValue>::iterator it = values_.find(val.v);
  if (it != values_.end()) {
    it->second.refCount--;
    if (it->second.refCount <= 0) {
#ifdef USE_NAPI
      if (it->second.ref != NULL) {
        napi_delete_reference(addon_detail::tls_env(), it->second.ref);
        it->second.ref = NULL;
      }
#else
      if (it->second.persistent != NULL) {
        it->second.persistent->Reset();
        delete it->second.persistent;
        it->second.persistent = NULL;
      }
#endif
      values_.erase(it);
    }
  }
}

void Context::clear() {
  std::map<uint64_t, StoredValue>::iterator it;
  for (it = values_.begin(); it != values_.end(); ++it) {
#ifdef USE_NAPI
    if (it->second.ref != NULL) {
      napi_delete_reference(addon_detail::tls_env(), it->second.ref);
      it->second.ref = NULL;
    }
#else
    if (it->second.persistent != NULL) {
      it->second.persistent->Reset();
      delete it->second.persistent;
      it->second.persistent = NULL;
    }
#endif
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

// ─── C -> JS Conversions ─────────────────────────────────────────────────────
// These use ADDON_* value creation macros which are backend-neutral.

jsvalue bool_to_jsvalue(jscontext ctx, bool val) {
  jsbridge::Context* c = getCtx(ctx);
  if (!c) { jsvalue r = {0}; return r; }
  return c->store(ADDON_BOOL(val));
}

jsvalue int8_to_jsvalue(jscontext ctx, int8 val) {
  jsbridge::Context* c = getCtx(ctx);
  if (!c) { jsvalue r = {0}; return r; }
  return c->store(ADDON_INT(val));
}

jsvalue uint8_to_jsvalue(jscontext ctx, uint8 val) {
  jsbridge::Context* c = getCtx(ctx);
  if (!c) { jsvalue r = {0}; return r; }
  return c->store(ADDON_INT(val));
}

jsvalue int16_to_jsvalue(jscontext ctx, int16 val) {
  jsbridge::Context* c = getCtx(ctx);
  if (!c) { jsvalue r = {0}; return r; }
  return c->store(ADDON_INT(val));
}

jsvalue uint16_to_jsvalue(jscontext ctx, uint16 val) {
  jsbridge::Context* c = getCtx(ctx);
  if (!c) { jsvalue r = {0}; return r; }
  return c->store(ADDON_INT(val));
}

jsvalue int32_to_jsvalue(jscontext ctx, int32 val) {
  jsbridge::Context* c = getCtx(ctx);
  if (!c) { jsvalue r = {0}; return r; }
  return c->store(ADDON_INT(val));
}

jsvalue uint32_to_jsvalue(jscontext ctx, uint32 val) {
  jsbridge::Context* c = getCtx(ctx);
  if (!c) { jsvalue r = {0}; return r; }
  return c->store(ADDON_UINT(val));
}

jsvalue int64_to_jsvalue(jscontext ctx, int64 val) {
  jsbridge::Context* c = getCtx(ctx);
  if (!c) { jsvalue r = {0}; return r; }
  // No BigInt in V8 3.x / early N-API — use Number (loses precision for large values)
  return c->store(ADDON_NUMBER(static_cast<double>(val)));
}

jsvalue uint64_to_jsvalue(jscontext ctx, uint64 val) {
  jsbridge::Context* c = getCtx(ctx);
  if (!c) { jsvalue r = {0}; return r; }
  return c->store(ADDON_NUMBER(static_cast<double>(val)));
}

jsvalue float_to_jsvalue(jscontext ctx, float val) {
  jsbridge::Context* c = getCtx(ctx);
  if (!c) { jsvalue r = {0}; return r; }
  return c->store(ADDON_NUMBER(val));
}

jsvalue double_to_jsvalue(jscontext ctx, double val) {
  jsbridge::Context* c = getCtx(ctx);
  if (!c) { jsvalue r = {0}; return r; }
  return c->store(ADDON_NUMBER(val));
}

// ─── JS -> C Conversions ─────────────────────────────────────────────────────
// These use ADDON_TO_* macros for type coercion.

bool jsvalue_to_bool(jscontext ctx, jsvalue val) {
  jsbridge::Context* c = getCtx(ctx);
  if (!c) return false;
  ADDON_VALUE v = c->retrieve(val);
  return ADDON_TO_BOOL(v);
}

int8 jsvalue_to_int8(jscontext ctx, jsvalue val) {
  jsbridge::Context* c = getCtx(ctx);
  if (!c) return 0;
  ADDON_VALUE v = c->retrieve(val);
  return static_cast<int8>(ADDON_TO_INT32_DEFAULT(v, 0));
}

uint8 jsvalue_to_uint8(jscontext ctx, jsvalue val) {
  jsbridge::Context* c = getCtx(ctx);
  if (!c) return 0;
  ADDON_VALUE v = c->retrieve(val);
  return static_cast<uint8>(ADDON_TO_UINT32_DEFAULT(v, 0));
}

int16 jsvalue_to_int16(jscontext ctx, jsvalue val) {
  jsbridge::Context* c = getCtx(ctx);
  if (!c) return 0;
  ADDON_VALUE v = c->retrieve(val);
  return static_cast<int16>(ADDON_TO_INT32_DEFAULT(v, 0));
}

uint16 jsvalue_to_uint16(jscontext ctx, jsvalue val) {
  jsbridge::Context* c = getCtx(ctx);
  if (!c) return 0;
  ADDON_VALUE v = c->retrieve(val);
  return static_cast<uint16>(ADDON_TO_UINT32_DEFAULT(v, 0));
}

int32 jsvalue_to_int32(jscontext ctx, jsvalue val) {
  jsbridge::Context* c = getCtx(ctx);
  if (!c) return 0;
  ADDON_VALUE v = c->retrieve(val);
  return ADDON_TO_INT32_DEFAULT(v, 0);
}

uint32 jsvalue_to_uint32(jscontext ctx, jsvalue val) {
  jsbridge::Context* c = getCtx(ctx);
  if (!c) return 0;
  ADDON_VALUE v = c->retrieve(val);
  return ADDON_TO_UINT32_DEFAULT(v, 0);
}

int64 jsvalue_to_int64(jscontext ctx, jsvalue val) {
  jsbridge::Context* c = getCtx(ctx);
  if (!c) return 0;
  ADDON_VALUE v = c->retrieve(val);
  return static_cast<int64>(ADDON_TO_DOUBLE_DEFAULT(v, 0.0));
}

uint64 jsvalue_to_uint64(jscontext ctx, jsvalue val) {
  jsbridge::Context* c = getCtx(ctx);
  if (!c) return 0;
  ADDON_VALUE v = c->retrieve(val);
  return static_cast<uint64>(ADDON_TO_DOUBLE_DEFAULT(v, 0.0));
}

float jsvalue_to_float(jscontext ctx, jsvalue val) {
  jsbridge::Context* c = getCtx(ctx);
  if (!c) return 0.0f;
  ADDON_VALUE v = c->retrieve(val);
  return static_cast<float>(ADDON_TO_DOUBLE_DEFAULT(v, 0.0));
}

double jsvalue_to_double(jscontext ctx, jsvalue val) {
  jsbridge::Context* c = getCtx(ctx);
  if (!c) return 0.0;
  ADDON_VALUE v = c->retrieve(val);
  return ADDON_TO_DOUBLE_DEFAULT(v, 0.0);
}

jsvalue jsvalue_to_jsvalue(jscontext ctx, jsvalue val) {
  return val;  // Identity
}

// ─── TypedArray Pointer Access ───────────────────────────────────────────────
// Uses ADDON_IS_TYPEDARRAY / ADDON_GET_TYPEDARRAY_DATA for backend-neutral
// zero-copy access. NAN backend uses V8 3.x external array data API;
// N-API backend uses napi_get_typedarray_info.

int8* jsvalue_to_int8_ptr(jscontext ctx, jsvalue val) {
  jsbridge::Context* c = getCtx(ctx);
  if (!c) return NULL;
  ADDON_VALUE v = c->retrieve(val);
  if (!ADDON_IS_TYPEDARRAY(v)) return NULL;
  return static_cast<int8*>(ADDON_GET_TYPEDARRAY_DATA(v));
}

uint8* jsvalue_to_uint8_ptr(jscontext ctx, jsvalue val) {
  jsbridge::Context* c = getCtx(ctx);
  if (!c) return NULL;
  ADDON_VALUE v = c->retrieve(val);
  if (!ADDON_IS_TYPEDARRAY(v)) return NULL;
  return static_cast<uint8*>(ADDON_GET_TYPEDARRAY_DATA(v));
}

int16* jsvalue_to_int16_ptr(jscontext ctx, jsvalue val) {
  jsbridge::Context* c = getCtx(ctx);
  if (!c) return NULL;
  ADDON_VALUE v = c->retrieve(val);
  if (!ADDON_IS_TYPEDARRAY(v)) return NULL;
  return static_cast<int16*>(ADDON_GET_TYPEDARRAY_DATA(v));
}

uint16* jsvalue_to_uint16_ptr(jscontext ctx, jsvalue val) {
  jsbridge::Context* c = getCtx(ctx);
  if (!c) return NULL;
  ADDON_VALUE v = c->retrieve(val);
  if (!ADDON_IS_TYPEDARRAY(v)) return NULL;
  return static_cast<uint16*>(ADDON_GET_TYPEDARRAY_DATA(v));
}

int32* jsvalue_to_int32_ptr(jscontext ctx, jsvalue val) {
  jsbridge::Context* c = getCtx(ctx);
  if (!c) return NULL;
  ADDON_VALUE v = c->retrieve(val);
  if (!ADDON_IS_TYPEDARRAY(v)) return NULL;
  return static_cast<int32*>(ADDON_GET_TYPEDARRAY_DATA(v));
}

uint32* jsvalue_to_uint32_ptr(jscontext ctx, jsvalue val) {
  jsbridge::Context* c = getCtx(ctx);
  if (!c) return NULL;
  ADDON_VALUE v = c->retrieve(val);
  if (!ADDON_IS_TYPEDARRAY(v)) return NULL;
  return static_cast<uint32*>(ADDON_GET_TYPEDARRAY_DATA(v));
}

int64* jsvalue_to_int64_ptr(jscontext ctx, jsvalue val) {
#ifdef USE_NAPI
  // BigInt64Array is available in modern N-API
  jsbridge::Context* c = getCtx(ctx);
  if (!c) return NULL;
  ADDON_VALUE v = c->retrieve(val);
  if (!ADDON_IS_TYPEDARRAY(v)) return NULL;
  return static_cast<int64*>(ADDON_GET_TYPEDARRAY_DATA(v));
#else
  // BigInt64Array not available in V8 3.x
  (void)ctx; (void)val;
  return NULL;
#endif
}

uint64* jsvalue_to_uint64_ptr(jscontext ctx, jsvalue val) {
#ifdef USE_NAPI
  jsbridge::Context* c = getCtx(ctx);
  if (!c) return NULL;
  ADDON_VALUE v = c->retrieve(val);
  if (!ADDON_IS_TYPEDARRAY(v)) return NULL;
  return static_cast<uint64*>(ADDON_GET_TYPEDARRAY_DATA(v));
#else
  (void)ctx; (void)val;
  return NULL;
#endif
}

float* jsvalue_to_float_ptr(jscontext ctx, jsvalue val) {
  jsbridge::Context* c = getCtx(ctx);
  if (!c) return NULL;
  ADDON_VALUE v = c->retrieve(val);
  if (!ADDON_IS_TYPEDARRAY(v)) return NULL;
  return static_cast<float*>(ADDON_GET_TYPEDARRAY_DATA(v));
}

double* jsvalue_to_double_ptr(jscontext ctx, jsvalue val) {
  jsbridge::Context* c = getCtx(ctx);
  if (!c) return NULL;
  ADDON_VALUE v = c->retrieve(val);
  if (!ADDON_IS_TYPEDARRAY(v)) return NULL;
  return static_cast<double*>(ADDON_GET_TYPEDARRAY_DATA(v));
}

// ─── Type Checking ───────────────────────────────────────────────────────────

int _jsvalue_type(jscontext ctx, jsvalue val) {
  jsbridge::Context* c = getCtx(ctx);
  if (!c) return JSVALUE_UNDEFINED;

  if (val.v == JSVALUE_HANDLE_UNDEFINED) return JSVALUE_UNDEFINED;
  if (val.v == JSVALUE_HANDLE_NULL) return JSVALUE_NULL;

  ADDON_VALUE v = c->retrieve(val);

  if (ADDON_IS_UNDEFINED(v)) return JSVALUE_UNDEFINED;
  if (ADDON_IS_NULL(v)) return JSVALUE_NULL;
  if (ADDON_IS_BOOLEAN(v)) return JSVALUE_BOOLEAN;
  if (ADDON_IS_NUMBER(v)) return JSVALUE_NUMBER;
  if (ADDON_IS_STRING(v)) return JSVALUE_STRING;
  if (ADDON_IS_ARRAY(v)) return JSVALUE_ARRAY;
  if (ADDON_IS_FUNCTION(v)) return JSVALUE_FUNCTION;
  if (ADDON_IS_DATE(v)) return JSVALUE_DATE;

  // TypedArray subtype detection — APIs differ between backends
#ifdef USE_NAPI
  if (v.IsTypedArray()) {
    napi_typedarray_type taType;
    napi_get_typedarray_info(addon_detail::tls_env(), v, &taType, NULL, NULL, NULL, NULL);
    switch (taType) {
      case napi_int8_array:       return JSVALUE_INT8_ARRAY;
      case napi_uint8_array:      return JSVALUE_UINT8_ARRAY;
      case napi_int16_array:      return JSVALUE_INT16_ARRAY;
      case napi_uint16_array:     return JSVALUE_UINT16_ARRAY;
      case napi_int32_array:      return JSVALUE_INT32_ARRAY;
      case napi_uint32_array:     return JSVALUE_UINT32_ARRAY;
      case napi_float32_array:    return JSVALUE_FLOAT32_ARRAY;
      case napi_float64_array:    return JSVALUE_FLOAT64_ARRAY;
      case napi_bigint64_array:   return JSVALUE_INT64_ARRAY;
      case napi_biguint64_array:  return JSVALUE_UINT64_ARRAY;
      default: break;
    }
  }
#else
  // V8 3.x TypedArray API (ExternalArrayType)
  if (v->IsObject()) {
    Local<Object> obj = v.As<Object>();
    if (obj->HasIndexedPropertiesInExternalArrayData()) {
      ExternalArrayType type = obj->GetIndexedPropertiesExternalArrayDataType();
      switch (type) {
        case kExternalByteArray:          return JSVALUE_INT8_ARRAY;
        case kExternalUnsignedByteArray:  return JSVALUE_UINT8_ARRAY;
        case kExternalShortArray:         return JSVALUE_INT16_ARRAY;
        case kExternalUnsignedShortArray: return JSVALUE_UINT16_ARRAY;
        case kExternalIntArray:           return JSVALUE_INT32_ARRAY;
        case kExternalUnsignedIntArray:   return JSVALUE_UINT32_ARRAY;
        case kExternalFloatArray:         return JSVALUE_FLOAT32_ARRAY;
        case kExternalDoubleArray:        return JSVALUE_FLOAT64_ARRAY;
        default: break;
      }
    }
  }
#endif

  if (ADDON_IS_OBJECT(v)) return JSVALUE_OBJECT;

  return JSVALUE_UNDEFINED;
}

// ─── Memory Management ───────────────────────────────────────────────────────

void _jsvalue_addref(jscontext ctx, jsvalue val) {
  jsbridge::Context* c = getCtx(ctx);
  if (c) c->addRef(val);
}

void _jsvalue_release(jscontext ctx, jsvalue val) {
  jsbridge::Context* c = getCtx(ctx);
  if (c) c->release(val);
}

// ─── Object Creation and Manipulation ────────────────────────────────────────

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
    result = c->store(ADDON_OBJECT());
  }
  else if (fmt[0] == '[') {
    result = c->store(ADDON_ARRAY_EMPTY());
  }
  else if (fmt[0] == 's') {
    const char* str = va_arg(args, const char*);
    if (str) {
      result = c->store(ADDON_STRING(str));
    }
  }
  else if (fmt[0] == 'i') {
    int32 val = va_arg(args, int32);
    result = c->store(ADDON_INT(val));
  }
  else if (fmt[0] == 'd') {
    double val = va_arg(args, double);
    result = c->store(ADDON_NUMBER(val));
  }
  else if (fmt[0] == 'b') {
    int val = va_arg(args, int);
    result = c->store(ADDON_BOOL(val != 0));
  }
  else if (fmt[0] == 'n') {
    result.v = JSVALUE_HANDLE_NULL;
  }
  else if (fmt[0] == 'u') {
    result.v = JSVALUE_HANDLE_UNDEFINED;
  }

  va_end(args);
  return result;
}

int _jsvalue_fetch(jscontext ctx, jsvalue val, const char* fmt, ...) {
  jsbridge::Context* c = getCtx(ctx);
  if (!c || !fmt) return -1;

  ADDON_VALUE v = c->retrieve(val);
  if (!ADDON_IS_OBJECT(v)) return -1;

  ADDON_OBJECT_TYPE obj = ADDON_AS_OBJECT(v);

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
    // Method call — not implemented in basic version
    result = -1;
  }
  else if (name[0] == '[') {
    // Array index access: v:[i]
    int index = va_arg(args, int);
    ADDON_VALUE elem = ADDON_GET_INDEX(obj, index);

    if (type == 'v') {
      jsvalue* out = va_arg(args, jsvalue*);
      if (out) {
        *out = c->store(elem);
      }
    }
    else if (type == 'i') {
      result = ADDON_TO_INT32_DEFAULT(elem, 0);
    }
    else if (type == 'd') {
      double d = ADDON_TO_DOUBLE_DEFAULT(elem, 0.0);
      result = static_cast<int>(d);
    }
  }
  else {
    // Property access by name
    ADDON_VALUE propVal = ADDON_GET(obj, name);

    if (type == 'i') {
      result = ADDON_TO_INT32_DEFAULT(propVal, 0);
    }
    else if (type == 'd') {
      double d = ADDON_TO_DOUBLE_DEFAULT(propVal, 0.0);
      result = static_cast<int>(d);
    }
    else if (type == 's') {
      char** out = va_arg(args, char**);
      if (out && ADDON_IS_STRING(propVal)) {
        ADDON_UTF8(str, propVal);
        size_t len = ADDON_UTF8_LENGTH(str);
        *out = static_cast<char*>(malloc(len + 1));
        if (*out) {
          memcpy(*out, ADDON_UTF8_VALUE(str), len);
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
