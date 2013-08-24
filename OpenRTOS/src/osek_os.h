/* Copyright(C) 2013, OpenOSEK by Fan Wang(parai). All rights reserved.
 *
 * This file is part of OpenOSEK.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 * Email: parai@foxmail.com
 * Sourrce Open At: https://github.com/parai/OpenOSEK/
 */
#ifndef _OSEK_OS_H_
#define _OSEK_OS_H_
/* ================================ INCLUDEs  =============================== */
#include "Os.h"

/* ================================ MACROs    =============================== */
#define TASK_PC(name) TaskMain##name
/* ================================ TYPEs     =============================== */
/* Priority type of Task */
typedef uint8 PriorityType;
typedef uint16 StackSizeType;

/* ================================ FUNCTIONs =============================== */
IMPORT void knl_task_init(void);
#endif /* _OSEK_OS_H_ */
