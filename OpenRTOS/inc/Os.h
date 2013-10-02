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
#ifndef _OS_H_
#define _OS_H_

/* ================================ INCLUDEs  =============================== */
#include "Std_Types.h"
#include "oscfg.h"

/* ================================ MACROs    =============================== */
/* Conformance Class */

/* only basic tasks, limited to one activation request per task and one task per
 * priority, while all tasks have different priorities */
#define BCC1 0
/* like BCC1, plus more than one task per priority possible and multiple requesting
 * of task activation allowed */
#define BCC2 1
/* like BCC1, plus extended tasks */
#define ECC1 2
/* like ECC1, plus more than one task per priority possible and multiple requesting
 * of task activation allowed for basic tasks */
#define ECC2 3

/* Values for TaskStateType */
#define SUSPENDED (0x00u)
#define RUNNING   (0x01u)
#define READY     (0x02u)
#define WAITING   (0x03u)

/* OS Std-Errors */
#define E_OS_ACCESS     (1u)
#define E_OS_CALLEVEL   (2u)
#define E_OS_ID         (3u)
#define E_OS_LIMIT      (4u)
#define E_OS_NOFUNC     (5u)
#define E_OS_RESOURCE   (6u)
#define E_OS_STATE      (7u)
#define E_OS_VALUE      (8u)

/*  Constant of data type ResourceType (see (osek)chapter 8, Resource management).*/
#define RES_SCHEDULER 0

#define INVALID_APPMODE  0
#define OSDEFAULTAPPMODE 1 

#define TASK(TaskName) void TaskMain##TaskName(void)
#define ALARM(AlarmName) void AlarmMain##AlarmName(void) 
#define ALARMCALLBACK(cbk) void AlarmMain##cbk(void)
#define ISR(isrname) void ISRMain##isrname(void)

#define DeclareTask(TaskName)
#define DeclareAlarm(AlarmName)
#define DeclareResource(ResourceName)
#define DeclareEvent(EventName)  

//----------------------- OS Error Process -----------
/*
 *  OS Error Process Macors
 */
#define OSServiceId_ActivateTask    0
#define OSServiceId_TerminateTask   1
#define OSServiceId_ChainTask       2
#define OSServiceId_Schedule        3
#define OSServiceId_GetTaskID       4
#define OSServiceId_GetTaskState    5
#define OSServiceId_GetAlarmBase    6
#define OSServiceId_GetAlarm        7
#define OSServiceId_SetRelAlarm     8
#define OSServiceId_SetAbsAlarm     9
#define OSServiceId_CancelAlarm     10
#define OSServiceId_SetEvent        11
#define OSServiceId_ClearEvent      12
#define OSServiceId_GetEvent        13
#define OSServiceId_WaitEvent       14
#define OSServiceId_GetResource     15
#define OSServiceId_ReleaseResource 16
#define OSServiceId_StartOS         17
#define OSServiceId_ShutdownOS      18
#define OSErrorGetServiceId()				(_errorhook_svcid)

#define OSError_ActivateTask_TaskID()		(_errorhook_par1._tskid)
#define OSError_ChainTask_TaskID()			(_errorhook_par1._tskid)
#define OSError_GetTaskID_TaskID()			(_errorhook_par1._p_tskid)
#define OSError_GetTaskState_TaskID()		(_errorhook_par1._tskid)
#define OSError_GetTaskState_State()		(_errorhook_par2._p_state)
#define OSError_GetResource_ResID()			(_errorhook_par1._resid)
#define OSError_ReleaseResource_ResID()		(_errorhook_par1._resid)
#define OSError_SetEvent_TaskID()			(_errorhook_par1._tskid)
#define OSError_SetEvent_Mask()				(_errorhook_par2._mask)
#define OSError_ClearEvent_Mask()			(_errorhook_par1._mask)
#define OSError_GetEvent_TaskID()			(_errorhook_par1._tskid)
#define OSError_GetEvent_Mask()				(_errorhook_par2._p_mask)
#define OSError_WaitEvent_Mask()			(_errorhook_par1._mask)
#define OSError_GetAlarmBase_AlarmID()		(_errorhook_par1._almid)
#define OSError_GetAlarmBase_Info()			(_errorhook_par2._p_info)
#define OSError_GetAlarm_AlarmID()			(_errorhook_par1._palmid)
#define OSError_GetAlarm_Tick()				(_errorhook_par2._p_tick)
#define OSError_SetRelAlarm_AlarmID()		(_errorhook_par1._almid)
#define OSError_SetRelAlarm_increment()		(_errorhook_par2._incr)
#define OSError_SetRelAlarm_cycle()			(_errorhook_par3._cycle)
#define OSError_SetAbsAlarm_AlarmID()		(_errorhook_par1._almid)
#define OSError_SetAbsAlarm_start()			(_errorhook_par2._start)
#define OSError_SetAbsAlarm_cycle()			(_errorhook_par3._cycle)
#define OSError_CancelAlarm_AlarmID()		(_errorhook_par1._almid)
#define OSError_SignalCounter_CounterID()	(_errorhook_par1._cntid)

/* ================================ TYPEs     =============================== */
/* This data type identifies a task. */
typedef uint8 TaskType;
/* This data type points to a variable of TaskType. */
typedef TaskType* TaskRefType;
/* This data type identifies the state of a task. */
typedef uint8   TaskStateType;
/* This data type points to a variable of the data type TaskStateType. */
typedef TaskStateType * TaskStateRefType;
/* This data type represents count values in ticks. */
typedef uint32 TickType;
/* This data type points to the data type TickType. */
typedef TickType* TickRefType;
/* This data type represents a structure for storage of counter characteristics. 
 * The individual elements of the structure are: */
typedef struct
{
    /* Maximum possible allowed count value in ticks */
    TickType maxallowedvalue;
    /*  Number of ticks required to reach a counter-specific (significant) unit. */
    TickType ticksperbase;
    /* Smallest allowed value for the cycle-parameter of */
    /* SetRelAlarm/SetAbsAlarm) (only for systems with extended status). */
    TickType mincycle;
}AlarmBaseType;
/* This data type points to the data type AlarmBaseType. */
typedef AlarmBaseType * AlarmBaseRefType;
/* This data type represents an alarm object. */
typedef uint8 AlarmType;
/* This data type represents an Counter object. */
typedef uint8 CounterType;
/* Data type of the event mask. */
typedef uint32 EventMaskType;
/* Reference to an event mask. */
typedef EventMaskType* EventMaskRefType;
typedef uint8 ResourceType;
typedef uint8 StatusType;
typedef uint32 AppModeType;     /* ecah bit is mapped to a different application mode */
/* This data type represents the identification of system services. */
typedef uint8 OSServiceIdType;

//-------------------------------- TYPEs for ERROR   -------------------------
//this is a copy and paste from nxtOSEK
typedef union {
		TaskType			_tskid;
		TaskRefType			_p_tskid;
		TaskStateRefType	_p_state;
		ResourceType		_resid;
		EventMaskType		_mask;
		EventMaskRefType	_p_mask;
		AlarmType			_almid;
		AlarmBaseRefType	_p_info;
		TickRefType			_p_tick;
		TickType			_incr;
		TickType			_cycle;
		TickType			_start;
		AppModeType			_mode;
		CounterType			_cntid;
	} _ErrorHook_Par;

extern OSServiceIdType	_errorhook_svcid;
extern _ErrorHook_Par	_errorhook_par1, _errorhook_par2, _errorhook_par3;

/* ================================ DATAs     =============================== */

/* ================================ FUNCTIONs =============================== */
IMPORT StatusType GetCounterValue(CounterType CounterID,TickRefType Value);
IMPORT StatusType GetElapsedCounterValue(CounterType CounterID,
                                  TickRefType Value,TickRefType ElapsedValue);
IMPORT StatusType IncrementCounter(CounterType CounterID);

IMPORT StatusType GetAlarmBase( AlarmType AlarmID, AlarmBaseRefType pInfo );
IMPORT StatusType GetAlarm ( AlarmType AlarmID ,TickRefType Tick );
IMPORT StatusType SetRelAlarm ( AlarmType AlarmID , TickType Increment ,TickType Cycle );
IMPORT StatusType SetAbsAlarm ( AlarmType AlarmID , TickType Start ,TickType Cycle );
IMPORT StatusType CancelAlarm ( AlarmType AlarmID ); 
IMPORT StatusType SignalCounter(CounterType CounterID);

IMPORT StatusType ActivateTask ( TaskType TaskID );
IMPORT StatusType TerminateTask( void );
IMPORT StatusType ChainTask    ( TaskType TaskID );
IMPORT StatusType SleepTask    ( TickType Timeout );
IMPORT StatusType WakeUpTask   ( TaskType TaskID );
IMPORT StatusType Schedule     ( void );
IMPORT StatusType GetTaskID    ( TaskRefType TaskID );
IMPORT StatusType GetTaskState ( TaskType TaskID,TaskStateRefType State );

#if !defined(SIMULATE_ON_WIN)
IMPORT StatusType SetEvent  ( TaskType TaskID , EventMaskType Mask );
#else  // as it conflict with WINAPI SetEvent();
IMPORT StatusType osekSetEvent  ( TaskType TaskID , EventMaskType Mask );
#endif
IMPORT StatusType ClearEvent( EventMaskType Mask );
IMPORT StatusType GetEvent  ( TaskType TaskID , EventMaskRefType Event );
IMPORT StatusType WaitEvent ( EventMaskType Mask );


IMPORT AppModeType GetActiveApplicationMode(void);

IMPORT void EnterISR(void);
IMPORT void LeaveISR(void);

IMPORT void DisableAllInterrupts( void );
IMPORT void EnableAllInterrupts ( void );
IMPORT void SuspendAllInterrupts( void );
IMPORT void ResumeAllInterrupts ( void );
IMPORT void SuspendOSInterrupts( void );
IMPORT void ResumeOSInterrupts ( void );

IMPORT StatusType GetResource (ResourceType ResID);
IMPORT StatusType ReleaseResource ( ResourceType ResID );

IMPORT void StartOS ( AppModeType Mode );
IMPORT void ShutdownOS( StatusType Error );

IMPORT void ShutdownHook ( StatusType Error);
IMPORT void StartupHook(void);
IMPORT void ErrorHook(StatusType Error);
IMPORT void PreTaskHook(void);
IMPORT void PostTaskHook(void);


#endif /* _OS_H_ */
