/*
 * Copyright (C) 2015 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser General
 * Public License v2.1. See the file LICENSE in the top level directory for more
 * details.
 */

/**
 * @defgroup    drivers_cc2520 CC2520 radio driver
 * @ingroup     drivers_netdev
 * @{
 *
 * @file
 * @brief       Interface definition for the CC2520 driver
 *
 * @author      Thomas Eichinger <thomas.eichinger@fu-berlin.de>
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 */

#ifndef CC2520_H
#define CC2520_H

#include <stdint.h>

#include "periph/spi.h"
#include "periph/gpio.h"

#include "net/netdev.h"
#include "net/netdev/ieee802154.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   cc2520 device id
 */
#define CC2520_CHIPID_VAL (0x84)
  
/**
 * @brief   Maximum possible packet size in byte
 */
#define CC2520_PKT_MAXLEN       (IEEE802154_FRAME_LEN_MAX)
#define CC2520_MAX_DATA_LEN     (CC2520_PKT_MAXLEN)

/**
 * @brief   PAN ID configuration
 */
#define CC2520_PANID_DEFAULT    (CONFIG_IEEE802154_DEFAULT_PANID)

/**
  * @name    Channel configuration
  * @{
  */
#define CC2520_CHAN_MIN         (IEEE802154_CHANNEL_MIN)
#define CC2520_CHAN_MAX         (IEEE802154_CHANNEL_MAX)
#define CC2520_CHAN_DEFAULT     (CONFIG_IEEE802154_DEFAULT_CHANNEL)
#define CHANNEL_SPACING          5 //MHz
/** @} */

/**
 * @name    Default TX power configuration [in dBm]
 * @{
 *
 * Table 17: Output power and current consumption measured on the CC2520 reference design @ 
 * +3.0 V, +25°C, f c =2.440 GHz 
*/
#define         _5_DBM   (0xF7)
#define         _3_DBM   (0xF2)
#define         _2_DBM   (0xAB)
#define         _1_DBM   (0x13)
#define         _0_DBM   (0x32)
#define   _MINUS_2_DBM   (0x81)
#define   _MINUS_4_DBM   (0x88)
#define   _MINUS_7_DBM   (0x2C)
#define  _MINUS_18_DBM   (0x03)
   
#define CC2520_TXPOWER_MIN      (-18)//dBm
#define CC2520_TXPOWER_MAX      (5)//dBm
#define CC2520_TXPOWER_DEFAULT  (IEEE802154_DEFAULT_TXPOWER)
/** @} */
   
/**
 * @brief   RSSI offset
 */
#define CC2520_RSSI_OFFSET      (76)//dBm

/**
 * @brief   A couple of return values used in this driver
 */
enum {
    CC2520_RET_CHAN_OK      = 2,
};

/**
 * @brief   Struct holding all parameters needed for device initialization
 */
typedef struct cc2520_params {
    spi_t spi;              /**< SPI bus the device is connected to */
    spi_clk_t spi_clk;      /**< SPI speed to use */
    gpio_t pin_cs;          /**< pin connected to chip select */
    gpio_t pin_fifo;        /**< pin connected to the FIFO interrupt pin */
    gpio_t pin_fifop;       /**< pin connected to the FIFOP interrupt pin */
    gpio_t pin_cca;         /**< pin connected to CCA */
    gpio_t pin_sfd;         /**< pin connected to 'start of frame delimiter' */
    gpio_t pin_vrefen;      /**< pin connected to the Vref enable pin */
    gpio_t pin_reset;       /**< pin connected to the reset pin */
} cc2520_params_t;

/**
 * @brief   Device descriptor for CC2520 radio devices
 */
typedef struct {
    /* netdev fields */
    netdev_ieee802154_t netdev;   /**< netdev parent struct */
    /* device specific fields */
    cc2520_params_t params;       /**< hardware interface configuration */
    /* device state fields */
    uint8_t state;                /**< current state of the radio */
    uint16_t options;             /**< state of used options */
} cc2520_t;

/**
 * @brief   Setup the device descriptor for the given device
 *
 * @param[out] dev          device descriptor
 * @param[in]  params       device parameters
 *
 * @return                  0 on success
 * @return                  -1 on error
 */
void cc2520_setup(cc2520_t *dev, const cc2520_params_t *params);

/**
 * @brief   Initialize a given CC2520 device
 *
 * @param[out] dev          device descriptor
 *
 * @return                  0 on success
 * @return                  <0 on error
 */
int cc2520_init(cc2520_t *dev);

/**
 * @brief   Trigger a hardware reset and configure radio with default values
 *
 * @param[in] dev           device to reset
 *
 * @return  TODO
 */
int cc2520_reset(cc2520_t *dev);

/**
 * @brief   Trigger a clear channel assessment
 *
 * @param[in] dev           device to use
 *
 * @return                  true if channel is clear
 * @return                  false if channel is busy
 */
bool cc2520_cca(cc2520_t *dev);

/**
 * @brief   Get the short address of the given device
 *
 * @param[in]  dev          device to read from
 * @param[out] addr         memory to write the 2 byte address into
 */
void cc2520_get_addr_short(cc2520_t *dev, uint8_t *addr);

/**
 * @brief   Set the short address of the given device
 *
 * @param[in] dev           device to write to
 * @param[in] addr          (2-byte) short address to set
 */
void cc2520_set_addr_short(cc2520_t *dev, const uint8_t *addr);

/**
 * @brief   Get the configured long address of the given device
 *
 * @param[in]  dev           device to read from
 * @param[out] addr_long     buffer to save the read address
 *
 * @return                  the currently set (8-byte) long address
 */
void cc2520_get_addr_long(cc2520_t *dev, uint8_t *addr_long);

/**
 * @brief   Set the long address of the given device
 *
 * @param[in] dev           device to write to
 * @param[in] addr_long     (8-byte) long address to set
 */
void cc2520_set_addr_long(cc2520_t *dev, const uint8_t *addr_long);

/**
 * @brief   Get the configured PAN ID of the given device
 *
 * @param[in] dev           device to read from
 *
 * @return                  the currently set PAN ID
 */
uint16_t cc2520_get_pan(cc2520_t *dev);

/**
 * @brief   Set the PAN ID of the given device
 *
 * @param[in] dev           device to write to
 * @param[in] pan           PAN ID to set
 */
void cc2520_set_pan(cc2520_t *dev, uint16_t pan);

/**
 * @brief   Get the configured channel of the given device
 *
 * @param[in] dev           device to read from
 *
 * @return                  the currently set channel
 */
uint16_t cc2520_get_chan(cc2520_t *dev);

/**
 * @brief   Set the channel of the given device
 *
 * @param[in] dev           device to write to
 * @param[in] chan          channel to set
 */
int cc2520_set_chan(cc2520_t *dev, uint16_t chan);

/**
 * @brief   Get the configured transmission power of the given device [in dBm]
 *
 * @param[in] dev           device to read from
 *
 * @return                  configured transmission power in dBm
 */
int16_t cc2520_get_txpower(cc2520_t *dev);

/**
 * @brief   Set the transmission power of the given device [in dBm]
 *
 * If the device does not support the exact dBm value given, it will set a value
 * as close as possible to the given value. If the given value is larger or
 * lower then the maximal or minimal possible value, the min or max value is
 * set, respectively.
 *
 * @param[in] dev           device to write to
 * @param[in] txpower       transmission power in dBm
 */
void cc2520_set_txpower(cc2520_t *dev, int16_t txpower);

/**
 * @brief   Enable or disable driver specific options
 *
 * @param[in] dev           device to set/clear option flag for
 * @param[in] option        option to enable/disable
 * @param[in] state         true for enable, false for disable
 */
int cc2520_set_option(cc2520_t *dev, uint16_t option, bool state);

/**
 * @brief   Set the state of the given device (trigger a state change)
 *
 * @param[in] dev           device to change state of
 * @param[in] state         the targeted new state
 */
int cc2520_set_state(cc2520_t *dev, netopt_state_t state);

/**
 * @brief   Get the state of the given device
 *
 * @param[in] dev           device to change state of
 *
 * @return                  the device's current state
 */
netopt_state_t cc2520_get_state(cc2520_t *dev);

/**
 * @brief   Convenience function for simply sending data
 *
 * @note This function ignores the PRELOADING option
 *
 * @param[in] dev           device to use for sending
 * @param[in] iolist        data to send (must include IEEE802.15.4 header)
 *
 * @return                  number of bytes that were actually send
 * @return                  0 on error
 */
size_t cc2520_send(cc2520_t *dev, const iolist_t *iolist);

/**
 * @brief   Prepare for sending of data
 *
 * This function puts the given device into the TX state, so no receiving of
 * data is possible after it was called.
 *
 * @param[in] dev           device to prepare for sending
 * @param[in] iolist        data to prepare (must include IEEE802.15.4 header)
 */
size_t cc2520_tx_prepare(cc2520_t *dev, const iolist_t *iolist);

/**
 * @brief   Trigger sending of data previously loaded into transmit buffer
 *
 * @param[in] dev           device to trigger
 */
void cc2520_tx_exec(cc2520_t *dev);

/**
 * @brief   Read a chunk of data from the receive buffer of the given device
 *
 * @param[in]  dev          device to read from
 * @param[out] buf          buffer to write data to
 * @param[in]  max_len      number of bytes to read from device
 * @param[in]  info         to be removed
 *
 * @return                  the number of bytes in the Rx FIFO
 * @return                  the number of bytes written to @p buf if present
 */
int cc2520_rx(cc2520_t *dev, uint8_t *buf, size_t max_len, void *info);

void cc2520_clear_fifop_excep(const cc2520_t *dev);
#ifdef __cplusplus
}
#endif

#endif /* CC2520_H */
/** @} */
