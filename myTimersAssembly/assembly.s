#include "inc/hw_memmap.h"
#include "inc/hw_timer.h"
  name isr_asm
  section .text:CODE
  extern SRV
  public isr_asm_start
  
isr_asm_start:
  ;interrupt clear
  MOV R1, #1
  MOV32 R0, #0x40031000
  STR R1, [R0, #0x24]
  ;srv++
  MOV32 R0, SRV
  LDR R1, [R0]
  ADD R1, R1, #1
  STR R1, [R0]
  bx lr ;return
  end
  