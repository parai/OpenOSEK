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

/* ================================ FUNCTIONs =============================== */

LOCAL uint8          sus_all_cnt = 0;
LOCAL imask_t        sus_all_mask = 0;
LOCAL uint8          sus_os_cnt = 0;
LOCAL PriorityType   sus_os_ipl = 0;
/* |------------------+-------------------------------------------------------------------| */
/* | Syntax:          | void EnableAllInterrupts ( void )                                 | */
/* |------------------+-------------------------------------------------------------------| */
/* | Parameter (In):  | none                                                              | */
/* |------------------+-------------------------------------------------------------------| */
/* | Parameter (Out): | none                                                              | */
/* |------------------+-------------------------------------------------------------------| */
/* | Description:     | This service restores the state saved by DisableAllInterrupts.    | */
/* |------------------+-------------------------------------------------------------------| */
/* | Particularities: | The service may be called from an ISR category 1 and category     | */
/* |                  | 2 and from the task level, but not from hook routines.            | */
/* |                  | This service is a counterpart of DisableAllInterrupts service,    | */
/* |                  | which has to be called before, and its aim is the completion of   | */
/* |                  | the critical section of code. No API service calls are allowed    | */
/* |                  | within this critical section.                                     | */
/* |                  | The implementation should adapt this service to the target        | */
/* |                  | hardware providing a minimum overhead. Usually, this service      | */
/* |                  | enables recognition of interrupts by the central processing unit. | */
/* |------------------+-------------------------------------------------------------------| */
/* | Status:          | Standard:none                                                     | */
/* |                  | Extended:none                                                     | */
/* |------------------+-------------------------------------------------------------------| */
/* | Conformance:     | BCC1, BCC2, ECC1, ECC2                                            | */
/* |------------------+-------------------------------------------------------------------| */
void EnableAllInterrupts( void )
{
	ENABLE_INTERRUPT();
}
/* |------------------+--------------------------------------------------------------------| */
/* | Syntax:          | void DisableAllInterrupts ( void )                                 | */
/* |------------------+--------------------------------------------------------------------| */
/* | Parameter (In):  | none                                                               | */
/* |------------------+--------------------------------------------------------------------| */
/* | Parameter (Out): | none                                                               | */
/* |------------------+--------------------------------------------------------------------| */
/* | Description:     | This service disables all interrupts for which the hardware        | */
/* |                  | supports disabling. The state before is saved for the              | */
/* |                  | EnableAllInterrupts call.                                          | */
/* |------------------+--------------------------------------------------------------------| */
/* | Particularities: | The service may be called from an ISR category 1 and category      | */
/* |                  | 2 and from the task level, but not from hook routines.             | */
/* |                  | This service is intended to start a critical section of the code.  | */
/* |                  | This section shall be finished by calling the EnableAllInterrupts  | */
/* |                  | service. No API service calls are allowed within this critical     | */
/* |                  | section.                                                           | */
/* |                  | The implementation should adapt this service to the target         | */
/* |                  | hardware providing a minimum overhead. Usually, this service       | */
/* |                  | disables recognition of interrupts by the central processing unit. | */
/* |                  | Note that this service does not support nesting. If nesting is     | */
/* |                  | needed for critical sections e.g. for libraries                    | */
/* |                  | SuspendOSInterrupts/ResumeOSInterrupts or                          | */
/* |                  | SuspendAllInterrupt/ResumeAllInterrupts should be used.            | */
/* |------------------+--------------------------------------------------------------------| */
/* | Status:          | Standard:none                                                      | */
/* |                  | Extended:none                                                      | */
/* |------------------+--------------------------------------------------------------------| */
/* | Conformance:     | BCC1, BCC2, ECC1, ECC2                                             | */
/* |------------------+--------------------------------------------------------------------| */
void DisableAllInterrupts(void)
{
	DISABLE_INTERRUPT();
}

/* |------------------+------------------------------------------------------------------| */
/* | Syntax:          | void ResumeAllInterrupts ( void )                                | */
/* |------------------+------------------------------------------------------------------| */
/* | Parameter (In):  | none                                                             | */
/* |------------------+------------------------------------------------------------------| */
/* | Parameter (Out): | none                                                             | */
/* |------------------+------------------------------------------------------------------| */
/* | Description:     | This service restores the recognition status of all interrupts   | */
/* |                  | saved by the SuspendAllInterrupts service.                       | */
/* |------------------+------------------------------------------------------------------| */
/* | Particularities: | The service may be called from an ISR category 1 and category    | */
/* |                  | 2, from alarm-callbacks and from the task level, but not from    | */
/* |                  | all hook routines.                                               | */
/* |                  | This service is the counterpart of SuspendAllInterrupts service, | */
/* |                  | which has to have been called before, and its aim is the         | */
/* |                  | completion of the critical section of code. No API service       | */
/* |                  | calls beside SuspendAllInterrupts/ResumeAllInterrupts pairs      | */
/* |                  | and SuspendOSInterrupts/ResumeOSInterrupts pairs are allowed     | */
/* |                  | within this critical section.                                    | */
/* |                  | The implementation should adapt this service to the target       | */
/* |                  | hardware providing a minimum overhead.                           | */
/* |                  | SuspendAllInterrupts/ResumeAllInterrupts can be nested. In       | */
/* |                  | case of nesting pairs of the calls SuspendAllInterrupts and      | */
/* |                  | ResumeAllInterrupts the interrupt recognition status saved by    | */
/* |                  | the first call of SuspendAllInterrupts is restored by the last   | */
/* |                  | call of the ResumeAllInterrupts service.                         | */
/* |------------------+------------------------------------------------------------------| */
/* | Status:          | Standard:none                                                    | */
/* |                  | Extended:none                                                    | */
/* |------------------+------------------------------------------------------------------| */
/* | Conformance:     | BCC1, BCC2, ECC1, ECC2                                           | */
/* |------------------+------------------------------------------------------------------| */
void ResumeAllInterrupts ( void )
{
//	if (sus_all_cnt == 0) {
//		/*
//		 *  SuspendAllInterrupts hasn't been called before ResumeAllInterrupts
//		 *  It's an error, so just do nothig.
//		 */
//	}
//	else if (sus_all_cnt == 1) {
//		sus_all_cnt--;
//		knl_enable_int(sus_all_mask);
//	}
//	else {
//		sus_all_cnt--;
//	}
}

/* |------------------+------------------------------------------------------------------| */
/* | Syntax:          | void SuspendAllInterrupts ( void )                               | */
/* |------------------+------------------------------------------------------------------| */
/* | Parameter (In):  | none                                                             | */
/* |------------------+------------------------------------------------------------------| */
/* | Parameter (Out): | none                                                             | */
/* |------------------+------------------------------------------------------------------| */
/* | Description:     | This service saves the recognition status of all interrupts and  | */
/* |                  | disables all interrupts for which the hardware supports          | */
/* |                  | disabling.                                                       | */
/* |------------------+------------------------------------------------------------------| */
/* | Particularities: | 1.The service may be called from an ISR category 1               | */
/* |                  | and category 2, from alarm-callbacks and from the task level,    | */
/* |                  | but not from all hook routines.                                  | */
/* |                  | 2.This service is intended to protect a critical section of code | */
/* |                  | from interruptions of any kind.                                  | */
/* |                  | 3.This section shall be finished by calling the                  | */
/* |                  | ResumeAllInterrupts service. No API service calls beside         | */
/* |                  | SuspendAllInterrupts/ResumeAllInterrupts pairs and               | */
/* |                  | SuspendOSInterrupts/ResumeOSInterrupts pairs are allowed         | */
/* |                  | within this critical section.                                    | */
/* |                  | 4.The implementation should adapt this service to the target     | */
/* |                  | hardware providing a minimum overhead.                           | */
/* |------------------+------------------------------------------------------------------| */
/* | Status:          | Standard:none                                                    | */
/* |                  | Extended:none                                                    | */
/* |------------------+------------------------------------------------------------------| */
/* | Conformance:     | BCC1, BCC2, ECC1, ECC2                                           | */
/* |------------------+------------------------------------------------------------------| */
void SuspendAllInterrupts( void )
{
//	if (sus_all_cnt == 0xFFu) {
//		/*
//		 *  SuspendAllInterrupts has reached its max nest count
//		 *  So do nothing. May a ResumeAllInterrupts call has been forgot.
//		 */
//	}
//	else if (sus_all_cnt == 0) {
//		sus_all_mask = knl_disable_int();
//		sus_all_cnt++;
//	}
//	else {
//		sus_all_cnt++;
//	}
}

/* |------------------+------------------------------------------------------------------| */
/* | Syntax:          | void ResumeOSInterrupts ( void )                                 | */
/* |------------------+------------------------------------------------------------------| */
/* | Parameter (In):  | none                                                             | */
/* |------------------+------------------------------------------------------------------| */
/* | Parameter (Out): | none                                                             | */
/* |------------------+------------------------------------------------------------------| */
/* | Description:     |                                                                  | */
/* |                  | This service restores the recognition status of interrupts       | */
/* |                  | saved by the SuspendOSInterrupts service.                        | */
/* |------------------+------------------------------------------------------------------| */
/* | Particularities: | The service may be called from an ISR category 1 and category    | */
/* |                  | 2 and from the task level, but not from hook routines.           | */
/* |                  | This service is the counterpart of SuspendOSInterrupts service,  | */
/* |                  | which has to have been called before, and its aim is the         | */
/* |                  | completion of the critical section of code. No API service calls | */
/* |                  | beside SuspendAllInterrupts/ResumeAllInterrupts pairs and        | */
/* |                  | SuspendOSInterrupts/ResumeOSInterrupts pairs are allowed         | */
/* |                  | within this critical section.                                    | */
/* |                  | The implementation should adapt this service to the target       | */
/* |                  | hardware providing a minimum overhead.                           | */
/* |                  | SuspendOSInterrupts/ResumeOSInterrupts can be nested. In         | */
/* |                  | case of nesting pairs of the calls SuspendOSInterrupts and       | */
/* |                  | ResumeOSInterrupts the interrupt recognition status saved by     | */
/* |                  | the first call of SuspendOSInterrupts is restored by the last    | */
/* |                  | call of the ResumeOSInterrupts service.                          | */
/* |------------------+------------------------------------------------------------------| */
/* | Status:          | Standard:none                                                    | */
/* |                  | Extended:none                                                    | */
/* |------------------+------------------------------------------------------------------| */
/* | Conformance:     | BCC1, BCC2, ECC1, ECC2                                           | */
/* |------------------+------------------------------------------------------------------| */
void SuspendOSInterrupts( void )
{
//	if (sus_os_cnt == 0xFFu) {
//
//	}
//	else if (sus_os_cnt == 0) {
//		sus_os_ipl = knl_get_ipl();
//		knl_set_ipl(-1); //currently not supported that well
//		sus_os_cnt++;
//	}
//	else {
//		sus_os_cnt++;
//	}
}

/* |------------------+-------------------------------------------------------------------| */
/* | Syntax:          | void SuspendOSInterrupts ( void )                                 | */
/* |------------------+-------------------------------------------------------------------| */
/* | Parameter (In):  | none                                                              | */
/* |------------------+-------------------------------------------------------------------| */
/* | Parameter (Out): | none                                                              | */
/* |------------------+-------------------------------------------------------------------| */
/* | Description:     | This service saves the recognition status of interrupts of        | */
/* |                  | category 2 and disables the recognition of these interrupts.      | */
/* |------------------+-------------------------------------------------------------------| */
/* | Particularities: | The service may be called from an ISR and from the task level,    | */
/* |                  | but not from hook routines.                                       | */
/* |                  | This service is intended to protect a critical section of code.   | */
/* |                  | This section shall be finished by calling the ResumeOSInterrupts  | */
/* |                  | service.No API service calls beside                               | */
/* |                  | SuspendAllInterrupts/ResumeAllInterrupts pairs and                | */
/* |                  | SuspendOSInterrupts/ResumeOSInterrupts pairs are allowed          | */
/* |                  | within this critical section.                                     | */
/* |                  | The implementation should adapt this service to the target        | */
/* |                  | hardware providing a minimum overhead.                            | */
/* |                  | It is intended only to disable interrupts of category 2. However, | */
/* |                  | if this is not possible in an efficient way more interrupts may   | */
/* |                  | be disabled.                                                      | */
/* |------------------+-------------------------------------------------------------------| */
/* | Status:          | Standard:none                                                     | */
/* |                  | Extended:none                                                     | */
/* |------------------+-------------------------------------------------------------------| */
/* | Conformance:     | BCC1, BCC2, ECC1, ECC2                                            | */
/* |------------------+-------------------------------------------------------------------| */
void ResumeOSInterrupts( void )
{
//	if (sus_os_cnt == 0) {
//
//	}
//	else if (sus_os_cnt == 1) {
//		sus_os_cnt--;
//		knl_set_ipl(sus_os_ipl);
//	}
//	else {
//		sus_os_cnt--;
//	}
}
