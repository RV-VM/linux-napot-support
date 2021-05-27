#include <linux/mm_types.h>
#include <linux/mm.h>
#include <linux/highmem-internal.h>
#include <asm/cacheflush.h>
#include <asm/napot.h>
#include <linux/printk.h>

void clear_user_highpage(struct page *page, unsigned long vaddr)
{
	void *addr = kmap_atomic(page);
	memset(addr, 0, PAGE_SIZE * compound_nr(page));
	kunmap_atomic(addr);
}

struct page *
riscv_alloc_zeroed_highpage(gfp_t movableflags,
			struct vm_area_struct *vma,
			unsigned long vaddr)
{
	unsigned int order = 0;
	struct page *page = NULL;
#ifdef CONFIG_NAPOT_SUPPORT
	if (vma_use_napot(vma)) {
		order = get_napot_order(vma);
		movableflags |= __GFP_COMP;
	}
#endif
	page = alloc_pages_vma(GFP_HIGHUSER | movableflags,
			order, vma, vaddr, numa_node_id(), false);

	if (page)
		clear_user_highpage(page, vaddr);

	return page;
}

bool vma_use_napot(struct vm_area_struct *vma)
{
	bool ret = vma->vm_flags & VM_NAPOT_64K;
	return ret;
}
