/*
 * Copyright (C) 2017 Ken Rabold
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    boards_hifive1 SiFive HiFive1 RISC-V board
 * @ingroup     boards
 * @brief       Support for the SiFive HiFive1 RISC-V board
 * @{
 *
 * @file
 * @brief       Board specific definitions for the SiFive HiFive1 RISC-V board
 *
 * @author      Ken Rabold
 */

#ifndef BOARD_H
#define BOARD_H

#include "periph/gpio.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @name    Xtimer configuration
 * @{
 */
#define XTIMER_HZ                   (32768UL)
/** @} */

/**
 * @name    Macros for controlling the on-board LEDs
 * @{
 */
#define LED0_PIN            GPIO_PIN(0, 11) /* Red */
#define LED1_PIN            GPIO_PIN(0, 12) /* Green */
#define LED2_PIN            GPIO_PIN(0, 13) /* Blue */

#define LED0_ON             gpio_clear(LED0_PIN)
#define LED0_OFF            gpio_set(LED0_PIN)
#define LED0_TOGGLE         gpio_toggle(LED0_PIN)

#define LED1_ON             gpio_clear(LED1_PIN)
#define LED1_OFF            gpio_set(LED1_PIN)
#define LED1_TOGGLE         gpio_toggle(LED1_PIN)

#define LED2_ON             gpio_clear(LED2_PIN)
#define LED2_OFF            gpio_set(LED2_PIN)
#define LED2_TOGGLE         gpio_toggle(LED2_PIN)

#define CC2520_PARAM_CS            GPIO_PIN(0, 26)
#define CC2520_PARAM_FIFO          GPIO_PIN(0, 16)
#define CC2520_PARAM_FIFOP         GPIO_PIN(0, 18)
#define CC2520_PARAM_CCA           GPIO_PIN(0, 20)
#define CC2520_PARAM_SFD           GPIO_PIN(0, 21)
#define CC2520_PARAM_VREFEN        GPIO_PIN(0, 22)
#define CC2520_PARAM_RESET         GPIO_PIN(0, 23)

#define MODULE_CC2520 1

/** @} */

/**
 * @brief   Initialize board specific hardware, including clock, LEDs and std-IO
 */
void board_init(void);

/**
 * @brief   Initialize the board clock to use PLL and faster SPI access.
 */
void board_init_clock(void);

#ifdef __cplusplus
}
#endif

#endif /* BOARD_H */
/** @} */
