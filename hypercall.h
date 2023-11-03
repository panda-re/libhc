#ifndef MAGIC_VALUE
#error Needs MAGIC_VALUE
#endif
#define RETRY 0xDEADBEEF
// #if defined(__x86_64__)
#if defined(__x86_64__)
static inline int hc(int hc_type, void **s,int len) {
    uint64_t eax = MAGIC_VALUE;
    uint64_t ret = hc_type;
    int y = 0;
    do {
        ret = MAGIC_VALUE;
        volatile int x = 0;
        for(int i = 0; i< len; i++) {
            x |= *(((int*)s[i])+y);
        }
        asm __volatile__(
        "movq %1, %%rax \t\n\
        movq %2, %%rdi \t\n\
        movq %3, %%rsi \t\n\
        movq %4, %%rdx \t\n\
        cpuid \t\n\
        mov %%rax, %0 \t\n"
        : "=m" (ret) /* output operand */
        : "g" (eax), "g" (hc_type), "g" (s), "g" (len) /* input operands */
        : "rdi", "rsi", "rdx", "eax" /* clobbered registers */
        );
    } while (ret == RETRY);

    return ret;
}
#elif defined(__i386__) && !defined(__x86_64__)
static inline int hc(int hc_type,void **s,int len) {
    int eax = MAGIC_VALUE;
    int ret = MAGIC_VALUE;
    int y = 0;
    do {
        ret = MAGIC_VALUE;
        volatile int x = 0;
        for(int i = 0; i< len; i++) {
            x |= *(((int*)s[i])+y);
        }
        y++;
        asm __volatile__(
        "mov %1, %%eax \t\n\
        mov %2, %%ebx \t\n\
        mov %3, %%ecx \t\n\
        mov %4, %%edx \t\n\
        cpuid \t\n\
        mov %%eax, %0 \t\n"
        : "=g" (ret) /* output operand */
        : "g" (eax), "g" (hc_type), "g" (s), "g" (len) /* input operands */
        : "eax", "ebx", "ecx", "edx" /* clobbered registers */
        );
    } while (ret == RETRY);

    return ret;
}
#elif defined(__arm__)
static inline __attribute__((always_inline)) int hc(int hc_type, void **s, int len) {
    int ret = MAGIC_VALUE;
    int y = 0;
    do {
        ret = MAGIC_VALUE;
        volatile int x = 0;
        for (int i = 0; i < len; i++) {
            x |= *(((int*) s[i]) + y);
        }
        y++;
        asm __volatile__(
        "mov %%r7, %1 \t\n\
        mov %%r0, %2 \t\n\
        mov %%r1, %3 \t\n\
        mov %%r2, %4 \t\n\
        mov %%r3, %5 \t\n\
        mcr p7, 0, r0, c0, c0, 0 \t\n\
        mov %0, %%r0 \t\n"
      : "=g"(ret) /* no output registers */
      : "r" (MAGIC_VALUE), "r" (hc_type), "r" (s), "r" (len), "r" (0) /* input registers */
      : "r0", "r1", "r2", "r3", "r4", "r7" /* clobbered registers */);
    } while (ret == RETRY);

    return ret;
}
#elif defined(__mips64)
static inline int hc(int hc_type, void **s, int len) {
    int ret = MAGIC_VALUE;
    volatile int x = 0;
    int y = 0;
    do {
        ret = MAGIC_VALUE;
        volatile int x = 0;
        for(int i = 0; i < len; i++) {
            x |= *(((int*)s[i]) + y);
        }
        y++;

        asm __volatile__(
        "move $2, %1\t\n"
        "move $4, %2\t\n"
        "move $5, %3\t\n"
        "move $6, %4\t\n"
        "movz $0, $0, $0\t\n"
        "move %0, $2\t\n"
        : "=g"(ret) /* output operand */
        : "r" (MAGIC_VALUE), "r" (hc_type), "r" (s), "r" (len)  /* input operands */
        : "a0", "a1", "a2", "a3" /* clobbered registers */
        );
    } while (ret == RETRY);

    return ret;
}
#elif defined(mips) || defined(__mips__) || defined(__mips)
static inline int hc(int hc_type,void  **s,int len) {
    int ret;
    int y = 0;
    do {
        ret = MAGIC_VALUE;
        volatile int x = 0;
        y++;
        for(int i = 0; i < len; i++) {
            x |= *(((int*) s[i]) + y);
        }
        
        asm __volatile__(
        "move $2, %1\t\n"
        "move $4, %2\t\n"
        "move $5, %3\t\n"
        "move $6, %4\t\n"
        "movz $0, $0, $0\t\n"
        "move %0, $2\t\n"
        : "=g"(ret) /* output operand */
        : "r" (MAGIC_VALUE), "r" (hc_type), "r" (s), "r" (len)  /* input operands */
        : "a0", "a1", "a2", "a3" /* clobbered registers */
        );
    } while (ret == RETRY);

    return ret;
}
#elif defined(__aarch64__)
static inline __attribute__((always_inline)) int hc(int hc_type, void **s,int len) {
    int ret = MAGIC_VALUE;
    int y = 0;
    do {
        ret = MAGIC_VALUE;
        volatile int x = 0;
        for(int i = 0; i < len; i++) {
            x |= *(((int*) s[i]) + y);
        }
        asm __volatile__("stp x0, x1, [sp, #-16]! \t\n\
            stp x2, x3, [sp, #-16]! \t\n\
            mov x8, %1 \t\n\
            mov x0, %2 \t\n\
            mov x1, %3 \t\n\
            mov x2, %4 \t\n\
            msr S0_0_c5_c0_0, xzr \t\n\
            mov %0, x0 \t\n\
            ldp x0, x1, [sp], #16 \t\n\
            ldp x2, x3, [sp], #16 \t\n"
        : "=g"(ret) /* no output registers */
        : "r" (MAGIC_VALUE), "r" (hc_type), "r" (s), "r" (len), "r" (0) /* input registers */
        : "x0", "x1", "x2", "x3", "x4" /* clobbered registers */
        );
    } while (ret == RETRY);  

    return ret;
}
#else
#error Unsupported platform.
#endif
