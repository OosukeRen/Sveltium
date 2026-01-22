#include "function_call.h"
#include <cstring>
#include <cstdlib>

namespace calldll {

size_t getTypeSize(ArgType type) {
  switch (type) {
    case TYPE_VOID:    return 0;
    case TYPE_BOOL:    return 4;  // bool promoted to int on stack
    case TYPE_INT8:    return 4;  // promoted
    case TYPE_UINT8:   return 4;  // promoted
    case TYPE_INT16:   return 4;  // promoted
    case TYPE_UINT16:  return 4;  // promoted
    case TYPE_INT32:   return 4;
    case TYPE_UINT32:  return 4;
    case TYPE_INT64:   return 8;
    case TYPE_UINT64:  return 8;
    case TYPE_FLOAT:   return 4;
    case TYPE_DOUBLE:  return 8;
    case TYPE_POINTER: return 4;  // 32-bit
    case TYPE_STRING:  return 4;  // pointer
    case TYPE_WSTRING: return 4;  // pointer
    case TYPE_BUFFER:  return 4;  // pointer
    default:           return 4;
  }
}

size_t getTypeSlots(ArgType type) {
  // Each stack slot is 4 bytes on x86
  return (getTypeSize(type) + 3) / 4;
}

DLLFunction::DLLFunction(void* ptr, ArgType returnType,
                         const std::vector<ArgType>& argTypes,
                         CallConvention convention)
  : ptr_(ptr)
  , returnType_(returnType)
  , argTypes_(argTypes)
  , convention_(convention)
{
}

// Helper: build argument stack
static void buildStack(std::vector<uint32_t>& stack,
                       const std::vector<FunctionArg>& args) {
  // Push arguments right-to-left
  for (int i = static_cast<int>(args.size()) - 1; i >= 0; i--) {
    const FunctionArg& arg = args[i];

    switch (arg.type) {
      case TYPE_BOOL:
      case TYPE_INT8:
      case TYPE_INT16:
      case TYPE_INT32:
        stack.push_back(static_cast<uint32_t>(arg.value.int32Val));
        break;

      case TYPE_UINT8:
      case TYPE_UINT16:
      case TYPE_UINT32:
        stack.push_back(arg.value.uint32Val);
        break;

      case TYPE_INT64:
      case TYPE_UINT64:
        // 64-bit: push high then low (so low ends up at lower address)
        stack.push_back(static_cast<uint32_t>(arg.value.uint64Val >> 32));
        stack.push_back(static_cast<uint32_t>(arg.value.uint64Val & 0xFFFFFFFF));
        break;

      case TYPE_FLOAT:
        // Float is 4 bytes, push as uint32
        stack.push_back(*reinterpret_cast<const uint32_t*>(&arg.value.floatVal));
        break;

      case TYPE_DOUBLE:
        // Double is 8 bytes, push as two uint32s
        {
          const uint32_t* dwords = reinterpret_cast<const uint32_t*>(&arg.value.doubleVal);
          stack.push_back(dwords[1]);  // High dword
          stack.push_back(dwords[0]);  // Low dword
        }
        break;

      case TYPE_POINTER:
      case TYPE_STRING:
      case TYPE_WSTRING:
      case TYPE_BUFFER:
        stack.push_back(reinterpret_cast<uint32_t>(arg.value.ptrVal));
        break;

      default:
        stack.push_back(0);
        break;
    }
  }
}

// Type definitions for function pointers with various arg counts
typedef uint32_t (__cdecl *cdecl_fn)();
typedef uint32_t (__stdcall *stdcall_fn)();

// Cdecl variants (up to 8 args)
typedef uint32_t (__cdecl *cdecl_0)();
typedef uint32_t (__cdecl *cdecl_1)(uint32_t);
typedef uint32_t (__cdecl *cdecl_2)(uint32_t, uint32_t);
typedef uint32_t (__cdecl *cdecl_3)(uint32_t, uint32_t, uint32_t);
typedef uint32_t (__cdecl *cdecl_4)(uint32_t, uint32_t, uint32_t, uint32_t);
typedef uint32_t (__cdecl *cdecl_5)(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
typedef uint32_t (__cdecl *cdecl_6)(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
typedef uint32_t (__cdecl *cdecl_7)(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
typedef uint32_t (__cdecl *cdecl_8)(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);

// Stdcall variants (up to 8 args)
typedef uint32_t (__stdcall *stdcall_0)();
typedef uint32_t (__stdcall *stdcall_1)(uint32_t);
typedef uint32_t (__stdcall *stdcall_2)(uint32_t, uint32_t);
typedef uint32_t (__stdcall *stdcall_3)(uint32_t, uint32_t, uint32_t);
typedef uint32_t (__stdcall *stdcall_4)(uint32_t, uint32_t, uint32_t, uint32_t);
typedef uint32_t (__stdcall *stdcall_5)(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
typedef uint32_t (__stdcall *stdcall_6)(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
typedef uint32_t (__stdcall *stdcall_7)(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
typedef uint32_t (__stdcall *stdcall_8)(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);

FunctionArg DLLFunction::call(const std::vector<FunctionArg>& args) {
  FunctionArg result;
  result.type = returnType_;

  if (ptr_ == NULL) {
    return result;
  }

  // Build stack frame (in correct order for passing)
  std::vector<uint32_t> stack;

  // Process args left-to-right for the stack array
  for (size_t i = 0; i < args.size(); i++) {
    const FunctionArg& arg = args[i];

    switch (arg.type) {
      case TYPE_BOOL:
      case TYPE_INT8:
      case TYPE_INT16:
      case TYPE_INT32:
        stack.push_back(static_cast<uint32_t>(arg.value.int32Val));
        break;

      case TYPE_UINT8:
      case TYPE_UINT16:
      case TYPE_UINT32:
        stack.push_back(arg.value.uint32Val);
        break;

      case TYPE_INT64:
      case TYPE_UINT64:
        // 64-bit: push low then high
        stack.push_back(static_cast<uint32_t>(arg.value.uint64Val & 0xFFFFFFFF));
        stack.push_back(static_cast<uint32_t>(arg.value.uint64Val >> 32));
        break;

      case TYPE_FLOAT:
        stack.push_back(*reinterpret_cast<const uint32_t*>(&arg.value.floatVal));
        break;

      case TYPE_DOUBLE:
        {
          const uint32_t* dwords = reinterpret_cast<const uint32_t*>(&arg.value.doubleVal);
          stack.push_back(dwords[0]);
          stack.push_back(dwords[1]);
        }
        break;

      case TYPE_POINTER:
      case TYPE_STRING:
      case TYPE_WSTRING:
      case TYPE_BUFFER:
        stack.push_back(reinterpret_cast<uint32_t>(arg.value.ptrVal));
        break;

      default:
        stack.push_back(0);
        break;
    }
  }

  // Calculate expected slot count from declared argument types
  // This is important for stdcall where the callee cleans the stack
  size_t expectedSlots = 0;
  for (size_t i = 0; i < argTypes_.size(); i++) {
    expectedSlots += getTypeSlots(argTypes_[i]);
  }

  // Pad to 8 args if needed (for the switch statement)
  while (stack.size() < 8) {
    stack.push_back(0);
  }

  uint32_t resultLow = 0;

  // Call based on convention and expected slot count from declaration
  // NOT stack.size() which is always padded to 8+
  size_t slotCount = expectedSlots;

  if (convention_ == CALL_STDCALL) {
    switch (slotCount) {
      case 0: resultLow = ((stdcall_0)ptr_)(); break;
      case 1: resultLow = ((stdcall_1)ptr_)(stack[0]); break;
      case 2: resultLow = ((stdcall_2)ptr_)(stack[0], stack[1]); break;
      case 3: resultLow = ((stdcall_3)ptr_)(stack[0], stack[1], stack[2]); break;
      case 4: resultLow = ((stdcall_4)ptr_)(stack[0], stack[1], stack[2], stack[3]); break;
      case 5: resultLow = ((stdcall_5)ptr_)(stack[0], stack[1], stack[2], stack[3], stack[4]); break;
      case 6: resultLow = ((stdcall_6)ptr_)(stack[0], stack[1], stack[2], stack[3], stack[4], stack[5]); break;
      case 7: resultLow = ((stdcall_7)ptr_)(stack[0], stack[1], stack[2], stack[3], stack[4], stack[5], stack[6]); break;
      default: resultLow = ((stdcall_8)ptr_)(stack[0], stack[1], stack[2], stack[3], stack[4], stack[5], stack[6], stack[7]); break;
    }
  } else {
    // CALL_CDECL or CALL_FASTCALL (fastcall handled as cdecl for simplicity)
    switch (slotCount) {
      case 0: resultLow = ((cdecl_0)ptr_)(); break;
      case 1: resultLow = ((cdecl_1)ptr_)(stack[0]); break;
      case 2: resultLow = ((cdecl_2)ptr_)(stack[0], stack[1]); break;
      case 3: resultLow = ((cdecl_3)ptr_)(stack[0], stack[1], stack[2]); break;
      case 4: resultLow = ((cdecl_4)ptr_)(stack[0], stack[1], stack[2], stack[3]); break;
      case 5: resultLow = ((cdecl_5)ptr_)(stack[0], stack[1], stack[2], stack[3], stack[4]); break;
      case 6: resultLow = ((cdecl_6)ptr_)(stack[0], stack[1], stack[2], stack[3], stack[4], stack[5]); break;
      case 7: resultLow = ((cdecl_7)ptr_)(stack[0], stack[1], stack[2], stack[3], stack[4], stack[5], stack[6]); break;
      default: resultLow = ((cdecl_8)ptr_)(stack[0], stack[1], stack[2], stack[3], stack[4], stack[5], stack[6], stack[7]); break;
    }
  }

  uint32_t resultHigh = 0;

  // Convert return value based on type
  switch (returnType_) {
    case TYPE_VOID:
      break;

    case TYPE_BOOL:
      result.value.boolVal = (resultLow != 0);
      break;

    case TYPE_INT8:
      result.value.int8Val = static_cast<int8_t>(resultLow);
      break;

    case TYPE_UINT8:
      result.value.uint8Val = static_cast<uint8_t>(resultLow);
      break;

    case TYPE_INT16:
      result.value.int16Val = static_cast<int16_t>(resultLow);
      break;

    case TYPE_UINT16:
      result.value.uint16Val = static_cast<uint16_t>(resultLow);
      break;

    case TYPE_INT32:
      result.value.int32Val = static_cast<int32_t>(resultLow);
      break;

    case TYPE_UINT32:
      result.value.uint32Val = resultLow;
      break;

    case TYPE_INT64:
    case TYPE_UINT64:
      result.value.uint64Val =
        (static_cast<uint64_t>(resultHigh) << 32) | resultLow;
      break;

    case TYPE_FLOAT:
      // Float return is in ST(0), need FPU read
      // For simplicity, assume it was stored in EAX as uint32
      result.value.floatVal = *reinterpret_cast<float*>(&resultLow);
      break;

    case TYPE_DOUBLE:
      // Double return is in ST(0)
      {
        uint64_t combined =
          (static_cast<uint64_t>(resultHigh) << 32) | resultLow;
        result.value.doubleVal = *reinterpret_cast<double*>(&combined);
      }
      break;

    case TYPE_POINTER:
    case TYPE_STRING:
    case TYPE_WSTRING:
    case TYPE_BUFFER:
      result.value.ptrVal = reinterpret_cast<void*>(resultLow);
      break;

    default:
      break;
  }

  return result;
}

} // namespace calldll
