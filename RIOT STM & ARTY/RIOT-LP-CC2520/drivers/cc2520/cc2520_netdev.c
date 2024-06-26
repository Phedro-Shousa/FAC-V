/*
 * Copyright (C) 2015 Freie Universität Berlin
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
 * @brief       Netdev adaption for the cc2520 driver
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 * @author      Francisco Acosta <francisco.acosta@inria.fr>
 *
 * @}
 */

#include <string.h>
#include <assert.h>
#include <errno.h>

#include "net/eui64.h"
#include "net/ieee802154.h"
#include "net/netdev.h"
#include "net/netdev/ieee802154.h"
#include "xtimer.h"

#include "cc2520.h"
#include "cc2520_netdev.h"
#include "cc2520_internal.h"
#include "cc2520_registers.h"

#define ENABLE_DEBUG    (0)
#include "debug.h"

#define _MAX_MHR_OVERHEAD   (25)

static int _send(netdev_t *netdev, const iolist_t *iolist);
static int _recv(netdev_t *netdev, void *buf, size_t len, void *info);
static int _init(netdev_t *netdev);
static void _isr(netdev_t *netdev);
static int _get(netdev_t *netdev, netopt_t opt, void *val, size_t max_len);
static int _set(netdev_t *netdev, netopt_t opt, const void *val, size_t len);

const netdev_driver_t cc2520_driver = {
    .send = _send,
    .recv = _recv,
    .init = _init,
    .isr = _isr,
    .get = _get,
    .set = _set,
};

static void _irq_handler(void *arg)
{
    //printf("ISR_STM\n");    
    //printf("ISR_ARTY\n");    
    netdev_t *dev = (netdev_t *)arg;
    cc2520_t *dev_c = (cc2520_t *)dev;
    cc2520_clear_fifop_excep(dev_c);

    if(dev->event_callback) {
        dev->event_callback(dev, NETDEV_EVENT_ISR);
    }
}

static inline uint16_t to_u16(const void *buf)
{
    return *((const uint16_t *)buf);
}

static inline int16_t to_i16(const void *buf)
{
    return *((const int16_t *)buf);
}

static inline bool to_bool(const void *buf)
{
    return *((const bool *)buf);
}

static inline int w_u16(void *buf, uint16_t val)
{
    memcpy(buf, &val, sizeof(uint16_t));
    return sizeof(uint16_t);
}

static inline int w_i16(void *buf, int16_t val)
{
    memcpy(buf, &val, sizeof(int16_t));
    return sizeof(int16_t);
}

static inline int opt_state(void *buf, bool cond)
{
    *((netopt_enable_t *)buf) = !!(cond);
    return sizeof(netopt_enable_t);
}

static int _init(netdev_t *netdev)
{
    cc2520_t *dev = (cc2520_t *)netdev;

    uint8_t reg;

    /* initialize power and reset pins -> put the device into reset state */
    gpio_init(dev->params.pin_reset, GPIO_OUT);
    gpio_set(dev->params.pin_reset);
    gpio_init(dev->params.pin_vrefen, GPIO_OUT);
    gpio_clear(dev->params.pin_vrefen);

    /* initialize the input lines */
    gpio_init(dev->params.pin_cca, GPIO_IN);
    gpio_init(dev->params.pin_sfd, GPIO_IN);
    gpio_init(dev->params.pin_fifo, GPIO_IN);
    reg = gpio_init_int(dev->params.pin_fifop, GPIO_IN, GPIO_RISING, _irq_handler, dev);

    /* initialize the chip select line and the SPI bus */
    spi_init_cs(dev->params.spi, dev->params.pin_cs);

    /* power on and toggle reset */
    gpio_set(dev->params.pin_vrefen);
    gpio_clear(dev->params.pin_reset);
    xtimer_usleep(CC2520_RESET_DELAY);

    gpio_set(dev->params.pin_reset);
    xtimer_usleep(CC2520_RESET_DELAY);

    
    /* test the connection to the device by reading CHIP_ID register */
    reg = cc2520_reg_read(dev, CC2520_CHIPID);
    if (reg != CC2520_CHIPID_VAL){
      printf("\nid=0x%x\n", reg);
      DEBUG("cc2520: init: unable to communicate with device\n");
      return -1;
    } else if (reg == CC2520_CHIPID_VAL){
      DEBUG("cc2520: init: device replied with %#0x\n", reg);
    }

    /* turn on the oscillator and wait for it to be stable */
    cc2520_en_xosc(dev);
    if (!(cc2520_status(dev) & CC2520_XOSC16M_STABLE)) {
        DEBUG("cc2520: init: oscillator did not stabilize\n");
        return -1;
    }

#ifdef MODULE_NETSTATS_L2x
    memset(&netdev->stats, 0, sizeof(netstats_t));
#endif

    return cc2520_init((cc2520_t *)dev);
}

static void _isr(netdev_t *netdev)
{
    netdev->event_callback(netdev, NETDEV_EVENT_RX_COMPLETE);
}

static int _send(netdev_t *netdev, const iolist_t *iolist)
{
    cc2520_t *dev = (cc2520_t *)netdev;
    return (int)cc2520_send(dev, iolist);
}

static int _recv(netdev_t *netdev, void *buf, size_t len, void *info)
{
    cc2520_t *dev = (cc2520_t *)netdev;
    return (int)cc2520_rx(dev, buf, len, info);
}

static int _get(netdev_t *netdev, netopt_t opt, void *val, size_t max_len)
{
    if (netdev == NULL) {
        return -ENODEV;
    }

    cc2520_t *dev = (cc2520_t *)netdev;

    int ext = netdev_ieee802154_get(&dev->netdev, opt, val, max_len);
    if (ext > 0) {
        return ext;
    }

    switch (opt) {

        case NETOPT_ADDRESS:
            assert(max_len >= sizeof(uint16_t));
            cc2520_get_addr_short(dev, val);
            return sizeof(uint16_t);

        case NETOPT_ADDRESS_LONG:
            assert(max_len >= 8);
            cc2520_get_addr_long(dev, val);
            return 8;

        case NETOPT_NID:
            assert(max_len >= sizeof(uint16_t));
            return w_u16(val, cc2520_get_pan(dev));

        case NETOPT_CHANNEL:
            assert(max_len >= sizeof(uint16_t));
            return w_u16(val, cc2520_get_chan(dev));

        case NETOPT_TX_POWER:
            assert(max_len >= sizeof(int16_t));
            return w_i16(val, cc2520_get_txpower(dev));

        case NETOPT_STATE:
            assert(max_len >= sizeof(netopt_state_t));
            *((netopt_state_t *)val) = cc2520_get_state(dev);
            return sizeof(netopt_state_t);

        case NETOPT_IS_WIRED:
            return -ENOTSUP;

        case NETOPT_MAX_PACKET_SIZE:
            if (max_len < sizeof(int16_t)) {
                return -EOVERFLOW;
            }
            *((uint16_t *)val) = CC2520_MAX_DATA_LEN - _MAX_MHR_OVERHEAD;
            return sizeof(uint16_t);
            
        case NETOPT_IS_CHANNEL_CLR:
            return opt_state(val, cc2520_cca(dev)); //TODO: CHANGE HERE

        case NETOPT_AUTOACK:
            return opt_state(val, (dev->options & CC2520_OPT_AUTOACK));

        case NETOPT_CSMA:
            return opt_state(val, (dev->options & CC2520_OPT_CSMA));

        case NETOPT_PRELOADING:
            return opt_state(val, (dev->options & CC2520_OPT_PRELOADING));

        case NETOPT_PROMISCUOUSMODE:
            return opt_state(val, (dev->options & CC2520_OPT_PROMISCUOUS));

        case NETOPT_RX_START_IRQ:
            return opt_state(val, (dev->options & CC2520_OPT_TELL_RX_START));

        case NETOPT_RX_END_IRQ:
            return opt_state(val, (dev->options & CC2520_OPT_TELL_TX_END));

        case NETOPT_TX_START_IRQ:
            return opt_state(val, (dev->options & CC2520_OPT_TELL_RX_START));

        case NETOPT_TX_END_IRQ:
            return opt_state(val, (dev->options & CC2520_OPT_TELL_RX_END));

        default:
            return -ENOTSUP;
    }
}

static int _set(netdev_t *netdev, netopt_t opt, const void *val, size_t val_len)
{
    if (netdev == NULL) {
        return -ENODEV;
    }

    cc2520_t *dev = (cc2520_t *)netdev;

    int ext = netdev_ieee802154_set(&dev->netdev, opt, val, val_len);

    switch (opt) {
        case NETOPT_ADDRESS:
            assert(val_len == 2);
            cc2520_set_addr_short(dev, val);
            return 2;

        case NETOPT_ADDRESS_LONG:
            assert(val_len == 8);
            cc2520_set_addr_long(dev, val);
            return 8;

        case NETOPT_NID:
            assert(val_len == sizeof(uint16_t));
            cc2520_set_pan(dev, to_u16(val));
            return sizeof(uint16_t);

        case NETOPT_CHANNEL:
            assert(val_len == sizeof(uint16_t));
            return cc2520_set_chan(dev, to_u16(val));

        case NETOPT_TX_POWER:
            assert(val_len == sizeof(int16_t));
            cc2520_set_txpower(dev, to_i16(val));
            return sizeof(int16_t);

        case NETOPT_STATE:
            assert(val_len == sizeof(netopt_state_t));
            return cc2520_set_state(dev, *((netopt_state_t *)val));

        case NETOPT_AUTOACK:
            return cc2520_set_option(dev, CC2520_OPT_AUTOACK, to_bool(val));

        case NETOPT_CSMA:
            return cc2520_set_option(dev, CC2520_OPT_CSMA, to_bool(val));

        case NETOPT_PRELOADING:
            return cc2520_set_option(dev, CC2520_OPT_PRELOADING, to_bool(val));

        case NETOPT_PROMISCUOUSMODE:
            return cc2520_set_option(dev, CC2520_OPT_PROMISCUOUS, to_bool(val));

        case NETOPT_RX_START_IRQ:
            return cc2520_set_option(dev, CC2520_OPT_TELL_RX_START, to_bool(val));

        case NETOPT_RX_END_IRQ:
            return cc2520_set_option(dev, CC2520_OPT_TELL_RX_END, to_bool(val));

        case NETOPT_TX_START_IRQ:
            return cc2520_set_option(dev, CC2520_OPT_TELL_TX_START, to_bool(val));

        case NETOPT_TX_END_IRQ:
            return cc2520_set_option(dev, CC2520_OPT_TELL_TX_END, to_bool(val));

        default:
            return ext;
    }
    
    return 0;
}
