
#include "libxx.hxx"


void *operator new[](unsigned int nbytes)
{
    if (nbytes < 1) {
        nbytes = 1;
    }

    void *alloc = lib_malloc(nbytes);
    return alloc;
}
