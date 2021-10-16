/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (C) 2012 Regents of the University of California
 */

#ifndef _ASM_RISCV_PGTABLE_BITS_H
#define _ASM_RISCV_PGTABLE_BITS_H

/*
 * PTE format:
 * | XLEN-1  10 | 9             8 | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0
 *       PFN      reserved for SW   D   A   G   U   X   W   R   V
 */

#define _PAGE_ACCESSED_OFFSET 6

#define _PAGE_PRESENT   (1 << 0)
#define _PAGE_READ      (1 << 1)    /* Readable */
#define _PAGE_WRITE     (1 << 2)    /* Writable */
#define _PAGE_EXEC      (1 << 3)    /* Executable */
#define _PAGE_USER      (1 << 4)    /* User */
#define _PAGE_GLOBAL    (1 << 5)    /* Global */
#define _PAGE_ACCESSED  (1 << 6)    /* Set by hardware on any access */
#define _PAGE_DIRTY     (1 << 7)    /* Set by hardware on any write */
#define _PAGE_SOFT      (1 << 8)    /* Reserved for software */

#define _PAGE_RESERVE_0_SHIFT 54
#define _PAGE_RESERVE_1_SHIFT 55
#define _PAGE_RESERVE_2_SHIFT 56
#define _PAGE_RESERVE_3_SHIFT 57
#define _PAGE_RESERVE_4_SHIFT 58
#define _PAGE_RESERVE_5_SHIFT 59
#define _PAGE_RESERVE_6_SHIFT 60
#define _PAGE_RESERVE_7_SHIFT 61
#define _PAGE_RESERVE_8_SHIFT 62
#define _PAGE_NAPOT_SHIFT 63
#define _PAGE_RESERVE_0 (1UL << 54)
#define _PAGE_RESERVE_1 (1UL << 55)
#define _PAGE_RESERVE_2 (1UL << 56)
#define _PAGE_RESERVE_3 (1UL << 57)
#define _PAGE_RESERVE_4 (1UL << 58)
#define _PAGE_RESERVE_5 (1UL << 59)
#define _PAGE_RESERVE_6 (1UL << 60)
#define _PAGE_RESERVE_7 (1UL << 61)
#define _PAGE_RESERVE_8 (1UL << 62)

#define _PAGE_SPECIAL   _PAGE_SOFT
#define _PAGE_TABLE     _PAGE_PRESENT

/*
 * _PAGE_PROT_NONE is set on not-present pages (and ignored by the hardware) to
 * distinguish them from swapped out pages
 */
#define _PAGE_PROT_NONE _PAGE_READ

#define _PAGE_PFN_SHIFT 10
#define _PAGE_PFN_MASK (_PAGE_RESERVE_0 - (1UL << _PAGE_PFN_SHIFT))
/* now Svnapot only supports 64KB*/
#define NAPOT_CONT64KB_ORDER 4UL
#define NAPOT_CONT64KB_SHIFT (NAPOT_CONT64KB_ORDER + PAGE_SHIFT)
#define NAPOT_CONT64KB_SIZE (1UL << NAPOT_CONT64KB_SHIFT)
#define NAPOT_CONT64KB_MASK (NAPOT_CONT64KB_SIZE - 1)
#define NAPOT_64KB_PTE_NUM (1UL << 4UL)
#define _PAGE_NAPOT      (1UL << _PAGE_NAPOT_SHIFT)
#define NAPOT_64KB_MASK (7UL << _PAGE_PFN_SHIFT)

/* Set of bits to preserve across pte_modify() */
#define _PAGE_CHG_MASK  (~(unsigned long)(_PAGE_PRESENT | _PAGE_READ |	\
					  _PAGE_WRITE | _PAGE_EXEC |	\
					  _PAGE_USER | _PAGE_GLOBAL))
/*
 * when all of R/W/X are zero, the PTE is a pointer to the next level
 * of the page table; otherwise, it is a leaf PTE.
 */
#define _PAGE_LEAF (_PAGE_READ | _PAGE_WRITE | _PAGE_EXEC)

#endif /* _ASM_RISCV_PGTABLE_BITS_H */
