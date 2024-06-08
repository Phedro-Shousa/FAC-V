/*
 * Copyright (C) 2013 Freie Universität Berlin, Computer Systems & Telematics
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     sys_crypto
 * @{
 *
 * @file
 * @brief       implementation of the AES cipher-algorithm
 *
 * @author      Freie Universitaet Berlin, Computer Systems & Telematics
 * @author      Nicolai Schmittberger <nicolai.schmittberger@fu-berlin.de>
 * @author      Fabrice Bellard
 * @author      Zakaria Kasmi <zkasmi@inf.fu-berlin.de>
 *
 * @author      Unwired Devices LLC
 * @author      Oleg Artamonov <oleg@unwds.com>
 *
 * @note        Integrated in QEMU by Fabrice Bellard from the OpenSSL project.
 *              @version 3.0 (December 2000). Optimised ANSI C code for the
 *              Rijndael cipher (now AES).
 *
 * @author      Vincent Rijmen <vincent.rijmen@esat.kuleuven.ac.be>
 * @author      Antoon Bosselaers <antoon.bosselaers@esat.kuleuven.ac.be>
 * @author      Paulo Barreto <paulo.barreto@terra.com.br>
 *
 * @}
 */

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdalign.h>
#include "crypto/HACCV.h"

//#define MODULE_AES_HARDWARE_MMIO
//#define MODULE_AES_HARDWARE_ROCC_REG
#define MODULE_AES_HARDWARE_ROCC_MEM


//#define AES_192
//#define AES_256

void HACCV_init(const uint8_t *key)
{
          uint32_t *new_key;
          new_key = (uint32_t*)(key);

          #ifdef MODULE_AES_HARDWARE_MMIO
          
               while ((reg_read8(STATUS) & 0x2) == 0) ;
               reg_write32(KEY, new_key[0]);
               reg_write32(KEY+4, new_key[1]);
               reg_write32(KEY+8, new_key[2]);
               reg_write32(KEY+12, new_key[3]);

               #ifdef AES_192
                    reg_write32(KEY+16, new_key[4]);
                    reg_write32(KEY+20, new_key[5]);  
               #endif 

               #ifdef AES_256
                    reg_write32(KEY+16, new_key[4]);
                    reg_write32(KEY+20, new_key[5]);  
                    reg_write32(KEY+24, new_key[6]);
                    reg_write32(KEY+28, new_key[7]); 
               #endif 

          #else
               
               ROCC_INSTRUCTION_2(new_key[0], new_key[1], 0);
               ROCC_INSTRUCTION_2(new_key[2], new_key[3], 1); 
               
               #ifdef AES_192
                    ROCC_INSTRUCTION_2(new_key[4], new_key[5], 2);   
               #endif 

               #ifdef AES_256
                    ROCC_INSTRUCTION_2(new_key[4], new_key[5], 2);   
                    ROCC_INSTRUCTION_2(new_key[6], new_key[7], 3);   
               #endif 
          
          #endif /* MODULE_AES_HARDWARE_MMIO */
  }

void HACCV_encrypt(const uint8_t *plainBlock, uint8_t *cipherBlock)
{
          #ifdef MODULE_AES_HARDWARE_ROCC_MEM

               ROCC_INSTRUCTION_2(4, 0, 7);
               ROCC_INSTRUCTION_2(plainBlock, cipherBlock, 5);

          #else
          #ifdef MODULE_AES_HARDWARE_ROCC_REG
          
               uint32_t *new_plainBlock, *new_cipherBlock;
               new_plainBlock = (uint32_t*)(plainBlock);
               new_cipherBlock = (uint32_t*)(cipherBlock);
               ROCC_INSTRUCTION_2(new_plainBlock[0], new_plainBlock[1], 6);
               ROCC_INSTRUCTION_2(new_plainBlock[2], new_plainBlock[3], 6);
               ROCC_INSTRUCTION_2(0, 0, 8);
               ROCC_INSTRUCTION(new_cipherBlock[0], 0, 0, 7);
               ROCC_INSTRUCTION(new_cipherBlock[1], 1, 1, 7);
               ROCC_INSTRUCTION(new_cipherBlock[2], 2, 2, 7);
               ROCC_INSTRUCTION(new_cipherBlock[3], 3, 3, 7);
               
          #else
          #ifdef MODULE_AES_HARDWARE_MMIO
          
               uint32_t *new_plainBlock, *new_cipherBlock;
               new_plainBlock = (uint32_t*)(plainBlock);
               new_cipherBlock = (uint32_t*)(cipherBlock);
               // wait for peripheral to be ready
               while ((reg_read8(STATUS) & 0x2) == 0){
               __asm__ volatile("nop");
               } 
               reg_write32(MESSAGE, new_plainBlock[0]);
               reg_write32(MESSAGE+4, new_plainBlock[1]);
               reg_write32(MESSAGE+8, new_plainBlock[2]);
               reg_write32(MESSAGE+12, new_plainBlock[3]);
               reg_write32(EN_DE, 0x1);
               // wait for peripheral to complete
               while ((reg_read8(STATUS) & 0x2) == 0){
               __asm__ volatile("nop");
               __asm__ volatile("nop");
               } 
               new_cipherBlock[0] = reg_read32(MESSAGE);
               new_cipherBlock[1] = reg_read32(MESSAGE+4);
               new_cipherBlock[2] = reg_read32(MESSAGE+8);
               new_cipherBlock[3] = reg_read32(MESSAGE+12);

          #endif /* MODULE_AES_HARDWARE_ROCC_MEM */
          #endif /* MODULE_AES_HARDWARE_ROCC_REG */
          #endif /* MODULE_AES_HARDWARE_MMIO */
}


void HACCV_decrypt(const uint8_t *cipherBlock, uint8_t *plainBlock)
{     
          #ifdef MODULE_AES_HARDWARE_ROCC_MEM

               ROCC_INSTRUCTION_2(4, 0, 7);
               ROCC_INSTRUCTION_2(cipherBlock, plainBlock, 6);

          #else
          #ifdef MODULE_AES_HARDWARE_ROCC_REG
          
               uint32_t *new_plainBlock, *new_cipherBlock;
               new_plainBlock = (uint32_t*)(plainBlock);
               new_cipherBlock = (uint32_t*)(cipherBlock);
               ROCC_INSTRUCTION_2(new_cipherBlock[0], new_cipherBlock[1], 6);
               ROCC_INSTRUCTION_2(new_cipherBlock[2], new_cipherBlock[3], 6);
               ROCC_INSTRUCTION_2(0, 0, 9);
               ROCC_INSTRUCTION(new_plainBlock[0], 0, 0, 7);
               ROCC_INSTRUCTION(new_plainBlock[1], 1, 1, 7);
               ROCC_INSTRUCTION(new_plainBlock[2], 2, 2, 7);
               ROCC_INSTRUCTION(new_plainBlock[3], 3, 3, 7);
               
          #else
          #ifdef MODULE_AES_HARDWARE_MMIO
          
               uint32_t *new_plainBlock, *new_cipherBlock;
               new_plainBlock = (uint32_t*)(plainBlock);
               new_cipherBlock = (uint32_t*)(cipherBlock);
               // wait for peripheral to be ready
               while ((reg_read8(STATUS) & 0x2) == 0){
               __asm__ volatile("nop");
               } 
               reg_write32(MESSAGE, new_cipherBlock[0]);
               reg_write32(MESSAGE+4, new_cipherBlock[1]);
               reg_write32(MESSAGE+8, new_cipherBlock[2]);
               reg_write32(MESSAGE+12, new_cipherBlock[3]);
               reg_write32(EN_DE, 0x2);
               // wait for peripheral to complete
               while ((reg_read8(STATUS) & 0x2) == 0){
               __asm__ volatile("nop");
               } 
               new_plainBlock[0] = reg_read32(MESSAGE);
               new_plainBlock[1] = reg_read32(MESSAGE+4);
               new_plainBlock[2] = reg_read32(MESSAGE+8);
               new_plainBlock[3] = reg_read32(MESSAGE+12);

          #endif /* MODULE_AES_HARDWARE_ROCC_MEM */
          #endif /* MODULE_AES_HARDWARE_ROCC_REG */
          #endif /* MODULE_AES_HARDWARE_MMIO */
     
}