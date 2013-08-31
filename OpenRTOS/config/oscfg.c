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
#include "Os.h"
#include "osek_os.h"

/* ====================== Tasks ====================== */
IMPORT TASK(Task0);
IMPORT TASK(Task1);

EXPORT const FP knl_tcb_pc[] = 
{
	TASK_PC(Task0),
	TASK_PC(Task1),
};

EXPORT const PriorityType knl_tcb_ipriority[] = 
{
	Task0_ipriority,
	Task1_ipriority,
};

EXPORT const PriorityType knl_tcb_rpriority[] = 
{
	Task0_rpriority,
	Task1_rpriority,
};

EXPORT const StackSizeType knl_tcb_stksz[] = 
{
	Task0_stacksize,
	Task1_stacksize,
};

EXPORT const AppModeType knl_tcb_mode[] = 
{
	Task0_appmode,
	Task1_appmode,
};


/* ====================== Task Ready Queue ====================== */
LOCAL TaskType knl_5_queue[3];
LOCAL TaskType knl_10_queue[3];
EXPORT RDYQUE knl_rdyque = 
{
	/* top_pri= */ NUM_PRI,
	{/* tskque[] */
		{/* head= */ 0,/* tail= */ 0,/* length= */ 0, /* queue= */ NULL},
		{/* head= */ 0,/* tail= */ 0,/* length= */ 0, /* queue= */ NULL},
		{/* head= */ 0,/* tail= */ 0,/* length= */ 0, /* queue= */ NULL},
		{/* head= */ 0,/* tail= */ 0,/* length= */ 0, /* queue= */ NULL},
		{/* head= */ 0,/* tail= */ 0,/* length= */ 0, /* queue= */ NULL},
		{/* head= */ 0,/* tail= */ 0,/* length= */ 3, /* queue= */ knl_5_queue},
		{/* head= */ 0,/* tail= */ 0,/* length= */ 0, /* queue= */ NULL},
		{/* head= */ 0,/* tail= */ 0,/* length= */ 0, /* queue= */ NULL},
		{/* head= */ 0,/* tail= */ 0,/* length= */ 0, /* queue= */ NULL},
		{/* head= */ 0,/* tail= */ 0,/* length= */ 0, /* queue= */ NULL},
		{/* head= */ 0,/* tail= */ 0,/* length= */ 3, /* queue= */ knl_10_queue},
		{/* head= */ 0,/* tail= */ 0,/* length= */ 0, /* queue= */ NULL},
	},
	/* null */{/* head= */ 0,/* tail= */ 0,/* length= */ 0, /* queue= */ NULL},
};



