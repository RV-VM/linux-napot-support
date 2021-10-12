#include <linux/mm.h>
#include <linux/io.h>

void napot_modify(unsigned long vaddr, size_t size)
{
	pmd_t *pmdp;
	pte_t *ptep, *next;
	pte_t old_pte,new_pte;
	unsigned long end = vaddr + size;
	size_t napot_batch = NAPOT_CONT64KB_SIZE;
	if (size >= PMD_SIZE)
		return;
	if (size & NAPOT_CONT64KB_MASK)
		return;

	pmdp = pmd_off(&init_mm, vaddr);
	do {
		next = (pte_t *)pmd_page_vaddr(*pmdp) + pte_index(vaddr);
		old_pte = *ptep;

		new_pte = pte_mknapot(old_pte, 0);
		do {
			*ptep = new_pte;
		} while (ptep++, ptep != next);
	} while (vaddr += napot_batch, vaddr != end);
}

void __iomem *__ioremap(phys_addr_t addr, size_t size, unsigned long prot)
{
	unsigned long offset, vaddr;
	phys_addr_t last_addr;
	struct vm_struct *area;

	/* Disallow wrap-around or zero size */
	last_addr = addr + size - 1;
	if (!size || last_addr < addr)
		return NULL;

	/* Page-align mappings */
	offset = addr & (~PAGE_MASK);
	addr -= offset;
	size = PAGE_ALIGN(size + offset);

	area = get_vm_area_caller(size, VM_IOREMAP,
			__builtin_return_address(0));
	if (!area)
		return NULL;
	vaddr = (unsigned long)area->addr;

	if (ioremap_page_range(vaddr, vaddr + size, addr, __pgprot(prot))) {
		free_vm_area(area);
		return NULL;
	}

	napot_modify(vaddr, size);

	return (void __iomem *)(vaddr + offset);
}
EXPORT_SYMBOL(__ioremap);

void iounmap(volatile void __iomem *addr)
{
	vunmap((void *)((unsigned long)addr & PAGE_MASK));
}
EXPORT_SYMBOL(iounmap);
