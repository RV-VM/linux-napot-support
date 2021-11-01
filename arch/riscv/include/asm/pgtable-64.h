/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (C) 2012 Regents of the University of California
 */

#ifndef _ASM_RISCV_PGTABLE_64_H
#define _ASM_RISCV_PGTABLE_64_H

#include <linux/const.h>

#if CONFIG_PGTABLE_LEVELS > 2
typedef struct {
	unsigned long p4d;
} p4d_t;

static inline void set_pgd(pgd_t *pgdp, pgd_t pgd)
{
	*pgdp = pgd;
}

static inline int pgd_none(pgd_t pgd)
{
	return (pgd_val(pgd) == 0);
}

static inline int pgd_present(pgd_t pgd)
{
	return (pgd_val(pgd) & _PAGE_PRESENT);
}

static inline int pgd_bad(pgd_t pgd)
{
	return !pgd_present(pgd);
}

static inline void pgd_clear(pgd_t *pgdp)
{
	set_pgd(pgdp, __pgd(0));
}

static inline struct page *pgd_page(pgd_t pgd)
{
	return pfn_to_page(pgd_val(pgd) >> _PAGE_PFN_SHIFT);
}

static inline p4d_t *pgd_pgtable(pgd_t pgd)
{
	return (p4d_t *)pfn_to_page(pgd_val(pgd) >> _PAGE_PFN_SHIFT);
}

#define p4d_ERROR(p4d)				\
	pr_err("%s:%d: bad p4d " PTE_FMT ".\n", __FILE__, __LINE__, p4d_val(p4d))

#define P4D_SHIFT		39
#define PTRS_PER_P4D		(PAGE_SIZE / sizeof(p4d_t))
#define P4D_SIZE		(1UL << P4D_SHIFT)
#define P4D_MASK		(~(P4D_SIZE-1))

#define p4d_val(x)				((x).p4d)
#define __p4d(x)				((p4d_t) { (x) })

static inline unsigned long p4d_index(unsigned long address)
{
	return (address >> P4D_SHIFT) & (PTRS_PER_P4D - 1);
}
#define p4d_index p4d_index

static inline p4d_t *p4d_offset(pgd_t *pgd, unsigned long address)
{
	return pgd_pgtable(*pgd) + p4d_index(address);
}

static inline p4d_t pfn_p4d(unsigned long pfn, pgprot_t prot)
{
	return __p4d((pfn << _PAGE_PFN_SHIFT) | pgprot_val(prot));
}

static inline unsigned long _p4d_pfn(p4d_t p4d)
{
	return p4d_val(p4d) >> _PAGE_PFN_SHIFT;
}

static inline void set_p4d(p4d_t *p4dp, p4d_t p4d)
{
	*p4dp = p4d;
}

static inline int p4d_none(p4d_t p4d)
{
	return (p4d_val(p4d) == 0);
}

static inline int p4d_present(p4d_t p4d)
{
	return (p4d_val(p4d) & _PAGE_PRESENT);
}

static inline int p4d_bad(p4d_t p4d)
{
	return !p4d_present(p4d);
}

static inline void p4d_clear(p4d_t *p4dp)
{
	set_p4d(p4dp, __p4d(0));
}

#define pud_ERROR(pud)				\
	pr_err("%s:%d: bad pud " PTE_FMT ".\n", __FILE__, __LINE__, pud_val(pud))
typedef struct {
	unsigned long pud;
} pud_t;

#define PUD_SHIFT	30
#define PTRS_PER_PUD	(PAGE_SIZE / sizeof(pud_t))
#define PUD_SIZE  	(1UL << PUD_SHIFT)
#define PUD_MASK  	(~(PUD_SIZE-1))

static inline struct page *p4d_page(p4d_t p4d)
{
	return pfn_to_page(p4d_val(p4d) >> _PAGE_PFN_SHIFT);
}

static inline pud_t *p4d_pgtable(p4d_t p4d)
{
	return (pud_t *)pfn_to_virt(p4d_val(p4d) >> _PAGE_PFN_SHIFT);
}

#define pud_val(x)				((x).pud)
#define __pud(x)				((pud_t) { x })

static inline pud_t pfn_pud(unsigned long pfn, pgprot_t prot)
{
	return __pud((pfn << _PAGE_PFN_SHIFT) | pgprot_val(prot));
}

static inline unsigned long _pud_pfn(pud_t pud)
{
	return pud_val(pud) >> _PAGE_PFN_SHIFT;
}

#define PGDIR_SHIFT     48
#else /* CONFIG_PGTABLE_LEVELS > 2 */
#include <asm-generic/pgtable-nopud.h>
#define PGDIR_SHIFT     30
#endif /* CONFIG_PGTABLE_LEVELS > 2 */

/* Size of region mapped by a page global directory */
#define PGDIR_SIZE      (_AC(1, UL) << PGDIR_SHIFT)
#define PGDIR_MASK      (~(PGDIR_SIZE - 1))

#define PMD_SHIFT       21
/* Size of region mapped by a page middle directory */
#define PMD_SIZE        (_AC(1, UL) << PMD_SHIFT)
#define PMD_MASK        (~(PMD_SIZE - 1))

/* Page Middle Directory entry */
typedef struct {
	unsigned long pmd;
} pmd_t;

#define pmd_val(x)      ((x).pmd)
#define __pmd(x)        ((pmd_t) { (x) })

#define PTRS_PER_PMD    (PAGE_SIZE / sizeof(pmd_t))

static inline int pud_present(pud_t pud)
{
	return (pud_val(pud) & _PAGE_PRESENT);
}

static inline int pud_none(pud_t pud)
{
	return (pud_val(pud) == 0);
}

static inline int pud_bad(pud_t pud)
{
	return !pud_present(pud);
}

#define pud_leaf	pud_leaf
static inline int pud_leaf(pud_t pud)
{
	return pud_present(pud) && (pud_val(pud) & _PAGE_LEAF);
}

static inline void set_pud(pud_t *pudp, pud_t pud)
{
	*pudp = pud;
}

static inline void pud_clear(pud_t *pudp)
{
	set_pud(pudp, __pud(0));
}

static inline pmd_t *pud_pgtable(pud_t pud)
{
	return (pmd_t *)pfn_to_virt(pud_val(pud) >> _PAGE_PFN_SHIFT);
}

static inline struct page *pud_page(pud_t pud)
{
	return pfn_to_page(pud_val(pud) >> _PAGE_PFN_SHIFT);
}

static inline pmd_t pfn_pmd(unsigned long pfn, pgprot_t prot)
{
	return __pmd((pfn << _PAGE_PFN_SHIFT) | pgprot_val(prot));
}

static inline unsigned long _pmd_pfn(pmd_t pmd)
{
	return pmd_val(pmd) >> _PAGE_PFN_SHIFT;
}

#define mk_pmd(page, prot)    pfn_pmd(page_to_pfn(page), prot)

#define pmd_ERROR(e) \
	pr_err("%s:%d: bad pmd %016lx.\n", __FILE__, __LINE__, pmd_val(e))

#endif /* _ASM_RISCV_PGTABLE_64_H */
