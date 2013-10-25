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
#if(cfgOS_FLAG_NUM > 0)
/* ================================ MACROs    =============================== */

/* ================================ TYPEs     =============================== */

/* ================================ DATAs     =============================== */
EXPORT EventMaskType knl_fcb_set[cfgOS_FLAG_NUM];
EXPORT EventMaskType knl_fcb_wait[cfgOS_FLAG_NUM];
/* ================================ FUNCTIONs =============================== */
/* |------------------+----------------------------------------------------------| */
/* | Syntax:          | StatusType SetEvent ( TaskType <TaskID>                  | */
/* |                  | EventMaskType <Mask> )                                   | */
/* |------------------+----------------------------------------------------------| */
/* | Parameter (In):  | TaskID:Reference to the task for which one or several    | */
/* |                  | events are to be set.                                    | */
/* |                  | Mask:Mask of the events to be set                        | */
/* |------------------+----------------------------------------------------------| */
/* | Parameter (Out): | none                                                     | */
/* |------------------+----------------------------------------------------------| */
/* | Description:     | 1.The service may be called from an interrupt service    | */
/* |                  | routine and from the task level, but not from hook       | */
/* |                  | routines.                                                | */
/* |                  | 2.The events of task <TaskID> are set according to the   | */
/* |                  | event mask <Mask>. Calling SetEvent causes the task      | */
/* |                  | <TaskID> to be transferred to the ready state, if it     | */
/* |                  | was waiting for at least one of the events specified     | */
/* |                  | in <Mask>.                                               | */
/* |------------------+----------------------------------------------------------| */
/* | Particularities: | Any events not set in the event mask remain unchanged.   | */
/* |------------------+----------------------------------------------------------| */
/* | Status:          | Standard: 1.No error, E_OK                               | */
/* |                  | Extended: 2.Task <TaskID> is invalid, E_OS_ID            | */
/* |                  | 3.Referenced task is no extended task, E_OS_ACCESS       | */
/* |                  | 4.Events can not be set as the referenced task is in the | */
/* |                  | suspended state, E_OS_STATE                              | */
/* |------------------+----------------------------------------------------------| */
/* | Conformance:     | ECC1, ECC2                                               | */
/* |------------------+----------------------------------------------------------| */
#if !defined(SIMULATE_ON_WIN)
EXPORT StatusType SetEvent  ( TaskType TaskID , EventMaskType Mask )
#else  // as it conflict with WINAPI SetEvent();
EXPORT StatusType osekSetEvent  ( TaskType TaskID , EventMaskType Mask )
#endif
{
	StatusType ercd = E_OK;
	uint8 flgid ;
	OS_EXT_VALIDATE((TaskID < cfgOS_TASK_NUM),E_OS_ID);
	flgid = knl_tcb_flgid[TaskID];
	OS_EXT_VALIDATE((flgid != INVALID_FLAG),E_OS_ACCESS);
	OS_EXT_VALIDATE((SUSPENDED != knl_tcb_state[TaskID]),E_OS_STATE);

	BEGIN_CRITICAL_SECTION();
	knl_fcb_set[flgid] |= Mask;
	if((knl_fcb_set[flgid] & knl_fcb_wait[flgid]) != NO_EVENT)
	{
		knl_fcb_wait[flgid] = NO_EVENT;
		knl_tcb_state[TaskID] = READY;
		knl_make_runnable(TaskID);
	}
	END_CRITICAL_SECTION();
OS_VALIDATE_ERROR_EXIT()
	OsErrorProcess2(SetEvent,tskid,TaskID,mask,Mask);
	return ercd;
}

/* |------------------+---------------------------------------------------------| */
/* | Syntax:          | StatusType ClearEvent ( EventMaskType <Mask> )          | */
/* |------------------+---------------------------------------------------------| */
/* | Parameter (In)   | Mask:Mask of the events to be cleared                   | */
/* |------------------+---------------------------------------------------------| */
/* | Parameter (Out)  | none                                                    | */
/* |------------------+---------------------------------------------------------| */
/* | Description:     | The events of the extended task calling ClearEvent are  | */
/* |                  | cleared according to the event mask <Mask>.             | */
/* |------------------+---------------------------------------------------------| */
/* | Particularities: | The system service ClearEvent is restricted to extended | */
/* |                  | tasks which own the event.                              | */
/* |------------------+---------------------------------------------------------| */
/* | Status:          | Standard: 1.No error, E_OK                              | */
/* |                  | Extended: 1.Call not from extended task, E_OS_ACCESS    | */
/* |                  | 2.Call at interrupt level, E_OS_CALLEVEL                | */
/* |------------------+---------------------------------------------------------| */
/* | Conformance:     | ECC1, ECC2                                              | */
/* |------------------+---------------------------------------------------------| */
EXPORT StatusType ClearEvent( EventMaskType Mask )
{
	StatusType ercd = E_OK;
	uint8 flgid;
	OS_EXT_VALIDATE((0 == knl_taskindp),E_OS_CALLEVEL);
	flgid = knl_tcb_flgid[knl_curtsk];
	OS_EXT_VALIDATE((flgid != INVALID_FLAG),E_OS_ACCESS);

	BEGIN_DISABLE_INTERRUPT();
	knl_fcb_set[flgid] &= ~Mask;
	END_DISABLE_INTERRUPT();
OS_VALIDATE_ERROR_EXIT()
	OsErrorProcess1(ClearEvent,mask,Mask);
	return ercd;
}

/* |------------------+--------------------------------------------------------------| */
/* | Syntax:          | StatusType GetEvent ( TaskType <TaskID>                      | */
/* |                  | EventMaskRefType <Event> )                                   | */
/* |------------------+--------------------------------------------------------------| */
/* | Parameter (In):  | TaskID:Task whose event mask is to be returned.              | */
/* |------------------+--------------------------------------------------------------| */
/* | Parameter (Out): | Event:Reference to the memory of the return data.            | */
/* |------------------+--------------------------------------------------------------| */
/* | Description:     | 1.This service returns the current state of all event bits   | */
/* |                  | of the task <TaskID>, not the events that the task is        | */
/* |                  | waiting for.                                                 | */
/* |                  | 2.The service may be called from interrupt service routines, | */
/* |                  | task level and some hook routines (see Figure 12-1).         | */
/* |                  | 3.The current status of the event mask of task <TaskID> is   | */
/* |                  | copied to <Event>.                                           | */
/* |------------------+--------------------------------------------------------------| */
/* | Particularities: | The referenced task shall be an extended task.               | */
/* |------------------+--------------------------------------------------------------| */
/* | Status:          | Standard: No error, E_OK                                     | */
/* |                  | Extended: Task <TaskID> is invalid, E_OS_ID                  | */
/* |                  | Referenced task <TaskID> is not an extended task,            | */
/* |                  | E_OS_ACCESS                                                  | */
/* |                  | Referenced task <TaskID> is in the suspended state,          | */
/* |                  | E_OS_STATE                                                   | */
/* |------------------+--------------------------------------------------------------| */
/* | Conformance:     | ECC1, ECC2                                                   | */
/* |------------------+--------------------------------------------------------------| */
EXPORT StatusType GetEvent  ( TaskType TaskID , EventMaskRefType Event )
{
	StatusType ercd = E_OK;
	uint8 flgid;
	OS_EXT_VALIDATE((TaskID < cfgOS_TASK_NUM),E_OS_ID);
	flgid = knl_tcb_flgid[TaskID];
	OS_EXT_VALIDATE((flgid != INVALID_FLAG),E_OS_ACCESS);
	OS_EXT_VALIDATE((SUSPENDED != knl_tcb_state[TaskID]),E_OS_STATE);

	BEGIN_DISABLE_INTERRUPT();
	*Event = knl_fcb_set[flgid];
	END_DISABLE_INTERRUPT();
OS_VALIDATE_ERROR_EXIT()
	OsErrorProcess2(GetEvent,tskid,TaskID,p_mask,Event);
	return ercd;
}

/* |------------------+------------------------------------------------------------| */
/* | Syntax:          | StatusType WaitEvent ( EventMaskType <Mask> )              | */
/* |------------------+------------------------------------------------------------| */
/* | Parameter (In):  | Mask:Mask of the events waited for.                        | */
/* |------------------+------------------------------------------------------------| */
/* | Parameter (Out): | none                                                       | */
/* |------------------+------------------------------------------------------------| */
/* | Description:     | The state of the calling task is set to waiting, unless    | */
/* |                  | at least one of the events specified in <Mask> has         | */
/* |                  | already been set.                                          | */
/* |------------------+------------------------------------------------------------| */
/* | Particularities: | 1.This call enforces rescheduling, if the wait condition   | */
/* |                  | occurs. If rescheduling takes place, the internal resource | */
/* |                  | of the task is released while the task is in the waiting   | */
/* |                  | state.                                                     | */
/* |                  | 2.This service shall only be called from the extended task | */
/* |                  | owning the event.                                          | */
/* |------------------+------------------------------------------------------------| */
/* | Status:          | Standard:No error, E_OK                                    | */
/* |                  | Extended:Calling task is not an extended task, E_OS_ACCESS | */
/* |                  | Calling task occupies resources, E_OS_RESOURCE             | */
/* |                  | Call at interrupt level, E_OS_CALLEVEL                     | */
/* |------------------+------------------------------------------------------------| */
/* | Conformance:     | ECC1, ECC2                                                 | */
/* |------------------+------------------------------------------------------------| */
EXPORT StatusType WaitEvent ( EventMaskType Mask )
{
	StatusType ercd = E_OK;
    uint8 flgid;
    OS_EXT_VALIDATE((0 == knl_taskindp),E_OS_CALLEVEL);
    OS_EXT_VALIDATE((INVALID_RESOURCE == knl_tcb_resque[knl_curtsk]),E_OS_RESOURCE);
    flgid = knl_tcb_flgid[knl_curtsk];
    OS_EXT_VALIDATE((flgid != INVALID_FLAG),E_OS_ACCESS);

    BEGIN_CRITICAL_SECTION();
    if((knl_fcb_set[flgid] & Mask) == NO_EVENT)
    {
    	knl_fcb_wait[flgid] = Mask;
        knl_tcb_state[knl_curtsk] = WAITING;
        //release internal resource or for Non-Preemtable Task
        ReleaseInternalResource();
        knl_search_schedtsk();
    }
    END_CRITICAL_SECTION();
    //re-get internal resource or for Non-Preemtable task
    GetInternalResource();
OS_VALIDATE_ERROR_EXIT()
    OsErrorProcess1(WaitEvent,mask,Mask);
	return ercd;
}
#endif /* cfgOS_FLAG_NUM */
