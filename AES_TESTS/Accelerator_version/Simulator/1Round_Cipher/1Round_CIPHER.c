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

void read_key(uint32_t* result){
  //=================READ KEY===============
  uint32_t r1;
  for(int i = 0; i < 4; i += 2){
    ROCC_INSTRUCTION(r1, i, i, 4);
    result[i] = r1;
  }
}

void send_key(uint32_t* key_i){
//=================SEND KEY===============
  uint32_t dummy;
  ROCC_INSTRUCTION(dummy, key_i[0], key_i[1], 0);
  ROCC_INSTRUCTION(dummy, key_i[2], key_i[3], 1);
}


void encript_message(uint32_t* message_i, uint32_t* result, int size_i){
//=================SEND MESSAGE===============
  uint32_t dummy;
  for(int i = 0; i < size_i; i += 2){
    ROCC_INSTRUCTION(dummy, message_i[i], message_i[i+1], 6);
  }
  ROCC_INSTRUCTION(dummy, size_i, size_i, 10);
  if(size_i%4){
    size_i = (size_i / 4)*4 + 4;
  }
  ROCC_INSTRUCTION(dummy, 0, 0, 8);
  for(int i = 0; i < size_i; i++){
    ROCC_INSTRUCTION(dummy, i, i, 7);
    result[i] = dummy;
  }
}

void decript_message(uint32_t* message_i, uint32_t* result, int size_i){
//=================SEND MESSAGE===============
  uint32_t dummy;
  for(int i = 0; i < size_i; i += 2){
    ROCC_INSTRUCTION(dummy, message_i[i], message_i[i+1], 6);
  }
  ROCC_INSTRUCTION(dummy, size_i, size_i, 10);
  if(size_i%4){
    size_i = (size_i / 4)*4 + 4;
  }
  ROCC_INSTRUCTION(dummy, 0, 0, 9);
  for(int i = 0; i < size_i; i++){
    ROCC_INSTRUCTION(dummy, i, i, 7);
    result[i] = dummy;
  }
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
   char* message = "1234567890987654321";

  int size_c = strlen(message);
  int size_i = size_c/4;
  if(size_c%4){
    size_i++;
  }
  int size_result_c = size_c;
  if(size_result_c%16){
    size_result_c = (size_c / 16)*16 + 16;
  }
  int size_result_i = size_result_c/4;
  
  
  union cipher{
  char message_c[size_c] ;
  uint32_t message_i[size_i];
  }cipher1;

  for(int i = 0; i < size_c; i++){
    cipher1.message_c[i] = message[i];
  }

//========================RESULT UNION INSERT===================

  union result{
    char result_c[size_result_c];
    uint32_t result_i[size_result_i];
  }result1;
  
//========================PRINT MESSAGE AND KEY===================

  printf("\nMESSAGE\nChar: "); 
  
  for(int i = 0; i < size_c; i++){
    printf("%c", cipher1.message_c[i]);
  }
  printf("\nInt: "); 
  for(int i = 0; i < size_i; i++){
    printf("%08x ", cipher1.message_i[i]);
  }
  printf("\n"); 
// ================================= 128===========================

  send_key(key1.key_i);
  
  encript_message(cipher1.message_i, result1.result_i, size_i);
  printf("\nENCRIPTED\nInt:");
  for(int i = 0; i < size_result_i; i++){
    printf("%08x ", result1.result_i[i]);
  }

  decript_message(result1.result_i, result1.result_i, size_result_i);
  printf("\nDECRIPTED\nInt:");
  for(int i = 0; i < size_result_i; i++){
    printf("%08x ", result1.result_i[i]);
  }
  printf("\nChar: ");
  for(int i = 0; i < size_result_c; i++){
    printf("%c", result1.result_c[i]);
  }

// ===========================================================
  
  printf("\n");
  printf("\n");
  return 0;
}
