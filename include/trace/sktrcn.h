/**
 * sktrc.h - Simple Kernel Trace No-Op's header
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

#ifndef SKTRCN_H
#define SKTRCN_H
#define SKTRC_HASH(flags, sign, mod, func) 0
#define sktrc(hash, fmt, ...) if (0);
#define sktrcn(hash, fmt, ...) if (0);
#endif
