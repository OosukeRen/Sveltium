/*
 * addon_api_napi.h — N-API backend for addon_api.h
 *
 * Maps ADDON_* macros to node-addon-api / N-API calls.
 * Used for modern NW.js builds (0.35+).
 *
 * Methods use raw napi_callback signatures. ADDON_ENV constructs
 * a Napi::CallbackInfo from the raw parameters and stores the env
 * in thread-local storage for helper functions that need to create
 * JS values outside of a direct method scope.
 */

#pragma once

#include <napi.h>
#include <string>
#include <vector>
#include <cstdint>
#include <climits>
#include <cmath>

// ─── Internal helpers ──────────────────────────────────────────────────────

namespace addon_detail {

// Thread-local env — enables value creation macros outside method scope.
// Set once at method entry (via ADDON_ENV) or module init, stays valid
// for the entire synchronous call chain.
inline napi_env& tls_env() {
  static thread_local napi_env e = nullptr;
  return e;
}
inline Napi::Env env() { return Napi::Env(tls_env()); }

// UTF-8 string wrapper — matches Nan::Utf8String interface
struct Utf8String {
  std::string str_;
  Utf8String(Napi::Value val) : str_(val.As<Napi::String>().Utf8Value()) {}
  const char* operator*() const { return str_.c_str(); }
  size_t length() const { return str_.length(); }
};

// N-API has no direct IsInt32 — check if the number is an exact int32
inline bool is_int32(Napi::Value v) {
  if (!v.IsNumber()) return false;
  double d = v.As<Napi::Number>().DoubleValue();
  int32_t i = static_cast<int32_t>(d);
  return d == static_cast<double>(i) && d >= INT32_MIN && d <= INT32_MAX;
}

// No-op constructor for factory-only classes (no user-facing JS constructor)
inline napi_value noop_ctor(napi_env env, napi_callback_info info) {
  napi_value this_arg;
  napi_get_cb_info(env, info, nullptr, nullptr, &this_arg, nullptr);
  return this_arg;
}

// TypedArray data pointer extraction via raw N-API
inline void* get_typedarray_data(Napi::Value val) {
  void* data = nullptr;
  napi_get_typedarray_info(tls_env(), val, nullptr, nullptr, &data, nullptr, nullptr);
  return data;
}

// ─── Constructor template builder ──────────────────────────────────────────
// Accumulates property descriptors, then builds the class via napi_define_class.
// Mirrors the NAN pattern: create template → add methods/accessors → GetFunction.

struct AddonCtorTemplate {
  std::string class_name;
  napi_callback ctor_fn;
  std::vector<napi_property_descriptor> descriptors;
  napi_value cached_fn; // set on first finalization

  AddonCtorTemplate() : ctor_fn(noop_ctor), cached_fn(nullptr) {}
  explicit AddonCtorTemplate(napi_callback fn) : ctor_fn(fn), cached_fn(nullptr) {}
};

inline void add_method(AddonCtorTemplate& t, const char* name, napi_callback fn) {
  napi_property_descriptor desc = {};
  desc.utf8name = name;
  desc.method = fn;
  desc.attributes = napi_default;
  t.descriptors.push_back(desc);
}

inline void add_accessor(AddonCtorTemplate& t, const char* name, napi_callback getter) {
  napi_property_descriptor desc = {};
  desc.utf8name = name;
  desc.getter = getter;
  desc.attributes = napi_default;
  t.descriptors.push_back(desc);
}

// Finalize template into a constructor function (cached on first call)
inline Napi::Function finalize_class(AddonCtorTemplate& t) {
  if (!t.cached_fn) {
    napi_define_class(tls_env(), t.class_name.c_str(), NAPI_AUTO_LENGTH,
      t.ctor_fn, nullptr, t.descriptors.size(), t.descriptors.data(), &t.cached_fn);
  }
  return Napi::Function(env(), t.cached_fn);
}

// Create a new instance from a constructor function
inline Napi::Object new_instance(Napi::Function cons) {
  napi_value result;
  napi_new_instance(tls_env(), cons, 0, nullptr, &result);
  return Napi::Object(env(), result);
}

// Create instance from a template (finalize + instantiate in one step)
inline Napi::Object template_new_instance(AddonCtorTemplate& tpl) {
  return new_instance(finalize_class(tpl));
}

// ─── Persistent handle overloads ───────────────────────────────────────────

inline void persistent_reset(Napi::FunctionReference& ref, Napi::Function fn) {
  ref = Napi::Persistent(fn);
}

inline void persistent_reset(Napi::FunctionReference& ref, AddonCtorTemplate& tpl) {
  ref = Napi::Persistent(finalize_class(tpl));
}

// ─── ObjectWrap base class ─────────────────────────────────────────────────
// Uses low-level napi_wrap/napi_unwrap to avoid CRTP (Napi::ObjectWrap<T>
// requires the class itself as template parameter, which doesn't fit the
// NAN-style macro pattern where classes inherit from a common base).

class WrapBase {
public:
  virtual ~WrapBase() {}

  void Wrap(Napi::Object obj) {
    napi_wrap(tls_env(), obj, this, destructor_cb, nullptr, nullptr);
  }

  template<typename T>
  static T* Unwrap(Napi::Object obj) {
    void* result = nullptr;
    napi_unwrap(tls_env(), obj, &result);
    return static_cast<T*>(result);
  }

private:
  static void destructor_cb(napi_env, void* data, void*) {
    delete static_cast<WrapBase*>(data);
  }
};

} // namespace addon_detail

// ─── Type aliases ───────────────────────────────────────────────────────────

#define ADDON_VALUE         Napi::Value
#define ADDON_OBJECT_TYPE   Napi::Object
#define ADDON_ARRAY_TYPE    Napi::Array
#define ADDON_STRING_TYPE   Napi::String

// ─── Method signatures & module registration ────────────────────────────────

// Methods and getters use raw napi_callback signature.
// ADDON_ENV (placed at method start) constructs info from the raw params.
// Note: no 'static' — class declarations add it: "static ADDON_METHOD(name);"
#define ADDON_METHOD(name) \
  napi_value name(napi_env raw_env, napi_callback_info raw_info)

#define ADDON_GETTER(name) \
  napi_value name(napi_env raw_env, napi_callback_info raw_info)

// Module init wrapper: adapts void body to Napi::Object return signature.
// Body function uses 'target' to match NAN convention.
#define ADDON_MODULE_INIT(name) \
  static void name##_body(Napi::Object target); \
  Napi::Object name(Napi::Env env, Napi::Object exports) { \
    addon_detail::tls_env() = static_cast<napi_env>(env); \
    name##_body(exports); \
    return exports; \
  } \
  void name##_body(Napi::Object target)

#define ADDON_MODULE(modname, init) NODE_API_MODULE(modname, init)

// Sub-init parameters: single object (env is in TLS, set by module init)
#define ADDON_INIT_PARAMS Napi::Object exports

// Call sub-init — 'target' comes from ADDON_MODULE_INIT body parameter
#define ADDON_CALL_SUB_INIT(fn) fn(target)

// Environment capture — constructs CallbackInfo and sets TLS env.
// Must be the first statement in every ADDON_METHOD / ADDON_GETTER body.
#define ADDON_ENV \
  addon_detail::tls_env() = raw_env; \
  Napi::CallbackInfo info(raw_env, raw_info)

// ─── Argument access ────────────────────────────────────────────────────────

#define ADDON_ARG(i)              info[i]
#define ADDON_ARG_COUNT()         static_cast<int>(info.Length())
#define ADDON_THIS()              info.This().As<Napi::Object>()
#define ADDON_HOLDER()            info.This().As<Napi::Object>()
#define ADDON_IS_CONSTRUCT_CALL() (!info.NewTarget().IsUndefined())

// ─── Return values ──────────────────────────────────────────────────────────

// N-API callbacks return napi_value. Returning nullptr means undefined.
#define ADDON_RETURN(val)         return static_cast<napi_value>(val)
#define ADDON_RETURN_NULL()       return static_cast<napi_value>(addon_detail::env().Null())
#define ADDON_RETURN_UNDEFINED()  return nullptr
#define ADDON_VOID_RETURN()       return nullptr

// ─── Value creation ─────────────────────────────────────────────────────────

#define ADDON_STRING(str)    Napi::String::New(addon_detail::env(), str)
#define ADDON_BOOL(val)      Napi::Boolean::New(addon_detail::env(), static_cast<bool>(val))
#define ADDON_INT(val)       Napi::Number::New(addon_detail::env(), static_cast<int32_t>(val))
#define ADDON_UINT(val)      Napi::Number::New(addon_detail::env(), static_cast<uint32_t>(val))
#define ADDON_NUMBER(val)    Napi::Number::New(addon_detail::env(), static_cast<double>(val))
#define ADDON_BOOLEAN(val)   Napi::Boolean::New(addon_detail::env(), static_cast<bool>(val))
#define ADDON_INTEGER(val)   Napi::Number::New(addon_detail::env(), static_cast<double>(val))
#define ADDON_OBJECT()       Napi::Object::New(addon_detail::env())
#define ADDON_ARRAY(len)     Napi::Array::New(addon_detail::env(), static_cast<size_t>(len))
#define ADDON_ARRAY_EMPTY()  Napi::Array::New(addon_detail::env())
#define ADDON_NULL()         addon_detail::env().Null()
#define ADDON_UNDEFINED()    addon_detail::env().Undefined()
#define ADDON_EXTERNAL(ptr)  Napi::External<void>::New(addon_detail::env(), static_cast<void*>(ptr))

// ─── Type checking ──────────────────────────────────────────────────────────

#define ADDON_IS_STRING(v)    (v).IsString()
#define ADDON_IS_NUMBER(v)    (v).IsNumber()
#define ADDON_IS_BOOLEAN(v)   (v).IsBoolean()
#define ADDON_IS_OBJECT(v)    (v).IsObject()
#define ADDON_IS_ARRAY(v)     (v).IsArray()
#define ADDON_IS_NULL(v)      (v).IsNull()
#define ADDON_IS_UNDEFINED(v) (v).IsUndefined()
#define ADDON_IS_INT32(v)     addon_detail::is_int32(v)
#define ADDON_IS_EXTERNAL(v)  (v).IsExternal()
#define ADDON_IS_FUNCTION(v)  (v).IsFunction()
#define ADDON_IS_DATE(v)      (v).IsObject()

// Direct boolean coercion (truthy/falsy)
#define ADDON_BOOL_VALUE(v)   (v).ToBoolean().Value()

// ─── Type conversion ────────────────────────────────────────────────────────

#define ADDON_TO_INT32(v)  (v).As<Napi::Number>().Int32Value()
#define ADDON_TO_UINT32(v) (v).As<Napi::Number>().Uint32Value()
#define ADDON_TO_DOUBLE(v) (v).As<Napi::Number>().DoubleValue()
#define ADDON_TO_INT(v)    (v).As<Napi::Number>().Int32Value()
#define ADDON_TO_BOOL(v)   (v).ToBoolean().Value()

#define ADDON_TO_INT32_DEFAULT(v, d)  ((v).IsNumber() ? (v).As<Napi::Number>().Int32Value() : (d))
#define ADDON_TO_UINT32_DEFAULT(v, d) ((v).IsNumber() ? (v).As<Napi::Number>().Uint32Value() : (d))
#define ADDON_TO_DOUBLE_DEFAULT(v, d) ((v).IsNumber() ? (v).As<Napi::Number>().DoubleValue() : (d))

#define ADDON_TO_OBJECT(v) (v).ToObject()

// Casting
#define ADDON_CAST_ARRAY(v)         (v).As<Napi::Array>()
#define ADDON_AS_ARRAY(v)           (v).As<Napi::Array>()
#define ADDON_AS_OBJECT(v)          (v).As<Napi::Object>()
#define ADDON_CAST_EXTERNAL_PTR(v)  (v).As<Napi::External<void>>().Data()

// ─── String handling ────────────────────────────────────────────────────────

#define ADDON_UTF8(name, val)    addon_detail::Utf8String name(val)
#define ADDON_UTF8_VALUE(name)   (*name)
#define ADDON_UTF8_LENGTH(name)  (name).length()

// ─── Object / Array operations ──────────────────────────────────────────────

#define ADDON_SET(obj, key, val)     (obj).Set(key, val)
#define ADDON_SET_INDEX(arr, i, val) (arr).Set(static_cast<uint32_t>(i), val)
#define ADDON_GET(obj, key)          (obj).Get(key)
#define ADDON_GET_INDEX(arr, i)      (arr).Get(static_cast<uint32_t>(i))
#define ADDON_HAS(obj, key)          (obj).Has(key)
#define ADDON_LENGTH(arr)            (arr).Length()

// ─── Error handling ─────────────────────────────────────────────────────────

#define ADDON_THROW_ERROR(msg) \
  Napi::Error::New(addon_detail::env(), msg).ThrowAsJavaScriptException()

#define ADDON_THROW_TYPE_ERROR(msg) \
  Napi::TypeError::New(addon_detail::env(), msg).ThrowAsJavaScriptException()

// ─── Scope management ───────────────────────────────────────────────────────

// N-API manages handle lifetimes internally via reference counting.
// HandleScope is kept for memory optimization hints.
// EscapableHandleScope and Escape are no-ops — values survive scope closure.
#define ADDON_HANDLE_SCOPE()    Napi::HandleScope scope(addon_detail::env())
#define ADDON_ESCAPABLE_SCOPE() /* no-op: N-API handles don't need escaping */
#define ADDON_ESCAPE(val)       (val)

// ─── Buffer ─────────────────────────────────────────────────────────────────

#define ADDON_BUFFER_IS(val)     (val).IsBuffer()
#define ADDON_BUFFER_DATA(val)   (val).As<Napi::Buffer<char>>().Data()
#define ADDON_BUFFER_LENGTH(val) (val).As<Napi::Buffer<char>>().Length()
#define ADDON_COPY_BUFFER(data, sz) \
  Napi::Buffer<char>::Copy(addon_detail::env(), data, static_cast<size_t>(sz))

// ─── Function export (flat addons) ──────────────────────────────────────────

// Creates a JS function from a napi_callback and sets it on the exports object
#define ADDON_EXPORT_FUNCTION(exports, name, fn) \
  do { \
    napi_value _fn_val; \
    napi_create_function(addon_detail::tls_env(), name, NAPI_AUTO_LENGTH, \
      fn, nullptr, &_fn_val); \
    (exports).Set(name, Napi::Value(addon_detail::env(), _fn_val)); \
  } while(0)

#define ADDON_SET_METHOD(obj, name, fn) \
  do { \
    napi_value _fn_val; \
    napi_create_function(addon_detail::tls_env(), name, NAPI_AUTO_LENGTH, \
      fn, nullptr, &_fn_val); \
    (obj).Set(name, Napi::Value(addon_detail::env(), _fn_val)); \
  } while(0)

// ─── ObjectWrap ─────────────────────────────────────────────────────────────

#define ADDON_OBJECT_WRAP                      addon_detail::WrapBase
#define ADDON_UNWRAP(Class, holder)            addon_detail::WrapBase::Unwrap<Class>(holder)
#define ADDON_WRAP(instance)                   this->Wrap(instance)

// Constructor template — builds class incrementally, finalizes on GetFunction
#define ADDON_NEW_CTOR_TEMPLATE()              addon_detail::AddonCtorTemplate()
#define ADDON_NEW_CTOR_TEMPLATE_WITH(fn)       addon_detail::AddonCtorTemplate(fn)
#define ADDON_SET_CLASS_NAME(tpl, name)        (tpl).class_name = (name)
#define ADDON_SET_INTERNAL_FIELD_COUNT(tpl, n) /* no-op: napi_wrap handles this */
#define ADDON_SET_PROTOTYPE_METHOD(tpl, name, fn) addon_detail::add_method((tpl), name, fn)
#define ADDON_SET_ACCESSOR(tpl, name, getter)  addon_detail::add_accessor((tpl), name, getter)

#define ADDON_GET_CTOR_FUNCTION(tpl)           addon_detail::finalize_class(tpl)
#define ADDON_NEW_INSTANCE(cons)               addon_detail::new_instance(cons)
#define ADDON_TEMPLATE_NEW_INSTANCE(tpl)       addon_detail::template_new_instance(tpl)

// Persistent handles — both function and template map to FunctionReference
#define ADDON_PERSISTENT_FUNCTION              Napi::FunctionReference
#define ADDON_PERSISTENT_TEMPLATE              Napi::FunctionReference
#define ADDON_PERSISTENT_RESET(p, val)         addon_detail::persistent_reset(p, val)
#define ADDON_PERSISTENT_GET(p)                (p).Value()

// ─── TypedArray ─────────────────────────────────────────────────────────────

#define ADDON_IS_TYPEDARRAY(obj)          (obj).IsTypedArray()
#define ADDON_GET_TYPEDARRAY_DATA(obj)    addon_detail::get_typedarray_data(obj)
