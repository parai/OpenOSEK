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
#ifndef _OSEK_OS_H_
#define _OSEK_OS_H_
/* ================================ INCLUDEs  =============================== */
#include "Os.h"
#include "portable.h"

/* ================================ MACROs    =============================== */
#if((cfgOS_SCHEDULE == osNonPreemptive) && (cfgOS_FLAG_NUM == 0))
#define ReleaseInternalResource()
#define GetInternalResource()
#else
#define ReleaseInternalResource() { knl_tcb_curpri[knl_curtsk] = knl_tcb_ipriority[knl_curtsk]; }
#define GetInternalResource() { knl_tcb_curpri[knl_curtsk] = knl_tcb_rpriority[knl_curtsk]; }
#endif

#define INVALID_TASK      ((TaskType)0xFF)
#define INVALID_ALARM     ((AlarmType)0xFF)
#define INVALID_FLAG      ((uint8)0xFF)
#define INVALID_RESOURCE  ((ResourceType)0xFF)
#define INVALID_PRIORITY  ((PriorityType)0xFF)

#define NO_EVENT 0

#define ALARM_STOPPED   ((TickType)0xFFFFFFFFUL)
#define EXTENDED 0
#define STANDARD 1

#define TASK_PC(name) TaskMain##name

#define BITMAPSZ	( sizeof(uint8) * 8 )
#define NUM_BITMAP	( ((cfgOS_MAX_PRIORITY+1) + BITMAPSZ - 1) / BITMAPSZ )
#define NUM_PRI     (cfgOS_MAX_PRIORITY+1)

#define OS_VALIDATE(_true,_ercd)		\
	do{									\
		if(!(_true))					\
		{								\
			ercd = _ercd;				\
			goto Error_Exit;			\
		}								\
	}while(FALSE)

#define OS_VALIDATE_ERROR_EXIT() Error_Exit:

#define OS_STD_VALIDATE(_true,_ercd) OS_VALIDATE(_true,_ercd)
#if(cfgOS_STATUS == EXTENDED)
#define OS_EXT_VALIDATE(_true,_ercd) OS_VALIDATE(_true,_ercd)
#else
#define OS_EXT_VALIDATE(_true,_ercd)
#endif

#define knl_make_active(_taskid)	\
{	\
	knl_make_ready(_taskid);	\
	knl_make_runnable(_taskid);	\
}

#define knl_preempt()	\
{		\
	knl_ready_queue_insert_top(knl_schedtsk);	\
	knl_search_schedtsk();	\
}

// OSEK-OS Error Process
#if(cfgOS_ERRORHOOK == 1)
// For Service without Parameters
#define OsErrorProcess0(ServiceName)	\
do{												\
	if(E_OK != ercd)	\
	{	\
		BEGIN_CRITICAL_SECTION();	\
		_errorhook_svcid = OSServiceId_##ServiceName;	\
		knl_call_errorhook(ercd);	\
		END_CRITICAL_SECTION();	\
	}	\
}while(0)
// For Service with 1 Parameter
#define OsErrorProcess1(ServiceName,ParamName1,param1)	\
do{												\
	if(E_OK != ercd)	\
	{	\
    	BEGIN_CRITICAL_SECTION();	\
    	_errorhook_svcid = OSServiceId_##ServiceName;	\
    	_errorhook_par1._##ParamName1 = (param1);	\
    	knl_call_errorhook(ercd);	\
    	END_CRITICAL_SECTION();	\
	}	\
}while(0)

// For Service with 2 Parameters
#define OsErrorProcess2(ServiceName,ParamName1,param1,ParamName2,param2)	\
do{												\
	if(E_OK != ercd)	\
	{	\
    	BEGIN_CRITICAL_SECTION();	\
    	_errorhook_svcid = OSServiceId_##ServiceName;	\
    	_errorhook_par1._##ParamName1 = (param1);	\
    	_errorhook_par2._##ParamName2 = (param2);	\
    	knl_call_errorhook(ercd);	\
    	END_CRITICAL_SECTION();	\
	}	\
}while(0)

// For Service with 3 Parameters
#define OsErrorProcess3(ServiceName,ParamName1,param1,ParamName2,param2,ParamName3,param3)	\
do{												\
	if(E_OK != ercd)	\
	{	\
    	BEGIN_CRITICAL_SECTION();	\
    	_errorhook_svcid = OSServiceId_##ServiceName;	\
    	_errorhook_par1._##ParamName1 = (param1);	\
    	_errorhook_par2._##ParamName2 = (param2);	\
    	_errorhook_par2._##ParamName3 = (param3);	\
    	knl_call_errorhook(ercd);	\
    	END_CRITICAL_SECTION();	\
	}	\
}while(0)
#else
#define OsErrorProcess0(ServiceName)
#define OsErrorProcess1(ServiceName,ParamName,param1)
#define OsErrorProcess2(ServiceName,ParamName1,param1,ParamName2,param2)
#define OsErrorProcess3(ServiceName,ParamName1,param1,ParamName2,param2,ParamName3,param3)
#endif
/* ================================ TYPEs     =============================== */
/* Priority type of Task */
typedef uint8 PriorityType;
typedef uint16 StackSizeType;

#if((cfgOS_MULTIPLY_ACTIVATION == 1) || (cfgOS_MULTIPLY_PRIORITY == 1))
typedef struct
{
	uint8 head;
	uint8 tail;
	const uint8 length;
	const TaskRefType queue; /* size at least should be 2 */
}TaskReadyQueueType;   /* per priority, FIFO queue */

typedef struct
{
	PriorityType top_pri;
	TaskReadyQueueType tskque[NUM_PRI];
	const TaskReadyQueueType null;
	uint8	bitmap[NUM_BITMAP];	/* Bitmap area per priority */
}RDYQUE;
#else
typedef struct
{
	PriorityType top_pri;
	TaskType tskque[NUM_PRI];
	const TaskType null;
	uint8	bitmap[NUM_BITMAP];	/* Bitmap area per priority */
}RDYQUE;
#endif

/* ================================ DATAs     =============================== */
// task control block
IMPORT const FP            knl_tcb_pc[];
IMPORT const PriorityType  knl_tcb_ipriority[];
IMPORT const PriorityType  knl_tcb_rpriority[];
IMPORT const uint8         knl_tcb_max_activation[];
IMPORT const StackSizeType knl_tcb_stksz[];
IMPORT uint8* const        knl_tcb_stack[];
IMPORT const AppModeType   knl_tcb_mode[];
IMPORT const uint8         knl_tcb_flgid[];
IMPORT TaskStateType       knl_tcb_state[];
IMPORT PriorityType        knl_tcb_curpri[];
IMPORT uint8               knl_tcb_activation[];
IMPORT ResourceType        knl_tcb_resque[];

IMPORT AppModeType knl_appmode;
IMPORT uint8    knl_taskindp;   /* task in independent part nested level */
IMPORT uint8    knl_dispatch_disabled;
IMPORT volatile TaskType knl_curtsk;
IMPORT volatile TaskType knl_schedtsk;
IMPORT RDYQUE knl_rdyque;

// counter control block
IMPORT const TickType knl_ccb_max[];
IMPORT const TickType knl_ccb_tpb[];
IMPORT const TickType knl_ccb_min[];
IMPORT    AlarmType   knl_ccb_head[];
IMPORT    TickType    knl_ccb_value[];

// alarm control block
IMPORT const CounterType knl_acb_counter[];
IMPORT const CounterType knl_acb_mode[];
IMPORT const FP          knl_acb_action[];
IMPORT const TickType    knl_acb_time[];
IMPORT const TickType    knl_acb_cycle[];
IMPORT AlarmType         knl_acb_next[];
IMPORT AlarmType         knl_acb_prev[];
IMPORT    TickType       knl_acb_value[];
IMPORT    TickType       knl_acb_period[];

// flag control block
IMPORT EventMaskType knl_fcb_set[];
IMPORT EventMaskType knl_fcb_wait[];

// resources control block
IMPORT const PriorityType knl_rcb_priority[];
IMPORT ResourceType       knl_rcb_next[];
/* ================================ FUNCTIONs =============================== */
IMPORT void knl_call_errorhook(StatusType ercd);
/* ------------ tasks ------------- */
IMPORT void knl_task_init(void);
IMPORT void knl_make_ready(TaskType taskid);
IMPORT void knl_make_runnable(TaskType taskid);
IMPORT void knl_ready_queue_insert(TaskType taskid);
IMPORT void knl_ready_queue_insert_top(TaskType taskid);
IMPORT TaskType knl_ready_queue_top(void);
IMPORT void knl_bitmap_set(PriorityType priority);
IMPORT void knl_bitmap_clear(PriorityType priority);
IMPORT PriorityType knl_bitmap_search(PriorityType from);
IMPORT void knl_search_schedtsk(void);
IMPORT void knl_reschedule( void );

/* ------------ alarms ------------- */
IMPORT void knl_alarm_counter_init(void);
IMPORT TickType knl_add_ticks(TickType almval,TickType incr,TickType maxval2);
IMPORT TickType knl_diff_tick(TickType curval, TickType almval, TickType maxval2);
IMPORT void knl_alarm_insert(AlarmType alarm);
IMPORT void knl_alarm_remove(AlarmType alarm);

/* ------------ resources ------------- */
IMPORT void knl_resource_init(void);

#endif /* _OSEK_OS_H_ */
