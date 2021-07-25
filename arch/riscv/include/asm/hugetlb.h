/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _ASM_RISCV_HUGETLB_H
#define _ASM_RISCV_HUGETLB_H

#include <asm/page.h>

extern pte_t arch_make_huge_pte(pte_t entry, struct vm_area_struct *vma,
				struct page *page, int writable);
#define arch_make_huge_pte arch_make_huge_pte
#define __HAVE_ARCH_HUGE_SET_HUGE_PTE_AT
extern void set_huge_pte_at(struct mm_struct *mm, unsigned long addr,
			    pte_t *ptep, pte_t pte);
#define __HAVE_ARCH_HUGE_PTEP_GET_AND_CLEAR
extern pte_t huge_ptep_get_and_clear(struct mm_struct *mm,
				     unsigned long addr, pte_t *ptep);
#define __HAVE_ARCH_HUGE_PTEP_CLEAR_FLUSH
extern void huge_ptep_clear_flush(struct vm_area_struct *vma,
				  unsigned long addr, pte_t *ptep);
#define __HAVE_ARCH_HUGE_PTEP_SET_ACCESS_FLAGS
extern int huge_ptep_set_access_flags(struct vm_area_struct *vma,
				      unsigned long addr, pte_t *ptep,
				      pte_t pte, int dirty);
#define __HAVE_ARCH_HUGE_PTEP_SET_WRPROTECT
extern void huge_ptep_set_wrprotect(struct mm_struct *mm,
				    unsigned long addr, pte_t *ptep);
#define __HAVE_ARCH_HUGE_PTE_CLEAR
extern void huge_pte_clear(struct mm_struct *mm, unsigned long addr,
			   pte_t *ptep, unsigned long sz);

void __init riscv_hugetlb_cma_reserve(void);

#include <asm-generic/hugetlb.h>

#endif /* _ASM_RISCV_HUGETLB_H */
