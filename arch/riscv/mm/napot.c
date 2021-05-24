#include <asm/napot.h>
#include <linux/highmem.h>
#include <linux/mm.h>
#include <linux/rmap.h>
#include <linux/mm_types.h>
#include <linux/printk.h>

vm_fault_t do_anonymous_napot_pages(struct vm_fault *vmf)
{
	struct vm_area_struct *vma = vmf->vma;
	struct page *page;
	vm_fault_t ret = 0;
	pte_t entry;
	unsigned long start_addr, end_addr;

	pr_info("need to solve napot ptes, addr = 0x%lx \n", vmf->address);
	pr_info("region of vma is [%lx, %lx] \n", vma->vm_start, vma->vm_end);

	if (pte_alloc(vma->vm_mm, vmf->pmd))
		return VM_FAULT_OOM;

	/* Allocate our own private page. */
	if (unlikely(anon_vma_prepare(vma)))
		goto oom;

	start_addr = vmf->address & NAPOT_MASK(4);
	end_addr = start_addr + NAPOT_PAGE_SIZE(4);
	pr_info("start_add = 0x%lx, end_addr = 0x%lx \n", start_addr, end_addr);
	page = alloc_zeroed_user_highpage_movable(vma, start_addr);
	if(!page)
		goto oom;

	if (mem_cgroup_charge(page, vma->vm_mm, GFP_KERNEL))
		goto oom_free_page;

	entry = pte_mknapot(mk_pte(page, vma->vm_page_prot), 4);
	pr_info("after mknapot pte is 0x%lx \n", pte_val(entry));
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

vm_fault_t do_napot_pages(struct vm_fault *vmf)
{
	pte_t entry = vmf->orig_pte;
	if (!vmf->pte) {
		return do_anonymous_napot_pages(vmf);
	/*else if(vmf->flags & FAULT_FLAG_WRITE) {*/
		//if (!pte_write(entry))
			//return do_napot_wp_pages(vmf);
	} else {
		pr_info("do_napot_pages failed!\n");
	}
	return 0;
}
