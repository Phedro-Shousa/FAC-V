
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdalign.h>

#ifdef HACCV
     #ifdef MODULE_AES_HARDWARE_MMIO

          #define STATUS 0x2000
          #define KEY 0x2004
          #define MESSAGE 0x2014
          #define EN_DE 0x2024
          
          static inline void reg_write32(uintptr_t addr, uint32_t data)
          {
               volatile uint32_t *ptr = (volatile uint32_t *) addr;
               *ptr = data;
          }

          static inline uint32_t reg_read32(uintptr_t addr)
          {
               volatile uint32_t *ptr = (volatile uint32_t *) addr;
               return *ptr;
          }

          static inline uint8_t reg_read8(uintptr_t addr)
          {
               volatile uint8_t *ptr = (volatile uint8_t *) addr;
               return *ptr;
          }
          
          static inline void reg_write8(uintptr_t addr, uint8_t data)
          {
               volatile uint8_t *ptr = (volatile uint8_t *) addr;
               *ptr = data;
          }

     #else

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

     #endif /* MODULE_AES_HARDWARE_MMIO */


     void HACCV_init(const uint8_t *key);

     void HACCV_encrypt(const uint8_t *plainBlock, uint8_t *cipherBlock);

     void HACCV_decrypt(const uint8_t *plainBlock, uint8_t *cipherBlock);

#endif /* HACCV */
