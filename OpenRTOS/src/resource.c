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
EXPORT ResourceType        knl_tcb_resque[cfgOS_TASK_NUM];
EXPORT ResourceType        knl_rcb_next[cfgOS_S_RES_NUM];
LOCAL  PriorityType        knl_rcb_tskpri[cfgOS_S_RES_NUM];

/* ================================ FUNCTIONs =============================== */
/* |------------------+-----------------------------------------------------------| */
/* | Syntax:          | StatusType GetResource ( ResourceType <ResID> )           | */
/* |------------------+-----------------------------------------------------------| */
/* | Parameter (In):  | ResID:Reference to resource                               | */
/* |------------------+-----------------------------------------------------------| */
/* | Parameter (Out): | none                                                      | */
/* |------------------+-----------------------------------------------------------| */
/* | Description:     | This call serves to enter critical sections in the code   | */
/* |                  | that are assigned to the resource referenced by <ResID>.  | */
/* |                  | A critical section shall always be left using             | */
/* |                  | ReleaseResource.                                          | */
/* |------------------+-----------------------------------------------------------| */
/* | Particularities: | 1.The OSEK priority ceiling protocol for resource         | */
/* |                  | management is described in chapter 8.5.                   | */
/* |                  | 2.Nested resource occupation is only allowed if the       | */
/* |                  | inner critical sections are completely executed within    | */
/* |                  | the surrounding critical section (strictly stacked,       | */
/* |                  | see chapter 8.2, Restrictions when using resources).      | */
/* |                  | Nested occupation of one and the same resource is         | */
/* |                  | also forbidden!                                           | */
/* |                  | 3.It is recommended that corresponding calls to           | */
/* |                  | GetResource and ReleaseResource appear within the         | */
/* |                  | same function.                                            | */
/* |                  | 4.It is not allowed to use services which are points      | */
/* |                  | of rescheduling for non preemptable tasks (TerminateTask, | */
/* |                  | ChainTask, Schedule and WaitEvent, see chapter 4.6.2)     | */
/* |                  | in critical sections. Additionally, critical sections     | */
/* |                  | are to be left before completion of an interrupt service  | */
/* |                  | routine.                                                  | */
/* |                  | 5.Generally speaking, critical sections should be short.  | */
/* |                  | 6.The service may be called from an ISR and from task     | */
/* |                  | level (see Figure 12-1).                                  | */
/* |------------------+-----------------------------------------------------------| */
/* | Status:          | Standard:1.No error, E_OK                                 | */
/* |                  | Extended:1.Resource <ResID> is invalid, E_OS_ID           | */
/* |                  | 2.Attempt to get a resource which is already occupied     | */
/* |                  | by any task or ISR, or the statically assigned priority   | */
/* |                  | of the calling task or interrupt routine is higher than   | */
/* |                  | the calculated ceiling priority, E_OS_ACCESS              | */
/* |------------------+-----------------------------------------------------------| */
/* | Conformance:     | BCC1, BCC2, ECC1, ECC2                                    | */
/* |------------------+-----------------------------------------------------------| */
StatusType GetResource (ResourceType ResID)
{
	StatusType ercd = E_OK;
	OS_EXT_VALIDATE((0 == knl_taskindp),E_OS_CALLEVEL); //add as share resource with ISR was not supported
	OS_EXT_VALIDATE((ResID < cfgOS_S_RES_NUM),E_OS_ID);
	OS_EXT_VALIDATE((INVALID_PRIORITY == knl_rcb_tskpri[ResID]),E_OS_ACCESS);
    OS_EXT_VALIDATE((knl_rcb_priority[ResID] >= knl_tcb_ipriority[knl_curtsk]),E_OS_ACCESS);

    BEGIN_DISABLE_INTERRUPT();
    knl_rcb_tskpri[ResID] = knl_tcb_curpri[knl_curtsk];  // save task old priority
    if(knl_rcb_priority[ResID] > knl_tcb_ipriority[knl_curtsk])
    {
    	knl_tcb_curpri[knl_curtsk] = knl_rcb_priority[ResID];
    }
    // Insert it at task resource queue head
    knl_rcb_next[ResID] = knl_tcb_resque[knl_curtsk];
    knl_tcb_resque[knl_curtsk] = ResID;
    END_DISABLE_INTERRUPT();
OS_VALIDATE_ERROR_EXIT()
    OsErrorProcess1(GetResource,resid,ResID);
	return ercd;
}

/* |------------------+------------------------------------------------------------| */
/* | Syntax:          | StatusType ReleaseResource ( ResourceType <ResID> )        | */
/* |------------------+------------------------------------------------------------| */
/* | Parameter (In):  | ResID:Reference to resource                                | */
/* |------------------+------------------------------------------------------------| */
/* | Parameter (Out): | none                                                       | */
/* |------------------+------------------------------------------------------------| */
/* | Description:     | ReleaseResource is the counterpart of GetResource and      | */
/* |                  | serves to leave critical sections in the code that are     | */
/* |                  | assigned to the resource referenced by <ResID>.            | */
/* |------------------+------------------------------------------------------------| */
/* | Particularities: | For information on nesting conditions, see particularities | */
/* |                  | of GetResource.                                            | */
/* |                  | The service may be called from an ISR and from task level  | */
/* |                  | (see Figure 12-1).                                         | */
/* |------------------+------------------------------------------------------------| */
/* | Status:          | Standard: No error, E_OK                                   | */
/* |                  | Extended: Resource <ResID> is invalid, E_OS_ID             | */
/* |                  | Attempt to release a resource which is not occupied by     | */
/* |                  | any task or ISR, or another resource shall be released     | */
/* |                  | before, E_OS_NOFUNC                                        | */
/* |                  | Attempt to release a resource which has a lower ceiling    | */
/* |                  | priority than the statically assigned priority of the      | */
/* |                  | calling task or interrupt routine, E_OS_ACCESS             | */
/* |------------------+------------------------------------------------------------| */
/* | Conformance:     | BCC1, BCC2, ECC1, ECC2                                     | */
/* |------------------+------------------------------------------------------------| */
StatusType ReleaseResource ( ResourceType ResID )
{
	StatusType ercd = E_OK;
	OS_EXT_VALIDATE((0 == knl_taskindp),E_OS_CALLEVEL); //add as share resource with ISR was not supported
	OS_EXT_VALIDATE((ResID < cfgOS_S_RES_NUM),E_OS_ID);
	OS_EXT_VALIDATE(knl_tcb_resque[knl_curtsk] == ResID,E_OS_NOFUNC);
    OS_EXT_VALIDATE((knl_rcb_priority[ResID] >= knl_tcb_ipriority[knl_curtsk]),E_OS_ACCESS);

    BEGIN_CRITICAL_SECTION();
    knl_tcb_curpri[knl_curtsk] = knl_rcb_tskpri[ResID];
    // remove it at head
    knl_tcb_resque[knl_curtsk] = knl_rcb_next[ResID];
    knl_rcb_tskpri[ResID] = INVALID_PRIORITY;
    if(knl_tcb_curpri[knl_curtsk] > knl_rdyque.top_pri)
    {
        knl_preempt();
    }
    END_CRITICAL_SECTION();
OS_VALIDATE_ERROR_EXIT()
    OsErrorProcess1(ReleaseResource,resid,ResID);
	return ercd;
}

EXPORT void knl_resource_init(void)
{
	uint8 i;
	for(i=0;i<cfgOS_S_RES_NUM;i++)
	{
		knl_rcb_tskpri[i] = INVALID_PRIORITY;
	}
}
