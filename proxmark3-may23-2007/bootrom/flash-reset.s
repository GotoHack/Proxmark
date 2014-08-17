.extern CMain
    
.text
.code 32
.align 0

.global start
start:
    b       Reset
    b       UndefinedInstruction
    b       SoftwareInterrupt
    b       PrefetchAbort
    b       DataAbort
    b       Reserved
    b       Irq
    b       Fiq

Reset:
    ldr     sp,     = 0x00203ff8
    bl      CMain

Fiq:
    b       Fiq
UndefinedInstruction:
    b       UndefinedInstruction
SoftwareInterrupt:
    b       SoftwareInterrupt
PrefetchAbort:
    b       PrefetchAbort
DataAbort:
    b       DataAbort
Reserved:
    b       Reserved
Irq:
    b       Irq

.global CallRam
CallRam:
    ldr     r3,     = 0x00200000
    bx      r3
