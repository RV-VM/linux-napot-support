/* SPDX-License-Identifier: GPL-2.0-only */
#ifndef _ASM_RISCV_VMALLOC_H
#define _ASM_RISCV_VMALLOC_H

#include <asm/pgtable-bits.h>

#define arch_vmap_pte_range_map_size arch_vmap_pte_range_map_size
static inline unsigned long arch_vmap_pte_range_map_size(unsigned long addr, unsigned long end,
		u64 pfn, unsigned int max_page_shift)
{
	bool is_napot_addr = !(addr & NAPOT_CONT64KB_MASK);
	bool pfn_align_napot = !(pfn & (NAPOT_64KB_PTE_NUM - 1UL));
	bool space_enough = ((end - addr) >= NAPOT_CONT64KB_SIZE);

	if (is_napot_addr && pfn_align_napot && space_enough
			&& max_page_shift >= NAPOT_CONT64KB_SHIFT)
		return NAPOT_CONT64KB_SIZE;

	return PAGE_SIZE;
}

#endif /* _ASM_RISCV_VMALLOC_H */
