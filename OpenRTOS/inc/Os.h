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

/* ================================ MACROs    =============================== */
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
/* OS Ext-Errors */
#define E_OS_ASSERT     (9u)    /* serious problem encountered when assert */

#define INVALID_TASK ((TaskType)-1)

/*  Constant of data type ResourceType (see (osek)chapter 8, Resource management).*/
#define RES_SCHEDULER 0

#define INVALID_APPMODE  0
#define OSDEFAULTAPPMODE 1 

#define TASK(TaskName) void TaskMain##TaskName(void)
#define ALARM(AlarmName) void AlarmMain##AlarmName(void) 
#define ALARMCALLBACK(cbk) void AlarmMain##cbk(void)    

#define DeclareTask(TaskName)
#define DeclareAlarm(AlarmName)
#define DeclareResource(ResourceName)
#define DeclareEvent(EventName)  

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

/* ================================ INCLUDEs  =============================== */
#include "oscfg.h"

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

IMPORT StatusType ActivateTask ( TaskType TaskID );
IMPORT StatusType TerminateTask( void );
IMPORT StatusType ChainTask    ( TaskType TaskID );
IMPORT StatusType SleepTask    ( TickType Timeout );
IMPORT StatusType WakeUpTask   ( TaskType TaskID );
IMPORT StatusType Schedule     ( void );
IMPORT StatusType GetTaskID    ( TaskRefType TaskID );
IMPORT StatusType GetTaskState ( TaskType TaskID,TaskStateRefType State );

IMPORT StatusType SetEvent  ( TaskType TaskID , EventMaskType Mask );
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
