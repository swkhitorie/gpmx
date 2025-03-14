#ifndef DEV_OPS_SPI_H_
#define DEV_OPS_SPI_H_

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define SPI_LOCK(d,l) (d)->ops->lock(d,l)

#define SPI_SELECT(d,id,s) ((d)->ops->select(d,id,s))

#define SPI_SETFREQUENCY(d,f) ((d)->ops->setfrequency(d,f))

#define SPI_SETMODE(d,m) \
    do { if ((d)->ops->setmode) (d)->ops->setmode(d,m); } while (0)

#define SPI_SETBITS(d,b) \
    do { if ((d)->ops->setbits) (d)->ops->setbits(d,b); } while (0)

#ifdef CONFIG_SPI_EXCHANGE
#define SPI_SNDBLOCK(d,b,l) ((d)->ops->exchange(d,b,0,l))
#else
#define SPI_SNDBLOCK(d,b,l) ((d)->ops->sndblock(d,b,l))
#endif

#ifdef CONFIG_SPI_EXCHANGE
#define SPI_RECVBLOCK(d,b,l) ((d)->ops->exchange(d,0,b,l))
#else
#define SPI_RECVBLOCK(d,b,l) ((d)->ops->recvblock(d,b,l))
#endif

#ifdef CONFIG_SPI_EXCHANGE
#define SPI_EXCHANGE(d,t,r,l) ((d)->ops->exchange(d,t,r,l))
#endif

enum spi_mode_e
{
    SPIDEV_MODE0 = 0,     /* CPOL=0 CHPHA=0 */
    SPIDEV_MODE1,         /* CPOL=0 CHPHA=1 */
    SPIDEV_MODE2,         /* CPOL=1 CHPHA=0 */
    SPIDEV_MODE3,         /* CPOL=1 CHPHA=1 */
};

struct spi_dev_s;
struct spi_ops_s
{
    int (*setup)(struct spi_dev_s *dev);

    uint32_t (*setfrequency)(struct spi_dev_s *dev, uint32_t frequency);

    void (*setmode)(struct spi_dev_s *dev, enum spi_mode_e mode);

    void (*setbits)(struct spi_dev_s *dev, int nbits);

    int (*lock)(struct spi_dev_s *dev, bool lock);

    int (*select)(struct spi_dev_s *dev, uint32_t devid, bool selected);

    int (*exchange)(struct spi_dev_s *dev,
        const void *txbuffer, void *rxbuffer, size_t nwords);

    int (*sndblock)(struct spi_dev_s *dev, const void *buffer, size_t nwords);

    int (*recvblock)(struct spi_dev_s *dev, void *buffer, size_t nwords);
};

struct spi_dev_s
{
    uint32_t frequency;
    enum spi_mode_e mode;
    uint8_t nbits;

    /* Driver interface */
    const struct spi_ops_s *ops;  /* Arch-specific operations */
    void                   *priv; /* Used by the arch-specific logic */
};

#ifdef cplusplus
extern "C" {
#endif

int spi_register(const char *path, struct spi_dev_s *dev);

#ifdef cplusplus
}
#endif

#endif
