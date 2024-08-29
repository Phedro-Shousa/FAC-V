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
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <limits.h>

#define HACCV
//#define MODULE_AES_HARDWARE_MMIO
#define MODULE_AES_HARDWARE_ROCC_REG
//#define MODULE_AES_HARDWARE_ROCC_MEM


          #define STR1(x) #x
          #ifndef STR
          #define STR(x) STR1(x)
          #endif

          #define CUSTOM_0 0b0001011
          #define CUSTOM_1 0b0101011
          #define CUSTOM_2 0b1011011
          #define CUSTOM_3 0b1111011

          #define ROCC_INSTRUCTION(rd , rs1 , rs2 , func7) \
          __asm__ volatile (".insn r " STR(CUSTOM_0) ", " STR(0x7) ", " STR(func7) ", %0, %1, %2" \
               : "=r"(rd) \
               : "r"(rs1), "r"(rs2))

          #define ROCC_INSTRUCTION_2(rs1, rs2, func7)                                   \
          {                                                                                  \
          __asm__ volatile(                                                                    \
               ".insn r " STR(CUSTOM_0) ", " STR(0x3) ", " STR(func7) ", x0, %0, %1" \
               :                                                                            \
               : "r"(rs1), "r"(rs2));                                                       \
          }

typedef uint32_t u32;

typedef unsigned char                uint8_t;

// TEST ECB ENCRYPT
//31303030 33303230 35303430 37303630
static uint8_t TEST_1_KEY[] = {
    0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7,
    0x8, 0x9, 0xA, 0xB, 0xC, 0xD, 0xE, 0xF
};
static uint8_t TEST_1_KEY_LEN = 16;

//34333231 38373635 38393039 34353637
static uint8_t TEST_1_PLAIN[] = {
    0x8, 0x9, 0xA, 0xB, 0xC, 0xD, 0xE, 0xF,
    0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7,
};
static uint8_t TEST_1_PLAIN_LEN = 16;

//69c4e0d86a7b0430d8cdb78070b4c55a
static uint8_t TEST_1_CIPHER[] = {    
    0x37, 0x29, 0xa3, 0x6c, 0xaf, 0xe9, 0x84, 0xff,
    0x46, 0x22, 0x70, 0x42, 0xee, 0x24, 0x83, 0xf6
};
static uint8_t TEST_1_CIPHER_LEN = 16;

static void test_encrypt_op(uint8_t* key, uint8_t key_len, uint8_t* input,
    uint8_t input_len, uint8_t* output, uint8_t output_len)
{   
    (void)(input_len);
    (void)(output);
    (void)(output_len);
    uint8_t data[16], data2[16];
    
    printf("\n\n");
    printf("KEY: ");
    for (int i = 0; i < TEST_1_KEY_LEN; i++) {
        printf("%02x", key[i]);
    }
    printf("\n\n");
    printf("Input: ");
    for (int i = 0; i < TEST_1_PLAIN_LEN; i++) {
        printf("%02x", input[i]);
    }
    printf("\n\n");

    __asm__ volatile(
                  "li a0, 0x00018000;"  
                  "csrrs x0, mstatus, a0;");	
               
    uint32_t *new_key;
    new_key = (uint32_t*)(key);
    ROCC_INSTRUCTION_2(new_key[0], new_key[1], 0);
    ROCC_INSTRUCTION_2(new_key[2], new_key[3], 1);   
    

    uint32_t *new_plainBlock, *new_cipherBlock;
    new_plainBlock = (uint32_t*)(input);
    new_cipherBlock = (uint32_t*)(data);
    ROCC_INSTRUCTION_2(new_plainBlock[0], new_plainBlock[1], 6);
    ROCC_INSTRUCTION_2(new_plainBlock[2], new_plainBlock[3], 6);
    //ROCC_INSTRUCTION_2(4, 4, 10);
    ROCC_INSTRUCTION_2(0, 0, 8);
    ROCC_INSTRUCTION(new_cipherBlock[0], 0, 0, 7);
    ROCC_INSTRUCTION(new_cipherBlock[1], 1, 1, 7);
    ROCC_INSTRUCTION(new_cipherBlock[2], 2, 2, 7);
    ROCC_INSTRUCTION(new_cipherBlock[3], 3, 3, 7);


    new_plainBlock = (uint32_t*)(data2);
    new_cipherBlock = (uint32_t*)(data);
    ROCC_INSTRUCTION_2(new_cipherBlock[0], new_cipherBlock[1], 6);
    ROCC_INSTRUCTION_2(new_cipherBlock[2], new_cipherBlock[3], 6);
    //ROCC_INSTRUCTION_2(4, 4, 10);
    ROCC_INSTRUCTION_2(0, 0, 9);
    ROCC_INSTRUCTION(new_plainBlock[0], 0, 0, 7);
    ROCC_INSTRUCTION(new_plainBlock[1], 1, 1, 7);
    ROCC_INSTRUCTION(new_plainBlock[2], 2, 2, 7);
    ROCC_INSTRUCTION(new_plainBlock[3], 3, 3, 7);

    printf("\n\n");
    printf("Encrypted: ");
    for (int i = 0; i < TEST_1_CIPHER_LEN; i++) {
        printf("%02x", data[i]);
    }
    printf("\n\n");
    printf("Decrypted: ");
    for (int i = 0; i < TEST_1_CIPHER_LEN; i++) {
        printf("%02x", data2[i]);
    }
    printf("\n\n");

}


int main(void){
	test_encrypt_op(TEST_1_KEY, TEST_1_KEY_LEN, TEST_1_PLAIN, TEST_1_PLAIN_LEN, TEST_1_CIPHER, TEST_1_CIPHER_LEN);
	return 0;
}


