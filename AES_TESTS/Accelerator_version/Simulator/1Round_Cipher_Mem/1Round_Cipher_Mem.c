// See LICENSE for license details.

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STR1(x) #x
#ifndef STR
#define STR(x) STR1(x)
#endif

#define CUSTOM_0 0b0001011
#define CUSTOM_1 0b0101011
#define CUSTOM_2 0b1011011
#define CUSTOM_3 0b1111011

#define ROCC_INSTRUCTION(rd , rs1 , rs2 , func7) \
    asm volatile (".insn r " STR(CUSTOM_0) ", " STR(0x7) ", " STR(func7) ", %0, %1, %2" \
        : "=r"(rd) \
        : "r"(rs1), "r"(rs2))


void send_key(uint32_t* key_i){
  uint32_t dummy;
  ROCC_INSTRUCTION(dummy, key_i[0], key_i[1], 0);
  ROCC_INSTRUCTION(dummy, key_i[2], key_i[3], 1);
}

void read_key(uint32_t* key){
  uint32_t r1;
  for(int i = 0; i < 4; i++){
    ROCC_INSTRUCTION(r1, i, i, 2);
    key[i] = r1;
  }
}

void encrypt(uint32_t* address_load, uint32_t* address_store, int size_i){
  uint32_t dummy;
  ROCC_INSTRUCTION(dummy, size_i, 0, 5);
  ROCC_INSTRUCTION(dummy, address_load, address_store, 3);
}

void decrypt(uint32_t* address_load,uint32_t* address_store, int size_i){
  uint32_t dummy;
  ROCC_INSTRUCTION(dummy, size_i, 0, 5);
  ROCC_INSTRUCTION(dummy, address_load, address_store, 4);
}


int main( int argc, char *argv[] ) {
  
   asm volatile(
	"li a0, 0x00018000;"  
	"csrrs x0, mstatus, a0;");	


//========================KEY UNION INSERT===================
  char key[16] = "0001020304050607";
  union key{
    char key_c[16];
    uint32_t key_i[4];
  }key1;
  
  for(int i = 0; i < 16; i++){
    key1.key_c[i] = key[i];
  }
  printf("\nKEY\nChar: "); 
  for(int i = 0; i < 16; i++){
    printf("%c", key1.key_c[i]);
  }
  printf("\nInt: "); 
  for(int i = 0; i < 4; i++){
    printf("%08x ", key1.key_i[i]);
  }
//========================MESSAGE UNION INSERT===================
   char* message = "12345678909876541234567890987654";

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

  printf("\nMESSAGE\nChar: "); 
  
  for(int i = 0; i < size_c; i++){
    printf("%c", cipher1.message_c[i]);
  }
  printf("\nInt: "); 
  for(int i = 0; i < size_i; i++){
    printf("%08x ", cipher1.message_i[i]);
  }
  printf("\n"); 
  int read[4];
// =================================128===========================

  send_key(key1.key_i);
  printf("\n[HW INFO] SEND KEY");
  
  read_key(read);
  printf("\n[HW INFO] READ KEY: ");
  for(int i = 0; i < 4; i++){
    printf("%08x ", read[i]);
  }

  encrypt(cipher1.message_i, result1.result_i,size_i);
  printf("\nENCRYPTED\nInt:");
  for(int i = 0; i < size_result_i; i++){
    printf("%08x ", result1.result_i[i]);
  }

  decrypt(result1.result_i, cipher1.message_i,size_result_i);
  printf("\nDECRYPTED\nInt:");
  for(int i = 0; i < size_result_i; i++){
    printf("%08x ", cipher1.message_i[i]);
  }
  printf("\nChar: ");
  for(int i = 0; i < size_result_c; i++){
    printf("%c", cipher1.message_c[i]);
  }

// ===========================================================
  
  printf("\n");
  printf("\n");
  return 0;
}

