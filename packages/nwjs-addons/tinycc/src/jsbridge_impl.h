#ifndef JSBRIDGE_IMPL_H
#define JSBRIDGE_IMPL_H

#include "addon_api.h"
#include <vector>
#include <map>

extern "C" {
  #include "../include/jsbridge.h"
}

namespace jsbridge {

// JSContext implementation
// Manages a pool of JS value handles that C code can reference via opaque uint64 keys.
// NAN backend stores values as v8::Persistent<Value>* (heap-allocated, manual lifecycle).
// N-API backend stores values as napi_ref (opaque reference, cleaned up via napi_delete_reference).
class Context {
public:
  Context();
  ~Context();

  // Store a JS value and return a jsvalue handle
  jsvalue store(ADDON_VALUE value);

  // Retrieve a JS value from a jsvalue handle
  ADDON_VALUE retrieve(jsvalue val);

  // Add reference to a jsvalue (prevent GC)
  void addRef(jsvalue val);

  // Release reference to a jsvalue
  void release(jsvalue val);

  // Clear all stored values
  void clear();

  // Get the current context (thread-local)
  static Context* current();

  // Set the current context (before calling C functions)
  static void setCurrent(Context* ctx);

private:
  // Value storage with reference counting
  struct StoredValue {
#ifdef USE_NAPI
    napi_ref ref;
    int refCount;
    StoredValue() : ref(NULL), refCount(0) {}
#else
    // Heap-allocated to avoid copy issues with non-copyable Persistent in V8 3.x
    v8::Persistent<v8::Value>* persistent;
    int refCount;
    StoredValue() : persistent(NULL), refCount(0) {}
#endif
  };

  std::map<uint64_t, StoredValue> values_;
  uint64_t nextHandle_;
};

// RAII helper to set/restore context
class ContextScope {
public:
  ContextScope(Context* ctx);
  ~ContextScope();

private:
  Context* previous_;
};

} // namespace jsbridge

#endif // JSBRIDGE_IMPL_H
