// SPDX-License-Identifier: GPL-2.0-only
/*
 *  src/gwbot/include/gwbot/cpu.h
 *
 *  Hexdump macro for GNUWeebBot
 *
 *  Copyright (C) 2021  Ammar Faizi
 */

#ifndef GWBOT__CPU_H
#define GWBOT__CPU_H

#include <gwbot/base.h>


static __always_inline void __cpu_relax()
{
	asm volatile("rep; nop;");
}

#define cpu_relax __cpu_relax

#endif /* #ifndef GWBOT__CPU_H */
