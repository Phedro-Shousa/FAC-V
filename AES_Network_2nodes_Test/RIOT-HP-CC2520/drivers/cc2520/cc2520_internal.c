/*
 * Copyright (C) 2015-2016 Freie Universität Berlin
 *               2016 Inria
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     drivers_cc2520
 * @{
 *
 * @file
 * @brief       Implementation of driver internal functions
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 * @author      Francisco Acosta <francisco.acosta@inria.fr>
 *
 * @}
 */

#include "periph/spi.h"
#include "periph/gpio.h"
#include "xtimer.h"

#include "cc2520_internal.h"
#include "cc2520_registers.h"

#define SPI_BUS             (dev->params.spi)
#define SPI_CS              (dev->params.pin_cs)
#define SPI_MODE            (SPI_MODE_0)
#define SPI_CLK             (dev->params.spi_clk)


uint8_t cc2520_strobe(const cc2520_t *dev, const uint8_t command)
{
    uint8_t res;

    spi_acquire(SPI_BUS, SPI_CS, SPI_MODE, SPI_CLK);
    res = spi_transfer_byte(SPI_BUS, SPI_CS, false, command);
    spi_release(SPI_BUS);

    return res;
}

void cc2520_reg_write(const cc2520_t *dev,
                      const uint8_t addr,
                      const uint8_t value)
{
    uint8_t tmp[] = { (CC2520_INS_MEMWR  | (addr >> 8 & 0xff)),
                      (addr & 0xff),
                      (uint8_t) value };
        
    spi_acquire(SPI_BUS, SPI_CS, SPI_MODE, SPI_CLK);
    spi_transfer_bytes(SPI_BUS, SPI_CS, false, tmp, NULL, 3);    
    spi_release(SPI_BUS);
}

uint8_t cc2520_reg_read(const cc2520_t *dev, const uint8_t addr)
{
  uint8_t res;

    uint8_t tmp[] = { (CC2520_INS_MEMRD  | (addr >> 8 & 0xff)),
                      (addr & 0xff)};
    
    spi_acquire(SPI_BUS, SPI_CS, SPI_MODE, SPI_CLK);    
    spi_transfer_bytes(SPI_BUS, SPI_CS, true, tmp, NULL, 2);
    spi_transfer_bytes(SPI_BUS, SPI_CS, false, NULL, &res, 1); 
    spi_release(SPI_BUS);
    
    return res;
}

void cc2520_ram_read(const cc2520_t *dev, const uint16_t addr,
                     uint8_t *data, const size_t len)
{
    uint8_t tmp[] = { (CC2520_INS_MEMRD | (addr >> 8 & 0xff)),
                      (addr & 0xff) };

    spi_acquire(SPI_BUS, SPI_CS, SPI_MODE, SPI_CLK);
    
    spi_transfer_bytes(SPI_BUS, SPI_CS, true, tmp, NULL, 2);    
    spi_transfer_bytes(SPI_BUS, SPI_CS, false, NULL, data, len);
    
    spi_release(SPI_BUS);
}

void cc2520_ram_write(const cc2520_t *dev, const uint16_t addr,
                      const uint8_t *data, const size_t len)
{
    uint8_t tmp[] = { (CC2520_INS_MEMWR | (addr >> 8 & 0xff)),
                      (addr & 0xff) };

    spi_acquire(SPI_BUS, SPI_CS, SPI_MODE, SPI_CLK);
    spi_transfer_bytes(SPI_BUS, SPI_CS, true, tmp, NULL, 2);
    spi_transfer_bytes(SPI_BUS, SPI_CS, false, data, NULL, len);
    spi_release(SPI_BUS);
}

void cc2520_fifo_read(const cc2520_t *dev, uint8_t *data, const size_t len)
{
    uint8_t tmp[] = {CC2520_INS_RXBUF};
      
    spi_acquire(SPI_BUS, SPI_CS, SPI_MODE, SPI_CLK);
    spi_transfer_bytes(SPI_BUS, SPI_CS, true, tmp, NULL, 1);
    spi_transfer_bytes(SPI_BUS, SPI_CS, false, NULL, data, len);
    spi_release(SPI_BUS);
}

void cc2520_fifo_write(const cc2520_t *dev, uint8_t *data, const size_t len)
{
    uint8_t tmp[] = {CC2520_INS_TXBUF};
    spi_acquire(SPI_BUS, SPI_CS, SPI_MODE, SPI_CLK);
    spi_transfer_bytes(SPI_BUS, SPI_CS, true, tmp, NULL, 1);
    spi_transfer_bytes(SPI_BUS, SPI_CS, false, data, NULL, len);
    spi_release(SPI_BUS);
}

void cc2520_flush_rx(const cc2520_t *dev)
{
    uint8_t dummy;
    
    spi_acquire(SPI_BUS, SPI_CS, SPI_MODE, SPI_CLK);
    /* read and discard dummy */
    spi_transfer_bytes(SPI_BUS, SPI_CS, true, NULL, &dummy, 1);   
    spi_release(SPI_BUS);
    
    cc2520_strobe(dev, CC2520_INS_SFLUSHRX);
    cc2520_strobe(dev, CC2520_INS_SFLUSHRX);
}

                      
uint8_t cc2520_status(cc2520_t *dev)
{
    return cc2520_strobe(dev, CC2520_INS_SNOP);
}

uint8_t cc2520_state(cc2520_t *dev)
{
    return (uint8_t)cc2520_reg_read(dev, CC2520_FSMSTAT0);
}

void cc2520_en_xosc(cc2520_t *dev)
{
    cc2520_strobe(dev, CC2520_INS_SXOSCON);
    xtimer_usleep(CC2520_XOSCON_DELAY);
}
