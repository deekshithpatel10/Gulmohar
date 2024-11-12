.data 
.dword 0x123445, 
17894, 
-2345, -12, 666, 
.word 0xaf23, 
-34, 0xf234, 875
.byte 0x23, -12, 66, 0xef

.half 0x1111, 0x23e3, -233, 99

.text 
lui x2, 0x10
lui x3, 0x23
lhu x4, 0(x2)
lw x17, 2(x2)
sd x17, 0(x3)
sw x4, 1(x3)
lwu x31, 30(x2)
lh x4, 12(x2)
lb x25, 2(x3)
lb x12, 23(x2)
lbu x11, 17(x2)
ld x27, 44(x2)
sh x27, 10(x3)
lhu x16, 31(x2)
ld x28, 7(x2)
addi x28, x28, 0xf
sb x28, 8(x3)