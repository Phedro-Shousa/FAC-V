/*
 * Copyright (C) 2014 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     examples
 * @{
 *
 * @file
 * @brief       Hello World application
 *
 * @author      Kaspar Schleiser <kaspar@schleiser.de>
 * @author      Ludwig Knüpfer <ludwig.knuepfer@fu-berlin.de>
 *
 * @}
 */

#include <stdio.h>
#include <string.h>
#include "net/netdev.h"
#include "net/netdev/ieee802154.h"
#include <stdlib.h>
#include "cpu.h"
#include "periph_conf.h"
#include "periph/gpio.h"
#include "periph/spi.h"

#include "cc2520.h"
#include "cc2520_params.h"
#include "net/netdev.h"
#include "net/netdev/ieee802154.h"
#define MODULE_CC2520 1

#define BENCH_REDOS             (1000)
#define BENCH_SMALL             (2)
#define BENCH_LARGE             (100)
#define BENCH_PAYLOAD           ('b')
#define BENCH_REGADDR           (0x23)

#define BUF_SIZE                (512U)

#define cc2520_NUMOF        ARRAY_SIZE(cc2520_params)

static struct {
    spi_t dev;
    spi_mode_t mode;
    spi_clk_t clk;
    spi_cs_t cs;
} spicong_s;


static cc2520_t cc2520_devs[cc2520_NUMOF];

int main(void)
{
    printf("Init SPI driver\n");

    int dev = 0;
    spicong_s.dev = SPI_DEV(dev);
    spicong_s.mode = SPI_MODE_0;
    spicong_s.clk = SPI_CLK_100KHZ;
    spicong_s.cs = (spi_cs_t)GPIO_PIN(1, 6);

    spi_init(dev);

    int tmp = spi_init_cs(spicong_s.dev, spicong_s.cs);
    if (tmp != SPI_OK) {
        printf("error: unable to initialize the given chip select line %i\n", tmp);
        return 1;
    }

    printf("Init CC2520\n");    
        for (unsigned i = 0; i < cc2520_NUMOF; i++) {
            cc2520_setup(&cc2520_devs[i], &cc2520_params[i]);
            cc2520_init(&cc2520_devs[i]);
        }
    
    printf("Sucess\n");

    return 0;
}
