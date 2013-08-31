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
#include <pthread.h>
#include <sched.h>
#include <signal.h>
#include <errno.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include <limits.h>

/* ================================ MACROs    =============================== */
/* Posix Signal definitions that can be changed or read as appropriate. */
#define SIG_SUSPEND					SIGUSR1  //  = 10
#define SIG_RESUME					SIGUSR2  //  = 12
/* Enable the following hash defines to make use of the real-time tick where time progresses at real-time. */
#define SIG_TICK					SIGALRM  //  = 14
#define TIMER_TYPE					ITIMER_REAL  // = 0

/* ================================ TYPEs     =============================== */

/* ================================ DATAs     =============================== */
LOCAL pthread_t knl_tcb_sp[cfgOS_TASK_NUM];
LOCAL pthread_t portMainThread = NULL;
// for porting
LOCAL pthread_once_t portSigSetupThread = PTHREAD_ONCE_INIT;
/* used for thread start policy */
LOCAL pthread_mutex_t portSingleThreadMutex = PTHREAD_MUTEX_INITIALIZER;
LOCAL pthread_mutex_t portSuspendResumeThreadMutex = PTHREAD_MUTEX_INITIALIZER;
LOCAL pthread_attr_t portThreadAttr;
LOCAL volatile int portSentinel = 0;
LOCAL volatile int portServicingTick = FALSE;
LOCAL volatile int portInterruptsEnabled = FALSE;

/* ================================ FUNCTIONs =============================== */
LOCAL void* portWaitForStart(void* taskid);
LOCAL void portSetupSignalsAndSchedulerPolicy(void);
LOCAL void portSuspendThread( pthread_t thread );
LOCAL void portResumeThread( pthread_t thread );
LOCAL void portSuspendSignalHandler(int sig);
LOCAL void portResumeSignalHandler(int sig);
LOCAL void portSystemTickHandler( int sig );
LOCAL void portSetupTimerInterrupt( void );
LOCAL void l_dispatch0(void);

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
	(void)pthread_once( &portSigSetupThread, portSetupSignalsAndSchedulerPolicy );

	if ( (pthread_t)NULL == portMainThread ){
		portMainThread = pthread_self();
	}

	knl_dispatch_disabled=1;    /* Dispatch disable */
	knl_curtsk = INVALID_TASK;
	l_dispatch0();
}

EXPORT void knl_setup_context(TaskType taskid)
{
	(void)pthread_once( &portSigSetupThread, portSetupSignalsAndSchedulerPolicy );
	if ( 0 == pthread_mutex_lock( &portSingleThreadMutex ) )
	{
		portSentinel = 0;
		(void)pthread_create(&knl_tcb_sp[taskid],&portThreadAttr,
				portWaitForStart,(void*)(int*)taskid);
		(void)pthread_mutex_unlock( &portSingleThreadMutex );
		while(0 == portSentinel);
	}
}
LOCAL void l_dispatch0(void)
{
	portInterruptsEnabled = TRUE; //enable interrupt
	while(INVALID_TASK == knl_schedtsk)
	{
		;; //wait here
	}
	knl_curtsk = knl_schedtsk;
	knl_dispatch_disabled=0;    /* Dispatch enable */

	/* resume task */
	portResumeThread(knl_tcb_sp[knl_curtsk]);
}
EXPORT void knl_dispatch_entry(void)
{
	knl_dispatch_disabled=1;    /* Dispatch disable */
	portSuspendThread(knl_tcb_sp[knl_curtsk]);
	knl_curtsk = INVALID_TASK;
	l_dispatch0();
}

LOCAL void* portWaitForStart(void* taskid)
{
	printf("in portWaitForStart(%d).\n",(int)(TaskType)(TaskRefType)taskid);

	if ( 0 == pthread_mutex_lock( &portSingleThreadMutex ) )
	{
		portSuspendThread( pthread_self() );
	}

	knl_tcb_pc[(TaskType)(TaskRefType)taskid]();
	return NULL;
}

LOCAL void portSetupSignalsAndSchedulerPolicy(void)
{
	struct sigaction sigsuspendself, sigresume, sigtick;

	pthread_mutex_init(&portSingleThreadMutex,NULL);
	pthread_mutex_init(&portSuspendResumeThreadMutex,NULL);
	/* No need to join the threads. */
	pthread_attr_init( &portThreadAttr );
	pthread_attr_setdetachstate( &portThreadAttr, PTHREAD_CREATE_DETACHED );

	/* The following code would allow for configuring the scheduling of this task as a Real-time task.
	 * The process would then need to be run with higher privileges for it to take affect.
	int iPolicy;
	int iResult;
	int iSchedulerPriority;
	iResult = pthread_getschedparam( pthread_self(), &iPolicy, &iSchedulerPriority );
	iResult = pthread_attr_setschedpolicy( &xThreadAttributes, SCHED_FIFO );
	iPolicy = SCHED_FIFO;
	iResult = pthread_setschedparam( pthread_self(), iPolicy, &iSchedulerPriority );		*/



	printf("in SetupSignalsAndSchedulerPolicy()\n");

	sigsuspendself.sa_flags = 0;
	sigsuspendself.sa_handler = portSuspendSignalHandler;
	sigfillset( &sigsuspendself.sa_mask );

	sigresume.sa_flags = 0;
	sigresume.sa_handler = portResumeSignalHandler;
	sigfillset( &sigresume.sa_mask );

	sigtick.sa_flags = 0;
	sigtick.sa_handler = portSystemTickHandler;
	sigfillset( &sigtick.sa_mask );

	if ( 0 != sigaction( SIG_SUSPEND, &sigsuspendself, NULL ) )
	{
		printf( "Problem installing SIG_SUSPEND_SELF\n" );
	}
	if ( 0 != sigaction( SIG_RESUME, &sigresume, NULL ) )
	{
		printf( "Problem installing SIG_RESUME\n" );
	}
	if ( 0 != sigaction( SIG_TICK, &sigtick, NULL ) )
	{
		printf( "Problem installing SIG_TICK\n" );
	}

	portSetupTimerInterrupt();

	printf( "Running as PID: %d\n", getpid() );

}

LOCAL void portSuspendThread( pthread_t thread )
{
	int xResult = pthread_mutex_lock( &portSuspendResumeThreadMutex );
	if ( 0 == xResult )
	{
		/* Set-up for the Suspend Signal handler? */
		portSentinel = 0;
		xResult = pthread_mutex_unlock( &portSuspendResumeThreadMutex );
		xResult = pthread_kill( thread, SIG_SUSPEND );
		while ( ( 0 == portSentinel ) && ( TRUE != portServicingTick ) )
		{
			sched_yield();
		}
	}
}
LOCAL void portResumeThread( pthread_t thread )
{
	if ( 0 == pthread_mutex_lock( &portSuspendResumeThreadMutex ) )
	{

		if ( pthread_self() != thread )
		{
			pthread_kill( thread, SIG_RESUME );
		}

		pthread_mutex_unlock( &portSuspendResumeThreadMutex );
	}
}

LOCAL void portSuspendSignalHandler(int sig)
{
	sigset_t xSignals;

	/* Only interested in the resume signal. */
	sigemptyset( &xSignals );
	sigaddset( &xSignals, SIG_RESUME );
	portSentinel = 1;

	/* Unlock the Single thread mutex to allow the resumed task to continue. */
	if ( 0 != pthread_mutex_unlock( &portSingleThreadMutex ) )
	{
		printf( "Releasing someone else's lock.\n" );
	}

	/* Wait on the resume signal. */
	if ( 0 != sigwait( &xSignals, &sig ) )
	{
		printf( "SSH: Sw %d\n", sig );
	}
}

LOCAL void portResumeSignalHandler(int sig)
{
	/* Yield the Scheduler to ensure that the yielding thread completes. */
	if ( 0 == pthread_mutex_lock( &portSingleThreadMutex ) )
	{
		(void)pthread_mutex_unlock( &portSingleThreadMutex );
	}
}

LOCAL void portSystemTickHandler( int sig )
{
	//printf("int portSystemTickHandler(0x%x).\n",sig);
	if ( ( TRUE == portInterruptsEnabled ) && ( TRUE != portServicingTick ) )
	{
		if ( 0 == pthread_mutex_trylock( &portSingleThreadMutex ) ){
			portServicingTick = TRUE;

			if(FALSE){
				//if ok,do dispatch here
			}else{
				/* Release the lock as we are Resuming. */
				(void)pthread_mutex_unlock( &portSingleThreadMutex );
			}
			portServicingTick = FALSE;
		}
	}
}


/*
 * Setup the systick timer to generate the tick interrupts at the required
 * frequency.
 */
LOCAL void portSetupTimerInterrupt( void )
{
	struct itimerval itimer, oitimer;
	int xMicroSeconds = 1000000 / 100; //100 ticks per second

	/* Initialise the structure with the current timer information. */
	if ( 0 == getitimer( TIMER_TYPE, &itimer ) )
	{
		/* Set the interval between timer events. */
		itimer.it_interval.tv_sec = 0;
		itimer.it_interval.tv_usec = xMicroSeconds;
		/* Set the current count-down. */
		itimer.it_value.tv_sec = 0;
		itimer.it_value.tv_usec = xMicroSeconds;

		/* Set-up the timer interrupt. */
		if ( 0 != setitimer( TIMER_TYPE, &itimer, &oitimer ) )
		{
			printf( "Set Timer problem.\n" );
		}
	}
	else
	{
		printf( "Get Timer problem.\n" );
	}
}
