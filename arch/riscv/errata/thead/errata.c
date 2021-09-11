// SPDX-License-Identifier: GPL-2.0-only

#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/bug.h>
#include <linux/mm.h>
#include <asm/patch.h>
#include <asm/alternative.h>
#include <asm/vendorid_list.h>
#include <asm/errata_list.h>
#include <asm/pgtable.h>

/*
 * T-HEAD C9xx PTE format:
 * | 63 | 62 | 61 | 60 | 59-8 | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0
 *   SO   C    B    SH   RSW    D   A   G   U   X   W   R   V
 *   ^    ^    ^    ^    ^
 * BIT(63): SO - Strong Order
 * BIT(62): C  - Cacheable
 * BIT(61): B  - Bufferable
 * BIT(60): SH - Shareable
 *
 * MT_MASK : [63 - 59]
 * MT_PMA  : C + SH
 * MT_NC   : (none)
 * MT_IO   : SO
 */
void __init thead_errata_setup_vm(unsigned long archid, unsigned long impid)
{
	int i;

#ifdef CONFIG_64BIT
	__riscv_svpbmt.mask	= 0xf800000000000000;
	__riscv_svpbmt.mt_pma	= 0x5000000000000000;
	__riscv_svpbmt.mt_nc	= 0x0;
	__riscv_svpbmt.mt_io	= 0x8000000000000000;
#endif

	for (i = 0; i < 16; i++)
		pgprot_val(protection_map[i]) |= __riscv_svpbmt.mt_pma;
}
