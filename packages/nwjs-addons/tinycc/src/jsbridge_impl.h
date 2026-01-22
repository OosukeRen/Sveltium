#ifndef JSBRIDGE_IMPL_H
#define JSBRIDGE_IMPL_H

#include <nan.h>
#include <vector>
#include <map>

extern "C" {
  #include "../include/jsbridge.h"
}

namespace jsbridge {

// JSContext implementation
// Manages a pool of V8 value handles that C code can reference
class Context {
public:
  Context();
  ~Context();

  // Store a V8 value and return a jsvalue handle
  jsvalue store(v8::Local<v8::Value> value);

  // Retrieve a V8 value from a jsvalue handle
  v8::Local<v8::Value> retrieve(jsvalue val);

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
  // Using pointers to avoid copy issues with Persistent
  struct StoredValue {
    v8::Persistent<v8::Value>* persistent;
    int refCount;

    StoredValue() : persistent(NULL), refCount(0) {}
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
