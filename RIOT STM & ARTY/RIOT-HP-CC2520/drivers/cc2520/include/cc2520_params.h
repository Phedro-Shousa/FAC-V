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
		GPIO_PIN(0, 3) - SCK
		GPIO_PIN(0, 4) - MISO 
		GPIO_PIN(0, 5) - MOSI
		GPIO_PIN(0, 26) - Chip Select

		* INPUTs
		GPIO_PIN(0, 10)	- FIFOP	
		GPIO_PIN(0, 9) - FIFO
		GPIO_PIN(0, 11) - CCA
		GPIO_PIN(0, 12) - SFD

		* OUTPUTs
		GPIO_PIN(0, 1) - Reset
		GPIO_PIN(0, 0) - VREG / VEN
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

#if 1
#ifndef CC2520_PARAM_CS
#define CC2520_PARAM_CS         (GPIO_PIN(PORT_D, 14)) 
#endif
#ifndef CC2520_PARAM_FIFO
#define CC2520_PARAM_FIFO       (GPIO_PIN(PORT_A, 3))   
#endif
#ifndef CC2520_PARAM_FIFOP
#define CC2520_PARAM_FIFOP      (GPIO_PIN(PORT_F, 15))   
#endif
#ifndef CC2520_PARAM_CCA
#define CC2520_PARAM_CCA        (GPIO_PIN(PORT_F, 10))     
#endif
#ifndef CC2520_PARAM_SFD
#define CC2520_PARAM_SFD        (GPIO_PIN(PORT_F, 5))      
#endif
#ifndef CC2520_PARAM_VREFEN
#define CC2520_PARAM_VREFEN     (GPIO_PIN(PORT_E, 9))    
#endif
#ifndef CC2520_PARAM_RESET
#define CC2520_PARAM_RESET      (GPIO_PIN(PORT_F, 3))   
#endif

#else
#ifndef CC2520_PARAM_CS
#define CC2520_PARAM_CS         (GPIO_PIN(0, 26)) 
#endif
#ifndef CC2520_PARAM_FIFO
#define CC2520_PARAM_FIFO       (GPIO_PIN(0, 9))   
#endif
#ifndef CC2520_PARAM_FIFOP
#define CC2520_PARAM_FIFOP      (GPIO_PIN(0, 10))   
#endif
#ifndef CC2520_PARAM_CCA
#define CC2520_PARAM_CCA        (GPIO_PIN(0, 11))     
#endif
#ifndef CC2520_PARAM_SFD
#define CC2520_PARAM_SFD        (GPIO_PIN(0, 12))      
#endif
#ifndef CC2520_PARAM_VREFEN
#define CC2520_PARAM_VREFEN     (GPIO_PIN(0, 0))    
#endif
#ifndef CC2520_PARAM_RESET
#define CC2520_PARAM_RESET      (GPIO_PIN(0, 1))   
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
