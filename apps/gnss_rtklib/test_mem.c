#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <inttypes.h>
#include <string.h>
#include <stdlib.h>
#include "rtxmem.h"

/* ============================================================
 * 跨位宽可移植辅助
 * ============================================================ */

/* 平台位宽编译期检查：只允许 32/64 位 */
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(void *) == 4 || sizeof(void *) == 8,
               "rtxmem_test: only 32-bit or 64-bit platforms supported");
#else
typedef char _rtxmem_platform_check[
    (sizeof(void *) == 4 || sizeof(void *) == 8) ? 1 : -1];
#endif

/* 8 字节对齐属性（GCC/Clang / MSVC / C11 三选一） */
#if defined(__GNUC__) || defined(__clang__)
#  define ALIGNAS_8   __attribute__((aligned(8)))
#elif defined(_MSC_VER)
#  define ALIGNAS_8   __declspec(align(8))
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
#  include <stdalign.h>
#  define ALIGNAS_8   alignas(8)
#else
#  define ALIGNAS_8
#endif

/* size_t 打印格式：默认 C99 %zu；极简 libc 可定义 RTXMEM_USE_LU_FMT 回退 */
#if defined(RTXMEM_USE_LU_FMT)
#  define SZ_FMT(x)   ((unsigned long)(x))
#  define SZ_STR      "%lu"
#else
#  define SZ_FMT(x)   (x)
#  define SZ_STR      "%zu"
#endif

static int g_pass, g_fail;

#define TEST_ASSERT(cond) do { \
    if (cond) { g_pass++; } \
    else { \
        printf("  [FAIL] %s:%d: %s\n", __FILE__, __LINE__, #cond); \
        g_fail++; \
    } \
} while (0)

static size_t align8(size_t x) { return (x + 7U) & ~(size_t)7U; }

/* ============================================================
 * 平台信息 + 布局自检
 * ============================================================ */

static void print_platform(void)
{
    printf("=== Platform Info ===\n");
    printf("  pointer bits        = " SZ_STR "\n",
           SZ_FMT(sizeof(void *) * 8));
    printf("  sizeof(size_t)      = " SZ_STR "\n",
           SZ_FMT(sizeof(size_t)));
    printf("  sizeof(mem_head_t)  = " SZ_STR "\n",
           SZ_FMT(sizeof(mem_head_t)));
    printf("  sizeof(mem_block_t) = " SZ_STR "\n",
           SZ_FMT(sizeof(mem_block_t)));
    printf("  MB_INFO_TYPE_MASK   = 0x%" PRIxPTR "\n",
           (uintptr_t)MB_INFO_TYPE_MASK);
    printf("  MB_INFO_LEN_MASK    = 0x%" PRIxPTR "\n",
           (uintptr_t)MB_INFO_LEN_MASK);
    printf("  info length bits    = " SZ_STR "\n",
           SZ_FMT(sizeof(size_t) * 8 - 2));
    printf("  SIZE_MAX            = 0x%" PRIxPTR "\n",
           (uintptr_t)SIZE_MAX);
    printf("\n");
}

static void test_platform_layout(void)
{
    printf("=== test_platform_layout ===\n");

    /* mem_head_t = { size_t size; size_t used; } → 2 * sizeof(size_t) */
    TEST_ASSERT(sizeof(mem_head_t) == 2 * sizeof(size_t));
    TEST_ASSERT(sizeof(mem_head_t) % 8 == 0);

    /* mem_block_t = { void *next; size_t info; }
     * 标准 ABI 下：32 位 4+4=8；64 位 8+8=16
     * 加 >= 检查兜底非常规 ABI
     */
    TEST_ASSERT(sizeof(mem_block_t) >= 2 * sizeof(void *));
    TEST_ASSERT(sizeof(mem_block_t) % 8 == 0);

#if UINTPTR_MAX == 0xFFFFFFFFU
    TEST_ASSERT(sizeof(mem_head_t) == 8);
    TEST_ASSERT(sizeof(mem_block_t) == 8);
    TEST_ASSERT(sizeof(size_t) == 4);
#elif UINTPTR_MAX == 0xFFFFFFFFFFFFFFFFULL
    TEST_ASSERT(sizeof(mem_head_t) == 16);
    TEST_ASSERT(sizeof(mem_block_t) == 16);
    TEST_ASSERT(sizeof(size_t) == 8);
#endif
    printf("\n");
}

/* ============================================================
 * 功能测试
 * ============================================================ */

static void test_init(void)
{
    static uint8_t pool[2048] ALIGNAS_8;
    const size_t init_used = sizeof(mem_head_t) + sizeof(mem_block_t);

    printf("=== test_init ===\n");
    TEST_ASSERT(rtx_mem_init(NULL, 1024) == 0);         /* NULL */
    TEST_ASSERT(rtx_mem_init(pool + 1, 1024) == 0);     /* 非 8 对齐 */
    TEST_ASSERT(rtx_mem_init(pool, 1023) == 0);         /* size 非 8 对齐 */
    TEST_ASSERT(rtx_mem_init(pool, 8) == 0);            /* 太小 */
    TEST_ASSERT(rtx_mem_init(pool, sizeof(pool)) == 1);
    TEST_ASSERT(rtx_mem_used(pool) == init_used);
    printf("  init_used = " SZ_STR "\n\n", SZ_FMT(init_used));
}

static void test_alloc_free_basic(void)
{
    static uint8_t pool[2048] ALIGNAS_8;
    const size_t init_used = sizeof(mem_head_t) + sizeof(mem_block_t);

    printf("=== test_alloc_free_basic ===\n");
    TEST_ASSERT(rtx_mem_init(pool, sizeof(pool)) == 1);

    void *a = rtx_mem_alloc(pool, 100, 0);
    void *b = rtx_mem_alloc(pool, 200, 1);
    void *c = rtx_mem_alloc(pool, 50,  2);
    TEST_ASSERT(a && b && c);

    /* 对齐 */
    TEST_ASSERT(((uintptr_t)a & 7U) == 0);
    TEST_ASSERT(((uintptr_t)b & 7U) == 0);
    TEST_ASSERT(((uintptr_t)c & 7U) == 0);

    /* 不重叠：a 的数据区结尾不越过 b */
    TEST_ASSERT((uint8_t *)a + 100 < (uint8_t *)b);
    TEST_ASSERT((uint8_t *)b + 200 < (uint8_t *)c);

    /* used 精确增加 = align8(size + sizeof(mem_block_t)) */
    size_t bs_a = align8(100 + sizeof(mem_block_t));
    size_t bs_b = align8(200 + sizeof(mem_block_t));
    size_t bs_c = align8(50  + sizeof(mem_block_t));
    TEST_ASSERT(rtx_mem_used(pool) == init_used + bs_a + bs_b + bs_c);

    /* 写入/回读，验证不越界 */
    memset(a, 0xAA, 100);
    memset(b, 0xBB, 200);
    memset(c, 0xCC, 50);
    int ok = 1;
    for (size_t i = 0; i < 100; i++) if (((uint8_t *)a)[i] != 0xAA) ok = 0;
    for (size_t i = 0; i < 200; i++) if (((uint8_t *)b)[i] != 0xBB) ok = 0;
    for (size_t i = 0; i < 50;  i++) if (((uint8_t *)c)[i] != 0xCC) ok = 0;
    TEST_ASSERT(ok);

    TEST_ASSERT(rtx_mem_free(pool, a) == 1);
    TEST_ASSERT(rtx_mem_free(pool, b) == 1);
    TEST_ASSERT(rtx_mem_free(pool, c) == 1);
    TEST_ASSERT(rtx_mem_used(pool) == init_used);

    /* 全部释放后 first-fit 应回到 a 的位置 */
    void *d = rtx_mem_alloc(pool, 100, 0);
    TEST_ASSERT(d == a);
    rtx_mem_free(pool, d);
    printf("\n");
}

static void test_alloc_errors(void)
{
    static uint8_t pool[1024] ALIGNAS_8;

    printf("=== test_alloc_errors ===\n");
    TEST_ASSERT(rtx_mem_init(pool, sizeof(pool)) == 1);

    TEST_ASSERT(rtx_mem_alloc(NULL, 10, 0) == NULL);            /* mem=NULL */
    TEST_ASSERT(rtx_mem_alloc(pool, 0, 0) == NULL);             /* size=0 */
    TEST_ASSERT(rtx_mem_alloc(pool, 10, 0x4) == NULL);          /* type 越界 */
    TEST_ASSERT(rtx_mem_alloc(pool, SIZE_MAX, 0) == NULL);      /* 溢出保护 */
    TEST_ASSERT(rtx_mem_alloc(pool, 4096, 0) == NULL);          /* 超过池容量 */
    printf("\n");
}

/* 32/64 位边界：溢出保护检查位于池容量检查之前，
 * 这两条断言只验证"不会 UB / 不会崩溃"，返回值均为 NULL */
static void test_size_limits(void)
{
    static uint8_t pool[256] ALIGNAS_8;

    printf("=== test_size_limits ===\n");
    TEST_ASSERT(rtx_mem_init(pool, sizeof(pool)) == 1);

    /* 刚好通过溢出检查，但池太小 → NULL（走容量检查） */
    TEST_ASSERT(rtx_mem_alloc(pool,
        SIZE_MAX - sizeof(mem_block_t) - 7U, 0) == NULL);
    /* 触发溢出保护 → NULL（走溢出检查） */
    TEST_ASSERT(rtx_mem_alloc(pool,
        SIZE_MAX - sizeof(mem_block_t) - 6U, 0) == NULL);
    printf("\n");
}

static void test_free_errors(void)
{
    static uint8_t pool[1024] ALIGNAS_8;

    printf("=== test_free_errors ===\n");
    TEST_ASSERT(rtx_mem_init(pool, sizeof(pool)) == 1);

    TEST_ASSERT(rtx_mem_free(NULL, (void *)(uintptr_t)0x1000) == 0);
    TEST_ASSERT(rtx_mem_free(pool, NULL) == 0);

    void *a = rtx_mem_alloc(pool, 64, 0);
    void *b = rtx_mem_alloc(pool, 64, 0);
    TEST_ASSERT(a && b);
    TEST_ASSERT(rtx_mem_free(pool, b) == 1);
    TEST_ASSERT(rtx_mem_free(pool, b) == 0);   /* 重复 free（非首块） */
    TEST_ASSERT(rtx_mem_free(pool, a) == 1);
    printf("\n");
}

static void test_max_used(void)
{
    static uint8_t pool[2048] ALIGNAS_8;

    printf("=== test_max_used ===\n");
    TEST_ASSERT(rtx_mem_init(pool, sizeof(pool)) == 1);

    size_t init_used = rtx_mem_used(pool);
    void  *a = rtx_mem_alloc(pool, 500, 0);
    TEST_ASSERT(a != NULL);

    size_t peak = rtx_mem_used(pool);
    TEST_ASSERT(peak > init_used);

    TEST_ASSERT(rtx_mem_free(pool, a) == 1);
    TEST_ASSERT(rtx_mem_used(pool) < peak);

    void *b = rtx_mem_alloc(pool, 100, 0);
    TEST_ASSERT(b == a);
    rtx_mem_free(pool, b);
    printf("\n");
}

static void test_fragmentation(void)
{
    static uint8_t pool[2048] ALIGNAS_8;

    printf("=== test_fragmentation ===\n");
    TEST_ASSERT(rtx_mem_init(pool, sizeof(pool)) == 1);

    void *a = rtx_mem_alloc(pool, 100, 0);
    void *b = rtx_mem_alloc(pool, 100, 0);
    void *c = rtx_mem_alloc(pool, 100, 0);
    TEST_ASSERT(a && b && c);

    TEST_ASSERT(rtx_mem_free(pool, b) == 1);

    /* first-fit 从 a 末尾分配，正好复用 b 的 header 位置 */
    void *d = rtx_mem_alloc(pool, 100, 0);
    printf("  b=%p d=%p\n", b, d);
    TEST_ASSERT(d == b);

    TEST_ASSERT(rtx_mem_free(pool, a) == 1);
    TEST_ASSERT(rtx_mem_free(pool, d) == 1);
    TEST_ASSERT(rtx_mem_free(pool, c) == 1);
    TEST_ASSERT(rtx_mem_used(pool) == sizeof(mem_head_t) + sizeof(mem_block_t));
    printf("\n");
}

static void test_exhaust(void)
{
    static uint8_t pool[1024] ALIGNAS_8;

    printf("=== test_exhaust ===\n");
    TEST_ASSERT(rtx_mem_init(pool, sizeof(pool)) == 1);

    /* init 后第一个 block 与最后一个 block header 之间的 hole
     * hole = size - sizeof(mem_head_t) - sizeof(mem_block_t)
     * alloc 的 block_size = align8(req + sizeof(mem_block_t))
     * 因此能分配的最大 req = hole - sizeof(mem_block_t)
     * （hole、sizeof(mem_block_t) 都是 8 的倍数，align8 后恰好等于 hole）
     * 32 位：hole=1008，max_req=1000
     * 64 位：hole= 976，max_req= 960
     */
    size_t hole    = sizeof(pool) - sizeof(mem_head_t) - sizeof(mem_block_t);
    size_t max_req = hole - sizeof(mem_block_t);
    printf("  hole=" SZ_STR " max_req=" SZ_STR "\n",
           SZ_FMT(hole), SZ_FMT(max_req));

    void *a = rtx_mem_alloc(pool, max_req, 0);
    TEST_ASSERT(a != NULL);
    TEST_ASSERT(rtx_mem_alloc(pool, 1, 0) == NULL);   /* 再多 1 字节就失败 */
    TEST_ASSERT(rtx_mem_free(pool, a) == 1);
    printf("\n");
}

static void test_stress(void)
{
    static uint8_t pool[4096] ALIGNAS_8;
    void  *blocks[16] = {0};

    printf("=== test_stress ===\n");
    TEST_ASSERT(rtx_mem_init(pool, sizeof(pool)) == 1);

    srand(1234);
    for (int round = 0; round < 2000; round++) {
        size_t idx = (size_t)(rand() % 16);
        if (blocks[idx]) {
            TEST_ASSERT(rtx_mem_free(pool, blocks[idx]) == 1);
            blocks[idx] = NULL;
        } else {
            blocks[idx] = rtx_mem_alloc(pool,
                                        (size_t)(16 + rand() % 64),
                                        (uint32_t)(rand() % 4));
            /* 池小可能失败，不视为错误 */
        }
    }
    for (size_t i = 0; i < sizeof(blocks) / sizeof(blocks[0]); i++) {
        if (blocks[i]) TEST_ASSERT(rtx_mem_free(pool, blocks[i]) == 1);
    }
    TEST_ASSERT(rtx_mem_used(pool) == sizeof(mem_head_t) + sizeof(mem_block_t));
    printf("\n");
}

static void test_mask_width(void)
{
    printf("=== test_mask_width ===\n");
    /* 低 2 位是 type，其余为 len */
    TEST_ASSERT((MB_INFO_LEN_MASK & (size_t)0x3U) == 0);
    /* 两个掩码并集覆盖全部有效位 */
    TEST_ASSERT((MB_INFO_LEN_MASK | MB_INFO_TYPE_MASK) == SIZE_MAX);
    /* 掩码位宽随 size_t 自动适配 */
    TEST_ASSERT(MB_INFO_LEN_MASK == (size_t)~(size_t)3U);
    printf("\n");
}

int test_mem_main(void)
{
    print_platform();
    test_platform_layout();
    test_init();
    test_alloc_free_basic();
    test_alloc_errors();
    test_size_limits();
    test_free_errors();
    test_max_used();
    test_fragmentation();
    test_exhaust();
    test_stress();
    test_mask_width();

    printf("=== Result: %d passed, %d failed ===\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
