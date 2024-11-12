.data
.dword 0x2341

.text
add x2, x0, x0
add x3, x0, x0
addi x12, x0, -34
addi x13, x0, 12
addi x14, x0, 2041
addi x15, x0, -2021
addi x16, x0, 1234
addi x17, x0, -1
addi x18, x0, 73
addi x19, x0, -21
addi x10, x1, 1234
sub x11, x2, x3
and x14, x8, x9
or x15, x10, x11
xor x16, x12, x13
sll x17, x14, x15
srl x18, x1, x2
sra x19, x3, x4

addi x20, x5, 1678
andi x21, x6, -456
ori x22, x7, 12
xori x23, x8, 67

slli x24, x15, 5
srli x25, x16, 3
srai x26, x20, 3

beq x20, x22, label1
blt x24, x25, label3
bge x26, x27, label7
bltu x28, x29, label1
bgeu x30, x31, label3

jal x0, label7

label1:
add x10, x1, x4
sub x11, x2, x25
andi x14, x5, -567
ori x15, x6, 901
xor x16, x7, x12
addi x0, x16, 123
addi x23, x0, 7
sll x17, x8, x23
ori x5, x0, 12
srl x18, x9, x5
addi x20, x11, -567
sub x21, x12, x7


label3:
add x3, x1, x21
addi x4, x2, -345
andi x7, x5, -5
ori x8, x6, 34
xori x9, x7, -456
slli x10, x8, 17
srli x11, x9, 12
srai x12, x10, 24
addi x13, x11, 600
sub x14, x12, x12



label7:
add x20, x1, x19
sub x21, x2, x17
andi x24, x5, -890
ori x25, x6, -1
xor x26, x7, x3
sll x27, x8, x0