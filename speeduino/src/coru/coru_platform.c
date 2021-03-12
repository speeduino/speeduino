/*
 * coru, platform specific functions
 *
 * Copyright (c) 2019 Christopher Haster
 * Distributed under the MIT license
 */

#if defined(__MK64FX512__) || defined(__MK66FX1M0__) || defined(STM32F407xx)
#include "coru.h"
#include "coru_platform.h"


// Terminate a coroutine, defined in coru.c
//
// Must be called when coroutine's main function returns.
extern void coru_halt(void);


// Platform specific operations

// x86 32-bits
#if defined(__i386__)

// Setup stack
int coru_plat_init(void **psp, uintptr_t **pcanary,
        void (*cb)(void*), void *data,
        void *buffer, size_t size) {
    // check that stack is aligned
    CORU_ASSERT((uint32_t)buffer % 4 == 0 && size % 4 == 0);
    uint32_t *sp = (uint32_t*)((char*)buffer + size);

    // setup stack
    sp[-7] = 0;                     // edi
    sp[-6] = 0;                     // esi
    sp[-5] = 0;                     // ebx
    sp[-4] = 0;                     // ebp
    sp[-3] = (uint32_t)cb;          // ret to cb(data)
    sp[-2] = (uint32_t)coru_halt;   // ret to coru_halt()
    sp[-1] = (uint32_t)data;        // arg to cb(data)

    // setup stack pointer and canary
    *psp = &sp[-7];
    *pcanary = &sp[-size/sizeof(uint32_t)];
    return 0;
}

// Swap stacks
uintptr_t coru_plat_yield(void **sp, uintptr_t arg);
__asm__ (
    ".globl coru_plat_yield \n"
    "coru_plat_yield: \n"
    "\t mov 8(%esp), %eax \n"   // save arg to eax, return this later
    "\t mov 4(%esp), %edx \n"   // load new esp to edx
    "\t push %ebp \n"           // push callee saved registers
    "\t push %ebx \n"
    "\t push %esi \n"
    "\t push %edi \n"
    "\t xchg %esp, (%edx) \n"   // swap stack
    "\t pop %edi \n"            // pop callee saved registers
    "\t pop %esi \n"
    "\t pop %ebx \n"
    "\t pop %ebp \n"
    "\t ret \n"                 // return eax
);

// x86 64-bits
#elif defined(__amd64__)

// Here we need a prologue to get data to the callback when
// we startup a coroutine.
void coru_plat_prologue(void);
__asm__ (
    ".globl coru_plat_prologue \n"
    "coru_plat_prologue: \n"
    "\t mov %r13, %rdi \n"  // tail call cb(data)
    "\t jmp *%r12 \n"
);

// Setup stack
int coru_plat_init(void **psp, uintptr_t **pcanary,
        void (*cb)(void*), void *data,
        void *buffer, size_t size) {
    // check that stack is aligned
    CORU_ASSERT((uint64_t)buffer % 4 == 0 && size % 4 == 0);
    uint64_t *sp = (uint64_t*)((char*)buffer + size);

    // setup stack
    sp[-8] = 0;                             // rbx
    sp[-7] = 0;                             // rbp
    sp[-6] = (uint64_t)cb;                  // r12
    sp[-5] = (uint64_t)data;                // r13
    sp[-4] = 0;                             // r14
    sp[-3] = 0;                             // r15
    sp[-2] = (uint64_t)coru_plat_prologue;  // ret to coru_plat_prologue()
    sp[-1] = (uint64_t)coru_halt;           // ret to coru_halt()

    // setup stack pointer and canary
    *psp = &sp[-8];
    *pcanary = &sp[-size/sizeof(uint64_t)];
    return 0;
}

// Swap stacks
uintptr_t coru_plat_yield(void **sp, uintptr_t arg);
__asm__ (
    ".globl coru_plat_yield \n"
    "coru_plat_yield: \n"
    "\t push %r15 \n"           // push callee saved registers
    "\t push %r14 \n"
    "\t push %r13 \n"
    "\t push %r12 \n"
    "\t push %rbp \n"
    "\t push %rbx \n"
    "\t xchg %rsp, (%rdi) \n"   // swap stack
    "\t pop %rbx \n"            // pop callee saved registers
    "\t pop %rbp \n"
    "\t pop %r12 \n"
    "\t pop %r13 \n"
    "\t pop %r14 \n"
    "\t pop %r15 \n"
    "\t mov %rsi, %rax \n"      // return arg
    "\t ret \n"
);

// ARM thumb mode
#elif defined(__thumb__)

// Here we need a prologue to get both data and coru_halt
// into the appropriate registers.
void coru_plat_prologue(void);
__asm__ (
    ".thumb_func \n"
    ".global coru_plat_prologue \n"
    "coru_plat_prologue: \n"
    "\t mov lr, r6 \n"      // setup lr to ret to coru_halt()
    "\t mov r0, r5 \n"      // tail call cb(data)
    "\t bx r4 \n"
);

// Setup stack
int coru_plat_init(void **psp, uintptr_t **pcanary,
        void (*cb)(void*), void *data,
        void *buffer, size_t size) {
    // check that stack is aligned
    CORU_ASSERT((uint32_t)buffer % 4 == 0 && size % 4 == 0);
    uint32_t *sp = (uint32_t*)((char*)buffer + size);

    // setup stack
    sp[-9] = 0;                             // r8
    sp[-8] = 0;                             // r9
    sp[-7] = 0;                             // r10
    sp[-6] = 0;                             // r11
    sp[-5] = (uint32_t)cb;                  // r4
    sp[-4] = (uint32_t)data;                // r5
    sp[-3] = (uint32_t)coru_halt;           // r6
    sp[-2] = 0;                             // r7
    sp[-1] = (uint32_t)coru_plat_prologue;  // ret to coru_plat_prologue

    // setup stack pointer and canary
    *psp = &sp[-9];
    *pcanary = &sp[-size/sizeof(uint32_t)];
    return 0;
}

// Swap stacks
uintptr_t coru_plat_yield(void **sp, uintptr_t arg);
__asm__ (
    ".thumb_func \n"
    ".global coru_plat_yield \n"
    "coru_plat_yield: \n"
    "\t push {r4,r5,r6,r7,lr} \n"   // push callee saved registers
    "\t mov r4, r8 \n"              // yes we need these moves, thumb1 can
    "\t mov r5, r9 \n"              // only push r0-r7 at the same time
    "\t mov r6, r10 \n"
    "\t mov r7, r11 \n"
    "\t push {r4,r5,r6,r7} \n"
    "\t mov r2, sp \n"              // swap stack, takes several instructions
    "\t ldr r3, [r0] \n"            // here because thumb1 can't load/store sp
    "\t str r2, [r0] \n"
    "\t mov sp, r3 \n"
    "\t mov r0, r1 \n"              // return arg
    "\t pop {r4,r5,r6,r7} \n"       // pop callee saved registers and return
    "\t mov r8, r4 \n"
    "\t mov r9, r5 \n"
    "\t mov r10, r6 \n"
    "\t mov r11, r7 \n"
    "\t pop {r4,r5,r6,r7,pc} \n"
);

// MIPS
#elif defined(__mips__)

// Here we need a prologue to get both data and coru_halt
// into the appropriate registers.
void coru_plat_prologue(void);
__asm__ (
    ".globl coru_plat_prologue \n"
    "coru_plat_prologue: \n"
    "\t move $ra, $s2 \n"       // setup $ra to return to core_halt()
    "\t addiu $sp, $sp, -4 \n"  // tail call cb(data)
    "\t move $a0, $s1 \n"
    "\t j $s0 \n"
);

int coru_plat_init(void **psp, uintptr_t **pcanary,
        void (*cb)(void*), void *data,
        void *buffer, size_t size) {
    // check that stack is aligned
    CORU_ASSERT((uint32_t)buffer % 4 == 0 && size % 4 == 0);
    uint32_t *sp = (uint32_t*)((char*)buffer + size);

    // setup stack
    sp[-10] = (uint32_t)cb;                 // $s0
    sp[-9 ] = (uint32_t)data;               // $s1
    sp[-8 ] = (uint32_t)coru_halt;          // $s2
    sp[-7 ] = 0;                            // $s3
    sp[-6 ] = 0;                            // $s4
    sp[-5 ] = 0;                            // $s5
    sp[-4 ] = 0;                            // $s6
    sp[-3 ] = 0;                            // $s7
    sp[-2 ] = 0;                            // $fp
    sp[-1 ] = (uint32_t)coru_plat_prologue; // $ra

    // setup stack pointer and canary
    *psp = &sp[-10];
    *pcanary = &sp[-size/sizeof(uint32_t)];
    return 0;
}

// Swap stacks
uintptr_t coru_plat_yield(void **sp, uintptr_t arg);
__asm__ (
    ".globl coru_plat_yield \n"
    "coru_plat_yield: \n"
    "\t addiu $sp, $sp, -40 \n" // push callee saved registers
    "\t sw $s0,  0($sp) \n"
    "\t sw $s1,  4($sp) \n"
    "\t sw $s2,  8($sp) \n"
    "\t sw $s3, 12($sp) \n"
    "\t sw $s4, 16($sp) \n"
    "\t sw $s5, 20($sp) \n"
    "\t sw $s6, 24($sp) \n"
    "\t sw $s7, 28($sp) \n"
    "\t sw $fp, 32($sp) \n"
    "\t sw $ra, 36($sp) \n"
    "\t lw $t0, ($a0) \n"       // swap stack
    "\t sw $sp, ($a0) \n"
    "\t move $sp, $t0 \n"
    "\t lw $s0,  0($sp) \n"     // pop callee saved registers
    "\t lw $s1,  4($sp) \n"
    "\t lw $s2,  8($sp) \n"
    "\t lw $s3, 12($sp) \n"
    "\t lw $s4, 16($sp) \n"
    "\t lw $s5, 20($sp) \n"
    "\t lw $s6, 24($sp) \n"
    "\t lw $s7, 28($sp) \n"
    "\t lw $fp, 32($sp) \n"
    "\t lw $ra, 36($sp) \n"
    "\t addiu $sp, $sp, 40 \n"
    "\t move $v0, $a1 \n"       // return arg
    "\t j $ra \n"
);

#else
#error "Unknown platform! Please update coru_platform.c"
#endif
#endif
