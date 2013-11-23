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
typedef struct
{
	HANDLE thread[cfgOS_TASK_NUM];  // the context of the Task
	HANDLE thread2[cfgOS_TASK_NUM]; // the backup context of the Task
	HANDLE irqEvent;	// notify the portable the generation of IRQn
	volatile imask_t imask;
	HANDLE criticalMutex;  // ensure system critical protect
	HANDLE oneTaskMutex;   // ensure only one Task is running.
	volatile unsigned long pendingIRQn; // Each bit means a ISR pending
	volatile unsigned long dispatchInIRQnRequested;
	FP IRQnHandle[32];
	unsigned long isRunning;
}portOsRTE_Type;
/* ================================ DATAs     =============================== */
LOCAL portOsRTE_Type portOsRte;
EXPORT volatile unsigned long portDispatchInISRReq = False;
/* ================================ FUNCTIONs =============================== */
LOCAL DWORD WINAPI portSimulatedPeripheralTimer( LPVOID lpParameter );
LOCAL void portProcessSimulatedInterrupts( void );
LOCAL void l_dispatch0(void);
LOCAL void portStartTaskEntry(DWORD taskid);
LOCAL void knl_system_timer(void);
LOCAL void knl_force_dispatch_impl(void);
LOCAL void knl_dispatch_entry(void);

EXPORT void portEnterCriticalSection(void)
{
	WaitForSingleObject( portOsRte.criticalMutex, INFINITE );
}
EXPORT void portExitCriticalSection(void)
{
	ReleaseMutex(portOsRte.criticalMutex);
}

EXPORT HANDLE portCreSecondaryThread(HANDLE thread_entry,PVOID thread_param)
{
	/* Start the thread that simulates the IRQn peripheral to generate
	interrupts.  The priority is set below that of the simulated
	interrupt handler so the interrupt event mutex is used for the
	handshake / overrun protection. */
	HANDLE handle = CreateThread( NULL, 0, thread_entry, thread_param, 0, NULL );
	if( handle != NULL )
	{
		SetThreadPriority( handle, THREAD_PRIORITY_BELOW_NORMAL );
		SetThreadPriorityBoost( handle, TRUE );
		SetThreadAffinityMask( handle, 0x01 );
	}
	else
	{
		assert(False);
	}
	return handle;
}

EXPORT void knl_install_isr(IRQn_Type isrNbr,FP isrFp)
{
	if((isrNbr<eIRQnNumber) && (isrNbr < 32))
	{
		portOsRte.IRQnHandle[isrNbr] = isrFp;
	}
	else
	{
		devAssert(False,"IRQn install error.\n");
	}
}

EXPORT void portOsStartupHook(void)
{
	memset(&portOsRte,0u,sizeof(portOsRte));
	// ============= Install IRQn handle
	knl_install_isr(eIRQnForceDispatch,knl_force_dispatch_impl);
	knl_install_isr(eIRQnDispatch,knl_dispatch_entry);
	knl_install_isr(eIRQnSystemTick,knl_system_timer);
	// ============= Create Event and Mutex
	portOsRte.oneTaskMutex = CreateMutex( NULL, FALSE, NULL );
	portOsRte.criticalMutex = CreateMutex( NULL, FALSE, NULL );
	portOsRte.irqEvent = CreateEvent( NULL, FALSE, FALSE, NULL );
	if( (NULL == portOsRte.criticalMutex) || (NULL == portOsRte.irqEvent))
	{
		assert(False);
	}
	// ============= Main Thread and Secondary Thread Process
	/* Set the priority of this thread such that it is above the priority of
	the threads that run tasks.  This higher priority is required to ensure
	simulated interrupts take priority over tasks. */
	if( GetCurrentThread() != NULL )
	{
		if( False == SetThreadPriority( GetCurrentThread(), THREAD_PRIORITY_NORMAL ) )
		{
			assert(False);
		}
		SetThreadPriorityBoost( GetCurrentThread(), TRUE );
		SetThreadAffinityMask( GetCurrentThread(), 0x01 );
	}
	portCreSecondaryThread(portSimulatedPeripheralTimer,NULL);
}
EXPORT imask_t knl_disable_int( void )
{
	imask_t mask = portOsRte.imask;
	portOsRte.imask = False;
	return mask;
}
EXPORT void knl_enable_int( imask_t mask )
{
	portOsRte.imask = mask;
}
EXPORT void knl_force_dispatch(void)
{
	assert( 0u == (portOsRte.pendingIRQn&(1u<<eIRQnForceDispatch)) );
	assert(NULL != portOsRte.thread[knl_schedtsk]);
	printf("{");
	portOsRte.imask = True;  // Enable ISR firstly and then dispatch
	portGenerateSimulatedInterrupt(eIRQnForceDispatch);
	Sleep(10);

	if(False == portOsRte.isRunning)
	{
		portOsRte.isRunning = True;
		knl_dispatch_disabled = 0; /* Dispatch enable */
		portProcessSimulatedInterrupts();
	}
	else
	{
		for(;;)
		{   // retry if Error
			portGenerateSimulatedInterrupt(eIRQnForceDispatch);
			Sleep(10);
		}
	}
}
LOCAL void knl_force_dispatch_impl(void)
{
	printf("}");
	assert( 0u != (portOsRte.pendingIRQn&(1u<<eIRQnForceDispatch)) );
	knl_dispatch_disabled=1;    /* Dispatch disable */
	if(knl_curtsk != INVALID_TASK)
	{
		if( READY == knl_tcb_state[knl_curtsk])
		{
			if( (portOsRte.thread2[knl_curtsk] == NULL) ||
				(-1 == TerminateThread(portOsRte.thread2[knl_curtsk],0)))
			{
				assert(False);
			}
			portOsRte.thread2[knl_curtsk] = NULL;
		}
		else if(SUSPENDED== knl_tcb_state[knl_curtsk])
		{
			if( (portOsRte.thread[knl_curtsk] == NULL) ||
				(-1 == TerminateThread(portOsRte.thread[knl_curtsk],0)))
			{
				assert(False);
			}
			portOsRte.thread[knl_curtsk] = NULL;
			portOsRte.thread2[knl_curtsk] = NULL;
		}
	}

	knl_curtsk = INVALID_TASK;
	l_dispatch0();
}

EXPORT void knl_setup_context(TaskType taskid)
{
	LPTHREAD_START_ROUTINE pc = (LPTHREAD_START_ROUTINE)portStartTaskEntry;
	if(portOsRte.thread[taskid] != NULL)
	{
		portOsRte.thread2[taskid] = portOsRte.thread[taskid]; // back up as multiply activation
	}
	portOsRte.thread[taskid]=CreateThread( NULL, 0, ( LPTHREAD_START_ROUTINE ) pc, (LPVOID)taskid, CREATE_SUSPENDED, NULL );
	assert(portOsRte.thread[taskid] != NULL);
	SetThreadAffinityMask( portOsRte.thread[taskid], 0x01 );
	SetThreadPriorityBoost( portOsRte.thread[taskid], TRUE );
	SetThreadPriority( portOsRte.thread[taskid], THREAD_PRIORITY_IDLE );
}
LOCAL void l_dispatch0(void)
{
	DWORD ercd = 0;
	portOsRte.imask = True; //enable interrupt
	while(INVALID_TASK == knl_schedtsk)
	{ // Note, for Win implementation, an Idle task must be created.
		assert(False);
	}
	knl_curtsk = knl_schedtsk;
	knl_dispatch_disabled=0;    /* Dispatch enable */

	assert(INVALID_TASK != knl_curtsk);
	/* resume task */
	ercd = ResumeThread(portOsRte.thread[knl_curtsk]);
	if(-1 == ercd)
	{
		assert(False);
	}
}
EXPORT void knl_dispatch(void)									
{	
	printf("<");
	portGenerateSimulatedInterrupt(eIRQnDispatch);
	Sleep(1);											
}
LOCAL void knl_dispatch_entry(void)
{
	// assert( 0u != (portOsRte.pendingIRQn&(1u<<eIRQnDispatch)) );
	printf(">");
	knl_dispatch_disabled=1;    /* Dispatch disable */
	assert(portOsRte.thread[knl_curtsk] != NULL);
	assert(-1 != SuspendThread( portOsRte.thread[knl_curtsk]));
	knl_curtsk = INVALID_TASK;

	l_dispatch0();
}

LOCAL void knl_system_timer(void)
{
	EnterISR();
#if(cfgOS_ALARM_NUM > 0)
	(void)SignalCounter(SystemTimer);
#endif
	LeaveISR();
}

LOCAL DWORD WINAPI portSimulatedPeripheralTimer( LPVOID lpParameter )
{

	/* Just to prevent compiler warnings. */
	( void ) lpParameter;
	Sleep(100);
	for(;;)
	{
		/* Wait until the timer expires and we can access the simulated interrupt
		variables.  *NOTE* this is not a 'real time' way of generating tick
		events as the next wake time should be relative to the previous wake
		time, not the time that Sleep() is called.  It is done this way to
		prevent overruns in this very non real time simulated/emulated
		environment. */

		Sleep( 1 ); //sleep 1ms

		portGenerateSimulatedInterrupt(eIRQnSystemTick);
	}

	#ifdef __GNUC__
		/* Should never reach here - MingW complains if you leave this line out,
		MSVC complains if you put it in. */
		return 0;
	#endif
}

LOCAL void portProcessSimulatedInterrupts( void )
{
	int i;
	void *pvObjectList[ 2 ];

	/* Going to block on the mutex that ensured exclusive access to the simulated
	interrupt objects, and the event that signals that a simulated interrupt
	should be processed. */
	pvObjectList[ 0 ] = portOsRte.criticalMutex;
	pvObjectList[ 1 ] = portOsRte.irqEvent;

	for(;;)
	{
		if(True == portOsRte.imask)
		{
			WaitForMultipleObjects( sizeof( pvObjectList ) / sizeof( void * ), pvObjectList, True, INFINITE );

			/* For each interrupt we are interested in processing, each of which is
			represented by a bit in the 32bit ulPendingInterrupts variable. */
			for( i = 0; i < eIRQnNumber; i++ )
			{
				/* Is the simulated interrupt pending? */
				if( portOsRte.pendingIRQn & ( 1UL << i ) )
				{
					/* Is a handler installed? */
					if( portOsRte.IRQnHandle[ i ] != NULL )
					{
						/* Run the actual handler. */
						portOsRte.IRQnHandle[ i ]();
					}
					/* Clear the interrupt pending bit. */
					portOsRte.pendingIRQn &= ~( 1UL << i );
				}
			}
			if((portDispatchInISRReq)
				&& (knl_curtsk != knl_schedtsk) 
				&& (!knl_dispatch_disabled) 
				&&(!knl_taskindp) )
			{
				if(INVALID_TASK == knl_schedtsk)
				{
					assert(False);
				}
				printf("<");
				knl_dispatch_entry();
				portDispatchInISRReq = False;
			}
			ReleaseMutex( portOsRte.criticalMutex );
		}
	}
}

/*-----------------------------------------------------------*/

EXPORT void portGenerateSimulatedInterrupt( IRQn_Type ulInterruptNumber )
{

	if( ( ulInterruptNumber < eIRQnNumber ) && ( portOsRte.criticalMutex != NULL ) )
	{
		/* Yield interrupts are processed even when critical nesting is non-zero. */
		WaitForSingleObject( portOsRte.criticalMutex, INFINITE );
		portOsRte.pendingIRQn |= ( 1 << ulInterruptNumber );

		/* The simulated interrupt is now held pending, but don't actually process it
		yet if this call is within a critical section.  It is possible for this to
		be in a critical section as calls to wait for mutexes are accumulative. */

		SetEvent( portOsRte.irqEvent );

		ReleaseMutex( portOsRte.criticalMutex );
	}
}
LOCAL void portStartTaskEntry(DWORD taskid)
{
	assert(taskid == knl_curtsk);
	WaitForSingleObject( portOsRte.oneTaskMutex, INFINITE );
	ReleaseMutex( portOsRte.oneTaskMutex );
	GetInternalResource();
	knl_tcb_pc[knl_curtsk]();

	assert(False);
}
