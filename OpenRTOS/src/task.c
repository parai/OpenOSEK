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

/* ================================ INCLUDEs  =============================== */
#include "osek_os.h"

/* ================================ MACROs    =============================== */

/* ================================ TYPEs     =============================== */

/* ================================ DATAs     =============================== */
EXPORT TaskStateType       knl_tcb_state[cfgOS_TASK_NUM];
EXPORT PriorityType        knl_tcb_curpri[cfgOS_TASK_NUM];
EXPORT TaskType knl_curtsk;
EXPORT TaskType knl_schedtsk;

/* ================================ FUNCTIONs =============================== */
EXPORT StatusType TerminateTask(void)
{
    StatusType ercd = E_OK;
    
    return ercd;
}



//OS-impl internal function
EXPORT void knl_task_init(void)
{
	uint8 i;
	for(i=0; i<cfgOS_TASK_NUM; i++)
	{
		knl_tcb_state[i] = SUSPENDED;
		if((knl_tcb_mode[i]&knl_appmode) != 0u)
		{
			knl_make_active(i);
		}
	}
}
EXPORT void knl_make_active(TaskType taskid)
{
	knl_make_ready(taskid);
	knl_make_runnable(taskid);
}

EXPORT void knl_make_ready(TaskType taskid)
{
	knl_tcb_state[taskid] = READY;
	knl_tcb_curpri[taskid] = knl_tcb_ipriority[taskid];
	knl_setup_context(taskid);
}
EXPORT void knl_make_runnable(TaskType taskid)
{
	if(INVALID_TASK != knl_schedtsk)
	{
		if(knl_tcb_curpri[taskid] > knl_tcb_curpri[knl_schedtsk])
		{   /* taskid has higher priority */
			//when task is non-preemtable,its priority will be the highest when run.
			knl_ready_queue_insert_top(knl_schedtsk);
		}
		else
		{   /* taskid has lower priority */
			knl_ready_queue_insert(taskid);
			return;
		}
	}
	knl_schedtsk = taskid;
}
EXPORT void knl_ready_queue_insert_top(TaskType taskid)
{

}

EXPORT void knl_ready_queue_insert(TaskType taskid)
{

}
