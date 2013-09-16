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
	return ercd;
}

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
