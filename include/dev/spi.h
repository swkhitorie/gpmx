#ifndef DEV_OPS_SPI_H_
#define DEV_OPS_SPI_H_

#include <stdint.h>
#include <stdbool.h>

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
