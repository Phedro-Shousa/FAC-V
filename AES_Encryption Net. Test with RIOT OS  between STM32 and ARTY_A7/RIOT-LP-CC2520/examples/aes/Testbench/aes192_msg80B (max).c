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

#include "crypto/aes.h"
#include "crypto/modes/ecb.h"
#include "crypto/ciphers.h"

#define HACCV

typedef uint32_t u32;

typedef unsigned char                uint8_t;


static uint8_t TEST_1_KEY_LEN = 24;

static uint8_t TEST_1_PLAIN[] = {
    0x8, 0x9, 0xA, 0xB, 0xC, 0xD, 0xE, 0xF,
    0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7,
    0x8, 0x9, 0xA, 0xB, 0xC, 0xD, 0xE, 0xF,
    0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7,
    0x8, 0x9, 0xA, 0xB, 0xC, 0xD, 0xE, 0xF,
    0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7,
    0x8, 0x9, 0xA, 0xB, 0xC, 0xD, 0xE, 0xF,
    0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7,
    0x8, 0x9, 0xA, 0xB, 0xC, 0xD, 0xE, 0xF,
    0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7,
};
static uint8_t TEST_1_PLAIN_LEN = 80;

static void test_encrypt_op( uint8_t key_len, uint8_t* input,
    uint8_t input_len)
{   
    (void)(input_len);
    cipher_context_t ctx;
    uint8_t data[input_len], data2[input_len];
    uint8_t key[] = {
    0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7,
    0x8, 0x9, 0xA, 0xB, 0xC, 0xD, 0xE, 0xF,
    0x8, 0x9, 0xA, 0xB, 0xC, 0xD, 0xE, 0xF
    };
    printf("START\n\n");

    volatile uint32_t t1;
    volatile uint32_t t2;
    volatile uint32_t timei, timee, timed;
    


    uint8_t block_size = AES_BLOCK_SIZE;
    
    if (input_len % block_size != 0) {
        printf("ERROR SIZE MESSAGE");
        return;
    }

/*
    printf("\n\n");
    printf("KEY: ");
    for (int i = 0; i < key_len; i++) {
        printf("%02x", key[i]);
    }
    printf("\n\n");
    printf("Input: ");
    for (int i = 0; i < TEST_1_PLAIN_LEN; i++) {
        printf("%02x", input[i]);
    }
    printf("\n\n");
*/

    for(int i=0; i<1000; i++){
    __asm__ volatile ("csrr %0, mcycle" : "=r" (t1));
    aes_init(&ctx, key, key_len);   
    __asm__ volatile ("csrr %0, mcycle" : "=r" (t2));
    timei = t2-t1;
    __asm__ volatile ("csrr %0, mcycle" : "=r" (t1));
        aes_encrypt(&ctx, input, data);  
        aes_encrypt(&ctx, input + 16, data + 16);  
        aes_encrypt(&ctx, input + 32, data + 32);  
        aes_encrypt(&ctx, input + 48, data + 48);  
        aes_encrypt(&ctx, input + 64, data + 64);  
    __asm__ volatile ("csrr %0, mcycle" : "=r" (t2));
    timee = t2-t1;
    __asm__ volatile ("csrr %0, mcycle" : "=r" (t1));
        aes_decrypt(&ctx, data, data2);  
        aes_decrypt(&ctx, data + 16, data2 + 16);  
        aes_decrypt(&ctx, data + 32, data2 + 32);  
        aes_decrypt(&ctx, data + 48, data2 + 48);  
        aes_decrypt(&ctx, data + 64, data2 + 64);  
    __asm__ volatile ("csrr %0, mcycle" : "=r" (t2));
    timed = t2-t1;
    printf("%ld %ld %ld\n", timei, timee, timed);
    key[0]= key[0]+1;
    }


/*

    printf("\n\n");
    printf("Encrypted: ");
    for (int i = 0; i < input_len; i++) {
        printf("%02x", data[i]);
    }
    printf("\n\n");
    printf("Decrypted: ");
    for (int i = 0; i < input_len; i++) {
        printf("%02x", data2[i]);
    }
    printf("\n\n");
*/
}

int main(void){
	test_encrypt_op(TEST_1_KEY_LEN, TEST_1_PLAIN, TEST_1_PLAIN_LEN);
	return 0;
}

