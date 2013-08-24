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

#ifndef OSCFG_H_H
#define OSCFG_H_H


/* ====================== General ======================= */
#define cfgOS_STATUS EXTENDED
#define cfgOS_ERRORHOOK 1
#define cfgOS_PRETASKHOOK 0
#define cfgOS_POSTTASKHOOK 1
#define cfgOS_SHUTDOWDHOOK 1
#define cfgOS_STARTUPHOOK 1

#define cfgOS_TASK_NUM 2
#define cfgOS_EVENT_NUM 4
#define cfgOS_S_RES_NUM 2
#define cfgOS_I_RES_NUM 1
#define cfgOS_COUNTER_NUM 1
#define cfgOS_ALARM_NUM 2
#define cfgOS_MAX_PRIORITY 6

/* ====================== Tasks ====================== */

/* Task0 configuation */
#define Task0 0
#define Task0_ipriority 0
#define Task0_rpriority 5
			/* [ResourceCAN0,ResourceStdOut,] */
#define Task0_activation 1
#define Task0_stacksize 256
#define Task0_autostart TRUE
#define Task0_schedule FULL
#define Task0_appmode (INVALID_APPMODE | OSDEFAULTAPPMODE)
#define Task0_eventhandle 0
			/* [Event1,Event2,Event3,] */

/* Task1 configuation */
#define Task1 1
#define Task1_ipriority 5
#define Task1_rpriority (cfgOS_MAX_PRIORITY)
			/* [ResourceCAN0,ResourceCAN1,ResourceStdOut,] */
#define Task1_activation 1
#define Task1_stacksize 200
#define Task1_autostart TRUE
#define Task1_schedule NON
#define Task1_appmode (INVALID_APPMODE | OSDEFAULTAPPMODE)
#define Task1_eventhandle 1
			/* [Event5,] */

/* ====================== Events ====================== */
#define Event1 0x8
#define Event2 0x1
#define Event3 0x2
#define Event5 0x1

/* ====================== Resources ====================== */
#define ResourceCAN0 0 /* property = STANDARD */
#define ResourceCAN0_priority 5
#define ResourceCAN1 1 /* property = STANDARD */
#define ResourceCAN1_priority 5
#define ResourceStdOut 0 /* property = INTERNAL */
#define ResourceStdOut_priority 5

#endif /* OSCFG_H_H */

