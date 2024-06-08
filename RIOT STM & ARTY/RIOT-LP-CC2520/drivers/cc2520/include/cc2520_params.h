/*
 * Copyright (C) 2016 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     drivers_cc2520
 *
 * @{
 * @file
 * @brief       Default configuration for the CC2520 driver
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 */

#ifndef CC2520_PARAMS_H
#define CC2520_PARAMS_H

#include "board.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
		--------- * GPIO's * --------
		* SPI			
		PA13 - SCK
		PA14 - MISO 
		PA15 - MOSI
		PB6 - Chip Select

		* INPUTs
		PA5	- FIFOP	
		PA1 - FIFO
		PA2 - CCA
		PA3 - SFD

		* OUTPUTs
		PB0 - Reset
		PB10 - VREG / VEN
*/
  
/**
 * @name    Set default configuration parameters for the CC2520 driver
 * @{
 */
#ifndef CC2520_PARAM_SPI
#define CC2520_PARAM_SPI        (SPI_DEV(0))
#endif
#ifndef CC2520_PARAM_SPI_CLK
#define CC2520_PARAM_SPI_CLK    SPI_CLK_10MHZ  
#endif

#if 0
#ifndef CC2520_PARAM_CS
#define CC2520_PARAM_CS         (GPIO_PIN(PORT_D, 14)) 
#endif
#ifndef CC2520_PARAM_FIFO
#define CC2520_PARAM_FIFO       (GPIO_PIN(PORT_F, 1))   
#endif
#ifndef CC2520_PARAM_FIFOP
#define CC2520_PARAM_FIFOP      (GPIO_PIN(PORT_F, 15))   
#endif
#ifndef CC2520_PARAM_CCA
#define CC2520_PARAM_CCA        (GPIO_PIN(PORT_F, 2))     
#endif
#ifndef CC2520_PARAM_SFD
#define CC2520_PARAM_SFD        (GPIO_PIN(PORT_B, 10))      
#endif
#ifndef CC2520_PARAM_VREFEN
#define CC2520_PARAM_VREFEN     (GPIO_PIN(PORT_E, 9))    
#endif
#ifndef CC2520_PARAM_RESET
#define CC2520_PARAM_RESET      (GPIO_PIN(PORT_F, 3))   
#endif

#else

#define PIN_0_OFFSET 16
#define PIN_1_OFFSET 17
#define PIN_2_OFFSET 18
#define PIN_3_OFFSET 19
#define PIN_4_OFFSET 20
#define PIN_5_OFFSET 21
#define PIN_6_OFFSET 22
#define PIN_7_OFFSET 23
#define PIN_8_OFFSET 0
#define PIN_9_OFFSET 1
#define PIN_10_OFFSET 2
#define PIN_11_OFFSET 3
#define PIN_12_OFFSET 4
#define PIN_13_OFFSET 5
//#define PIN_14_OFFSET 8 //This pin is not connected on either board.
#define PIN_15_OFFSET 9
#define PIN_16_OFFSET 10
#define PIN_17_OFFSET 11
#define PIN_18_OFFSET 12
#define PIN_19_OFFSET 13

#ifndef CC2520_PARAM_CS
#define CC2520_PARAM_CS         (GPIO_PIN(0, PIN_10_OFFSET)) 
#endif
#ifndef CC2520_PARAM_FIFO
#define CC2520_PARAM_FIFO       (GPIO_PIN(0, 0))   
#endif
#ifndef CC2520_PARAM_FIFOP
#define CC2520_PARAM_FIFOP      (GPIO_PIN(0, PIN_2_OFFSET))  
#endif
#ifndef CC2520_PARAM_CCA
#define CC2520_PARAM_CCA        (GPIO_PIN(0, 0))     
#endif
#ifndef CC2520_PARAM_SFD
#define CC2520_PARAM_SFD        (GPIO_PIN(0, 0))      
#endif
#ifndef CC2520_PARAM_VREFEN
#define CC2520_PARAM_VREFEN     (GPIO_PIN(0, PIN_6_OFFSET))    
#endif
#ifndef CC2520_PARAM_RESET
#define CC2520_PARAM_RESET      (GPIO_PIN(0, PIN_17_OFFSET))   
#endif
#endif


#ifndef CC2520_PARAMS
#define CC2520_PARAMS           { .spi        = CC2520_PARAM_SPI,     \
                                  .spi_clk    = CC2520_PARAM_SPI_CLK, \
                                  .pin_cs     = CC2520_PARAM_CS,      \
                                  .pin_fifo   = CC2520_PARAM_FIFO,    \
                                  .pin_fifop  = CC2520_PARAM_FIFOP,   \
                                  .pin_cca    = CC2520_PARAM_CCA,     \
                                  .pin_sfd    = CC2520_PARAM_SFD,     \
                                  .pin_vrefen = CC2520_PARAM_VREFEN,  \
                                  .pin_reset  = CC2520_PARAM_RESET }
#endif
/**@}*/

/**
 * @brief   CC2520 configuration
 */
static const cc2520_params_t cc2520_params[] =
{
    CC2520_PARAMS
};

#ifdef __cplusplus
}
#endif

#endif /* CC2520_PARAMS_H */
/** @} */
