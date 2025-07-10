/**
 * sktrc.h - Simple Kernel Trace's header
 * Copyright (C) 2025  Sidney PEPO <sidneypepo@disroot.org>
 *
 * This file is part of Simple Kernel Trace.
 *
 * Simple Kernel Trace is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * Simple Kernel Trace is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with Simple Kernel Trace; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  US
 */

#ifndef SKTRC_H
#define SKTRC_H
#include <linux/types.h>

#define SKTRC_HASH(flags, sign, mod, func) \
	(((flags & 0x3) << 30) + ((sign & 0x3) << 28) + \
	 ((mod & 0xfff) << 16) + (func & 0xffff))

extern u_int8_t sktrc(u_int32_t hash, const char *fmt, ...);
#define sktrcn(hash, fmt, ...) if (0);
#endif
