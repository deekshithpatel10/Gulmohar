.data
.byte 10, 6, 4, 5, 2

.text
lui x3, 0x10
lui x2, 0x50
addi x8, x0, 5
addi x17, x0, 0
for_loop: bge x17, x8, exit
add x5, x3, x17
lb x10, 0(x5)
jal x1, fibo
add x5, x3, x17
sb x10, 0(x5)
addi x17, x17, 1
beq x0, x0, for_loop

fibo: addi x21, x0, 1
addi x22, x0, 2
beq x10, x21, return_base_case
beq x10, x22, return_base_case
addi sp, sp, -24
sd x1, 0(sp)
sd x10, 8(sp)
sd x23, 16(sp)
addi x10, x10, -1
jal x1, fibo
add x23, x10, x0
ld x10, 8(sp)
addi x10, x10, -2
jal x1, fibo
add x10, x10, x23
ld x23, 16(sp)
ld x1, 0(sp)
addi sp, sp, 24
jalr x0, 0(x1)
return_base_case: addi x10, x0, 1
jalr x0, 0(x1)

exit: add x0, x0, x0