LEEK16 Architecture
===================

This is a cute little RISC machine designed to have an easy (I hope) instruction set to implement in hardware. The opcodes for everything are essentially temporary untill I figure out more specifics of the hardware. For now, I think this is a pretty good representation of the capabilities of the final architecture.

Instruction format
------------------

Instructions are one of the following 5 types. During the operation. The result of any operation will either be written to the register destination, or the address in memory stored in destination depending on the operation. If the destination register is 0, then the output is discarded

<table style="width:100%; table-layout:fixed">
    <tr align="center">
        <th>
        <th> 4 bits
        <th> 4 bits
        <th> 4 bits
        <th> 4 bits
    <tr align="center">
        <th> IIR
        <td> op code
        <td colspan="2"> immediate
        <td> destination
    <tr align="center">
        <th> RIR
        <td> op code
        <td> register
        <td> immediate
        <td> destination
    <tr align="center">
        <th> RRR
        <td> op code
        <td> register A
        <td> register B
        <td> destination
    <tr align="center">
        <th> IR
        <td> 0000
        <td> op code
        <td> immediate
        <td> destination
    <tr align="center">
        <th> RR
        <td> 0000
        <td> op code
        <td> register
        <td> destination
</table>

Registers
---------

There are 16 registers. All are 16 bits in length. There are 6 special purpose registers. All may be written to (potentially breaking things) with the exception of MBZ.

```
0  = 0x0 = MBZ
11 = 0xb = AUX
12 = 0xc = IHP
13 = 0xd = FLAGS
14 = 0xe = STACK
15 = 0xf = PC
```
The MBZ register is the **M**ust **B**e **Z**ero register. It will always hold the value 0, any attempts to write to it will be ignored.
The AUX registers is reserved for latter use but has no current use.
The IHP register is the **I**nterrupt **H**andler **P**ointer. It points to a subroutine in memory that is called when an interrupt occurs.
The STACK register is used for PUSH and POP operations. It points to the most recently PUSHed value.
The PC register is the program counter. This points to the next operation to be executed by the processor.
The bits in the FLAGS register correspond to flags set by operations. Any flag without a designation should be treated as reserved. Currently designated flags are shown in the following table.

<table style="width:100%; table-layout: fixed">
    <tr align="center">
        <th> 0 <th> 1 <th> 2 <th> 3 <th> 4 <th> 5 <th> 6 <th> 7
    <tr align="center">
        <td> ZERO
        <td> NEG
        <td> CARRY
        <td> OVER
        <td> ICF
        <td>
        <td>
        <td> ISFs
    <tr align="center">
        <th> 8 <th> 9 <th> 10 <th> 11 <th> 12 <th> 13 <th> 14 <th> 15
    <tr align="center">
        <td> ISF0
        <td> ISF1
        <td> ISF2
        <td> ISF3
        <td> ISF4
        <td> ISF5
        <td> ISF6
        <td> ISF7
</table>

The ZERO flag is set if the result of an arithmetic or logic operation is 0. Similarly, the NEG flag is set if the result of an arithmetic or logic opreation is negative when it is interpreted in twos-complement. The CARRY flag is set set if there is an unsigned overflow of an addition opperation, or an unsigned underflow of a subtraction operation (a borrow). The OVER flag is set if there is a signed overflow from addition or subtraction.
The ICF is the **I**nterrupt **C**ontrol **F**lag. It's state affects how interrupts are handled. Flags 7 ~ 15 are **I**nterrupt **S**ignal **F**lags. They are set if the system recieves an interrupt signal along the corresponding interrupt line (ISFs is for software interrupts).

Interrupts
----------

This architecture as support for up to 8 hardware interrupt lines. When an interrupt signal is recieved, one of the Interrupt Signal Flags is set. If in addition the Interrupt Control Flag is set, normal operation is interrupted and the interrupt handler is called. Concretely, the value of PC is pushed to the stack as if PUSH were called and the PC register is set to the value of the IHP register. Additionally the Interrupt Control Flag is cleared. Because this architecture does not feature privileged execution contexts, Interrupt Signal Flags and the Interrupt Control Flag can be set and cleared from any userspace code. Although it is *possible* it is really not a good idea to modify these flags under normal operation.

Opperations Reference
---------------------

|           | Mode | Op Code |
|-----------|------|---------|
| **NOP**   | RR   | 0x00    |
| **MOV**   | RR   | 0x01    |
| **REL+**  | IIR  | 0x1     |
| **REL-**  | IIR  | 0x2     |
||||
| **ADD**   | RRR  | 0x3     |
| **ADDC**  | RRR  | 0x4     |
| **ADDi**  | RIR  | 0x5     |
| **SUB**   | RRR  | 0x6     |
| **SUBB**  | RRR  | 0x7     |
| **SUBi**  | RIR  | 0x8     |
| **MUL**   | RRR  | 0x9     |
| **DIV**   | RRR  | 0xa     |
| **ROT**   | RRR  | 0xb     |
| **ROTi**  | RIR  | 0xc     |
||||
| **OR**    | RRR  | 0xd     |
| **AND**   | RRR  | 0xe     |
| **XOR**   | RRR  | 0xf     |
| **NOT**   | RR   | 0x02    |
||||
| **STORE** | RR   | 0x03    |
| **LOAD**  | RR   | 0x04    |
| **PUSH**  | RR   | 0x05    |
| **POP**   | RR   | 0x06    |
||||
| **FPRED** | IR   | 0x07    |
| **FSET**  | IR   | 0x08    |
| **FCLR**  | IR   | 0x09    |
| **FTOG**  | IR   | 0x0a    |
||||
| **INTR**  | RR   | 0x0b    |
| **WFI**   | RR   | 0x0c    |

Operations
----------

### Move and Set

#### NOP
`0000 0000 0000 0000`  
RR type.  
Does no opperation.

#### MOV
`0000 0001 [rA] [rD]`  
RR type.  
Copies the value of rA into rD.

#### REL+
`0001 [  off  ] [rD]`  
IIR type.  
Stores the value of rPC + off in rD. You can use this for unconditional jumps if rD is set to rPC.

#### REL-
`0010 [  off  ] [rD]`  
IIR type.  
Stores the value of rPC - off in rD. You can use this for unconditional jumps if rD is set to rPC.

### Arithmetic

#### ADD
`0011 [rA] [rB] [rD]`  
RRR type.  
Stores the result of rA+rB in register rD. Sets the carry, overflow, negative, and zero flags.

#### ADDC
`0100 [rA] [rB] [rD]`  
RRR type.  
Add with carry. Stores the result of rA+rB+fCARRY in register rD. Sets the carry, overflow, negative, and zero flags:

#### ADDi
`0101 [rA] [iB] [rD]`  
RIR type.  
Stores the result of rA+iB in register rD. Sets the carry, overflow, negative, and zero flags.

#### SUB
`0110 [rA] [rB] [rD]`  
RRR type.  
Stores the result of rA-rB in register rD. Sets the carry (for a borrow), overflow, negative, and zero flags.

#### SUBB
`0111 [rA] [rB] [rD]`  
RRR type.  
Subtraction with borrow. Stores the result of rA-rB-fCARRY in register rD. Sets the carry, overflow, negative, and zero flags.

#### SUBi
`1000 [rA] [iB] [rD]`  
RIR type.  
Stores the result of rA-iB in register rD. Sets the carry (for a borrow), overflow, negative, and zero flags.

#### MUL
`1001 [rA] [rB] [rD]`  
RRR type.  
Calculates the product of rA and rB. The lower word of the result is stored in rD. The upper word is stored in rAUX.

#### DIV
`1010 [rA] [rB] [rD]`  
RRR type.  
Computes the division rAUX:rA / rB. The quotient is stored in rD and the remainder is stored in rAUX.

#### ROT
`1011 [rA] [rB] [rD]`  
RRR type.  
Shifts rA to the left rB places (with wrapping) and stores the result in rD.

#### ROTi
`1100 [rA] [iB] [rD]`  
RIR type.  
Shifts rA to the left iB places (with wrapping) and stores the result in rD.

### Logic

#### OR
`1101 [rA] [rB] [rD]`  
RRR type.  
Performs a bitwise OR on rA and rB and stores the result in rD. Sets the zero flag.

#### AND
`1110 [rA] [rB] [rD]`  
RRR type.  
Performs a bitwise AND on rA and rB and stores the result in rD. Sets the zero flag.

#### XOR
`1111 [rA] [rB] [rD]`  
RRR type.  
Performs a bitwse XOR on rA and rB and stores the result in rD. Sets the zero flag.

#### NOT
`0000 0010 [rA] [rD]`  
RR type.  
Performs a bitwse NOT on rA and stores the result in rD. Sets the zero flag.

### Memory

#### STORE
`0000 0011 [rA] [ad]`  
RR type.  
Stores rA in the address ad in memory.

#### LOAD
`0000 0100 [ad] [rD]`  
RR type.  
Stores the value at address ad in rD.

#### PUSH
`0000 0101 [rA] 1110`  
RR type.  
Stores rA at the address in rSTACK, then increments rSTACK.

#### POP
`0000 0110 1110 [rD]`  
RR type.  
Stores the value at address rSTACK in rD, then decrements rSTACK.

### Flag Operations

#### FPRED
`0000 0111 [iA] 1111`  
IR type.  
Flag Predicate. This predicates the following command on the iA'th bit in rFLAGS.
If the bit is set, the following command will be executed. If the bit is not set, the following command will be skipped.

#### FSET
`0000 1000 [iA] 1101`  
IR type.  
Flag Set. Sets the iA'th bit of rFLAGS.

#### FCLR
`0000 1001 [iA] 1101`  
IR type.  
Flag Clear. Clears the iA'th bit of rFLAGS.

#### FTOG
`0000 1010 [iA] 1101`  
IR type.  
Flag Toggle. Toggles the iA'th bit of rFLAGS.

### Interrupts

#### INTER
`0000 1011 1100 1111`  
RR type.  
Interrupt. Performs a software interrupt. This behaves exactly like a hardware interrupt, the corresponding Interrupt Signal Flag is ISFs.

#### WFI
`0000 1100 0000 1111`  
RR type.  
Wait for Interrupt.
Normal operation is suspended untill an interrupt is recieved.
If fICF is set, it will handle the interrupt in the normal manner.
if it is not set, operation will simply resume when the interrupt is recieved.
