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
	return ercd;
}
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
	return ercd;
}
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
	return ercd;
}
EXPORT StatusType WaitEvent ( EventMaskType Mask )
{
	StatusType ercd = E_OK;
    uint8 flgid;
    OS_EXT_VALIDATE((0 == knl_taskindp),E_OS_CALLEVEL);
    // OS_EXT_VALIDATE(,E_OS_RESOURCE);
    flgid = knl_tcb_flgid[knl_curtsk];
    OS_EXT_VALIDATE((flgid != INVALID_FLAG),E_OS_ACCESS);

    BEGIN_CRITICAL_SECTION();
    if((knl_fcb_set[flgid] & Mask) == NO_EVENT)
    {
    	knl_fcb_wait[flgid] = Mask;
        knl_tcb_state[knl_curtsk] = WAITING;
        //release internal resource or for Non-Preemtable Task
        ReleaseInternalResource();
        // TODO: serious problem may happen if knl_curtsk is multiple-activate-able.
        // So when knl_curtsk is in multiple-activate state, knl_search_schedtsk may
        // re-activate this task again. So the result is that the task may continue to run,
        // but with-out the decrease of the activation counter.
        // so may lead to system-panic. That is: when terminate task, may lead to a knl_make_ready()
        // without any affect to the system schedule.
        knl_search_schedtsk();
        // In-fact, here should assert(knl_curtsk != knl_schedtsk);
        // to solve this problem
        if(knl_curtsk == knl_schedtsk)
        { // knl_curtsk encounters multiple-activate request.
        	if(knl_tcb_activation[knl_curtsk] > 0)
        	{
        		knl_tcb_activation[knl_curtsk]--;
        	}
        }
    }
    END_CRITICAL_SECTION();
    //re-get internal resource or for Non-Preemtable task
    GetInternalResource();
OS_VALIDATE_ERROR_EXIT()
	return ercd;
}
#endif /* cfgOS_FLAG_NUM */
