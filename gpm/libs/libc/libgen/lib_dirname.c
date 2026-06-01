
#include <string.h>
#include <libgen.h>

char *dirname(char *path)
{
    char *p;
    int  len;

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
    * therefore, must be the end of the directory component.
    */

    p = strrchr(path, '/');
    if (p) {
        do {
            /* Handle the case where the only '/' in the string is the at the
            * beginning of the path.
            */

            if (p == path) {
                return "/";
            }

            /* No, the directory component is the substring before the '/'. */
            *p-- = '\0';
        } while (*p == '/');

        return path;
    }

    /* There is no '/' in the path */
    return ".";
}
