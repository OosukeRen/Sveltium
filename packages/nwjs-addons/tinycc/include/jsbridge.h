#ifndef __jsbridge
#define __jsbridge

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Type definitions for portability */
typedef int8_t   int8;
typedef uint8_t  uint8;
typedef int16_t  int16;
typedef uint16_t uint16;
typedef int32_t  int32;
typedef uint32_t uint32;
typedef int64_t  int64;
typedef uint64_t uint64;

/* JS value handle */
typedef struct jsvalue { uint64 v; } jsvalue;

/* JS context handle */
struct JSContext;
typedef struct JSContext* jscontext;

/* ========================================
 * C -> JS Value Conversions
 * ======================================== */

jsvalue bool_to_jsvalue(jscontext ctx, bool val);
jsvalue int8_to_jsvalue(jscontext ctx, int8 val);
jsvalue uint8_to_jsvalue(jscontext ctx, uint8 val);
jsvalue int16_to_jsvalue(jscontext ctx, int16 val);
jsvalue uint16_to_jsvalue(jscontext ctx, uint16 val);
jsvalue int32_to_jsvalue(jscontext ctx, int32 val);
jsvalue uint32_to_jsvalue(jscontext ctx, uint32 val);
jsvalue int64_to_jsvalue(jscontext ctx, int64 val);
jsvalue uint64_to_jsvalue(jscontext ctx, uint64 val);
jsvalue float_to_jsvalue(jscontext ctx, float val);
jsvalue double_to_jsvalue(jscontext ctx, double val);

/* ========================================
 * JS -> C Value Conversions
 * ======================================== */

bool   jsvalue_to_bool(jscontext ctx, jsvalue val);
int8   jsvalue_to_int8(jscontext ctx, jsvalue val);
uint8  jsvalue_to_uint8(jscontext ctx, jsvalue val);
int16  jsvalue_to_int16(jscontext ctx, jsvalue val);
uint16 jsvalue_to_uint16(jscontext ctx, jsvalue val);
int32  jsvalue_to_int32(jscontext ctx, jsvalue val);
uint32 jsvalue_to_uint32(jscontext ctx, jsvalue val);
int64  jsvalue_to_int64(jscontext ctx, jsvalue val);
uint64 jsvalue_to_uint64(jscontext ctx, jsvalue val);
float  jsvalue_to_float(jscontext ctx, jsvalue val);
double jsvalue_to_double(jscontext ctx, jsvalue val);

/* Identity conversion (for generics) */
jsvalue jsvalue_to_jsvalue(jscontext ctx, jsvalue val);

/* ========================================
 * TypedArray Direct Pointers
 * Zero-copy access to typed array data
 * ======================================== */

int8*   jsvalue_to_int8_ptr(jscontext ctx, jsvalue val);
uint8*  jsvalue_to_uint8_ptr(jscontext ctx, jsvalue val);
int16*  jsvalue_to_int16_ptr(jscontext ctx, jsvalue val);
uint16* jsvalue_to_uint16_ptr(jscontext ctx, jsvalue val);
int32*  jsvalue_to_int32_ptr(jscontext ctx, jsvalue val);
uint32* jsvalue_to_uint32_ptr(jscontext ctx, jsvalue val);
int64*  jsvalue_to_int64_ptr(jscontext ctx, jsvalue val);
uint64* jsvalue_to_uint64_ptr(jscontext ctx, jsvalue val);
float*  jsvalue_to_float_ptr(jscontext ctx, jsvalue val);
double* jsvalue_to_double_ptr(jscontext ctx, jsvalue val);

/* ========================================
 * Object/Array Creation and Manipulation
 * ======================================== */

/**
 * Create new JS value using format string
 *
 * Format specifiers:
 *   "{"     - Create empty object {}
 *   "["     - Create empty array []
 *   "s"     - Create string (next vararg: const char*)
 *   "i"     - Create integer (next vararg: int32)
 *   "d"     - Create double (next vararg: double)
 *   "b"     - Create boolean (next vararg: int as bool)
 *   "n"     - Create null
 *   "u"     - Create undefined
 *
 * Example: jsvalue_new("{s:name,i:age}", "John", 30)
 *          Creates: { name: "John", age: 30 }
 */
jsvalue _jsvalue_new(jscontext ctx, const char* fmt, ...);

/**
 * Fetch property or call method on JS value
 *
 * Format specifiers for getting:
 *   "i:prop"    - Get property as int32
 *   "d:prop"    - Get property as double
 *   "s:prop"    - Get property as string (vararg: char** output)
 *   "v:prop"    - Get property as jsvalue
 *   "v:[idx]"   - Get array element at index (vararg: int index)
 *
 * Format specifiers for calling:
 *   ":method()"     - Call method with no args
 *   ":method(i)"    - Call method with int arg
 *   ":method(i,s)"  - Call method with int and string args
 *
 * Returns: int result or error code
 */
int _jsvalue_fetch(jscontext ctx, jsvalue val, const char* fmt, ...);

/* ========================================
 * Type Checking
 * ======================================== */

typedef enum JSVALUE_TYPE {
  JSVALUE_UNDEFINED = 0,
  JSVALUE_NUMBER,
  JSVALUE_BOOLEAN,
  JSVALUE_STRING,
  JSVALUE_ARRAY,
  JSVALUE_OBJECT,
  JSVALUE_DATE,
  JSVALUE_FUNCTION,
  JSVALUE_ERROR,
  JSVALUE_NULL,

  /* TypedArray types (0x10+) */
  JSVALUE_INT8_ARRAY = 0x10,
  JSVALUE_UINT8_ARRAY,
  JSVALUE_INT16_ARRAY,
  JSVALUE_UINT16_ARRAY,
  JSVALUE_INT32_ARRAY,
  JSVALUE_UINT32_ARRAY,
  JSVALUE_INT64_ARRAY,
  JSVALUE_UINT64_ARRAY,
  JSVALUE_FLOAT32_ARRAY,
  JSVALUE_FLOAT64_ARRAY,

} JSVALUE_TYPE;

int _jsvalue_type(jscontext ctx, jsvalue val);

/* ========================================
 * Memory Management
 * ======================================== */

/**
 * Increment reference count for jsvalue
 * Must be called if storing jsvalue beyond current function scope
 */
void _jsvalue_addref(jscontext ctx, jsvalue val);

/**
 * Decrement reference count for jsvalue
 * Must be called to release jsvalues obtained via addref
 */
void _jsvalue_release(jscontext ctx, jsvalue val);

/* ========================================
 * Global Context and Convenience Macros
 * ======================================== */

/* Global context - set by runtime before calling C functions */
extern jscontext __jscontext;

/* Convenience macros that use global context */
#define jsvalue_new(format, ...) _jsvalue_new(__jscontext, format, ##__VA_ARGS__)
#define jsvalue_fetch(val, format, ...) _jsvalue_fetch(__jscontext, val, format, ##__VA_ARGS__)
#define jsvalue_type(val) _jsvalue_type(__jscontext, val)
#define jsvalue_addref(val) _jsvalue_addref(__jscontext, val)
#define jsvalue_release(val) _jsvalue_release(__jscontext, val)

/* ========================================
 * Function Signature Helpers
 * ======================================== */

/**
 * Signature for JS-callable C functions
 * Example:
 *   jsvalue my_function(jscontext ctx, jsvalue arg1, jsvalue arg2) {
 *     int a = jsvalue_to_int32(ctx, arg1);
 *     int b = jsvalue_to_int32(ctx, arg2);
 *     return int32_to_jsvalue(ctx, a + b);
 *   }
 */
typedef jsvalue (*jsfunction_0)(jscontext ctx);
typedef jsvalue (*jsfunction_1)(jscontext ctx, jsvalue a);
typedef jsvalue (*jsfunction_2)(jscontext ctx, jsvalue a, jsvalue b);
typedef jsvalue (*jsfunction_3)(jscontext ctx, jsvalue a, jsvalue b, jsvalue c);
typedef jsvalue (*jsfunction_4)(jscontext ctx, jsvalue a, jsvalue b, jsvalue c, jsvalue d);
typedef jsvalue (*jsfunction_n)(jscontext ctx, int argc, jsvalue* argv);

#ifdef __cplusplus
}
#endif

#endif /* __jsbridge */
