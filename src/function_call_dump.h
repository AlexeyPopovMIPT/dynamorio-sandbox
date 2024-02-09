#ifndef FUNCTION_CALL_DUMP_H
#define FUNCTION_CALL_DUMP_H

#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <inttypes.h>

// Library will need this for macro magic later
typedef void const* fcd_pointer;

enum arg_type {
    char_ty,
    int16_t_ty, uint16_t_ty,
    int32_t_ty, uint32_t_ty,
    int64_t_ty, uint64_t_ty,
    float_ty, 
    double_ty,
    fcd_pointer_ty,
    STRING,
    RAW_BYTES,
    STRUCT,
    ENUM_SIZE
};

#ifndef FCD_PRINT_MAX
#define FCD_PRINT_MAX 20
#endif
#ifndef FCD_OUTPUT_CUT_MARKER
#define FCD_OUTPUT_CUT_MARKER "..."
#endif

static const char *format_specifier[] = 
    {"%c", "%hd", "%hu", "%d", "%u", "%lld", "%llu", "%f", "%lf", "%p",
     "%s", NULL, NULL};

static_assert (sizeof (format_specifier) / sizeof (*format_specifier)
                  == (size_t) ENUM_SIZE);

typedef struct fcd_arg {
    char const *name;
    void *value; /* pointer to value (for STRING, RAW_BYTES, STRUCT),
                    bit-casted value for other types */
    enum arg_type type;

    // unused unless type is STRUCT or RAW_BYTES
    union {
        // Rule how to print argument, for STRUCT
        void (*print_innards) (struct fcd_arg const *, FILE *);
        // byte sequence length for RAW_BYTES
        size_t size;
    } printer;
} fcd_arg;

typedef struct fcd_call {
    char const *name;
    fcd_arg *return_value;
    size_t arg_count;
    fcd_arg *args;
} fcd_call;

size_t fcd_fwrite (const void *buf, size_t size, size_t nmemb, FILE *file) {
    size_t ret;
    if (nmemb > FCD_PRINT_MAX) {
        ret = fwrite (buf, size, FCD_PRINT_MAX, file);
        fprintf (file, FCD_OUTPUT_CUT_MARKER);
    } else
        ret = fwrite (buf, size, nmemb, file);
    return ret;
}

static void print_arg (FILE *file, fcd_arg const *arg) {
    enum arg_type type = arg->type;
    fprintf (file, "%s=", arg->name);

    switch (type) {
    case STRUCT:
        arg->printer.print_innards (arg, file);
        return;
    
    case RAW_BYTES:
        fprintf (file, "\"");
        fcd_fwrite (arg->value, 1, arg->printer.size, file);
        fprintf (file, "\"");
        return;

    case STRING:
        if (arg->value == NULL)
            fprintf (file, "NULL");
        else
            fprintf (file, format_specifier[STRING], 
                     (const char *)arg->value);
        return;

#   pragma GCC diagnostic push
#   pragma GCC diagnostic ignored "-Wpointer-to-int-cast"
#   define CASE(TYPE)                               \
    case TYPE##_ty:                                 \
        fprintf (file, format_specifier[TYPE##_ty], \
                    (TYPE)(arg->value));            \
        break

        CASE(char); CASE(int16_t); CASE(uint16_t); CASE(int32_t);
        CASE(uint32_t); CASE(int64_t); CASE(uint64_t); CASE(fcd_pointer);

#   undef CASE
#   define CASE(TYPE) {                                     \
    case TYPE##_ty:                                         \
        TYPE tmp;                                           \
        memcpy (&tmp, (arg->value), sizeof(TYPE)); \
        fprintf (file, format_specifier[TYPE##_ty], tmp);   \
        break; }

        CASE(float) CASE(double)

#   undef CASE
#   pragma GCC diagnostic pop

    default:
        fprintf (stderr, "Incorrect arg type: %d\n", type);
        fprintf (file, "???");
        return;
    }
}
void fcd_call_dump (FILE *file, fcd_call const *call) {
    fprintf (file, "%s(", call->name);

    for (int i = 0; i <= call->arg_count - 1; ++i) {
        print_arg (file, call->args + i);
        if (__glibc_likely(i < call->arg_count - 1))
            fprintf (file, ", ");
    }
    
    fprintf (file, ")");
    if (call->return_value) {
        fprintf (file, " ");
        print_arg (file, call->return_value);
    }
    fprintf (file, ";\n");
}



#define DUMP_ARG(arg, arg_format)  #arg "=" arg_format 
#define DUMP_CALL_0(file, func_name)                                         \
    fprintf (file, func_name " ()\n")
#define DUMP_CALL_1(file, func_name, arg1, arg1f)                           \
    fprintf (file, func_name " (" DUMP_ARG(arg1, arg1f) ")\n",              \
                   arg1)
#define DUMP_CALL_2(file, func_name, arg1, arg1f, arg2, arg2f)              \
    fprintf (file, func_name " (" DUMP_ARG(arg1, arg1f) ", "                \
                                  DUMP_ARG(arg2, arg2f) ")\n",              \
                   arg1, arg2)
#define DUMP_CALL_3(file, func_name, arg1, arg1f, arg2, arg2f, arg3, arg3f)  \
    fprintf (file, func_name " (" DUMP_ARG(arg1, arg1f) ", "                 \
                                  DUMP_ARG(arg2, arg2f) ", "                 \
                                  DUMP_ARG(arg3, arg3f) ")\n",               \
                   arg1, arg2, arg3)
#define DUMP_CALL_4(file, func_name, arg1, arg1f, arg2, arg2f, arg3, arg3f, \
                                     arg4, arg4f)                           \
    fprintf (file, func_name " (" DUMP_ARG(arg1, arg1f) ", "                \
                                  DUMP_ARG(arg2, arg2f) ", "                \
                                  DUMP_ARG(arg3, arg3f) ", "                \
                                  DUMP_ARG(arg4, arg4f) ")\n",              \
                   arg1, arg2, arg3, arg4)
#define DUMP_CALL_5(file, func_name, arg1, arg1f, arg2, arg2f, arg3, arg3f,  \
                                     arg4, arg4f, arg5, arg5f)               \
    fprintf (file, func_name " (" DUMP_ARG(arg1, arg1f) ", "                 \
                                  DUMP_ARG(arg2, arg2f) ", "                 \
                                  DUMP_ARG(arg3, arg3f) ", "                 \
                                  DUMP_ARG(arg4, arg4f) ", "                 \
                                  DUMP_ARG(arg5, arg5f) ")\n",               \
                   arg1, arg2, arg3, arg4, arg5)
#define DUMP_CALL_6(file, func_name, arg1, arg1f, arg2, arg2f, arg3, arg3f, \
                                     arg4, arg4f, arg5, arg5f, arg6, arg6f) \
    fprintf (file, func_name " (" DUMP_ARG(arg1, arg1f) ", "                \
                                  DUMP_ARG(arg2, arg2f) ", "                \
                                  DUMP_ARG(arg3, arg3f) ", "                \
                                  DUMP_ARG(arg4, arg4f) ", "                \
                                  DUMP_ARG(arg5, arg5f) ", "                \
                                  DUMP_ARG(arg6, arg6f) ")\n",              \
                   arg1, arg2, arg3, arg4, arg5, arg6)


#endif // !FUNCTION_CALL_DUMP_H
