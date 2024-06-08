#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <limits.h>

#include "aes.h"
#include "ecb.h"
#include "ciphers.h"

typedef uint32_t u32;

typedef unsigned char                uint8_t;

// TEST ECB ENCRYPT
//31303030 33303230 35303430 37303630
static uint8_t TEST_1_KEY[] = {
    0x31, 0x30, 0x30, 0x30, 0x33, 0x30, 0x32, 0x30,
    0x35, 0x30, 0x34, 0x30, 0x37, 0x30, 0x36, 0x30
};
static uint8_t TEST_1_KEY_LEN = 16;

//34333231 38373635 38393039 34353637
static uint8_t TEST_1_PLAIN[] = {
    0x34, 0x33, 0x32, 0x31, 0x38, 0x37, 0x36, 0x35,
    0x38, 0x39, 0x30, 0x39, 0x34, 0x35, 0x36, 0x37,
};
static uint8_t TEST_1_PLAIN_LEN = 16;

//f3e5a9a8 dbdb16d1 d731001e 0c8c58ad
static uint8_t TEST_1_CIPHER[] = {
    0xf3, 0xe5, 0xa9, 0xa8, 0xdb, 0xdb, 0x16, 0xd1,
    0xd7, 0x31, 0x00, 0x1e, 0x0c, 0x8c, 0x58, 0xad,
};
static uint8_t TEST_1_CIPHER_LEN = 16;

static void test_encrypt_op(uint8_t* key, uint8_t key_len, uint8_t* input,
    uint8_t input_len, uint8_t* output, uint8_t output_len)
{
    cipher_t cipher;
    int len, err, cmp;
    uint8_t data[16], data2[16];

    clock_t t1, t2;
    int time_taken;
    t1 = clock();
    err = cipher_init(&cipher, CIPHER_AES_128, key, key_len);
    t2 = clock();
    time_taken = ((int)t2)-((int)t1);
    printf("TIME INIT CIPHER: %d T1: %d T2: %d\n", time_taken, t1, t2);
    printf("err = %d\n", err);

    t1 = clock();
    len = cipher_encrypt_ecb(&cipher, input, input_len, data);
    t2 = clock();
    time_taken = ((int)t2)-((int)t1);
    printf("TIME ENCRYPT: %d T1: %d T2: %d\n", time_taken, t1, t2);
    printf("len = %d\n", len);
    printf("output_len = %d\n", output_len);

    t1 = clock();
    len = cipher_decrypt_ecb(&cipher, data, input_len, data2);
    t2 = clock();
    time_taken = ((int)t2)-((int)t1);
    printf("TIME DECRYPT: %d T1: %d T2: %d\n", time_taken, t1, t2);
    printf("len = %d\n", len);

    printf("Key:\n");
    for (int i = 0; i < TEST_1_KEY_LEN; i++) {
        printf("%02x", key[i]);
    }
    printf("\n");
    printf("Input: \n");
    for (int i = 0; i < TEST_1_PLAIN_LEN; i++) {
        printf("%02x", input[i]);
    }
    printf("\n");
    printf("Output: \n");
    for (int i = 0; i < TEST_1_CIPHER_LEN; i++) {
        printf("%02x", data[i]);
    }
    printf("\n");
    printf("Expected Output: \n");
    for (int i = 0; i < TEST_1_CIPHER_LEN; i++) {
        printf("%02x", output[i]);
    }
    printf("\n");
    printf("Decryption: \n");
    for (int i = 0; i < TEST_1_CIPHER_LEN; i++) {
        printf("%02x", data2[i]);
    }

    printf("\n");

}


int main(){
	test_encrypt_op(TEST_1_KEY, TEST_1_KEY_LEN, TEST_1_PLAIN, TEST_1_PLAIN_LEN, TEST_1_CIPHER, TEST_1_CIPHER_LEN);
	return 0;
}

