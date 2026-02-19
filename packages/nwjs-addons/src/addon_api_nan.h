/*
 * addon_api_nan.h — NAN backend for addon_api.h
 *
 * Maps ADDON_* macros to NAN 2.5.1 + V8 API calls.
 * Used for legacy NW.js builds (0.12.3, 0.15.4).
 */

#pragma once

#include <nan.h>

// ─── Type aliases ───────────────────────────────────────────────────────────

#define ADDON_VALUE           v8::Local<v8::Value>
#define ADDON_OBJECT_TYPE     v8::Local<v8::Object>
#define ADDON_ARRAY_TYPE      v8::Local<v8::Array>
#define ADDON_STRING_TYPE     v8::Local<v8::String>

// ─── Method signatures & module registration ────────────────────────────────

#define ADDON_METHOD(name)          NAN_METHOD(name)
#define ADDON_GETTER(name)          NAN_GETTER(name)
#define ADDON_MODULE_INIT(name)     NAN_MODULE_INIT(name)
#define ADDON_MODULE(modname, init) NODE_MODULE(modname, init)

// Sub-init function parameters (for individual addon Init functions)
#define ADDON_INIT_PARAMS           v8::Local<v8::Object> exports

// Call a sub-init from the main module init
// NAN passes only target; N-API will also pass env
#define ADDON_CALL_SUB_INIT(fn)     fn(target)

// Environment capture — no-op for NAN (V8 isolate is implicit)
#define ADDON_ENV

// ─── Argument access ────────────────────────────────────────────────────────

#define ADDON_ARG(i)                info[i]
#define ADDON_ARG_COUNT()           info.Length()
#define ADDON_THIS()                info.This()
#define ADDON_HOLDER()              info.Holder()
#define ADDON_IS_CONSTRUCT_CALL()   info.IsConstructCall()

// ─── Return values ──────────────────────────────────────────────────────────

#define ADDON_RETURN(val)           do { info.GetReturnValue().Set(val); return; } while(0)
#define ADDON_RETURN_NULL()         do { info.GetReturnValue().SetNull(); return; } while(0)
#define ADDON_RETURN_UNDEFINED()    do { info.GetReturnValue().SetUndefined(); return; } while(0)
#define ADDON_VOID_RETURN()         return

// ─── Value creation ─────────────────────────────────────────────────────────

#define ADDON_STRING(str)           Nan::New(str).ToLocalChecked()
#define ADDON_BOOL(val)             Nan::New(static_cast<bool>(val))
#define ADDON_INT(val)              Nan::New(static_cast<int32_t>(val))
#define ADDON_UINT(val)             Nan::New<v8::Integer>(static_cast<uint32_t>(val))
#define ADDON_NUMBER(val)           Nan::New<v8::Number>(static_cast<double>(val))
#define ADDON_BOOLEAN(val)          Nan::New<v8::Boolean>(val)
#define ADDON_INTEGER(val)          Nan::New<v8::Integer>(val)
#define ADDON_OBJECT()              Nan::New<v8::Object>()
#define ADDON_ARRAY(len)            Nan::New<v8::Array>(static_cast<uint32_t>(len))
#define ADDON_ARRAY_EMPTY()         Nan::New<v8::Array>()
#define ADDON_NULL()                Nan::Null()
#define ADDON_UNDEFINED()           Nan::Undefined()
#define ADDON_EXTERNAL(ptr)         Nan::New<v8::External>(ptr)

// ─── Type checking ──────────────────────────────────────────────────────────

#define ADDON_IS_STRING(v)          ((v)->IsString())
#define ADDON_IS_NUMBER(v)          ((v)->IsNumber())
#define ADDON_IS_BOOLEAN(v)         ((v)->IsBoolean())
#define ADDON_IS_OBJECT(v)          ((v)->IsObject())
#define ADDON_IS_ARRAY(v)           ((v)->IsArray())
#define ADDON_IS_NULL(v)            ((v)->IsNull())
#define ADDON_IS_UNDEFINED(v)       ((v)->IsUndefined())
#define ADDON_IS_INT32(v)           ((v)->IsInt32())
#define ADDON_IS_EXTERNAL(v)        ((v)->IsExternal())
#define ADDON_IS_FUNCTION(v)        ((v)->IsFunction())
#define ADDON_IS_DATE(v)            ((v)->IsDate())

// Direct boolean coercion (V8 BooleanValue)
#define ADDON_BOOL_VALUE(v)         ((v)->BooleanValue())

// ─── Type conversion ────────────────────────────────────────────────────────

#define ADDON_TO_INT32(v)           Nan::To<int32_t>(v).FromJust()
#define ADDON_TO_UINT32(v)          Nan::To<uint32_t>(v).FromJust()
#define ADDON_TO_DOUBLE(v)          Nan::To<double>(v).FromJust()
#define ADDON_TO_INT(v)             Nan::To<int>(v).FromJust()
#define ADDON_TO_BOOL(v)            Nan::To<bool>(v).FromJust()

#define ADDON_TO_INT32_DEFAULT(v, d)  Nan::To<int32_t>(v).FromMaybe(d)
#define ADDON_TO_UINT32_DEFAULT(v, d) Nan::To<uint32_t>(v).FromMaybe(d)
#define ADDON_TO_DOUBLE_DEFAULT(v, d) Nan::To<double>(v).FromMaybe(d)

#define ADDON_TO_OBJECT(v)          Nan::To<v8::Object>(v).ToLocalChecked()

// Casting
#define ADDON_CAST_ARRAY(v)         v8::Local<v8::Array>::Cast(v)
#define ADDON_AS_ARRAY(v)           (v).As<v8::Array>()
#define ADDON_AS_OBJECT(v)          (v).As<v8::Object>()
#define ADDON_CAST_EXTERNAL_PTR(v)  v8::Local<v8::External>::Cast(v)->Value()

// ─── String handling ────────────────────────────────────────────────────────

#define ADDON_UTF8(name, val)       Nan::Utf8String name(val)
#define ADDON_UTF8_VALUE(name)      (*name)
#define ADDON_UTF8_LENGTH(name)     (name).length()

// ─── Object / Array operations ──────────────────────────────────────────────

#define ADDON_SET(obj, key, val)        Nan::Set(obj, ADDON_STRING(key), val)
#define ADDON_SET_INDEX(arr, i, val)    Nan::Set(arr, static_cast<uint32_t>(i), val)
#define ADDON_GET(obj, key)             Nan::Get(obj, ADDON_STRING(key)).ToLocalChecked()
#define ADDON_GET_INDEX(arr, i)         Nan::Get(arr, static_cast<uint32_t>(i)).ToLocalChecked()
#define ADDON_HAS(obj, key)             Nan::Has(obj, ADDON_STRING(key)).FromJust()
#define ADDON_LENGTH(arr)               (arr)->Length()

// ─── Error handling ─────────────────────────────────────────────────────────

#define ADDON_THROW_ERROR(msg)      Nan::ThrowError(msg)
#define ADDON_THROW_TYPE_ERROR(msg) Nan::ThrowTypeError(msg)

// ─── Scope management ───────────────────────────────────────────────────────

#define ADDON_HANDLE_SCOPE()        Nan::HandleScope scope
#define ADDON_ESCAPABLE_SCOPE()     Nan::EscapableHandleScope scope
#define ADDON_ESCAPE(val)           scope.Escape(val)

// ─── Buffer ─────────────────────────────────────────────────────────────────

#define ADDON_BUFFER_IS(val)        node::Buffer::HasInstance(val)
#define ADDON_BUFFER_DATA(val)      node::Buffer::Data(val)
#define ADDON_BUFFER_LENGTH(val)    node::Buffer::Length(val)
#define ADDON_COPY_BUFFER(data, sz) Nan::CopyBuffer(data, sz).ToLocalChecked()

// ─── Function export (flat addons) ──────────────────────────────────────────

#define ADDON_EXPORT_FUNCTION(exports, name, fn) \
  Nan::Set(exports, ADDON_STRING(name), \
    Nan::GetFunction(Nan::New<v8::FunctionTemplate>(fn)).ToLocalChecked())

#define ADDON_SET_METHOD(obj, name, fn) Nan::SetMethod(obj, name, fn)

// ─── ObjectWrap ─────────────────────────────────────────────────────────────

#define ADDON_OBJECT_WRAP                        Nan::ObjectWrap
#define ADDON_UNWRAP(Class, holder)              Nan::ObjectWrap::Unwrap<Class>(holder)
#define ADDON_WRAP(instance)                     this->Wrap(instance)

#define ADDON_NEW_CTOR_TEMPLATE()                Nan::New<v8::FunctionTemplate>()
#define ADDON_NEW_CTOR_TEMPLATE_WITH(fn)         Nan::New<v8::FunctionTemplate>(fn)
#define ADDON_SET_CLASS_NAME(tpl, name)          (tpl)->SetClassName(ADDON_STRING(name))
#define ADDON_SET_INTERNAL_FIELD_COUNT(tpl, n)   (tpl)->InstanceTemplate()->SetInternalFieldCount(n)
#define ADDON_SET_PROTOTYPE_METHOD(tpl, name, fn) Nan::SetPrototypeMethod(tpl, name, fn)
#define ADDON_SET_ACCESSOR(tpl, name, getter)    \
  Nan::SetAccessor((tpl)->InstanceTemplate(), ADDON_STRING(name), getter)

#define ADDON_GET_CTOR_FUNCTION(tpl)             (tpl)->GetFunction()
#define ADDON_NEW_INSTANCE(cons)                 Nan::NewInstance(cons).ToLocalChecked()
#define ADDON_TEMPLATE_NEW_INSTANCE(tpl)         (tpl)->InstanceTemplate()->NewInstance()

// Persistent handles
#define ADDON_PERSISTENT_FUNCTION                Nan::Persistent<v8::Function>
#define ADDON_PERSISTENT_TEMPLATE                Nan::Persistent<v8::FunctionTemplate>
#define ADDON_PERSISTENT_RESET(p, val)           (p).Reset(val)
#define ADDON_PERSISTENT_GET(p)                  Nan::New(p)

// ─── TypedArray (legacy V8 3.x API) ────────────────────────────────────────

#define ADDON_IS_TYPEDARRAY(obj) \
  ((obj)->IsObject() && (obj).As<v8::Object>()->HasIndexedPropertiesInExternalArrayData())

#define ADDON_GET_TYPEDARRAY_DATA(obj) \
  ((obj).As<v8::Object>()->GetIndexedPropertiesExternalArrayData())
