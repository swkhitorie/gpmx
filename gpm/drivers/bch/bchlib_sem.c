#include <errno.h>
#include "bch.h"

int bchlib_semtake(struct bchlib_s *bch)
{
    return sem_wait(&bch->sem);
}
