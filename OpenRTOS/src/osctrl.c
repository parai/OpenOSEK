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
EXPORT AppModeType knl_appmode = INVALID_APPMODE;
EXPORT uint8    knl_taskindp = 0u;          /* task in independent part nested level */
EXPORT uint8    knl_dispatch_disabled = 0u; /* os dispatch state:enabled(0) or disabled(1) */

#if(cfgOS_ERRORHOOK == 1)
EXPORT OSServiceIdType	_errorhook_svcid;
EXPORT _ErrorHook_Par	_errorhook_par1, _errorhook_par2, _errorhook_par3;
#endif
/* ================================ FUNCTIONs =============================== */
/* |------------------+------------------------------------------------------| */
/* | Syntax:          | void StartOS ( AppModeType <Mode> )                  | */
/* |------------------+------------------------------------------------------| */
/* | Parameter (In):  | Mode:application mode                                | */
/* |------------------+------------------------------------------------------| */
/* | Parameter (Out): | none                                                 | */
/* |------------------+------------------------------------------------------| */
/* | Description:     | The user can call this system service to start the   | */
/* |                  | operating system in a specific mode, see chapter 5   | */
/* |                  | (os223.doc), Application modes.                      | */
/* |------------------+------------------------------------------------------| */
/* | Particularities: | Only allowed outside of the operating system,        | */
/* |                  | therefore implementation specific restrictions may   | */
/* |                  | apply. See also chapter 11.3, System start-up,       | */
/* |                  | especially with respect to systems where OSEK and    | */
/* |                  | OSEKtime coexist. This call does not need to return. | */
/* |------------------+------------------------------------------------------| */
/* | Conformance:     | BCC1, BCC2, ECC1, ECC2                               | */
/* |------------------+------------------------------------------------------| */
EXPORT void StartOS ( AppModeType AppMode )
{
	DISABLE_INTERRUPT();
	knl_dispatch_disabled = 1; /* Dispatch disable */
#if defined( OpenOSEKStartupHook )
	OpenOSEKStartupHook();
#endif
	knl_appmode = AppMode;
	knl_task_init();
#if(cfgOS_ALARM_NUM > 0)
	knl_alarm_counter_init();
#endif
	knl_resource_init();
#if(cfgOS_STARTUPHOOK == 1)
	StartupHook();
#endif
    knl_force_dispatch();
    
}

/* |------------------+------------------------------------------------------------------| */
/* | Syntax:          | void ShutdownOS ( StatusType <Error> )                           | */
/* |------------------+------------------------------------------------------------------| */
/* | Parameter (In):  | Error:error occurred                                             | */
/* |------------------+------------------------------------------------------------------| */
/* | Parameter (Out): | none                                                             | */
/* |------------------+------------------------------------------------------------------| */
/* | Description:     | 1.The user can call this system service to abort                 | */
/* |                  | the overall system (e.g. emergency off). The                     | */
/* |                  | operating system also calls this function internally,            | */
/* |                  | if it has reached an undefined internal state and is             | */
/* |                  | no longer ready to run.                                          | */
/* |                  | 2.If a ShutdownHook is configured the hook routine               | */
/* |                  | ShutdownHook is always called (with <Error> as argument)         | */
/* |                  | before shutting down the operating system.                       | */
/* |                  | 3.If ShutdownHook returns, further behaviour of ShutdownOS       | */
/* |                  | is implementation specific.                                      | */
/* |                  | 4.In case of a system where OSEK OS and OSEKtime OS coexist,     | */
/* |                  | ShutdownHook has to return. <Error> needs to be a valid          | */
/* |                  | error code supported by OSEK OS.                                 | */
/* |                  | 5.In case of a system where OSEK OS and OSEKtime OS coexist,     | */
/* |                  | <Error> might also be a value accepted by OSEKtime OS.           | */
/* |                  | In this case, if enabled by an OSEKtime configuration parameter, | */
/* |                  | OSEKtime OS will be shut down after OSEK OS shutdown.            | */
/* |------------------+------------------------------------------------------------------| */
/* | Particularities: | After this service the operating system is shut down.            | */
/* |                  | Allowed at task level, ISR level, in ErrorHook and StartupHook,  | */
/* |                  | and also called internally by the operating system.              | */
/* |                  | If the operating system calls ShutdownOS it never uses E_OK      | */
/* |                  | as the passed parameter value.                                   | */
/* |------------------+------------------------------------------------------------------| */
/* | Conformance:     | BCC1, BCC2, ECC1, ECC2                                           | */
/* |------------------+------------------------------------------------------------------| */
EXPORT void ShutdownOS( StatusType Error )
{
    DISABLE_INTERRUPT();
#if (cfgOS_SHUTDOWNHOOK == 1)
	ShutdownHook(Error);
#endif
    /* @req OS425: If ShutdownOS() is called and ShutdownHook() returns then the operating
       system shall disable all interrupts and enter an endless loop. */
	for ( ; ; )
    {
        /* Dead lopp here */
    }
}

EXPORT void EnterISR(void)
{
	if(knl_taskindp < 0xFF)
	{
		knl_taskindp++; /* Enter Task Independent Part */
	}
	ENABLE_INTERRUPT();
}

EXPORT void LeaveISR(void)
{

	DISABLE_INTERRUPT();
	if(knl_taskindp > 0)
	{
		knl_taskindp--;
	}
	if((0 == knl_taskindp) && (!knl_dispatch_disabled))
	{
		if(knl_curtsk != knl_schedtsk)
		{
			devTrace(tlPort,"OS:Dispatch in ISR has been requested!\n");
			knl_isr_dispatch();
		}
	}
	ENABLE_INTERRUPT();
}

#if(cfgOS_ERRORHOOK == 1)
EXPORT void knl_call_errorhook(StatusType ercd)
{
    static call_count = 0;
    call_count++;
    if(1 == call_count)
    {
        ErrorHook(ercd);
    }
    call_count--;
}
#endif
