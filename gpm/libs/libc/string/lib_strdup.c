#include <string.h>
#include <driver/drv_sched.h>

char *strdup(const char *s)
{
    if (s == NULL) {
        return NULL;
    }

    char *news = (char *)kmm_malloc(strlen(s) + 1);

    if (news) {
        strcpy(news, s);
    }

    return news;
}
