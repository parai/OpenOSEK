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
#include "osek_os.h"

/* ====================== Tasks ====================== */
IMPORT TASK(Task0);
IMPORT TASK(Task1);
IMPORT TASK(Task2);
IMPORT TASK(Task3);

EXPORT const FP knl_tcb_pc[] = 
{
	TASK_PC(Task0),
	TASK_PC(Task1),
	TASK_PC(Task2),
	TASK_PC(Task3),
};

EXPORT const PriorityType knl_tcb_ipriority[] = 
{
	Task0_ipriority,
	Task1_ipriority,
	Task2_ipriority,
	Task3_ipriority,
};

EXPORT const PriorityType knl_tcb_rpriority[] = 
{
	Task0_rpriority,
	Task1_rpriority,
	Task2_rpriority,
	Task3_rpriority,
};

LOCAL uint8 knl_Task0_stack[Task0_stacksize];
LOCAL uint8 knl_Task1_stack[Task1_stacksize];
LOCAL uint8 knl_Task2_stack[Task2_stacksize];
LOCAL uint8 knl_Task3_stack[Task3_stacksize];
EXPORT const StackSizeType knl_tcb_stksz[] = 
{
	Task0_stacksize,
	Task1_stacksize,
	Task2_stacksize,
	Task3_stacksize,
};

EXPORT const uint8* knl_tcb_stack[] = 
{
	(knl_Task0_stack+Task0_stacksize),
	(knl_Task1_stack+Task1_stacksize),
	(knl_Task2_stack+Task2_stacksize),
	(knl_Task3_stack+Task3_stacksize),
};

EXPORT const uint8 knl_tcb_max_activation[] = 
{
	(Task0_activation - 1),
	(Task1_activation - 1),
	(Task2_activation - 1),
	(Task3_activation - 1),
};

EXPORT const uint8 knl_tcb_flgid[] = 
{
	Task0_eventhandle,
	Task1_eventhandle,
	Task2_eventhandle,
	Task3_eventhandle,
};

EXPORT const AppModeType knl_tcb_mode[] = 
{
	Task0_appmode,
	Task1_appmode,
	Task2_appmode,
	Task3_appmode,
};


/* ====================== Task Ready Queue ====================== */
LOCAL TaskType knl_0_queue[4];
LOCAL TaskType knl_1_queue[2];
LOCAL TaskType knl_2_queue[4];
LOCAL TaskType knl_3_queue[4];
EXPORT RDYQUE knl_rdyque = 
{
	/* top_pri= */ NUM_PRI,
	{/* tskque[] */
		{/* head= */ 0,/* tail= */ 0,/* length= */ 4, /* queue= */ knl_0_queue},
		{/* head= */ 0,/* tail= */ 0,/* length= */ 2, /* queue= */ knl_1_queue},
		{/* head= */ 0,/* tail= */ 0,/* length= */ 4, /* queue= */ knl_2_queue},
		{/* head= */ 0,/* tail= */ 0,/* length= */ 4, /* queue= */ knl_3_queue},
	},
	/* null */{/* head= */ 0,/* tail= */ 0,/* length= */ 0, /* queue= */ NULL},
};


/* ====================== Counters ====================== */
EXPORT const TickType knl_ccb_max[] = 
{
	SystemTimer_maxallowedvalue,
};
EXPORT const TickType knl_ccb_tpb[] = 
{
	SystemTimer_ticksperbase,
};
EXPORT const TickType knl_ccb_min[] = 
{
	SystemTimer_mincycle,
};

/* ====================== Alarms ====================== */
EXPORT const CounterType knl_acb_counter[] = 
{
	WakeTask0_counter,
	WakeTask1_counter,
};
EXPORT const TickType knl_acb_time[] = 
{
	WakeTask0_time,
	WakeTask1_time,
};
EXPORT const TickType knl_acb_cycle[] = 
{
	WakeTask0_cycle,
	WakeTask1_cycle,
};
EXPORT const CounterType knl_acb_mode[] = 
{
	WakeTask0_appmode,
	WakeTask1_appmode,
};
LOCAL void AlarmMainWakeTask0(void)
{
	(void)ActivateTask(Task0);
}
LOCAL void AlarmMainWakeTask1(void)
{
	(void)ActivateTask(Task1);
}
EXPORT const FP knl_acb_action[] = 
{
	AlarmMainWakeTask0,
	AlarmMainWakeTask1,
};


