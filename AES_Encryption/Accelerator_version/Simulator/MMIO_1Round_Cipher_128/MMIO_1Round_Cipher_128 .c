#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mmio.h"

#define STATUS 0x2000
#define KEY 0x2004
#define MESSAGE 0x2014
#define EN_DE 0x2024


int main(void)
{

//========================KEY UNION INSERT===================
  char key[16] = "0001020304050607";
  union key{
    char key_c[16];
    uint32_t key_i[4];
  }key1;
  
  for(int i = 0; i < 16; i++){
    key1.key_c[i] = key[i];
  }
//========================MESSAGE UNION INSERT===================
   char* message = "1234567890987654";

  int size_c = strlen(message);
  int size_i = size_c/4;
  if(size_c%4){
    size_i++;
  }
  
  
  union cipher{
  char message_c[size_c] ;
  uint32_t message_i[size_i];
  }cipher1;

  for(int i = 0; i < size_c; i++){
    cipher1.message_c[i] = message[i];
  }

//========================RESULT SIZE===================


  int size_result_c = size_c;
  if(size_result_c%16){
    size_result_c = (size_c / 16)*16 + 16;
  }
  int size_result_i = size_result_c/4;

  union result{
    char result_c[size_result_c];
    uint32_t result_i[size_result_i];
  }result1;
  
//========================PRINT MESSAGE ===================

  printf("\nKEY\nChar: "); 
  for(int i = 0; i < 16; i++){
    printf("%c", key1.key_c[i]);
  }
  printf("\nInt: "); 
  for(int i = 0; i < 4; i++){
    printf("%08x ", key1.key_i[i]);
  }
  printf("\nMESSAGE\nChar: "); 
  
  for(int i = 0; i < size_c; i++){
    printf("%c", cipher1.message_c[i]);
  }
  printf("\nInt: "); 
  for(int i = 0; i < size_i; i++){
    printf("%08x ", cipher1.message_i[i]);
  }
  printf("\n"); 
//=======================================================

  uint32_t r1, r2, r3, r4;
  r1 = reg_read32(KEY);
  r2 = reg_read32(KEY+4);
  r3 = reg_read32(KEY+8);
  r4 = reg_read32(KEY+12);
  printf("\n[HW] KEY = %08x %08x %08x %08x", r1, r2, r3, r4);
  r1 = reg_read32(MESSAGE);
  r2 = reg_read32(MESSAGE+4);
  r3 = reg_read32(MESSAGE+8);
  r4 = reg_read32(MESSAGE+12);
  printf("\n[HW] MESSAGE = %08x %08x %08x %08x", r1, r2, r3, r4);

  // wait for peripheral to be ready
  while ((reg_read8(STATUS) & 0x2) == 0) ;

  reg_write32(KEY, key1.key_i[0]);
  reg_write32(KEY+4, key1.key_i[1]);
  reg_write32(KEY+8, key1.key_i[2]);
  reg_write32(KEY+12, key1.key_i[3]);

  reg_write32(MESSAGE, cipher1.message_i[0]);
  reg_write32(MESSAGE+4, cipher1.message_i[1]);
  reg_write32(MESSAGE+8, cipher1.message_i[2]);
  reg_write32(MESSAGE+12, cipher1.message_i[3]);

  reg_write8(EN_DE, 0x1);

  // wait for peripheral to complete
  while ((reg_read8(STATUS) & 0x1) == 0) ;

  r1 = reg_read32(KEY);
  r2 = reg_read32(KEY+4);
  r3 = reg_read32(KEY+8);
  r4 = reg_read32(KEY+12);
  printf("\n[HW] KEY = %08x %08x %08x %08x", r1, r2, r3, r4);
  r1 = reg_read32(MESSAGE);
  r2 = reg_read32(MESSAGE+4);
  r3 = reg_read32(MESSAGE+8);
  r4 = reg_read32(MESSAGE+12);
  printf("\n[HW] MESSAGE = %08x %08x %08x %08x", r1, r2, r3, r4);
  return 0;
}
// DOC include end: GCD test

