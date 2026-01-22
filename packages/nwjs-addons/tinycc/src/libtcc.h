/*
 * TinyCC (TCC) Library Header
 *
 * This is the public API header for libtcc.
 * The actual libtcc library must be linked separately.
 *
 * For XP compatibility, use a prebuilt 32-bit libtcc.a/libtcc.lib
 * or compile TCC from source with VS2015 v140_xp.
 *
 * TCC Project: https://bellard.org/tcc/
 */

#ifndef LIBTCC_H
#define LIBTCC_H

#ifdef __cplusplus
extern "C" {
#endif

/* TCC compilation state */
struct TCCState;
typedef struct TCCState TCCState;

/* Create a new TCC compilation context */
TCCState *tcc_new(void);

/* Free a TCC compilation context */
void tcc_delete(TCCState *s);

/* Set CONFIG_TCCDIR at runtime */
void tcc_set_lib_path(TCCState *s, const char *path);

/* Set error/warning display callback */
typedef void (*TCCErrorFunc)(void *opaque, const char *msg);
void tcc_set_error_func(TCCState *s, void *error_opaque, TCCErrorFunc error_func);

/* Set options as from command line (multiple supported) */
void tcc_set_options(TCCState *s, const char *str);

/*****************************/
/* Preprocessor */

/* Add include path */
int tcc_add_include_path(TCCState *s, const char *pathname);

/* Add system include path */
int tcc_add_sysinclude_path(TCCState *s, const char *pathname);

/* Define preprocessor symbol 'sym' with value 'value' */
void tcc_define_symbol(TCCState *s, const char *sym, const char *value);

/* Undefine preprocessor symbol 'sym' */
void tcc_undefine_symbol(TCCState *s, const char *sym);

/*****************************/
/* Compiling */

/* Add a file (C file, dll, object, library, etc.) */
int tcc_add_file(TCCState *s, const char *filename);

/* Compile a string containing C source */
int tcc_compile_string(TCCState *s, const char *buf);

/*****************************/
/* Linking */

/* Set output type.
 * MUST BE CALLED before any compilation.
 */
#define TCC_OUTPUT_MEMORY   1 /* output in memory (default) */
#define TCC_OUTPUT_EXE      2 /* executable file */
#define TCC_OUTPUT_DLL      3 /* dynamic library */
#define TCC_OUTPUT_OBJ      4 /* object file */
#define TCC_OUTPUT_PREPROCESS 5 /* preprocessed file (for -E) */

int tcc_set_output_type(TCCState *s, int output_type);

/* Equivalent to -Lpath option */
int tcc_add_library_path(TCCState *s, const char *pathname);

/* Equivalent to -lxxx option */
int tcc_add_library(TCCState *s, const char *libraryname);

/* Add a symbol to the compiled program */
int tcc_add_symbol(TCCState *s, const char *name, const void *val);

/* Output file (for TCC_OUTPUT_EXE, TCC_OUTPUT_DLL, TCC_OUTPUT_OBJ) */
int tcc_output_file(TCCState *s, const char *filename);

/* Link and run main() function, returning its return value.
 * DO NOT CALL tcc_relocate() before. */
int tcc_run(TCCState *s, int argc, char **argv);

/* Relocate the code.
 *
 * Use TCC_RELOCATE_AUTO to let TCC allocate memory.
 * Otherwise, provide a buffer of at least tcc_relocate(s, NULL) bytes.
 *
 * Returns -1 on error, required memory size with ptr==NULL,
 * or 0 on success.
 */
#define TCC_RELOCATE_AUTO (void*)1

int tcc_relocate(TCCState *s, void *ptr);

/* Return symbol value or NULL if not found */
void *tcc_get_symbol(TCCState *s, const char *name);

/* Set selinux/wxorx compatibility mode (0 = default, 1 = writeable, 2 = rx) */
void tcc_set_selinux(int mode);

#ifdef __cplusplus
}
#endif

#endif /* LIBTCC_H */
