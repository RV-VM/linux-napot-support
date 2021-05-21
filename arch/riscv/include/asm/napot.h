#ifndef _ASM_NAPOT_H
#define _ASM_NAPOT_H

#include <linux/mm_types.h>
#include <linux/mm.h>

#define NAPOT_PAGE_SIZE(x) (1 << (PAGE_SHIFT + x))
#define NAPOT_MASK(x) (~(NAPOT_PAGE_SIZE(x) - 1))

/* check whether the vma is managed with napot */
static inline bool vma_use_napot(struct vm_area_struct *vma)
{
	bool ret = vma->vm_flags & VM_NAPOT_64K;
	return ret;
}

static inline unsigned int get_napot_order(struct vm_area_struct *vma)
{
	unsigned int ret = (vma->vm_flags >> VM_NAPOT_SHIFT);
	return ret;
}

vm_fault_t do_anonymous_napot_pages(struct vm_fault *vmf);

vm_fault_t do_napot_pages(struct vm_fault *vmf);

#endif
