#include <string.h>
#include <libgen.h>

char *basename(char *path)
{
    char *p;
    int len;

    /* Handle some corner cases */
    if (!path || *path == '\0') {
        return ".";
    }

    /* Check for trailing slash characters */
    len = strlen(path);

    while (path[len - 1] == '/') {
      /* Remove trailing '/' UNLESS this would make a zero length string */

        if (len > 1) {
            path[len - 1] = '\0';
            len--;
        } else {
            return "/";
        }
    }

    /* Get the address of the last '/' which is not at the end of the path and,
    * therefore, must be just before the beginning of the filename component.
    */

    p = strrchr(path, '/');
    if (p) {
        return p + 1;
    }

    /* There is no '/' in the path */

    return path;
}
