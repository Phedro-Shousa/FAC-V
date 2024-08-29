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



int main( int argc, char *argv[] ) {
  
   asm volatile(
	"li a0, 0x00018000;"  
	"csrrs x0, mstatus, a0;");	

  uint32_t data = 0xdead;

  uint16_t resp=0, addr = 1;
  printf("\nresp = %08x &data = %d, data = %08x, addr = %d\n", resp, &data, data, addr);
  printf("LOAD");
  ROCC_INSTRUCTION(resp, &data, addr, 2);
  ROCC_INSTRUCTION(resp, &data, addr, 1);
  printf("\nresp = %08x &data = %d, data = %08x, addr = %d\n", resp, &data, data, addr);
  printf("STORE");
  ROCC_INSTRUCTION(resp, &data, addr, 4);
  printf("\nresp = %08x &data = %d, data = %08x, addr = %d\n", resp, &data, data, addr);
  printf("LOAD");
  ROCC_INSTRUCTION(resp, &data, addr, 2);
  ROCC_INSTRUCTION(resp, &data, addr, 1);
  printf("\nresp = %08x &data = %d, data = %08x, addr = %d\n", resp, &data, data, addr);

  return 0;
}

/*
    mem : {
      req : {
            flip ready : UInt<1>, 
            valid : UInt<1>, 
            bits : {
              addr : UInt<40>, 
              tag : UInt<7>, 
              cmd : UInt<5>, 
              typ : UInt<3>, 
              phys : UInt<1>, 
              data : UInt<64>}}, 
      s1_kill : UInt<1>, 
      s1_data : UInt<64>, 
      flip s2_nack : UInt<1>, 
      flip acquire : UInt<1>, 
      flip release : UInt<1>, 
      flip resp : {
        valid : UInt<1>, 
        bits : {
          addr : UInt<40>, 
          tag : UInt<7>, 
          cmd : UInt<5>, 
          typ : UInt<3>, 
          data : UInt<64>, 
          replay : UInt<1>, 
          has_data : UInt<1>, 
          data_word_bypass : UInt<64>, 
          store_data : UInt<64>}}, 
      flip replay_next : UInt<1>, 
      flip xcpt : {ma : {ld : UInt<1>, st : UInt<1>}, pf : {ld : UInt<1>, st : UInt<1>}}, 
      invalidate_lr : UInt<1>, 
      flip ordered : UInt<1>}, 
      */