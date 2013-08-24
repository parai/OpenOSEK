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
#include "Os.h"
#include "portable.h"
#include "osek_os.h"

/* ================================ MACROs    =============================== */

/* ================================ TYPEs     =============================== */

/* ================================ DATAs     =============================== */
EXPORT AppModeType knl_appmode = INVALID_APPMODE;
EXPORT uint8    knl_taskindp = 0u;   /* task in independent part nested level */
EXPORT uint8    knl_dispatch_disabled = 0u; /* os dispatch state:enabled(0) or disabled(1) */

/* ================================ FUNCTIONs =============================== */
EXPORT void StartOS ( AppModeType AppMode )
{
    knl_task_init();
    //knl_counter_init();
    //knl_alarm_init();
    //knl_resource_init();
    
}

EXPORT void ShutdownOS( StatusType Error )
{
    DISABLE_INTERRUPT();
#if (cfgOS_SHUT_DOWN_HOOK == STD_ON)
	ShutdownHook(Error);
#endif
    /* @req OS425: If ShutdownOS() is called and ShutdownHook() returns then the operating
       system shall disable all interrupts and enter an endless loop. */
	for ( ; ; )
    {
        /* Dead lopp here */
    }
}

