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
#include <signal.h>
#include <errno.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include <limits.h>

/* ================================ MACROs    =============================== */

/* ================================ TYPEs     =============================== */

/* ================================ DATAs     =============================== */
LOCAL boolean knl_is_scheduler_started = FALSE;
LOCAL pthread_t knl_tcb_sp[cfgOS_TASK_NUM];
/* used for thread start policy */
LOCAL pthread_mutex_t knl_single_thread_mutex = PTHREAD_MUTEX_INITIALIZER;
LOCAL pthread_attr_t knl_thread_attr;

/* ================================ FUNCTIONs =============================== */
LOCAL void knl_wait_for_start(void* taskid);
LOCAL void knl_scheduler_start(void);

EXPORT void knl_force_dispatch(void)
{
	if(FALSE == knl_is_scheduler_started)
	{
		knl_scheduler_start();
		knl_is_scheduler_started = TRUE;
	}
}

EXPORT void knl_setup_context(TaskType taskid)
{
	if(FALSE == knl_is_scheduler_started)
	{
		knl_scheduler_start();
		knl_is_scheduler_started = TRUE;
	}
	if ( 0 == pthread_mutex_lock( &knl_single_thread_mutex ) )
	{
		(void)pthread_create(&knl_tcb_sp[taskid],&knl_thread_attr,
						knl_wait_for_start,(void*)taskid);
		(void)pthread_mutex_unlock( &knl_single_thread_mutex );
	}


}

LOCAL void knl_wait_for_start(void* taskid)
{
	printf("in knl_wait_for_start(%d).\n",(TaskType)(TaskRefType)taskid);
}

LOCAL void knl_scheduler_start(void)
{
	pthread_mutex_init(&knl_single_thread_mutex,NULL);
	/* No need to join the threads. */
	pthread_attr_init( &knl_thread_attr );
	pthread_attr_setdetachstate( &knl_thread_attr, PTHREAD_CREATE_DETACHED );
}
