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
#if( (cfgOS_MULTIPLY_ACTIVATION == 0) && (cfgOS_MULTIPLY_PRIORITY == 0) )
EXPORT RDYQUE knl_rdyque;
#endif

#if(cfgOS_MULTIPLY_ACTIVATION == 1)
EXPORT uint8               knl_tcb_activation[cfgOS_TASK_NUM];
#endif
EXPORT volatile TaskType knl_curtsk;
EXPORT volatile TaskType knl_schedtsk;

/* ================================ FUNCTIONs =============================== */
/* |------------------+--------------------------------------------------------------| */
/* | Syntax:          | StatusType TerminateTask ( void )                            | */
/* |------------------+--------------------------------------------------------------| */
/* | Parameter (In):  | none                                                         | */
/* |------------------+--------------------------------------------------------------| */
/* | Parameter (Out): | none                                                         | */
/* |------------------+--------------------------------------------------------------| */
/* | Description:     | This service causes the termination of the calling task. The | */
/* |                  | calling task is transferred from the running state into the  | */
/* |                  | suspended state.                                             | */
/* |------------------+--------------------------------------------------------------| */
/* | Particularities: | 1) An internal resource assigned to the calling task is      | */
/* |                  | automatically released. Other resources occupied by the task | */
/* |                  | shall have been released before the call to TerminateTask.   | */
/* |                  | If a resource is still occupied in standard status the       | */
/* |                  | behaviour is undefined.                                      | */
/* |                  | 2) If the call was successful, TerminateTask does not return | */
/* |                  | to the call level and the status can not be evaluated.       | */
/* |                  | 3) If the version with extended status is used, the service  | */
/* |                  | returns in case of error, and provides a status which can be | */
/* |                  | evaluated in the application.                                | */
/* |                  | 4) If the service TerminateTask is called successfully, it   | */
/* |                  | enforces a rescheduling.                                     | */
/* |                  | 5) Ending a task function without call to TerminateTask or   | */
/* |                  | ChainTask is strictly forbidden and may leave the system in  | */
/* |                  | an undefined state.                                          | */
/* |------------------+--------------------------------------------------------------| */
/* | Status:          | Standard:                                                    | */
/* |                  | 1)No return to call level                                    | */
/* |                  | Extended:                                                    | */
/* |                  | 1) Task still occupies resources, E_OS_RESOURCE              | */
/* |                  | 2) Call at interrupt level, E_OS_CALLEVEL                    | */
/* |------------------+--------------------------------------------------------------| */
/* | Conformance:     | BCC1, BCC2, ECC1, ECC2                                       | */
/* |------------------+--------------------------------------------------------------| */
EXPORT StatusType TerminateTask(void)
{
    StatusType ercd = E_OK;
    OS_EXT_VALIDATE((0 == knl_taskindp),E_OS_CALLEVEL);
    OS_EXT_VALIDATE((INVALID_RESOURCE == knl_tcb_resque[knl_curtsk]),E_OS_RESOURCE);
    DISABLE_INTERRUPT();
    //release internal resource or for non-preemtable task
    ReleaseInternalResource();
#if( cfgOS_MULTIPLY_ACTIVATION == 1 )
    if(knl_tcb_activation[knl_curtsk] > 0)
    {
    	knl_tcb_activation[knl_curtsk] --;
    	knl_make_ready(knl_curtsk);
    	devTrace(tlOs,"TerminateTask Task %d, Goto READY state as Activation > 0.\n",(int)knl_curtsk);
    }
    else
#endif
    {
    	knl_tcb_state[knl_curtsk] = SUSPENDED;
    	devTrace(tlOs,"TerminateTask Task %d, Goto SUSPENDED state.\n",(int)knl_curtsk);
    }
    knl_search_schedtsk();
    knl_force_dispatch();

OS_VALIDATE_ERROR_EXIT()
    OsErrorProcess0(TerminateTask);
    return ercd;
}

/* |------------------+------------------------------------------------------------| */
/* | Syntax:          | StatusType ActivateTask ( TaskType <TaskID> )              | */
/* |------------------+------------------------------------------------------------| */
/* | Parameter (In):  | TaskID: Task reference                                     | */
/* |------------------+------------------------------------------------------------| */
/* | Parameter (Out): | none                                                       | */
/* |------------------+------------------------------------------------------------| */
/* | Description:     | The task <TaskID> is transferred from the suspended state  | */
/* |                  | into the ready state. The operating system ensures that    | */
/* |                  | the task code is being executed from the first statement.  | */
/* |------------------+------------------------------------------------------------| */
/* | Particularities: | 1) The service may be called from interrupt level and from | */
/* |                  | task level (see Figure 12-1(os223.doc)).                   | */
/* |                  | 2) Rescheduling after the call to ActivateTask depends on  | */
/* |                  | the place it is called from (ISR, non preemptable task,    | */
/* |                  | preemptable task).                                         | */
/* |                  | 3)If E_OS_LIMIT is returned the activation is ignored.     | */
/* |                  | 4)When an extended task is transferred from suspended      | */
/* |                  | state into ready state all its events are cleared.         | */
/* |------------------+------------------------------------------------------------| */
/* | Status:          | Standard:                                                  | */
/* |                  | 1) No error, E_OK                                          | */
/* |                  | 2) Too many task activations of <TaskID>, E_OS_LIMIT       | */
/* |                  | Extended:                                                  | */
/* |                  | 1) Task <TaskID> is invalid, E_OS_ID                       | */
/* |------------------+------------------------------------------------------------| */
/* | Conformance:     | BCC1, BCC2, ECC1, ECC2                                     | */
/* |------------------+------------------------------------------------------------| */
EXPORT StatusType ActivateTask ( TaskType TaskID )
{
	StatusType ercd = E_OK;
	OS_STD_VALIDATE((TaskID<cfgOS_TASK_NUM),E_OS_ID);

	BEGIN_CRITICAL_SECTION();
	if(SUSPENDED == knl_tcb_state[TaskID])
	{
		knl_make_active(TaskID);
		devTrace(tlOs,"ActivateTask Task %d in SUSPENDED state.\n",(int)TaskID);
	}
#if(cfgOS_MULTIPLY_ACTIVATION == 1)
	else
	{
		if(knl_tcb_activation[TaskID] < knl_tcb_max_activation[TaskID])
		{
			knl_ready_queue_insert(TaskID);
			knl_tcb_activation[TaskID] ++ ;
			devTrace(tlOs,"Multiply ActivateTask Task %d.\n",(int)TaskID);
		}
		else
		{
			ercd = E_OS_LIMIT;
		}
	}
#else
	else
	{
		ercd = E_OS_LIMIT;
	}
#endif
	END_CRITICAL_SECTION();
OS_VALIDATE_ERROR_EXIT()
	OsErrorProcess1(ActivateTask,tskid,TaskID);
	return ercd;
}

/* |------------------+-------------------------------------------------------------| */
/* | Syntax:          | StatusType ChainTask ( TaskType <TaskID> )                  | */
/* |------------------+-------------------------------------------------------------| */
/* | Parameter (In):  | TaskID Reference to the sequential succeeding task to       | */
/* |                  | be activated.                                               | */
/* |------------------+-------------------------------------------------------------| */
/* | Parameter (Out): | none                                                        | */
/* |------------------+-------------------------------------------------------------| */
/* | Description:     | This service causes the termination of the calling task.    | */
/* |                  | After termination of the calling task a succeeding task     | */
/* |                  | <TaskID> is activated. Using this service, it ensures       | */
/* |                  | that the succeeding task starts to run at the earliest      | */
/* |                  | after the calling task has been terminated.                 | */
/* |------------------+-------------------------------------------------------------| */
/* | Particularities: | 1. If the succeeding task is identical with the current     | */
/* |                  | task, this does not result in multiple requests. The task   | */
/* |                  | is not transferred to the suspended state, but will         | */
/* |                  | immediately become ready again.                             | */
/* |                  | 2. An internal resource assigned to the calling task is     | */
/* |                  | automatically released, even if the succeeding task is      | */
/* |                  | identical with the current task. Other resources occupied   | */
/* |                  | by the calling shall have been released before ChainTask    | */
/* |                  | is called. If a resource is still occupied in standard      | */
/* |                  | status the behaviour is undefined.                          | */
/* |                  | 3. If called successfully, ChainTask does not return to     | */
/* |                  | the call level and the status can not be evaluated.         | */
/* |                  | 4. In case of error the service returns to the calling      | */
/* |                  | task and provides a status which can then be evaluated      | */
/* |                  | in the application.                                         | */
/* |                  | 5.If the service ChainTask is called successfully, this     | */
/* |                  | enforces a rescheduling.                                    | */
/* |                  | 6. Ending a task function without call to TerminateTask     | */
/* |                  | or ChainTask is strictly forbidden and may leave the system | */
/* |                  | in an undefined state.                                      | */
/* |                  | 7. If E_OS_LIMIT is returned the activation is ignored.     | */
/* |                  | 8. When an extended task is transferred from suspended      | */
/* |                  | state into ready state all its events are cleared.          | */
/* |------------------+-------------------------------------------------------------| */
/* | Status:          | Standard:                                                   | */
/* |                  | 1. No return to call level                                  | */
/* |                  | 2. Too many task activations of <TaskID>, E_OS_LIMIT        | */
/* |                  | Extended:                                                   | */
/* |                  | 1. Task <TaskID> is invalid, E_OS_ID                        | */
/* |                  | 2. Calling task still occupies resources, E_OS_RESOURCE     | */
/* |                  | 3. Call at interrupt level, E_OS_CALLEVEL                   | */
/* |------------------+-------------------------------------------------------------| */
/* | Conformance:     | BCC1, BCC2, ECC1, ECC2                                      | */
/* |------------------+-------------------------------------------------------------| */
EXPORT  StatusType ChainTask    ( TaskType TaskID )
{
	StatusType ercd = E_OK;
	OS_STD_VALIDATE((TaskID<cfgOS_TASK_NUM),E_OS_ID);
	OS_EXT_VALIDATE((0 == knl_taskindp),E_OS_CALLEVEL);
	OS_EXT_VALIDATE((INVALID_RESOURCE == knl_tcb_resque[knl_curtsk]),E_OS_RESOURCE);
	DISABLE_INTERRUPT();
	if(TaskID == knl_curtsk)
	{	// chain to itself.
		//release internal resource or for non-preemtable task
		ReleaseInternalResource();
		knl_search_schedtsk();
		knl_make_active(TaskID);
	}
	else
	{

		if(SUSPENDED != knl_tcb_state[TaskID])
		{
#if((cfgOS_MULTIPLY_ACTIVATION == 1))
			if(knl_tcb_activation[TaskID] < knl_tcb_max_activation[TaskID])
			{
				knl_ready_queue_insert(TaskID);
				knl_tcb_activation[TaskID] ++ ;
			}
			else
#endif
			{
				ercd = E_OS_LIMIT;
				goto Error_Exit;
			}
		}

		//release internal resource or for non-preemtable task
		ReleaseInternalResource();
		//firstly terminate current running task
#if((cfgOS_MULTIPLY_ACTIVATION == 1))
		if(knl_tcb_activation[knl_curtsk] > 0)
		{
			knl_tcb_activation[knl_curtsk] --;
			knl_make_ready(knl_curtsk);
		}
		else
#endif
		{
			knl_tcb_state[knl_curtsk] = SUSPENDED;
		}
		// then search the next running task.
		knl_search_schedtsk();
		if(SUSPENDED == knl_tcb_state[TaskID])
		{
			knl_make_active(TaskID);
		}
	}
	knl_force_dispatch();

OS_VALIDATE_ERROR_EXIT()
	ENABLE_INTERRUPT();
	OsErrorProcess1(ChainTask,tskid,TaskID);
	return ercd;
}

/* |------------------+-------------------------------------------------------------| */
/* | Syntax:          | StatusType Schedule ( void )                                | */
/* |------------------+-------------------------------------------------------------| */
/* | Parameter (In):  | none                                                        | */
/* |------------------+-------------------------------------------------------------| */
/* | Parameter (Out): | none                                                        | */
/* |------------------+-------------------------------------------------------------| */
/* | Description:     | If a higher-priority task is ready, the internal resource   | */
/* |                  | of the task is released, the current task is put into the   | */
/* |                  | ready state, its context is saved and the higher-priority   | */
/* |                  | task is executed. Otherwise the calling task is continued.  | */
/* |------------------+-------------------------------------------------------------| */
/* | Particularities: | Rescheduling only takes place if the task an internal       | */
/* |                  | resource is assigned to the calling task during             | */
/* |                  | system generation. For these tasks, Schedule enables a      | */
/* |                  | processor assignment to other tasks with lower or equal     | */
/* |                  | priority than the ceiling priority of the internal resource | */
/* |                  | and higher priority than the priority of the calling task   | */
/* |                  | in application-specific locations. When returning from      | */
/* |                  | Schedule, the internal resource has been taken again.       | */
/* |                  | This service has no influence on tasks with no internal     | */
/* |                  | resource assigned (preemptable tasks).                      | */
/* |------------------+-------------------------------------------------------------| */
/* | Status:          | Standard:                                                   | */
/* |                  | 1. No error, E_OK                                           | */
/* |                  | Extended:                                                   | */
/* |                  | 1. Call at interrupt level, E_OS_CALLEVEL                   | */
/* |                  | 2. Calling task occupies resources, E_OS_RESOURCE           | */
/* |------------------+-------------------------------------------------------------| */
/* | Conformance:     | BCC1, BCC2, ECC1, ECC2                                      | */
/* |------------------+-------------------------------------------------------------| */
StatusType Schedule ( void )
{
    StatusType ercd = E_OK;
    OS_EXT_VALIDATE((0 == knl_taskindp),E_OS_CALLEVEL);
    OS_EXT_VALIDATE((INVALID_RESOURCE == knl_tcb_resque[knl_curtsk]),E_OS_RESOURCE);

	BEGIN_CRITICAL_SECTION();
	//if task has internal resource or task is non-premtable
	if((knl_rdyque.top_pri > knl_tcb_ipriority[knl_curtsk]) &&
		(NUM_PRI != knl_rdyque.top_pri))
	{	//release internal resource or for Non-Preemtable Task
    	ReleaseInternalResource();
        knl_reschedule();
    }
	END_CRITICAL_SECTION();

	//re-get internal resource or for Non-Preemtable task
	GetInternalResource();

OS_VALIDATE_ERROR_EXIT()
	OsErrorProcess0(Schedule);
    return ercd;
}

/* |------------------+---------------------------------------------------------| */
/* | Syntax:          | StatusType GetTaskID ( TaskRefType <TaskID> )           | */
/* |------------------+---------------------------------------------------------| */
/* | Parameter (In):  | none                                                    | */
/* |------------------+---------------------------------------------------------| */
/* | Parameter (Out): | TaskID Reference to the task which is currently running | */
/* |------------------+---------------------------------------------------------| */
/* | Description:     | GetTaskID returns the information about the TaskID of   | */
/* |                  | the task which is currently running.                    | */
/* |------------------+---------------------------------------------------------| */
/* | Particularities: | 1. Allowed on task level, ISR level and in several hook | */
/* |                  | routines (see Figure 12-1(os223)).                      | */
/* |                  | 2. This service is intended to be used by library       | */
/* |                  | functions and hook routines.                            | */
/* |                  | 3. If <TaskID> can't be evaluated (no task currently    | */
/* |                  | running), the service returns INVALID_TASK as TaskType. | */
/* |------------------+---------------------------------------------------------| */
/* | Status:          | Standard:  No error, E_OK                               | */
/* |                  | Extended:  No error, E_OK                               | */
/* |------------------+---------------------------------------------------------| */
/* | Conformance:     | BCC1, BCC2, ECC1, ECC2                                  | */
/* |------------------+---------------------------------------------------------| */
StatusType GetTaskID ( TaskRefType TaskID )
{
	*TaskID = knl_curtsk;
	return E_OK;
}

/* |------------------+-------------------------------------------------------| */
/* | Syntax:          | StatusType GetTaskState ( TaskType <TaskID>,          | */
/* |                  | TaskStateRefType <State> )                            | */
/* |------------------+-------------------------------------------------------| */
/* | Parameter (In):  | TaskID: Task reference                                | */
/* |------------------+-------------------------------------------------------| */
/* | Parameter (Out): | State: Reference to the state of the task <TaskID>    | */
/* |------------------+-------------------------------------------------------| */
/* | Description:     | Returns the state of a task (running, ready, waiting, | */
/* |                  | suspended) at the time of calling GetTaskState.       | */
/* |------------------+-------------------------------------------------------| */
/* | Particularities: | The service may be called from interrupt service      | */
/* |                  | routines, task level, and some hook routines (see     | */
/* |                  | Figure 12-1(os223.doc)). When a call is made from a   | */
/* |                  | task in a full preemptive system, the result may      | */
/* |                  | already be incorrect at the time of evaluation.       | */
/* |                  | When the service is called for a task, which is       | */
/* |                  | activated more than once, the state is set to         | */
/* |                  | running if any instance of the task is running.       | */
/* |------------------+-------------------------------------------------------| */
/* | Status:          | Standard: No error, E_OK                              | */
/* |                  | Extended: Task <TaskID> is invalid, E_OS_ID           | */
/* |------------------+-------------------------------------------------------| */
/* | Conformance:     | BCC1, BCC2, ECC1, ECC2                                | */
/* |------------------+-------------------------------------------------------| */
StatusType GetTaskState ( TaskType TaskID,TaskStateRefType State )
{
	StatusType ercd = E_OK;
	OS_EXT_VALIDATE((TaskID<cfgOS_TASK_NUM),E_OS_ID);
	if(READY== knl_tcb_state[TaskID])
	{
	    if(knl_curtsk == TaskID)
	    {
	        *State = RUNNING;
	    }
	    else
	    {
	        *State = READY;
	    }
	}
	else
	{
	    *State = knl_tcb_state[TaskID];
	}
OS_VALIDATE_ERROR_EXIT()
	OsErrorProcess2(GetTaskState,tskid,TaskID,p_state,State);
	return ercd;
}
//OS-impl internal function
EXPORT void knl_task_init(void)
{
	uint8 i;

	knl_schedtsk = knl_curtsk = INVALID_TASK;
	/* init ready queue */
	knl_rdyque.top_pri = NUM_PRI;
#if( (cfgOS_MULTIPLY_ACTIVATION == 1) || (cfgOS_MULTIPLY_PRIORITY == 1) )
	for ( i = 0; i < NUM_PRI; i++ ) {
		knl_rdyque.tskque[i].head = knl_rdyque.tskque[i].tail = 0; // Empty
	}
#else
	(void)memset(knl_rdyque.tskque,INVALID_TASK,sizeof(knl_rdyque.tskque)+sizeof(TaskType));
#endif
	(void)memset(knl_rdyque.bitmap, 0, sizeof(knl_rdyque.bitmap));

	knl_dispatch_disabled = 1; /* disable dispatch */

	for(i=0; i<cfgOS_TASK_NUM; i++)
	{
		knl_tcb_state[i] = SUSPENDED;
#if(cfgOS_MULTIPLY_ACTIVATION == 1)
		knl_tcb_activation[i] = 0;
#endif
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
	#if(cfgOS_FLAG_NUM > 0)
	{
		uint8 flgid;
		flgid = knl_tcb_flgid[taskid];
		if(flgid != INVALID_FLAG)
		{
			knl_fcb_wait[flgid]=NO_EVENT;
			knl_fcb_set[flgid]=NO_EVENT;
		}
	}
	#endif
#if(cfgOS_S_RES_NUM > 0)
	knl_tcb_resque[taskid] = INVALID_RESOURCE;
#endif
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
#if((cfgOS_MULTIPLY_ACTIVATION == 1) || (cfgOS_MULTIPLY_PRIORITY == 1))
	TaskReadyQueueType *tskque;

	tskque = &(knl_rdyque.tskque[top_pri]);
	if(tskque->head != tskque->tail)
	{  // not empty.
		knl_schedtsk = tskque->queue[tskque->head];
		devAction(tlOs,tskque->queue[tskque->head]=INVALID_TASK);
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
#else
	knl_schedtsk = knl_rdyque.tskque[top_pri];
	// knl_rdyque.tskque[top_pri] = INVALID_TASK;
	knl_bitmap_clear(top_pri);
	knl_rdyque.top_pri = knl_bitmap_search(top_pri);
#endif
}
//no matter what,will put current ready task to the ready queue
//and the high ready task <toptsk> will be dispatched.
EXPORT void knl_reschedule( void )
{
	TaskType toptsk;

	toptsk = knl_ready_queue_top();
	if ( knl_schedtsk != toptsk ){
		toptsk = knl_schedtsk;
		knl_search_schedtsk();	// will delete the toptsk, and make knl_schedtsk = toptsk.
	    knl_ready_queue_insert_top(toptsk);
	}
}

// just poll the top priority task in the ready queue
EXPORT TaskType knl_ready_queue_top(void)
{
	PriorityType top_pri = knl_rdyque.top_pri;
#if((cfgOS_MULTIPLY_ACTIVATION == 1) || (cfgOS_MULTIPLY_PRIORITY == 1))
	TaskReadyQueueType *tskque;

	tskque = &(knl_rdyque.tskque[top_pri]);
	if(tskque->head != tskque->tail)
	{  // not empty.
		return tskque->queue[tskque->head];
	}
	else
	{
		return INVALID_TASK;
	}
#else
	return knl_rdyque.tskque[top_pri];
#endif
}
EXPORT void knl_ready_queue_insert_top(TaskType taskid)
{
	PriorityType priority = knl_tcb_curpri[taskid];
#if((cfgOS_MULTIPLY_ACTIVATION == 1) || (cfgOS_MULTIPLY_PRIORITY == 1))
	TaskReadyQueueType *tskRdyQue = &knl_rdyque.tskque[priority];
	if(0 == tskRdyQue->head)
	{
		tskRdyQue->head = tskRdyQue->length - 1;
	}
	else
	{
		tskRdyQue->head --;
	}
	devAssert(tskRdyQue->tail!=tskRdyQue->head,"Error As Task Ready Queue Full when Push to Head.\n");
	tskRdyQue->queue[tskRdyQue->head] = taskid;
#else
	knl_rdyque.tskque[priority] = taskid;
#endif
	if((priority > knl_rdyque.top_pri) || (NUM_PRI == knl_rdyque.top_pri))
	{
		knl_rdyque.top_pri = priority;
	}
	knl_bitmap_set(priority);
}

EXPORT void knl_ready_queue_insert(TaskType taskid)
{
	PriorityType priority = knl_tcb_ipriority[taskid];
#if((cfgOS_MULTIPLY_ACTIVATION == 1) || (cfgOS_MULTIPLY_PRIORITY == 1))
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
	devAssert(tskRdyQue->tail!=tskRdyQue->head,"Error As Task Ready Queue Full when Push to Tail.\n");
#elif(cfgOS_MULTIPLY_PRIORITY == 0)
	knl_rdyque.tskque[priority] = taskid;
#endif
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
