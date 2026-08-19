#ifndef COMMON_H
#define COMMON_H

#ifndef NULL
#define NULL 0
#endif

#ifdef SKIP_ASM
#undef USE_INCLUDE_ASM
#endif

#ifdef USE_INCLUDE_ASM
__asm__(".include \"macro.inc\"\n");
#define INCLUDE_ASM(FOLDER, NAME)                                              \
    void __maspsx_include_asm_hack_##NAME() {                                  \
        __asm__(".text # maspsx-keep \n"                                       \
                "\t.align\t2 # maspsx-keep\n"                                  \
                "\t.set noreorder # maspsx-keep\n"                             \
                "\t.set noat # maspsx-keep\n"                                  \
                ".include \"" FOLDER "/" #NAME ".s\" # maspsx-keep\n"          \
                "\t.set reorder # maspsx-keep\n"                               \
                "\t.set at # maspsx-keep\n");                                  \
    }
#else
#define INCLUDE_ASM(FOLDER, NAME)
#endif

/* Pin a function's codegen to its reference .s while keeping the C body in the
 * translation unit as living documentation. Drop-in replacement for the
 * INCLUDE_ASM line in a parked function:
 *
 *   #ifndef NON_MATCHINGS
 *   MASPSX_OVERRIDE("asm/us/field/nonmatchings/field", FieldDebugPageAddPos);
 *   #else
 *   void FieldDebugPageAddPos(s16 page, s16 x, s16 y) { ... }
 *   #endif
 *
 * Unlike INCLUDE_ASM the symbol is the REAL function name (no
 * __maspsx_include_asm_hack_ wrapper), so the function is a normal typed,
 * callable C symbol whose bytes happen to be pinned to the reference .s. Use
 * for functions whose C is semantically correct but whose gcc-2.6.3 schedule
 * cannot reach the target (the $at-rematerialisation / s16-counter /
 * conserved-pair walls). Expands to nothing under NON_MATCHINGS, where the
 * #else C body compiles instead. */
#ifdef USE_INCLUDE_ASM
#define MASPSX_OVERRIDE(FOLDER, NAME)                                          \
    void __maspsx_override_hack_##NAME() {                                     \
        __asm__(".text # maspsx-keep \n"                                       \
                "\t.align\t2 # maspsx-keep\n"                                  \
                "\t.set noreorder # maspsx-keep\n"                             \
                "\t.set noat # maspsx-keep\n"                                  \
                ".include \"" FOLDER "/" #NAME ".s\" # maspsx-keep\n"          \
                "\t.set reorder # maspsx-keep\n"                               \
                "\t.set at # maspsx-keep\n");                                  \
    }
#else
#define MASPSX_OVERRIDE(FOLDER, NAME)
#endif

/* Opt ONE function into maspsx's $at re-materialisation post-pass: every
 * access through a global-array base that gcc CSEd into a register is split
 * back into the standalone `lui $at / addiu $at / addu $at / op` sequence
 * ASPSX 2.21 emitted per access. File scope, emits zero bytes (a bare `asm`
 * comment that cc1 passes through to the assembler untouched):
 *
 *   MASPSX_AT_REMAT(FieldDebugPageAddPos)
 *   void FieldDebugPageAddPos(s16 page, s16 x, s16 y) { ... }
 *
 * Mutually exclusive with MASPSX_OVERRIDE on the same function: the override
 * pins the bytes wholesale, this keeps gcc's codegen and repairs the idiom.
 * Only meaningful for units assembled as aspsx < 2.30 (the addiu_at idiom).
 * Note: `asm`, not `__asm__` — cc1 2.6.3 rejects a top-level `__asm__` whose
 * string begins with `#`, but passes a top-level `asm(...)` through. */
#ifdef USE_INCLUDE_ASM
#define MASPSX_AT_REMAT(NAME) asm("# maspsx-atremat:" #NAME)
#else
#define MASPSX_AT_REMAT(NAME)
#endif

typedef signed char s8;
typedef unsigned char u8;
typedef signed short s16;
typedef unsigned short u16;
typedef signed int s32;
typedef unsigned int u32;
typedef u8 unk_data;
typedef unsigned int* unk_ptr;

#define LEN(x) ((s32)(sizeof(x) / sizeof(*(x))))

#endif
