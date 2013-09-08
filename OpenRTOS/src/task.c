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
EXPORT uint8               knl_tcb_activation[cfgOS_TASK_NUM];
EXPORT TaskType knl_curtsk;
EXPORT TaskType knl_schedtsk;

/* ================================ FUNCTIONs =============================== */
EXPORT StatusType TerminateTask(void)
{
    StatusType ercd = E_OK;
    OS_EXT_VALIDATE((0 == knl_taskindp),E_OS_CALLEVEL);
    // OS_EXT_VALIDATE(,E_OS_RESOURCE);
    DISABLE_INTERRUPT();
    if(knl_tcb_activation[knl_curtsk] > 0)
    {
    	knl_tcb_activation[knl_curtsk] --;
    	knl_make_ready(knl_curtsk);
    }
    else
    {
    	knl_tcb_state[knl_curtsk] = SUSPENDED;
    }
    knl_search_schedtsk();
    knl_force_dispatch();

OS_VALIDATE_ERROR_EXIT()
    return ercd;
}
EXPORT StatusType ActivateTask ( TaskType TaskID )
{
	StatusType ercd = E_OK;
	OS_STD_VALIDATE((TaskID<cfgOS_TASK_NUM),E_OS_ID);
	BEGIN_CRITICAL_SECTION();
	if(SUSPENDED == knl_tcb_state[TaskID])
	{
		knl_make_active(TaskID);
	}
	else
	{
		if(knl_tcb_activation[TaskID] < knl_tcb_max_activation[TaskID])
		{
			knl_ready_queue_insert(TaskID);
			knl_tcb_activation[TaskID] ++ ;
		}
		else
		{
			ercd = E_OS_LIMIT;
		}
	}
	END_CRITICAL_SECTION();
OS_VALIDATE_ERROR_EXIT()
	return ercd;
}
EXPORT  StatusType ChainTask    ( TaskType TaskID )
{
	StatusType ercd = E_OK;
	OS_STD_VALIDATE((TaskID<cfgOS_TASK_NUM),E_OS_ID);
	OS_EXT_VALIDATE((0 == knl_taskindp),E_OS_CALLEVEL);
	//OS_EXT_VALIDATE(,E_OS_RESOURCE);
	DISABLE_INTERRUPT();
	if(TaskID == knl_curtsk)
	{	// chain to itself.
		knl_search_schedtsk();
		knl_make_active(TaskID);
	}
	else
	{
		//firstly terminate current running task
		knl_tcb_state[knl_curtsk] = SUSPENDED;
		if(SUSPENDED == knl_tcb_state[TaskID])
		{
			knl_make_active(TaskID);
		}
	}
	knl_force_dispatch();

OS_VALIDATE_ERROR_EXIT()
	return ercd;
}

//OS-impl internal function
EXPORT void knl_task_init(void)
{
	uint8 i;

	knl_schedtsk = knl_curtsk = INVALID_TASK;
	/* init ready queue */
	knl_rdyque.top_pri = NUM_PRI;
	for ( i = 0; i < NUM_PRI; i++ ) {
		knl_rdyque.tskque[i].head = knl_rdyque.tskque[i].tail = 0;
	}
	(void)memset(knl_rdyque.bitmap, 0, sizeof(knl_rdyque.bitmap));

	knl_dispatch_disabled = 1; /* disable dispatch */

	for(i=0; i<cfgOS_TASK_NUM; i++)
	{
		knl_tcb_state[i] = SUSPENDED;
		knl_tcb_activation[i] = 0;
		if((knl_tcb_mode[i]&knl_appmode) != 0u)
		{
			knl_make_active(i);
		}
	}
}
// ready to run, but not runnable(cann't be scheduled),
// not in the ready queue
EXPORT void knl_make_ready(TaskType taskid)
{
	knl_tcb_state[taskid] = READY;
	knl_tcb_curpri[taskid] = knl_tcb_ipriority[taskid];
	knl_setup_context(taskid);
}

// put into ready-queue
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

EXPORT void knl_search_schedtsk(void)
{
	PriorityType top_pri = knl_rdyque.top_pri;
	TaskReadyQueueType *tskque;

	tskque = &(knl_rdyque.tskque[top_pri]);
	if(tskque->head != tskque->tail)
	{  // not empty.
		knl_schedtsk = tskque->queue[tskque->head];
		if((tskque->head+1) < tskque->length)
		{
			tskque->head++;
		}
		else
		{
			tskque->head = 0;
		}
		if(tskque->head == tskque->tail)
		{
			knl_bitmap_clear(top_pri);
			knl_rdyque.top_pri = knl_bitmap_search(top_pri);
		}
	}
	else
	{
		knl_schedtsk = INVALID_TASK;
	}
}
EXPORT void knl_ready_queue_insert_top(TaskType taskid)
{
	PriorityType priority = knl_tcb_curpri[taskid];
	TaskReadyQueueType *tskRdyQue = &knl_rdyque.tskque[priority];
	if(0 == tskRdyQue->head)
	{
		tskRdyQue->head = tskRdyQue->length - 1;
	}
	else
	{
		tskRdyQue->head --;
	}
	tskRdyQue->queue[tskRdyQue->head] = taskid;

	if((priority > knl_rdyque.top_pri) || (NUM_PRI == knl_rdyque.top_pri))
	{
		knl_rdyque.top_pri = priority;
	}

	knl_bitmap_set(priority);
}

EXPORT void knl_ready_queue_insert(TaskType taskid)
{
	PriorityType priority = knl_tcb_ipriority[taskid];
	TaskReadyQueueType *tskRdyQue = &knl_rdyque.tskque[priority];

	tskRdyQue->queue[tskRdyQue->tail] = taskid;
	if((tskRdyQue->tail+1) < tskRdyQue->length)
	{
		tskRdyQue->tail++;
	}
	else
	{
		tskRdyQue->tail = 0;
	}

	if((priority > knl_rdyque.top_pri) || (NUM_PRI == knl_rdyque.top_pri))
	{
		knl_rdyque.top_pri = priority;
	}

	knl_bitmap_set(priority);
}

EXPORT void knl_bitmap_set(PriorityType priority)
{
	uint8* pb = knl_rdyque.bitmap;
	uint8 mask = (1<<(priority&0x07));
	pb += (priority>>3);
	*pb |= mask;
}

EXPORT void knl_bitmap_clear(PriorityType priority)
{
	uint8* pb = knl_rdyque.bitmap;
	uint8 mask = (1<<(priority&0x07));
	pb += (priority>>3);
	*pb &= (~mask);
}

// search from priority "from" to 0.
EXPORT PriorityType knl_bitmap_search(PriorityType from)
{
	uint8* pb;
	PriorityType offset;
	uint8 i;

	if(0 == from)
	{
		return NUM_PRI; // no ready task
	}

	pb = knl_rdyque.bitmap;
	pb += (from>>3);

	offset = 1;

	if(0 != (*pb))
	{
		for(i=(from&0x07) ; i>0 ; i--)
		{
			if(((*pb)&(1<<(i-1))) != 0)
			{
				return (from -offset);
			}
			else
			{
				offset ++;
			}
		}
		/* when here error */
	}
	else
	{
		offset += (from&0x07);
	}

	while(pb != knl_rdyque.bitmap)
	{
		pb--;
		if(0 != (*pb))
		{
			for(i=8 ; i>0 ; i--)
			{
				if(((*pb)&(1<<(i-1))) != 0)
				{
					return (from -offset);
				}
				else
				{
					offset ++;
				}
			}
		}
		else
		{
			offset += 8;
		}
	}

	return NUM_PRI;
}
