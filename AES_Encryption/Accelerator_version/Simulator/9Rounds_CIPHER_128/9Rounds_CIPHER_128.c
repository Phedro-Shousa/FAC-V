// See LICENSE for license details.

#include <assert.h>
#include <stdint.h>
#include <stdio.h>


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


int main() {

   asm volatile(
	"li a0, 0x00018000;"  
	"csrrs x0, mstatus, a0;");	

  uint32_t resp=0, r1 = 0, r2 = 1, r3=2, r4=3, r5=4, r6=5, r7=6, r8=7, r9, r10, r11;

// ================================= 128=============================

//=================SEND KEY===============
// KEY128: 000102030405060708090a0b0c0d0e0f
  ROCC_INSTRUCTION(resp, 0x00010203, 0x04050607, 0);
  ROCC_INSTRUCTION(resp, 0x08090a0b, 0x0c0d0e0f, 1);

//=================READ KEY===============


  ROCC_INSTRUCTION(r1, 0, 0, 4);
  ROCC_INSTRUCTION(r2, 1, 1, 4);
  ROCC_INSTRUCTION(r3, 2, 2, 4);
  ROCC_INSTRUCTION(r4, 3, 3, 4);
  
  printf("\n[INFO] KEY= %08x %08x %08x %08x \n", r1, r2, r3, r4);

  ROCC_INSTRUCTION(r1, 0, 0, 10);
  //=================SEND PLAINTEXT===============
  //00112233445566778899aabbccddeeff
  ROCC_INSTRUCTION(r1, 0x00112233, 0x44556677, 6);
  ROCC_INSTRUCTION(r1, 0x8899aabb, 0xccddeeff, 7);
  ROCC_INSTRUCTION(r1, 0, 0, 8);
  ROCC_INSTRUCTION(r2, 1, 1, 8);
  ROCC_INSTRUCTION(r3, 2, 2, 8);
  ROCC_INSTRUCTION(r4, 3, 3, 8);
  printf("[INFO] PLAINTEXT= %08x %08x %08x %08x \n", r1, r2, r3, r4);
  
  //=================READ CIPHER OUT===============
  //EN
  
  ROCC_INSTRUCTION(r1, 0, 0, 9);
  ROCC_INSTRUCTION(r2, 1, 1, 9);
  ROCC_INSTRUCTION(r3, 2, 2, 9);
  ROCC_INSTRUCTION(r4, 3, 3, 9);
  printf("[INFO] CIPHER OUT= %08x %08x %08x %08x \n", r1, r2, r3, r4);

  ROCC_INSTRUCTION(r1, 0, 0, 11);

  ROCC_INSTRUCTION(r1, 0x69c4e0d8, 0x6a7b0430, 6);
  ROCC_INSTRUCTION(r1, 0xd8cdb780, 0x70b4c55a, 7);

  ROCC_INSTRUCTION(r1, 0, 0, 11);

  ROCC_INSTRUCTION(r1, 0, 0, 9);
  ROCC_INSTRUCTION(r2, 1, 1, 9);
  ROCC_INSTRUCTION(r3, 2, 2, 9);
  ROCC_INSTRUCTION(r4, 3, 3, 9);
  printf("[INFO] INVERSE CIPHER OUT= %08x %08x %08x %08x \n", r1, r2, r3, r4);
  return 0;
}
