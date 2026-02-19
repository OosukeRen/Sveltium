#include "function_call.h"
#include <cstring>
#include <cstdlib>

namespace calldll {

// Slot size matches register width: 4 bytes on x86, 8 bytes on x64
static const size_t SLOT_SIZE = sizeof(uintptr_t);

size_t getTypeSize(ArgType type) {
  switch (type) {
    case TYPE_VOID:    return 0;
    case TYPE_BOOL:    return SLOT_SIZE;  // promoted to register width
    case TYPE_INT8:    return SLOT_SIZE;  // promoted
    case TYPE_UINT8:   return SLOT_SIZE;  // promoted
    case TYPE_INT16:   return SLOT_SIZE;  // promoted
    case TYPE_UINT16:  return SLOT_SIZE;  // promoted
    case TYPE_INT32:   return SLOT_SIZE;
    case TYPE_UINT32:  return SLOT_SIZE;
    case TYPE_INT64:   return 8;
    case TYPE_UINT64:  return 8;
    case TYPE_FLOAT:   return SLOT_SIZE;
    case TYPE_DOUBLE:  return 8;
    case TYPE_POINTER: return SLOT_SIZE;
    case TYPE_STRING:  return SLOT_SIZE;
    case TYPE_WSTRING: return SLOT_SIZE;
    case TYPE_BUFFER:  return SLOT_SIZE;
    default:           return SLOT_SIZE;
  }
}

size_t getTypeSlots(ArgType type) {
  return (getTypeSize(type) + SLOT_SIZE - 1) / SLOT_SIZE;
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

// Type alias for register-width slot (4 bytes on x86, 8 bytes on x64)
typedef uintptr_t slot_t;

// Function pointer typedefs using register-width slots (up to 8 args)
// On x64, __cdecl and __stdcall are identical (Microsoft x64 ABI)
typedef slot_t (__cdecl *cdecl_0)();
typedef slot_t (__cdecl *cdecl_1)(slot_t);
typedef slot_t (__cdecl *cdecl_2)(slot_t, slot_t);
typedef slot_t (__cdecl *cdecl_3)(slot_t, slot_t, slot_t);
typedef slot_t (__cdecl *cdecl_4)(slot_t, slot_t, slot_t, slot_t);
typedef slot_t (__cdecl *cdecl_5)(slot_t, slot_t, slot_t, slot_t, slot_t);
typedef slot_t (__cdecl *cdecl_6)(slot_t, slot_t, slot_t, slot_t, slot_t, slot_t);
typedef slot_t (__cdecl *cdecl_7)(slot_t, slot_t, slot_t, slot_t, slot_t, slot_t, slot_t);
typedef slot_t (__cdecl *cdecl_8)(slot_t, slot_t, slot_t, slot_t, slot_t, slot_t, slot_t, slot_t);

typedef slot_t (__stdcall *stdcall_0)();
typedef slot_t (__stdcall *stdcall_1)(slot_t);
typedef slot_t (__stdcall *stdcall_2)(slot_t, slot_t);
typedef slot_t (__stdcall *stdcall_3)(slot_t, slot_t, slot_t);
typedef slot_t (__stdcall *stdcall_4)(slot_t, slot_t, slot_t, slot_t);
typedef slot_t (__stdcall *stdcall_5)(slot_t, slot_t, slot_t, slot_t, slot_t);
typedef slot_t (__stdcall *stdcall_6)(slot_t, slot_t, slot_t, slot_t, slot_t, slot_t);
typedef slot_t (__stdcall *stdcall_7)(slot_t, slot_t, slot_t, slot_t, slot_t, slot_t, slot_t);
typedef slot_t (__stdcall *stdcall_8)(slot_t, slot_t, slot_t, slot_t, slot_t, slot_t, slot_t, slot_t);

FunctionArg DLLFunction::call(const std::vector<FunctionArg>& args) {
  FunctionArg result;
  result.type = returnType_;

  if (ptr_ == NULL) {
    return result;
  }

  // Build argument slots (register-width values)
  std::vector<slot_t> stack;

  for (size_t i = 0; i < args.size(); i++) {
    const FunctionArg& arg = args[i];

    switch (arg.type) {
      case TYPE_BOOL:
      case TYPE_INT8:
      case TYPE_INT16:
      case TYPE_INT32:
        stack.push_back(static_cast<slot_t>(arg.value.int32Val));
        break;

      case TYPE_UINT8:
      case TYPE_UINT16:
      case TYPE_UINT32:
        stack.push_back(static_cast<slot_t>(arg.value.uint32Val));
        break;

      case TYPE_INT64:
      case TYPE_UINT64:
#ifdef _WIN64
        // On x64, 64-bit values fit in a single slot
        stack.push_back(static_cast<slot_t>(arg.value.uint64Val));
#else
        // On x86, 64-bit values need two 32-bit slots (low, high)
        stack.push_back(static_cast<slot_t>(arg.value.uint64Val & 0xFFFFFFFF));
        stack.push_back(static_cast<slot_t>(arg.value.uint64Val >> 32));
#endif
        break;

      case TYPE_FLOAT: {
        slot_t slot = 0;
        memcpy(&slot, &arg.value.floatVal, sizeof(float));
        stack.push_back(slot);
        break;
      }

      case TYPE_DOUBLE: {
#ifdef _WIN64
        slot_t slot = 0;
        memcpy(&slot, &arg.value.doubleVal, sizeof(double));
        stack.push_back(slot);
#else
        const uint32_t* dwords = reinterpret_cast<const uint32_t*>(&arg.value.doubleVal);
        stack.push_back(dwords[0]);
        stack.push_back(dwords[1]);
#endif
        break;
      }

      case TYPE_STRING:
      case TYPE_WSTRING:
        // Use strValue.c_str() — ptrVal may be dangling after vector copy
        stack.push_back(reinterpret_cast<slot_t>(arg.strValue.c_str()));
        break;

      case TYPE_POINTER:
      case TYPE_BUFFER:
        stack.push_back(reinterpret_cast<slot_t>(arg.value.ptrVal));
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

  slot_t resultRaw = 0;

  // Call based on convention and expected slot count from declaration
  // NOT stack.size() which is always padded to 8+
  size_t slotCount = expectedSlots;

  if (convention_ == CALL_STDCALL) {
    switch (slotCount) {
      case 0: resultRaw = ((stdcall_0)ptr_)(); break;
      case 1: resultRaw = ((stdcall_1)ptr_)(stack[0]); break;
      case 2: resultRaw = ((stdcall_2)ptr_)(stack[0], stack[1]); break;
      case 3: resultRaw = ((stdcall_3)ptr_)(stack[0], stack[1], stack[2]); break;
      case 4: resultRaw = ((stdcall_4)ptr_)(stack[0], stack[1], stack[2], stack[3]); break;
      case 5: resultRaw = ((stdcall_5)ptr_)(stack[0], stack[1], stack[2], stack[3], stack[4]); break;
      case 6: resultRaw = ((stdcall_6)ptr_)(stack[0], stack[1], stack[2], stack[3], stack[4], stack[5]); break;
      case 7: resultRaw = ((stdcall_7)ptr_)(stack[0], stack[1], stack[2], stack[3], stack[4], stack[5], stack[6]); break;
      default: resultRaw = ((stdcall_8)ptr_)(stack[0], stack[1], stack[2], stack[3], stack[4], stack[5], stack[6], stack[7]); break;
    }
  } else {
    // CALL_CDECL or CALL_FASTCALL (fastcall handled as cdecl for simplicity)
    switch (slotCount) {
      case 0: resultRaw = ((cdecl_0)ptr_)(); break;
      case 1: resultRaw = ((cdecl_1)ptr_)(stack[0]); break;
      case 2: resultRaw = ((cdecl_2)ptr_)(stack[0], stack[1]); break;
      case 3: resultRaw = ((cdecl_3)ptr_)(stack[0], stack[1], stack[2]); break;
      case 4: resultRaw = ((cdecl_4)ptr_)(stack[0], stack[1], stack[2], stack[3]); break;
      case 5: resultRaw = ((cdecl_5)ptr_)(stack[0], stack[1], stack[2], stack[3], stack[4]); break;
      case 6: resultRaw = ((cdecl_6)ptr_)(stack[0], stack[1], stack[2], stack[3], stack[4], stack[5]); break;
      case 7: resultRaw = ((cdecl_7)ptr_)(stack[0], stack[1], stack[2], stack[3], stack[4], stack[5], stack[6]); break;
      default: resultRaw = ((cdecl_8)ptr_)(stack[0], stack[1], stack[2], stack[3], stack[4], stack[5], stack[6], stack[7]); break;
    }
  }

  // Convert return value based on type
  switch (returnType_) {
    case TYPE_VOID:
      break;

    case TYPE_BOOL:
      result.value.boolVal = (resultRaw != 0);
      break;

    case TYPE_INT8:
      result.value.int8Val = static_cast<int8_t>(resultRaw);
      break;

    case TYPE_UINT8:
      result.value.uint8Val = static_cast<uint8_t>(resultRaw);
      break;

    case TYPE_INT16:
      result.value.int16Val = static_cast<int16_t>(resultRaw);
      break;

    case TYPE_UINT16:
      result.value.uint16Val = static_cast<uint16_t>(resultRaw);
      break;

    case TYPE_INT32:
      result.value.int32Val = static_cast<int32_t>(resultRaw);
      break;

    case TYPE_UINT32:
      result.value.uint32Val = static_cast<uint32_t>(resultRaw);
      break;

    case TYPE_INT64:
    case TYPE_UINT64:
      result.value.uint64Val = static_cast<uint64_t>(resultRaw);
      break;

    case TYPE_FLOAT:
      memcpy(&result.value.floatVal, &resultRaw, sizeof(float));
      break;

    case TYPE_DOUBLE:
      memcpy(&result.value.doubleVal, &resultRaw, sizeof(double));
      break;

    case TYPE_POINTER:
    case TYPE_STRING:
    case TYPE_WSTRING:
    case TYPE_BUFFER:
      result.value.ptrVal = reinterpret_cast<void*>(resultRaw);
      break;

    default:
      break;
  }

  return result;
}

} // namespace calldll
