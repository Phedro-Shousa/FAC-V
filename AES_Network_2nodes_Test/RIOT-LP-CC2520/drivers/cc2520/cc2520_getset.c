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
 * @brief       Getter and setter functions for the cc2520 driver
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 * @author      Francisco Acosta <francisco.acosta@inria.fr>
 *
 * @}
 */

#include <string.h>
#include <errno.h>

#include "cc2520.h"
#include "cc2520_internal.h"
#include "cc2520_registers.h"
#include "periph/spi.h"

#define ENABLE_DEBUG    (0)
#include "debug.h"


void cc2520_get_addr_short(cc2520_t *dev, uint8_t *addr)
{
    uint8_t tmp[2];

    cc2520_ram_read(dev, CC2520RAM_SHORTADDR, tmp, 2);

    addr[0] = tmp[1];
    addr[1] = tmp[0];
}

void cc2520_set_addr_short(cc2520_t *dev, const uint8_t *addr)
{
    uint8_t tmp[2];
    tmp[0] = addr[1];
    tmp[1] = addr[0];

    memcpy(dev->netdev.short_addr, addr, 2);

#ifdef MODULE_SIXLOWPAN
    /* https://tools.ietf.org/html/rfc4944#section-12 requires the first bit to
     * 0 for unicast addresses */
    dev->netdev.short_addr[0] &= 0x7F;
#endif

    cc2520_ram_write(dev, CC2520RAM_SHORTADDR, tmp, 2);
}

void cc2520_get_addr_long(cc2520_t *dev, uint8_t *addr)
{
    cc2520_ram_read(dev, CC2520RAM_IEEEADDR, addr, 8);

    uint8_t *ap = (uint8_t *)(&addr);
    for (int i = 0; i < 8; i++) {
        ap[i] = dev->netdev.long_addr[i];
    }
}

void cc2520_set_addr_long(cc2520_t *dev, const uint8_t *addr)
{
    int i, j;
    uint8_t tmp[8];

    for (i = 0, j = 7; i < 8; i++, j--) {
        dev->netdev.long_addr[i] = addr[i];
        tmp[j] = addr[i];
    }

    cc2520_ram_write(dev, CC2520RAM_IEEEADDR, tmp, 8);
}

uint16_t cc2520_get_pan(cc2520_t *dev)
{
  le_uint16_t pan = {0};

    cc2520_ram_read(dev, CC2520RAM_PANID, pan.u8, 2);
    return pan.u16;
}

void cc2520_set_pan(cc2520_t *dev, uint16_t pan)
{
    dev->netdev.pan = pan;
    
    cc2520_ram_write(dev, CC2520RAM_PANID, (uint8_t *)&pan, 2);
}

uint16_t cc2520_get_chan(cc2520_t *dev)
{
    uint16_t chan;
    uint16_t freq = cc2520_reg_read(dev, CC2520_FREQCTRL);

    chan = (CC2520_CHAN_MIN + ((freq - CC2520_CHAN_MIN) / CHANNEL_SPACING));
    
    return chan;
}

int cc2520_set_chan(cc2520_t *dev, uint16_t chan)
{
  uint16_t freq;
  
    if ((chan < CC2520_CHAN_MIN) || (chan > CC2520_CHAN_MAX)) {
        DEBUG("cc2520: set channel: given channel invalid\n");
        return -ENOTSUP;
    }
    
    dev->netdev.chan = chan;

    freq = (CC2520_CHAN_MIN + ((chan - CC2520_CHAN_MIN) * CHANNEL_SPACING));
    
    cc2520_reg_write(dev, CC2520_FREQCTRL, freq);

    cc2520_en_xosc(dev);
    if (!(cc2520_status(dev) & CC2520_XOSC16M_STABLE)) {
        DEBUG("cc2520: init: oscillator did not stabilize\n");
        return -1;    
    }
    
    dev->netdev.chan = chan;
    
    return CC2520_RET_CHAN_OK;
}

int16_t cc2520_get_txpower(cc2520_t *dev)
{
  uint8_t val = cc2520_reg_read(dev, CC2520_TXPOWER);
 
  switch(val){
    case (0x03): return (-18);
      break;
    case (0x2C): return (-7);
      break;
    case (0x88): return (-4);
      break;
    case (0x81): return (-2);
      break;
    case (0x32): return (0);
      break;
    case (0x13): return (1);
      break;
    case (0xAB): return (2);
      break;
    case (0xF2): return (3);
      break;
    case (0xF7): return (5);
      break;
    default: return (0xFF);
      break;
    }
}

void cc2520_set_txpower(cc2520_t *dev, int16_t txpower)
{
  
  uint8_t val;
  
    if (txpower > CC2520_TXPOWER_MAX) {
        txpower = CC2520_TXPOWER_MAX;
    }
    else if (txpower < CC2520_TXPOWER_MIN) {
        txpower = CC2520_TXPOWER_MIN;
    }
      
    switch(txpower){
    case (-18): val = (0x03);
      break;
    case (-7):  val = (0x2C);
      break;
    case (-4):  val = (0x88);
      break;
    case (-2):  val = (0x81);
      break;
    case (0):   val = (0x32);
      break;
    case (1):   val = (0x13);
      break;
    case (2):   val = (0xAB);
      break;
    case (3):   val = (0xF2);
      break;
    case (5):   val = (0xF7);
      break;
    default:    val = (0x32);
      break;
    }

    cc2520_reg_write(dev, CC2520_TXCTRL, 0x94);
    /* Set selected TX power */
    cc2520_reg_write(dev, CC2520_TXPOWER, val);
    
    cc2520_en_xosc(dev);
    if (!(cc2520_status(dev) & CC2520_XOSC16M_STABLE)) {
        DEBUG("cc2520: init: oscillator did not stabilize\n");
    }
}

int cc2520_set_option(cc2520_t *dev, uint16_t option, bool state)
{
    uint16_t reg;

    /* set option field */
    if (state) {
        dev->options |= option;
        /* trigger option specific actions */
        switch (option) {
        case CC2520_OPT_FRAME_FILTER:
                DEBUG("cc2520: set_opt: CC2520_OPT_FRAME_FILTER\n");
                reg = cc2520_reg_read(dev, CC2520_FRMFILT0);
                
                reg |= CC2520_FRMFILT0_FRAME_FILTER_EN | 
                       CC2520_FRMFILT0_MAX_FRAME_VERSION_1 |
                       CC2520_FRMFILT0_MAX_FRAME_VERSION_0;
                
                cc2520_reg_write(dev, CC2520_FRMFILT0, reg);
                break;
            
        case CC2520_OPT_AUTOACK:
                DEBUG("cc2520: set_opt: CC2520_OPT_AUTOACK\n");
                reg = cc2520_reg_read(dev, CC2520_FRMCTRL0);
                reg |= CC2520_FRMCTRL0_AUTOACK;
                cc2520_reg_write(dev, CC2520_FRMCTRL0, reg);
                break;

            case CC2520_OPT_AUTOCRC:
                DEBUG("cc2520: set_opt: CC2520_OPT_AUTOCRC\n");
                reg = cc2520_reg_read(dev, CC2520_FRMCTRL0);
                reg |= CC2520_FRMCTRL0_AUTOCRC;
                cc2520_reg_write(dev, CC2520_FRMCTRL0, reg);
                break;

            case CC2520_OPT_CSMA:
                DEBUG("cc2520: set_opt: CC2520_OPT_CSMA\n");
                /* TODO: en/disable csma */
                break;

            case CC2520_OPT_PROMISCUOUS:
                DEBUG("cc2520: set_opt: CC2520_OPT_PROMISCUOUS\n");
                /* in promisc mode, AUTO ACKs and frame filtering should be disabled */
                reg = cc2520_reg_read(dev, CC2520_FRMCTRL0);
                reg &= ~(CC2520_FRMCTRL0_AUTOACK);
                cc2520_reg_write(dev, CC2520_FRMCTRL0, reg);
                
                reg = cc2520_reg_read(dev, CC2520_FRMFILT0);
                reg &= ~(CC2520_FRMFILT0_FRAME_FILTER_EN);
                cc2520_reg_write(dev, CC2520_FRMFILT0, reg);                
                break;

            case CC2520_OPT_PRELOADING:
                DEBUG("cc2520: set_opt: CC2520_OPT_PRELOADING\n");
                break;

            case CC2520_OPT_TELL_TX_START:
            case CC2520_OPT_TELL_TX_END:
            case CC2520_OPT_TELL_RX_START:
            case CC2520_OPT_TELL_RX_END:
                DEBUG("cc2520: set_opt: TX/RX START/END\n");
                /* TODO */
                break;

            default:
                return -ENOTSUP;
        }
    }
    else {
        dev->options &= ~(option);
        /* trigger option specific actions */
        switch (option) {
        case CC2520_OPT_FRAME_FILTER:
                DEBUG("cc2520: set_opt: CC2520_OPT_FRAME_FILTER\n");
                reg = cc2520_reg_read(dev, CC2520_FRMFILT0);
                
                reg &= ~(CC2520_FRMFILT0_FRAME_FILTER_EN | 
                       CC2520_FRMFILT0_MAX_FRAME_VERSION_1 |
                       CC2520_FRMFILT0_MAX_FRAME_VERSION_0);
                
                cc2520_reg_write(dev, CC2520_FRMFILT0, reg);
                break;
                
        case CC2520_OPT_AUTOACK:
                DEBUG("cc2520: set_opt: CC2520_OPT_AUTOACK\n");
                reg = cc2520_reg_read(dev, CC2520_FRMCTRL0);
                reg &= ~CC2520_FRMCTRL0_AUTOACK;
                cc2520_reg_write(dev, CC2520_FRMCTRL0, reg);
                break;

            case CC2520_OPT_AUTOCRC:
                DEBUG("cc2520: set_opt: CC2520_OPT_AUTOCRC\n");
                reg = cc2520_reg_read(dev, CC2520_FRMCTRL0);
                reg &= ~CC2520_FRMCTRL0_AUTOCRC;
                cc2520_reg_write(dev, CC2520_FRMCTRL0, reg);
                break;

            case CC2520_OPT_CSMA:
                DEBUG("cc2520: clr_opt: CC2520_OPT_CSMA\n");
                /* TODO: en/disable csma */
                break;
                
            case CC2520_OPT_PROMISCUOUS:
                DEBUG("cc2520: clr_opt: CC2520_OPT_PROMISCUOUS\n");
                reg = cc2520_reg_read(dev, CC2520_FRMCTRL0);
                reg |= CC2520_FRMFILT0_FRAME_FILTER_EN;
                cc2520_reg_write(dev, CC2520_FRMFILT0, reg); 
                /* re-enable AUTOACK only if the option was set */
                if (dev->options & CC2520_OPT_AUTOACK) {
                    reg |= CC2520_FRMCTRL0_AUTOACK;
                }
                cc2520_reg_write(dev, CC2520_FRMCTRL0, reg);
                break;

            case CC2520_OPT_PRELOADING:
                DEBUG("cc2520: clr_opt: CC2520_OPT_PRELOADING\n");
                break;

            case CC2520_OPT_TELL_TX_START:
            case CC2520_OPT_TELL_TX_END:
            case CC2520_OPT_TELL_RX_START:
            case CC2520_OPT_TELL_RX_END:
                DEBUG("cc2520: clr_opt: TX/RX START/END\n");
                /* TODO */
                break;

            default:
                return -ENOTSUP;
        }
    }
    return sizeof(netopt_enable_t);
}

int cc2520_set_state(cc2520_t *dev, netopt_state_t cmd)
{
    if ((cc2520_get_state(dev) == NETOPT_STATE_OFF) &&
        (cmd != NETOPT_STATE_OFF)) {
        cc2520_en_xosc(dev);
    }
    switch (cmd) {
        case NETOPT_STATE_OFF:
          cc2520_strobe(dev, CC2520_INS_SXOSCOFF);
            while (cc2520_state(dev) != CC2520_STATE_IDLE) {}
            break;
        case NETOPT_STATE_SLEEP:
            cc2520_strobe(dev, CC2520_INS_SRFOFF);
            while (cc2520_state(dev) != CC2520_STATE_IDLE) {}
            break;
        case NETOPT_STATE_IDLE:
            cc2520_strobe(dev, CC2520_INS_SFLUSHRX);
            cc2520_strobe(dev, CC2520_INS_SRXON);
            break;
        case NETOPT_STATE_TX:
            cc2520_tx_exec(dev);
            break;
        case NETOPT_STATE_RESET:
            cc2520_init(dev);
            break;
        case NETOPT_STATE_RX:
        default:
            DEBUG("cc2520: set_state: called with invalid target state\n");
            return -ENOTSUP;
    }
    return sizeof(netopt_state_t);
}

netopt_state_t cc2520_get_state(cc2520_t *dev)
{
    uint8_t cur_state = cc2520_status(dev);
          (void)(cur_state);

    if (!(cc2520_status(dev) & CC2520_XOSC16M_STABLE)) {
        return NETOPT_STATE_OFF;
    }
    else if (cc2520_status(dev) & CC2520_TX_ACTIVE) {
        return NETOPT_STATE_RX;
    }
    else if (cc2520_status(dev) & CC2520_TX_ACTIVE) {
        return NETOPT_STATE_TX;
    }
    else if (cc2520_status(dev) & CC2520_RSSI_VALID) {
        return NETOPT_STATE_IDLE;
    }
    else {
        return NETOPT_STATE_SLEEP;
    }
}

//netopt_state_t cc2520_get_state(cc2520_t *dev)
//{
//    uint8_t cur_state = cc2520_state(dev);
//
//    if (cur_state == 0) {
//        return NETOPT_STATE_OFF;
//    }
//    else if (cur_state == 1) {
//        return NETOPT_STATE_SLEEP;
//    }
//    else if (((cur_state >= 32) && (cur_state <=39)) || (cur_state == 56)) {
//        return NETOPT_STATE_TX;
//    }
//    else if ((cur_state >= 3) && (cur_state <= 6)) {
//        return NETOPT_STATE_IDLE;
//    }
//    else {
//        return NETOPT_STATE_RX;
//    }
//}
