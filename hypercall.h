#ifndef MAGIC_VALUE
#error Needs MAGIC_VALUE
#endif

#define RETRY 0xDEADBEEF

// Architecture-specific macros for register definitions and hypercall instruction
#if defined(__x86_64__)
#define SYSCALL_NUM_REG rax
#define ARG1_REG "rdi"
#define ARG2_REG "rsi"
#define ARG3_REG "rdx"
#define HYPERCALL_INSTR "cpuid"

#elif defined(__i386__) && !defined(__x86_64__)
#define SYSCALL_NUM_REG "eax"
#define ARG1_REG "ebx"
#define ARG2_REG "ecx"
#define ARG3_REG "edx"
#define HYPERCALL_INSTR "cpuid"
#define CLOBBER_LIST SYSCALL_NUM_REG

#elif defined(__arm__)
#define SYSCALL_NUM_REG "r7"
#define ARG1_REG "r0"
#define ARG2_REG "r1"
#define ARG3_REG "r2"
#define ARG4_REG "r3"
#define HYPERCALL_INSTR "mcr p7, 0, r0, c0, c0, 0"

#elif defined(__aarch64__)
#define SYSCALL_NUM_REG "x8"
#define ARG1_REG "x0"
#define ARG2_REG "x1"
#define ARG3_REG "x2"
#define HYPERCALL_INSTR "msr S0_0_c5_c0_0, xzr"

#elif defined(__mips64) || defined(mips) || defined(__mips__) || defined(__mips)
#define SYSCALL_NUM_REG "$2"
#define ARG1_REG "$4"
#define ARG2_REG "$5"
#define ARG3_REG "$6"
#define HYPERCALL_INSTR "movz $0, $0, $0"

#else
#error Unsupported platform.
#endif

#define HYPERCALL(ret, magic, type, data, length) \
    asm volatile( \
        HYPERCALL_INSTR "\n" \
        "mov %0, " SYSCALL_NUM_REG "\n" \
        : "=r" (ret) \
        : "r" (magic), "r" (type), "r" (data), "r" (length) \
        : )

static inline int hc(int hc_type, void **s, int len) {
    int ret = MAGIC_VALUE;
    int y = 0;
    do {
        ret = MAGIC_VALUE;
        volatile int x = 0;
        for (int i = 0; i < len; i++) {
            x |= *(((int*)s[i]) + y);
        }
        y++;

        // Register assignments
        register int magic asm(SYSCALL_NUM_REG) = MAGIC_VALUE;
        register int type asm(ARG1_REG) = hc_type;
        register void **data asm(ARG2_REG) = s;
        register int length asm(ARG3_REG) = len;

        // Hypercall invocation
        HYPERCALL(ret, magic, type, data, length);
    } while (ret == RETRY);
    return ret;
}
