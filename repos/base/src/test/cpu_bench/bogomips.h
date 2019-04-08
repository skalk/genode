static inline void bogomips() __attribute__((optimize("O0")));

static inline void bogomips()
{
	unsigned long rounds  = 0xffffffff;
	unsigned long rounds2 = 0xffffffff;
	asm volatile("mov r4, %0     \n"
	             "mov r5, #0     \n"
	             "mov r6, %1     \n"
	             "mov r7, #0     \n"
	             "1: cmp r6, r7  \n"
	             "bhi 4f         \n"
	             "add r7, r7, #1 \n"
	             "2: cmp r4, r5  \n"
	             "bhi 3f         \n"
	             "add r5, r5, #1 \n"
	             "b 2b           \n"
	             "3: b 1b        \n"
	             "4: nop" :: "r" (rounds), "r" (rounds2) : "r4", "r5", "r6", "r7");
};

