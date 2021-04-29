#ifndef _ASM_NAPOT_H
#define _ASM_NAPOT_H

#include <linux/mm_types.h>
#include <linux/gfp.h>
#include <linux/highmem.h>
#include <linux/mm.h>
#include <linux/printk.h>

#define NAPOT_PAGE_SIZE(x) (1 << (PAGE_SHIFT + x))
#define NAPOT_MASK(x) (~(NAPOT_PAGE_SIZE(x) - 1))

/* check whether the vma is managed with napot */
static inline bool vma_use_napot(struct vm_area_struct *vma)
{
	bool ret = vma->vm_flags & VM_NAPOT_64K;
	if(ret)
		pr_info("check napot: vm_flags = 0x%lx", vma->vm_flags);
	return ret;
}

static inline struct page *
__alloc_zeroed_compound_user_highpage(struct vm_area_struct *vma,
					unsigned long vaddr, unsigned int order)
{
	struct page *page = alloc_pages(GFP_HIGHUSER | __GFP_COMP, order);
	unsigned long i;
	if(!page)
		goto out;
	for(i = 0; i < compound_nr(page); ++i) {
		struct page *p = page + i;
		clear_user_highpage(p, vaddr);
	}
	i = 0;

out:
	return page;
}

static vm_fault_t do_anonymous_napot_pages(struct vm_fault *vmf)
{
	struct vm_area_struct *vma = vmf->vma;
	struct page *page;
	vm_fault_t ret = 0;
	pte_t entry;
	unsigned long start_addr, end_addr;
	unsigned int order = 0;

	pr_info("need to solve napot ptes, addr = 0x%lx \n", vmf->address);
	pr_info("region of vma is [%lx, %lx] \n", vma->vm_start, vma->vm_end);

	if (pte_alloc(vma->vm_mm, vmf->pmd))
		return VM_FAULT_OOM;

	/* Allocate our own private page. */
	if (unlikely(anon_vma_prepare(vma)))
		goto oom;
	// TODO check order
	order = 4;
	start_addr = vmf->address & NAPOT_MASK(order);
	end_addr = start_addr + NAPOT_PAGE_SIZE(order);
	pr_info("start_add = 0x%lx, end_addr = 0x%lx \n", start_addr, end_addr);
	page = __alloc_zeroed_compound_user_highpage(vma, start_addr, order);
	if(!page)
		goto oom;

	if (mem_cgroup_charge(page, vma->vm_mm, GFP_KERNEL))
		goto oom_free_page;

	entry = mk_pte(page, vma->vm_page_prot);
	if (vma->vm_flags & VM_WRITE)
		entry = pte_mkwrite(pte_mkdirty(entry));

	vmf->ptl = pte_lockptr(vma->vm_mm, vmf->pmd);
	spin_lock(vmf->ptl);
	vmf->pte = pte_offset_map(vmf->pmd, start_addr);
	if(!pte_none(*(vmf->pte))) {
		update_mmu_cache(vma, vmf->address, vmf->pte);
		goto release;
	}
	
	for(; start_addr < end_addr; start_addr += PAGE_SIZE, ++vmf->pte)
		set_pte_at(vma->vm_mm, start_addr, vmf->pte, entry);

unlock:
	spin_unlock(vmf->ptl);
	return ret;
release:
	put_page(page);
	goto unlock;
oom_free_page:
	put_page(page);
oom:
	return VM_FAULT_OOM;
}

#endif
