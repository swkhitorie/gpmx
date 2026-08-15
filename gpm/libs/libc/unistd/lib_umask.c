/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <gpmx/config.h>

#include <sys/stat.h>
#include <sys/types.h>

static mode_t g_umask;

mode_t umask(mode_t mask)
{
    mode_t prev;
    prev = g_umask;
    g_umask = mask;

    return prev;
}

mode_t getumask(void)
{
    return g_umask;
}

