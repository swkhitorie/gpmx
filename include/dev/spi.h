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
  CODE int      (*lock)(FAR struct spi_dev_s *dev, bool lock);
  CODE void     (*select)(FAR struct spi_dev_s *dev, uint32_t devid,
                  bool selected);
  CODE uint32_t (*setfrequency)(FAR struct spi_dev_s *dev, uint32_t frequency);

  CODE uint16_t (*send)(FAR struct spi_dev_s *dev, uint16_t wd);

  CODE void     (*exchange)(FAR struct spi_dev_s *dev,
                  FAR const void *txbuffer, FAR void *rxbuffer,
                  size_t nwords);

  CODE void     (*sndblock)(FAR struct spi_dev_s *dev,
                  FAR const void *buffer, size_t nwords);
  CODE void     (*recvblock)(FAR struct spi_dev_s *dev, FAR void *buffer,
                  size_t nwords);
};

struct spi_dev_s
{
    const struct spi_ops_s *ops;
};

#endif
