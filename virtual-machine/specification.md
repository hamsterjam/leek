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

There are 16 registers. All are 16 bits in length. There are 6 special purpose registers. All may be written to (potentially breaking things) with the exception of OBLIVION. As a write to register 0 is interpreted as discarding the output.

```
0  = 0x0 = OBLIVION
11 = 0xb = ARITH1
12 = 0xc = ARITH2
13 = 0xd = FLAGS
14 = 0xe = STACK
15 = 0xf = PC
```

The bits in the FLAGS register correspond to flags set by operations. Any flag without a designation should be treated as reserved.

<table style="width:100%; table-layout: fixed">
    <tr align="center">
        <th> 0 <th> 1 <th> 2 <th> 3 <th> 4 <th> 5 <th> 6 <th> 7
    <tr align="center">
        <td> carry
        <td> overflow
        <td> zero
        <td> negative
        <td>
        <td>
        <td>
        <td>
    <tr align="center">
        <th> 8 <th> 9 <th> 10 <th> 11 <th> 12 <th> 13 <th> 14 <th> 15
    <tr align="center">
        <td> </td>
        <td>
        <td>
        <td>
        <td>
        <td>
        <td>
        <td>
</table>

Opperations Reference
---------------------

|           | Mode | Op Code |
|-----------|------|---------|
| **NOP**       | RR   | 0x00    |
| **MOV**       | RR   | 0x01    |
| **HSET**      | IIR  | 0x1     |
| **LSET**      | IIR  | 0x2     |
||||
| **ADD**       | RRR  | 0x3     |
| **ADDi**      | RIR  | 0x4     |
| **SUB**       | RRR  | 0x5     |
| **SUBi**      | RIR  | 0x6     |
| **MUL**       | RRR  | 0x7     |
| **ROT**       | RRR  | 0x8     |
| **ROTi**      | RIR  | 0x9     |
||||
| **OR**        | RRR  | 0xa     |
| **AND**       | RRR  | 0xb     |
| **XOR**       | RRR  | 0xc     |
| **NOT**       | RR   | 0x02    |
||||
| **STORE**     | RR   | 0x03    |
| **LOAD**      | RR   | 0x04    |
| **PUSH**      | RR   | 0x05    |
| **POP**       | RR   | 0x06    |
||||
| **JMP+**      | IIR  | 0xd     |
| **JMP-**      | IIR  | 0xe     |
| **FJMP**      | IR   | 0x07    |
| **FSET**      | IR   | 0x08    |
| **FCLR**      | IR   | 0x09    |
| **FTOG**      | IR   | 0x0a    |

Operations
----------

### Move and Set

#### NOP
`0000 0000 0000 0000`

RR type

Does nothing for 1 CPU cycle

#### MOV
`0000 0001 [rA] [rD]`

RR type

Copies the value of rA into rD

#### HSET
`0001 [  iVal ] [rD]`

IIR type

Stores the immediate iVal in the upper 8 bits of register rD

#### LSET
`0010 [  iVal ] [rD]`

IIR type

Stores the immediate iVal in the lower 8 bits of register rD

### Arithmetic

#### ADD
`0011 [rA] [rB] [rD]`

RRR type

Stores the result of rA+rB in register rD. Sets the carry, overflow, negative, and zero flags.

#### ADDi
`0100 [rA] [iB] [rD]`

RIR type

Stores the result of rA+iB in register rD. Sets the carry, overflow, negative, and zero flags

#### SUB
`0101 [rA] [rB] [rD]`

RRR type

Stores the result of rA-rB in register rD. Sets the carry (for a borrow), overflow, negative, and zero flags.

#### SUBi
`0110 [rA] [iB] [rD]`

RIR type

Stores the result of rA-iB in register rD. Sets the carry (for a borrow), overflow, negative, and zero flags.

#### MUL
`0111 [rA] [rB] [rD]`

RRR type

Stores the result of rA\*rB in register rD. Sets the negative, and zero flags. The upper word of the result is stored in ARITH1.

#### ROT
`1000 [rA] [rB] [rD]`

RRR type

Shifts rA to the left rB places (with wrapping) and stores the result in rD.

#### ROTi
`1001 [rA] [iB] [rD]`

RIR type

Shifts rA to the left iB places (with wrapping) and stores the result in rD.

### Logic

#### OR
`1010 [rA] [rB] [rD]`

RRR type

Performs a bitwise OR on rA and rB and stores the result in rD. Sets the zero flag.

#### AND
`1011 [rA] [rB] [rD]`

RRR type

Performs a bitwise AND on rA and rB and stores the result in rD. Sets the zero flag.

#### XOR
`1100 [rA] [rB] [rD]`

RRR type

Performs a bitwse XOR on rA and rB and stores the result in rD. Sets the zero flag.

#### NOT
`0000 0010 [rA] [rD]`

RR type

Performs a bitwse NOT on rA and stores the result in rD. Sets the zero flag.

### Memory

#### STORE
`0000 0011 [rA] [ad]`

RR type

Stores rA in the address ad in memory.

#### LOAD
`0000 0100 [ad] [rD]`

RR type

Stores the value at address ad in rD.

#### PUSH
`0000 0101 [rA] 1110`

RR type

Stores rA at the address in STACK, then increments STACK.

#### POP
`0000 0110 1110 [rD]`

RR type

Stores the value at address STACK in rD, then decrements STACK.

### Jump and Flags

#### JMP
`1101 [  off  ] 1111`

IIR type

Unconditional forward jump. Sets PC to PC + off.

#### JMP
`1110 [  off  ] 1111`

IIR type

Unconditional backward jump. Sets PC to PC - off.

#### FJMP
`0000 0111 [iA] 1111`

IR type

If the iA'th bit of FLAGS is not set, sets the PC to PC + 1

#### FSET
`0000 1000 [iA] 1101`

IR type

Sets the iA'th bit of FLAGS

#### FCLR
`0000 1001 [iA] 1101`

IR type

Clears the iA'th bit of FLAGS

#### FTOG
`0000 1010 [iA] 1101`

IR type

Toggles the iA'th bit of FLAGS
