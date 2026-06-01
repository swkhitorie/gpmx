#ifndef __DRIVERS_BCH_BCH_H
#define __DRIVERS_BCH_BCH_H

#include <sys/types.h>
#include <stdint.h>
#include <stdbool.h>

#include <semaphore.h>
#include <gpm/fs/fs.h>


#define bchlib_semgive(d) sem_post(&(d)->sem)  /* To match bchlib_semtake */
#define MAX_OPENCNT       (255)                  /* Limit of uint8_t */

struct bchlib_s
{
  struct inode *inode;     /* I-node of the block driver */
  uint32_t sectsize;       /* The size of one sector on the device */
  size_t nsectors;         /* Number of sectors supported by the device */
  size_t sector;           /* The current sector in the buffer */
  sem_t sem;               /* For atomic accesses to this structure */
  uint8_t refs;            /* Number of references */
  bool dirty;              /* true: Data has been written to the buffer */
  bool readonly;           /* true: Only read operations are supported */
  bool unlinked;           /* true: The driver has been unlinked */
  uint8_t *buffer;         /* One sector buffer */
};

#undef EXTERN
#if defined(__cplusplus)
#define EXTERN extern "C"
extern "C"
{
#else
#define EXTERN extern
#endif

const struct file_operations bch_fops;

int  bchlib_semtake(struct bchlib_s *bch);
int  bchlib_flushsector(struct bchlib_s *bch);
int  bchlib_readsector(struct bchlib_s *bch, size_t sector);

#undef EXTERN
#if defined(__cplusplus)
}
#endif

#endif
