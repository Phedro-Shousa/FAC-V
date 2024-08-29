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
 * @brief       Implementation of public functions for cc2520 driver
 *
 * @author      Thomas Eichinger <thomas.eichinger@fu-berlin.de>
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 *
 * @}
 */

#include "luid.h"
#include "byteorder.h"
#include "net/ieee802154.h"
#include "net/gnrc.h"

#include "cc2520_internal.h"
#include "cc2520_netdev.h"
#include "cc2520_registers.h"

#define ENABLE_DEBUG (0)
#include "debug.h"


void cc2520_setup(cc2520_t * dev, const cc2520_params_t *params)
{
    /* set pointer to the devices netdev functions */
    dev->netdev.netdev.driver = &cc2520_driver;
    /* pull in device configuration parameters */
    memcpy(&dev->params, params, sizeof(cc2520_params_t));
    dev->state = CC2520_STATE_IDLE;
    /* reset device descriptor fields */
    dev->options = 0;
}

int cc2520_init(cc2520_t *dev)
{
    uint8_t addr[8];

    /* reset options and sequence number */
    dev->netdev.seq = 0;
    dev->netdev.flags = 0;

    /* set default address, channel, PAN ID, and TX power */
    luid_get(addr, sizeof(addr));
    /* make sure we mark the address as non-multicast and not globally unique */
    addr[0] &= ~(0x01);
    addr[0] |= 0x02;
    
    // make it a texas device
    addr[0] = 0x00;
    addr[1] = 0x12;
    addr[2] = 0x4b;
    addr[3] = 0x00;
    
    addr[4] = 0x00;
    addr[5] = 0x00;
    addr[6] = 0x00;
    addr[7] = 0x02;
    
    cc2520_set_addr_short(dev, &addr[6]);
    cc2520_set_addr_long(dev, addr);
    cc2520_set_pan(dev, CC2520_PANID_DEFAULT);
    cc2520_set_chan(dev, CC2520_CHAN_DEFAULT);
    cc2520_set_txpower(dev, 5);
    
    /* set default options */
    cc2520_set_option(dev, CC2520_OPT_AUTOACK, false);
    cc2520_set_option(dev, CC2520_OPT_AUTOCRC, true);
    cc2520_set_option(dev, CC2520_OPT_FRAME_FILTER, false);
 
    cc2520_set_option(dev, CC2520_OPT_CSMA, true);
    cc2520_set_option(dev, CC2520_OPT_TELL_TX_START, true);
    cc2520_set_option(dev, CC2520_OPT_TELL_RX_END, true);

#ifdef MODULE_NETSTATS_L2
    cc2520_set_option(dev, CC2520_OPT_TELL_RX_END, true);
#endif
    /* set default protocol*/
#ifdef MODULE_GNRC_SIXLOWPAN
    dev->netdev.proto = GNRC_NETTYPE_SIXLOWPAN;
#elif MODULE_GNRC
    dev->netdev.proto = GNRC_NETTYPE_UNDEF;
#endif

// Read chip id to check if cc2520 is present
  int rf_part_num = cc2520_reg_read(dev, CC2520_CHIPID);
  if (rf_part_num != CC2520_CHIPID_VAL){
	printf("\nid=0x%x\n", rf_part_num);
	printf("cc2520: init: unable to communicate with device\n");
	while(1);
  }

    /* set default RECOMMENDED settings Pag. 103 */

    /* Raises the CCA threshold from about -108dBm to about -84 dBm input level.  */
    cc2520_reg_write(dev, CC2520_CCACTRL0, 0xF8);
    /* Controls modem  */
    cc2520_reg_write(dev, CC2520_MDMCTRL0, 0x84);
    /* Controls modem. */
    cc2520_reg_write(dev, CC2520_MDMCTRL1, 0x14);
    /* Adjust currents in RX related analog modules */
    cc2520_reg_write(dev, CC2520_RXCTRL, 0x3F);
    /* Adjust currents in synthesizer. */
    cc2520_reg_write(dev, CC2520_FSCTRL, 0x5A);
    /* Adjust currents in VCO */
    cc2520_reg_write(dev, CC2520_FSCAL1, 0x2B);
    /* Adjust target value for AGC control loop */
    cc2520_reg_write(dev, CC2520_AGCCTRL1, 0x11);
    cc2520_reg_write(dev, CC2520_AGCCTRL2, 0xEB);
    /* Tune ADC performance.  */
    cc2520_reg_write(dev, CC2520_ADCTEST0, 0x10);    
    cc2520_reg_write(dev, CC2520_ADCTEST1, 0x0E);
    cc2520_reg_write(dev, CC2520_ADCTEST2, 0x03); 
    /* Disable external clock */
    cc2520_reg_write(dev, CC2520_EXTCLOCK, 0x00);
    /* Set bit 14 in RXENABLE. Used for backwards compatibility with CC2420.   */
    cc2520_reg_write(dev, CC2520_FRMCTRL1, 0x01);
    /* set the FIFOP threshold to maximum. */
    cc2520_reg_write(dev, CC2520_FIFOPCTRL, CC2520_PKT_MAXLEN);

    /* go into RX state */
    cc2520_set_state(dev, NETOPT_STATE_IDLE);
    
    return 0;
}


bool cc2520_cca(cc2520_t *dev)
{
    while (!(cc2520_status(dev) & CC2520_RSSI_VALID)) {}
    return gpio_read(dev->params.pin_cca);
}

size_t cc2520_send(cc2520_t *dev, const iolist_t *iolist)
{
    size_t n = cc2520_tx_prepare(dev, iolist);

    if ((n > 0) && !(dev->options & CC2520_OPT_PRELOADING)) {
        cc2520_tx_exec(dev);
    }

    return n;
}

size_t cc2520_tx_prepare(cc2520_t *dev, const iolist_t *iolist)
{
    size_t pkt_len = 2;     /* include the FCS (frame check sequence) */

    /* wait for any ongoing transmissions to be finished */
    DEBUG("cc2520: tx_exec: waiting for any ongoing transmission\n");
    while (cc2520_get_state(dev) & NETOPT_STATE_TX) {}

    /* get and check the length of the packet */
    for (const iolist_t *iol = iolist; iol; iol = iol->iol_next) {
        pkt_len += iol->iol_len;
    }
    if (pkt_len >= CC2520_PKT_MAXLEN) {
        DEBUG("cc2520: tx_prep: unable to send, pkt too large\n");
        return 0;
    }

    /* flush TX FIFO and write new packet to it */
    cc2520_strobe(dev, CC2520_INS_SFLUSHTX);
    /* push packet length to TX FIFO */
    cc2520_fifo_write(dev, (uint8_t *)&pkt_len, 1);
    /* push packet to TX FIFO */
    for (const iolist_t *iol = iolist; iol; iol = iol->iol_next) {
        cc2520_fifo_write(dev, iol->iol_base, iol->iol_len);
    }
    DEBUG("cc2520: tx_prep: loaded %i byte into the TX FIFO\n", (int)pkt_len);

    return pkt_len;
}

void cc2520_tx_exec(cc2520_t *dev)
{
    /* trigger the transmission */
    if (dev->options & CC2520_OPT_TELL_TX_START) {
        dev->netdev.netdev.event_callback(&dev->netdev.netdev,
                                          NETDEV_EVENT_TX_STARTED);
    }
    DEBUG("cc2520: tx_exec: TX_START\n");
    if (dev->options & CC2520_OPT_CSMA) {
        DEBUG("cc2520: tx_exec: triggering TX with CCA\n");
        cc2520_strobe(dev, CC2520_INS_STXONCCA);
    }
    else {
        DEBUG("cc2520: tx_exec: triggering TX without CCA\n");
        cc2520_strobe(dev, CC2520_INS_STXON);
    }
}

int cc2520_rx(cc2520_t *dev, uint8_t *buf, size_t max_len, void *info)
{
    (void)info;

    uint8_t len;
    uint8_t crc_corr;

    /* without a provided buffer, only readout the length and return it */
    if (buf == NULL) {
        /* get the packet length (without dropping it) (first byte in RX FIFO) */
        cc2520_ram_read(dev, CC2520RAM_RXFIFO, &len, 1);
        len -= 2;   /* subtract RSSI and FCF */
        DEBUG("cc2520: recv: packet of length %i in RX FIFO\n", (int)len);
    }
    else {
        /* read length byte */
        cc2520_fifo_read(dev, &len, 1);
        len -= 2;   /* subtract RSSI and FCF */

        /* if a buffer is given, read (and drop) the packet */
        len = (len > max_len) ? max_len : len;

        /* read fifo contents */
        DEBUG("cc2520: recv: reading %i byte of the packet\n", (int)len);
        cc2520_fifo_read(dev, buf, len);

        int8_t rssi;
        cc2520_fifo_read(dev, (uint8_t*)&rssi, 1);
        DEBUG("cc2520: recv: RSSI is %i\n", (int)rssi);

        /* fetch and check if CRC_OK bit (MSB) is set */
        cc2520_fifo_read(dev, &crc_corr, 1);
        if (!(crc_corr & 0x80)) {
            DEBUG("cc2520: recv: CRC_OK bit not set, dropping packet\n");
            /* drop the corrupted frame from the RXFIFO */
            len = 0;
        }
        if (info != NULL) {
            netdev_ieee802154_rx_info_t *radio_info = info;
            radio_info->rssi = CC2520_RSSI_OFFSET + rssi;
            radio_info->lqi = crc_corr & CC2520_CRCCOR_COR_MASK;
        }

        /* finally flush the FIFO */
        cc2520_strobe(dev, CC2520_INS_SFLUSHRX);
        cc2520_strobe(dev, CC2520_INS_SFLUSHRX);
    }

    return (int)len;
}
