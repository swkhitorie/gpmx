#ifndef GMSH_H_
#define GMSH_H_

#include <stdint.h>
#include <stdbool.h>

#if defined(__CC_ARM) || defined(__clang__) || defined(__GNUC__)
#define GMSH_SECTION(x) __attribute__((section(x)))
#define GMSH_USED       __attribute__((used))
#define GMSH_WEAK       __attribute__((weak))
#elif defined(__IAR_SYSTEMS_ICC__)
#define GMSH_SECTION(x) @x
#define GMSH_USED       __root
#define GMSH_WEAK       __weak
#else
#define GMSH_SECTION(x)
#define GMSH_USED
#define GMSH_WEAK
#endif

#ifdef _MSC_VER
#pragma section("FSymTab$f",read)
#endif

#ifdef __TI_COMPILER_VERSION__
#define __TI_FINSH_EXPORT_FUNCTION(f)  PRAGMA(DATA_SECTION(f,"FSymTab"))
#endif

typedef int (*gmsh_syscall_func)(int argc, char **argv);

#if defined(_MSC_VER)
#define MSH_FUNCTION_EXPORT_CMD(name, cmd, desc)      \
    const char __fsym_##cmd##_name[] = #cmd;          \
    const char __fsym_##cmd##_desc[] = #desc;         \
    __declspec(allocate("FSymTab$f"))                 \
    const struct gmsh_syscall __fsym_##cmd =          \
    {                                \
        __fsym_##cmd##_name,         \
        __fsym_##cmd##_desc,         \
        (gmsh_syscall_func)&name     \
    };

#pragma comment(linker, "/merge:FSymTab=mytext")

#elif defined(__TI_COMPILER_VERSION__)
#define MSH_FUNCTION_EXPORT_CMD(name, cmd, desc)      \
    __TI_FINSH_EXPORT_FUNCTION(__fsym_##cmd);         \
    const char __fsym_##cmd##_name[] = #cmd;          \
    const char __fsym_##cmd##_desc[] = #desc;         \
    const struct gmsh_syscall __fsym_##cmd =          \
    {                                \
        __fsym_##cmd##_name,         \
        __fsym_##cmd##_desc,         \
        (gmsh_syscall_func)&name     \
    };

#else

#define MSH_FUNCTION_EXPORT_CMD(name, cmd, desc)                              \
    const char __fsym_##cmd##_name[] GMSH_SECTION(".rodata.name") = #cmd;     \
    const char __fsym_##cmd##_desc[] GMSH_SECTION(".rodata.name") = #desc;    \
    GMSH_USED const struct gmsh_syscall __fsym_##cmd GMSH_SECTION("FSymTab")= \
    {                                \
        __fsym_##cmd##_name,         \
        __fsym_##cmd##_desc,         \
        (gmsh_syscall_func)&name     \
    };

#endif

struct gmsh_syscall {
    const char     *name;
    const char     *desc;
    gmsh_syscall_func func;
};

extern struct gmsh_syscall *_syscall_table_begin;
extern struct gmsh_syscall *_syscall_table_end;

#if defined(_MSC_VER) || (defined(__GNUC__) && defined(__x86_64__))
#define GMSH_NEXT_SYSCALL(index)  index=gmsh_syscall_next(index)
#else
#define GMSH_NEXT_SYSCALL(index)  index++
#endif

/**
 * interface example:
 *  char gmsh_console_getchar() {
 *      char c = (char)(-1);
 *      int ret = board_stream_in(0, &c, 1);
 *      if (ret != 1) task_delayms(10);
 *      return c;
 *  }
 *  void gsh_kprintf(const char *format, ...) {
 *      send loop:
 *         if (*(format+i) == '\n') {
 *             send "\r\n"
 *         }
 *         .....
 *  }
 */
#ifdef __cplusplus
extern "C" {
#endif

#if defined(_MSC_VER) || (defined(__GNUC__) && defined(__x86_64__))
struct gmsh_syscall *gmsh_syscall_next(struct gmsh_syscall *call);
#endif

GMSH_WEAK int gsh_kprintf(const char *format, ...);

#ifdef __cplusplus
}
#endif

#endif
