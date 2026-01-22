#ifndef FUNCTION_CALL_H
#define FUNCTION_CALL_H

#include <string>
#include <vector>
#include <stdint.h>

namespace calldll {

/**
 * Calling conventions
 */
enum CallConvention {
  CALL_CDECL = 0,     // Caller cleans stack, args right-to-left
  CALL_STDCALL = 1,   // Callee cleans stack, args right-to-left
  CALL_FASTCALL = 2   // First two args in ECX/EDX, callee cleans
};

/**
 * Argument types
 */
enum ArgType {
  TYPE_VOID = 0,
  TYPE_BOOL,
  TYPE_INT8,
  TYPE_UINT8,
  TYPE_INT16,
  TYPE_UINT16,
  TYPE_INT32,
  TYPE_UINT32,
  TYPE_INT64,
  TYPE_UINT64,
  TYPE_FLOAT,
  TYPE_DOUBLE,
  TYPE_POINTER,
  TYPE_STRING,    // char*
  TYPE_WSTRING,   // wchar_t*
  TYPE_BUFFER     // void* with length
};

/**
 * Argument value union
 */
union ArgValue {
  bool     boolVal;
  int8_t   int8Val;
  uint8_t  uint8Val;
  int16_t  int16Val;
  uint16_t uint16Val;
  int32_t  int32Val;
  uint32_t uint32Val;
  int64_t  int64Val;
  uint64_t uint64Val;
  float    floatVal;
  double   doubleVal;
  void*    ptrVal;
};

/**
 * Function argument with type
 */
struct FunctionArg {
  ArgType type;
  ArgValue value;
  std::string strValue;   // For string types
  size_t bufferSize;      // For buffer type

  FunctionArg() : type(TYPE_VOID), bufferSize(0) {
    memset(&value, 0, sizeof(value));
  }
};

/**
 * DLL Function wrapper
 * Handles calling native functions with various calling conventions
 */
class DLLFunction {
public:
  /**
   * Create function wrapper
   * @param ptr Function pointer
   * @param returnType Return value type
   * @param argTypes Argument types
   * @param convention Calling convention
   */
  DLLFunction(void* ptr, ArgType returnType,
              const std::vector<ArgType>& argTypes,
              CallConvention convention);

  /**
   * Call the function
   * @param args Arguments to pass
   * @returns Return value
   */
  FunctionArg call(const std::vector<FunctionArg>& args);

  /**
   * Get raw function pointer
   */
  void* pointer() const { return ptr_; }

  /**
   * Get return type
   */
  ArgType returnType() const { return returnType_; }

  /**
   * Get argument count
   */
  size_t argCount() const { return argTypes_.size(); }

private:
  void* ptr_;
  ArgType returnType_;
  std::vector<ArgType> argTypes_;
  CallConvention convention_;
};

/**
 * Get size of type in bytes (for stack allocation)
 */
size_t getTypeSize(ArgType type);

/**
 * Get stack slots needed for type (4 bytes per slot on x86)
 */
size_t getTypeSlots(ArgType type);

} // namespace calldll

#endif // FUNCTION_CALL_H
