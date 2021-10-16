// SPDX-License-Identifier: GPL-2.0
#include <linux/hugetlb.h>
#include <linux/err.h>

pte_t *huge_pte_alloc(struct mm_struct *mm, struct vm_area_struct *vma,
			unsigned long addr, unsigned long sz)
{
  pgd_t *pgdp = pgd_offset(mm, addr);
  p4d_t *p4dp = p4d_alloc(mm, pgdp, addr);
  pud_t *pudp = pud_alloc(mm, p4dp, addr);
  pmd_t *pmdp = pmd_alloc(mm, pudp, addr);

  if (sz == NAPOT_CONT64KB_SIZE) {
    if (!pmdp)
      return NULL;
    WARN_ON(addr & (sz - 1));
    return pte_alloc_map(mm, pmdp, addr);
  }

  return NULL;
}

pte_t *huge_pte_offset(struct mm_struct *mm,
		       unsigned long addr, unsigned long sz)
{
	pgd_t *pgdp;
	p4d_t *p4dp;
	pud_t *pudp;
	pmd_t *pmdp;
  pte_t *ptep = NULL;

	pgdp = pgd_offset(mm, addr);
	if (!pgd_present(READ_ONCE(*pgdp)))
		return NULL;

	p4dp = p4d_offset(pgdp, addr);
	if (!p4d_present(READ_ONCE(*p4dp)))
		return NULL;

	pudp = pud_offset(p4dp, addr);
  if (!pud_present(READ_ONCE(*pudp)))
    return NULL;

  pmdp = pmd_offset(pudp, addr);
  if (!pmd_present(READ_ONCE(*pmdp)))
    return NULL;

  if (sz == NAPOT_CONT64KB_SIZE) {
    ptep = pte_offset_kernel(pmdp, (addr & ~NAPOT_CONT64KB_MASK));
  }

  return ptep;
}

int napot_pte_num(pte_t pte)
{
	if (!(pte_val(pte) & NAPOT_64KB_MASK))
		return NAPOT_64KB_PTE_NUM;

	pr_warn("%s: unrecognized napot pte size 0x%lx\n",
		__func__, pte_val(pte));
	return 1;
}

static pte_t get_clear_flush(struct mm_struct *mm,
			     unsigned long addr,
			     pte_t *ptep,
			     unsigned long pte_num)
{
	pte_t orig_pte = huge_ptep_get(ptep);
	bool valid = pte_val(orig_pte);
	unsigned long i, saddr = addr;

	for (i = 0; i < pte_num; i++, addr += PAGE_SIZE, ptep++) {
		pte_t pte = ptep_get_and_clear(mm, addr, ptep);

		if (pte_dirty(pte))
			orig_pte = pte_mkdirty(orig_pte);

		if (pte_young(pte))
			orig_pte = pte_mkyoung(orig_pte);
	}

	if (valid) {
		struct vm_area_struct vma = TLB_FLUSH_VMA(mm, 0);

		flush_tlb_range(&vma, saddr, addr);
	}
	return orig_pte;
}

static void clear_flush(struct mm_struct *mm,
			     unsigned long addr,
			     pte_t *ptep,
			     unsigned long pte_num)
{
	struct vm_area_struct vma = TLB_FLUSH_VMA(mm, 0);
	unsigned long i, saddr = addr;

	for (i = 0; i < pte_num; i++, addr += PAGE_SIZE, ptep++)
		pte_clear(mm, addr, ptep);

	flush_tlb_range(&vma, saddr, addr);
}

pte_t arch_make_huge_pte(pte_t entry, unsigned int shift,
				       vm_flags_t flags)
{
	if (shift == NAPOT_CONT64KB_SHIFT)
		entry = pte_mknapot(entry, NAPOT_CONT64KB_SHIFT - PAGE_SHIFT);

	return entry;
}

void set_huge_pte_at(struct mm_struct *mm, unsigned long addr,
			    pte_t *ptep, pte_t pte)
{
	int i;
	int pte_num;

	if (!pte_napot(pte)) {
		set_pte_at(mm, addr, ptep, pte);
		return;
	}

	pte_num = napot_pte_num(pte);
	for (i = 0; i < pte_num; i++, ptep++, addr += PAGE_SIZE)
		set_pte_at(mm, addr, ptep, pte);
}

int huge_ptep_set_access_flags(struct vm_area_struct *vma,
				      unsigned long addr, pte_t *ptep,
				      pte_t pte, int dirty)
{
	pte_t orig_pte;
	int i;
	int pte_num;

	if (!pte_napot(pte))
		return ptep_set_access_flags(vma, addr, ptep, pte, dirty);

	pte_num = napot_pte_num(pte);
  ptep = huge_pte_offset(vma->vm_mm, addr, NAPOT_CONT64KB_SIZE);
	orig_pte = huge_ptep_get(ptep);

	if (pte_dirty(orig_pte))
		pte = pte_mkdirty(pte);

	if (pte_young(orig_pte))
		pte = pte_mkyoung(pte);

	for (i = 0; i < pte_num; i++, addr += PAGE_SIZE, ptep++)
		ptep_set_access_flags(vma, addr, ptep, pte, dirty);

  return true;
}

pte_t huge_ptep_get_and_clear(struct mm_struct *mm,
				     unsigned long addr, pte_t *ptep)
{
	int pte_num;
	pte_t orig_pte = huge_ptep_get(ptep);

	if (!pte_napot(orig_pte))
		return ptep_get_and_clear(mm, addr, ptep);

	pte_num = napot_pte_num(orig_pte);
	return get_clear_flush(mm, addr, ptep, pte_num);
}

void huge_ptep_set_wrprotect(struct mm_struct *mm,
				    unsigned long addr, pte_t *ptep)
{
	int i;
	int pte_num;
  pte_t pte = READ_ONCE(*ptep);

	if (!pte_napot(pte))
		return ptep_set_wrprotect(mm, addr, ptep);

	pte_num = napot_pte_num(pte);
  ptep = huge_pte_offset(mm, addr, NAPOT_CONT64KB_SIZE);

	for (i = 0; i < pte_num; i++, addr += PAGE_SIZE, ptep++)
		ptep_set_wrprotect(mm, addr, ptep);
}

void huge_ptep_clear_flush(struct vm_area_struct *vma,
				  unsigned long addr, pte_t *ptep)
{
	int pte_num;
  pte_t pte = READ_ONCE(*ptep);

	if (!pte_napot(pte)) {
		ptep_clear_flush(vma, addr, ptep);
		return;
	}

	pte_num = napot_pte_num(pte);
	clear_flush(vma->vm_mm, addr, ptep, pte_num);
}

void huge_pte_clear(struct mm_struct *mm, unsigned long addr,
			   pte_t *ptep, unsigned long sz)
{
	int i, pte_num;

	pte_num = napot_pte_num(READ_ONCE(*ptep));
	for (i = 0; i < pte_num; i++, addr += PAGE_SIZE, ptep++)
		pte_clear(mm, addr, ptep);
}

void riscv_set_huge_swap_pte_at(struct mm_struct *mm, unsigned long addr,
				 pte_t *ptep, pte_t pte, unsigned long sz)
{
	int i, pte_num;

	pte_num = napot_pte_num(READ_ONCE(*ptep));

	for (i = 0; i < pte_num; i++, ptep++)
		set_pte(ptep, pte);
}

int pud_huge(pud_t pud)
{
	return pud_leaf(pud);
}

int pmd_huge(pmd_t pmd)
{
	return pmd_leaf(pmd);
}

bool __init arch_hugetlb_valid_size(unsigned long size)
{
	if (size == HPAGE_SIZE)
		return true;
	else if (IS_ENABLED(CONFIG_64BIT) && size == PUD_SIZE)
		return true;
	else if (size == NAPOT_CONT64KB_SIZE)
		return true;
	else
		return false;
}

#ifdef CONFIG_CONTIG_ALLOC
static __init int gigantic_pages_init(void)
{
	/* With CONTIG_ALLOC, we can allocate gigantic pages at runtime */
	if (IS_ENABLED(CONFIG_64BIT))
		hugetlb_add_hstate(PUD_SHIFT - PAGE_SHIFT);
	return 0;
}
arch_initcall(gigantic_pages_init);
#endif

static int __init hugetlbpage_init(void)
{
	hugetlb_add_hstate(NAPOT_CONT64KB_SHIFT - PAGE_SHIFT);
	return 0;
}
arch_initcall(hugetlbpage_init);
