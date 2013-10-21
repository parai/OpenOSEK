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
#include <windows.h>
#include "osek_os.h"


/* ================================ MACROs    =============================== */

/* ================================ TYPEs     =============================== */

/* ================================ DATAs     =============================== */
EXPORT volatile long portDispatchInIsrRequested = FALSE;
LOCAL HANDLE knl_tcb_sp[cfgOS_TASK_NUM] = {NULL};
LOCAL HANDLE knl_tcb_old_sp[cfgOS_TASK_NUM] = {NULL};
LOCAL HANDLE portInterruptEventMutex = NULL;
LOCAL HANDLE portInterruptEvent = NULL;
LOCAL HANDLE portMainThread = NULL;
LOCAL HANDLE portIsrTickThread = NULL;
LOCAL volatile int portInterruptsEnabled = FALSE;
LOCAL volatile unsigned long portPendingInterrupts = 0L;
LOCAL int portProcessSimulatedInterruptsCalled = FALSE;
/* Handlers for all the simulated software interrupts.  The first two positions
are used for the Yield and Tick interrupts so are handled slightly differently,
all the other interrupts can be user defined. */
LOCAL FP portIsrHandler[ portMAX_INTERRUPTS ] = { 0 };

/* ================================ FUNCTIONs =============================== */
LOCAL void portStartDispatcher(void);
LOCAL DWORD WINAPI portSimulatedPeripheralTimer( LPVOID lpParameter );
LOCAL void portProcessSimulatedInterrupts( void );
LOCAL void l_dispatch0(void);
LOCAL void portWaitForStart(void);
LOCAL void knl_system_timer(void);

EXPORT imask_t knl_disable_int( void )
{
	imask_t mask = portInterruptsEnabled;
	portInterruptsEnabled = FALSE;
	return mask;
}
EXPORT void knl_enable_int( imask_t mask )
{
	portInterruptsEnabled = mask;
}
EXPORT void knl_force_dispatch(void)
{
	static int isDispatcherStarted = FALSE;
	TaskType curtsk = knl_curtsk;
	if(FALSE == isDispatcherStarted)
	{
		portStartDispatcher();
		isDispatcherStarted = TRUE;
		if(knl_schedtsk == INVALID_TASK)
		{
			knl_dispatch_disabled=0;    /* Dispatch enable */
			knl_curtsk = INVALID_TASK;
			goto first_start_without_task_ready;
		}
	}
	knl_dispatch_disabled=1;    /* Dispatch disable */

	knl_curtsk = INVALID_TASK;
	l_dispatch0();

	if(curtsk != INVALID_TASK)
	{
		if( READY == knl_tcb_state[curtsk])
		{
			if(-1 == TerminateThread(knl_tcb_old_sp[curtsk],0))
			{
				printf("Terminate Task %d failed.\n",curtsk);
			}
			knl_tcb_old_sp[curtsk] = NULL;
		}
		else if(SUSPENDED== knl_tcb_state[curtsk])
		{
			if(-1 == TerminateThread(knl_tcb_sp[curtsk],0))
			{
				printf("Terminate Task %d failed.\n",curtsk);
			}
			knl_tcb_sp[curtsk] = NULL;
		}
	}
first_start_without_task_ready:
	if(FALSE == portProcessSimulatedInterruptsCalled)
	{
		portProcessSimulatedInterruptsCalled = TRUE;
		portInterruptsEnabled = TRUE; // Enable Interrupt.
		portProcessSimulatedInterrupts();
	}
}

EXPORT void knl_setup_context(TaskType taskid)
{
	//FP pc = knl_tcb_pc[taskid];
	FP pc = portWaitForStart;
	knl_tcb_old_sp[taskid] = knl_tcb_sp[taskid];
	knl_tcb_sp[taskid]=CreateThread( NULL, 0, ( LPTHREAD_START_ROUTINE ) pc, NULL, CREATE_SUSPENDED, NULL );
	SetThreadAffinityMask( knl_tcb_sp[taskid], 0x01 );
	SetThreadPriorityBoost( knl_tcb_sp[taskid], TRUE );
	SetThreadPriority( knl_tcb_sp[taskid], THREAD_PRIORITY_IDLE );
}
LOCAL void l_dispatch0(void)
{
	portInterruptsEnabled = TRUE; //enable interrupt
	while(INVALID_TASK == knl_schedtsk)
	{
		Sleep(0); // release CPU give the right to other thread
	}
	knl_curtsk = knl_schedtsk;
	knl_dispatch_disabled=0;    /* Dispatch enable */

	/* resume task */
	if(-1 == ResumeThread(knl_tcb_sp[knl_curtsk]))
	{
		printf("Resume Task <%d> failed.\n",knl_curtsk);
	}
}

EXPORT void knl_dispatch_entry(void)
{
	TaskType curtsk;
	knl_dispatch_disabled=1;    /* Dispatch disable */
	curtsk = knl_curtsk;
	knl_curtsk = INVALID_TASK;

	l_dispatch0();
	if((curtsk != INVALID_TASK)&&(-1 == SuspendThread( knl_tcb_sp[curtsk])))
	{
		printf("Suspend Task %d failed.\n",curtsk);
	}
}

LOCAL void knl_system_timer(void)
{
	EnterISR();
#if(cfgOS_ALARM_NUM > 0)
	(void)SignalCounter(SystemTimer);
#endif
	LeaveISR();
}
LOCAL void portStartDispatcher(void)
{
	int lSuccess = TRUE;
	// install interrupt handler
	portIsrHandler[portINTERRUPT_TICK] = knl_system_timer;
	/* Create the events and mutexes that are used to synchronise all the
	threads. */
	portInterruptEventMutex = CreateMutex( NULL, FALSE, NULL );
	portInterruptEvent = CreateEvent( NULL, FALSE, FALSE, NULL );

	if( ( portInterruptEventMutex == NULL ) || ( portInterruptEvent == NULL ) )
	{
		lSuccess = FALSE;
	}

	/* Set the priority of this thread such that it is above the priority of
	the threads that run tasks.  This higher priority is required to ensure
	simulated interrupts take priority over tasks. */
	portMainThread = GetCurrentThread();
	if( portMainThread == NULL )
	{
		lSuccess = FALSE;
	}

	if( lSuccess == TRUE )
	{
		if( SetThreadPriority( portMainThread, THREAD_PRIORITY_NORMAL ) == 0 )
		{
			lSuccess = FALSE;
		}
		SetThreadPriorityBoost( portMainThread, TRUE );
		SetThreadAffinityMask( portMainThread, 0x01 );
	}

	if( lSuccess == TRUE )
	{
		/* Start the thread that simulates the timer peripheral to generate
		tick interrupts.  The priority is set below that of the simulated
		interrupt handler so the interrupt event mutex is used for the
		handshake / overrun protection. */
		portIsrTickThread = CreateThread( NULL, 0, portSimulatedPeripheralTimer, NULL, 0, NULL );
		if( portIsrTickThread != NULL )
		{
			SetThreadPriority( portIsrTickThread, THREAD_PRIORITY_BELOW_NORMAL );
			SetThreadPriorityBoost( portIsrTickThread, TRUE );
			SetThreadAffinityMask( portIsrTickThread, 0x01 );
		}
	}
	else
	{
		printf("Serious Error!\n");
		for(;;);
	}
}

LOCAL DWORD WINAPI portSimulatedPeripheralTimer( LPVOID lpParameter )
{

	/* Just to prevent compiler warnings. */
	( void ) lpParameter;

	for(;;)
	{
		/* Wait until the timer expires and we can access the simulated interrupt
		variables.  *NOTE* this is not a 'real time' way of generating tick
		events as the next wake time should be relative to the previous wake
		time, not the time that Sleep() is called.  It is done this way to
		prevent overruns in this very non real time simulated/emulated
		environment. */

		Sleep( 1 ); //sleep 1ms

		portGenerateSimulatedInterrupt(portINTERRUPT_TICK);
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
	pvObjectList[ 0 ] = portInterruptEventMutex;
	pvObjectList[ 1 ] = portInterruptEvent;

	for(;;)
	{
		if(TRUE == portInterruptsEnabled)
		{
			WaitForMultipleObjects( sizeof( pvObjectList ) / sizeof( void * ), pvObjectList, TRUE, INFINITE );

			/* For each interrupt we are interested in processing, each of which is
			represented by a bit in the 32bit ulPendingInterrupts variable. */
			for( i = 0; i < portMAX_INTERRUPTS; i++ )
			{
				/* Is the simulated interrupt pending? */
				if( portPendingInterrupts & ( 1UL << i ) )
				{
					/* Is a handler installed? */
					if( portIsrHandler[ i ] != NULL )
					{
						/* Run the actual handler. */
						portIsrHandler[ i ]();
					}
					/* Clear the interrupt pending bit. */
					portPendingInterrupts &= ~( 1UL << i );
				}
			}
			ReleaseMutex( portInterruptEventMutex );
			if((portDispatchInIsrRequested == TRUE) &&(knl_dispatch_disabled == 0))
			{
				// printf("ISR dispatch!\n");
				portDispatchInIsrRequested = FALSE;
				knl_dispatch_entry();
			}
		}
	}
}

/*-----------------------------------------------------------*/

EXPORT void portGenerateSimulatedInterrupt( unsigned long ulInterruptNumber )
{

	if( ( ulInterruptNumber < portMAX_INTERRUPTS ) && ( portInterruptEventMutex != NULL ) )
	{
		/* Yield interrupts are processed even when critical nesting is non-zero. */
		WaitForSingleObject( portInterruptEventMutex, INFINITE );
		portPendingInterrupts |= ( 1 << ulInterruptNumber );

		/* The simulated interrupt is now held pending, but don't actually process it
		yet if this call is within a critical section.  It is possible for this to
		be in a critical section as calls to wait for mutexes are accumulative. */

		SetEvent( portInterruptEvent );

		ReleaseMutex( portInterruptEventMutex );
	}
}
LOCAL void portWaitForStart(void)
{
	GetInternalResource();
	knl_tcb_pc[knl_curtsk]();
}
