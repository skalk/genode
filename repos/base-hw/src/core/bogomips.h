void bogomips(void);

void bogomips()
{
	asm volatile("str     x19, [sp, #-16]!\n"
				 "mov     w19, #0x0\n"
				 "1: mov     w0, #0xc9ff\n"
				 "movk    w0, #0xff9a, lsl #16\n"
				 "cmp     w19, w0\n"
				 "b.hi    2f\n"
				 "add     w19, w19, #0x1\n"
				 "b       1b\n"
				 "2: nop\n"
				 "ldr     x19, [sp], #16\n" ::: "x0", "x19");
};
