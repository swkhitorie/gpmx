#include <drv_qspi.h>

void _qspi_pin_config(struct qspi_dev_s *dev)
{
    int ret = 0;
    struct up_qspi_dev_s *priv = dev->priv;
    struct periph_pin_t *ncspin = &priv->ncspin;
    struct periph_pin_t *sckpin = &priv->sckpin;
    struct periph_pin_t *io0pin = &priv->io0pin;
    struct periph_pin_t *io1pin = &priv->io1pin;
    struct periph_pin_t *io2pin = &priv->io2pin;
    struct periph_pin_t *io3pin = &priv->io3pin;

    LOW_PERIPH_INITPIN(ncspin->port, ncspin->pin, IOMODE_AFPP, IO_NOPULL, IO_SPEEDMAX, ncspin->alternate);
    LOW_PERIPH_INITPIN(sckpin->port, sckpin->pin, IOMODE_AFPP, IO_NOPULL, IO_SPEEDMAX, sckpin->alternate);
    LOW_PERIPH_INITPIN(io0pin->port, io0pin->pin, IOMODE_AFPP, IO_NOPULL, IO_SPEEDMAX, io0pin->alternate);
    LOW_PERIPH_INITPIN(io1pin->port, io1pin->pin, IOMODE_AFPP, IO_NOPULL, IO_SPEEDMAX, io1pin->alternate);
    LOW_PERIPH_INITPIN(io2pin->port, io2pin->pin, IOMODE_AFPP, IO_NOPULL, IO_SPEEDMAX, io2pin->alternate);
    LOW_PERIPH_INITPIN(io3pin->port, io3pin->pin, IOMODE_AFPP, IO_NOPULL, IO_SPEEDMAX, io3pin->alternate);
}
