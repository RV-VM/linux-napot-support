#ifndef __ASM_MMAN_H__
#define __ASM_MMAN_H__

#include <uapi/asm-generic/mman.h>

#ifdef CONFIG_NAPOT_SUPPORT

unsigned long arch_calc_vm_flag_bits(unsigned long flags);
#define arch_calc_vm_flag_bits(flags) arch_calc_vm_flag_bits(flags)

#endif /* CONFIG_NAPOT_SUPPORT */

#endif /* ! __ASM_MMAN_H__ */
