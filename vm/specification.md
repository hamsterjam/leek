Opperations Reference
=====================

NOP		0x00
MOV		0x01
HSET	0x1
LSET	0x2

ADD		0x3
ADDi	0x4
SUB		0x5
SUBi	0x6
MUL		0x7
ROT		0x8
ROTi	0x9

OR		0xa
AND 	0xb
XOR		0xc
NOT 	0x02

STORE	0x03
LOAD	0x04
PUSH	0x05
POP		0x06

JMP		0xd
FJMP    0xe
FSET    0x07
FCLR    0x08
FTOG    0x09

Instruction format
==================

Instructions are one of the following 5 types. During the operation. The result of any operation will either be written to the register destination, or the address in memory stored in destination depending on the operation. If the destination register is 0, then the output is discarded

          |   4 bits    |   4 bits    |   4 bits    |   4 bits    |
IIR type:  [  op code  ] [       immediate         ] [destination]
RIR type:  [  op code  ] [     A     ] [ immediate ] [destination]
RRR type:  [  op code  ] [     A     ] [     B     ] [destination]
IR  type:  [    0xf    ] [  op code  ] [ immediate ] [destination]
RR  type:  [    0xf    ] [  op code  ] [     B     ] [destination]

Registers
=========

There are 16 registers. All are 16 bits in length. There are 6 special purpose registers. All may be written to (potentially breaking things) with the exception of OBLIVION. As a write to register 0 is interpreted as discarding the output.

0  = OBLIVION
11 = ARITH1
12 = ARITH2
13 = FLAGS
14 = STACK
15 = PC

The bits in the FLAG register correspond to flags set by operations. Any flag without a designation should be treated as reserved.

+---------+---------+---------+---------+---------+---------+---------+---------+
|    0    |    1    |    2    |    3    |    4    |    5    |    6    |    7    |
+---------+---------+---------+---------+---------+---------+---------+---------+
|  carry  |overflow |  zero   |negative |         |         |         |         |
+---------+---------+---------+---------+---------+---------+---------+---------+


+---------+---------+---------+---------+---------+---------+---------+---------+
|    8    |    9    |   10    |   11    |   12    |   13    |   14    |   15    |
+---------+---------+---------+---------+---------+---------+---------+---------+
|         |         |         |         |         |         |         |         |
+---------+---------+---------+---------+---------+---------+---------+---------+

Operations
==========

NOP: RR type
0000 0000 0000 0000
Does nothing for 1 CPU cycle

MOV: RR type
0000 0001 [rA] [rD]
Copies the value of rA into rD

HSET: IIR type
0001 [  iVal ] [rD]
Stores the immediate iVal in the upper 8 bits of register rD

LSET: IIR type
0010 [  iVal ] [rD]
Stores the immediate iVal in the lower 8 bits of register rD

ADD: RRR type
0011 [rA] [rB] [rD]
Stores the result of rA+rB in register rD. Sets the carry, overflow, negative, and zero flags.

ADDi: RIR type
0100 [rA] [iB] [rD]
Stores the result of rA+iB in register rD. Sets the carry, overflow, negative, and zero flags

SUB: RRR type
0101 [rA] [rB] [rD]
Stores the result of rA-rB in register rD. Sets the carry (for a borrow), overflow, negative, and zero flags.

SUBi: RIR type
0110 [rA] [iB] [rD]
Stores the result of rA-iB in register rD. Sets the carry (for a borrow), overflow, negative, and zero flags.

MUL: RRR type
0111 [rA] [rB] [rD]
Stores the result of rA*rB in register rD. Sets the negative, and zero flags. The upper word of the result is stored in ARITH1.

ROT: RRR type
1000 [rA] [rB] [rD]
Shifts rA to the left rB places (with wrapping) and stores the result in rD.

ROTi: RIR type
1001 [rA] [iB] [rD]
Shifts rA to the left iB places (with wrapping) and stores the result in rD.

OR: RRR type
1010 [rA] [rB] [rD]
Performs a bitwise OR on rA and rB and stores the result in rD. Sets the zero flag.

AND: RRR type
1011 [rA] [rB] [rD]
Performs a bitwise AND on rA and rB and stores the result in rD. Sets the zero flag.

XOR: RRR type
1100 [rA] [rB] [rD]
Performs a bitwse XOR on rA and rB and stores the result in rD. Sets the zero flag.

NOT: RR type
0000 0010 [rA] [rD]
Performs a bitwse NOT on rA and stores the result in rD. Sets the zero flag.

STORE: RR type
0000 0011 [rA] [ad]
Stores rA in the address ad in memory.

LOAD: RR type
0000 0100 [ad] [rD]
Stores the value at address ad in rD.

PUSH: RR type
0000 0101 [rA] 1110
Stores rA at the address in STACK, then increments STACK.

POP: RR type
0000 0110 0000 [rD]
Stores the value at address STACK in rD, then decrements STACK.

JMP: RIR type
1101 [rA] [iB] 1111
Unconditional jump. Sets PC to rA + iB

FJMP: RIR type
1110 [rA] [iB] 1111
If the iB'th bit of FLAGS is set, sets PC to the value of rA.

FSET: IR type
0000 0111 [iA] 1101
Sets the iA'th bit of FLAGS

FCLR: IR type
0000 1000 [iA] 1101
Clears the iA'th bit of FLAGS

FTOG: IR type
0000 1001 [iA] 1101
Toggles the iA'th bit of FLAGS
