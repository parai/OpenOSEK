
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
#define cfgOS_ERRORHOOK FALSE
#define cfgOS_PRETASKHOOK FALSE
#define cfgOS_POSTTASKHOOK FALSE
#define cfgOS_SHUTDOWNHOOK FALSE
#define cfgOS_STARTUPHOOK FALSE

#define cfgOS_TASK_NUM 0x4
#define cfgOS_FLAG_NUM 0x0
#define cfgOS_S_RES_NUM 0x1
#define cfgOS_I_RES_NUM 0x0
#define cfgOS_COUNTER_NUM 0x0
#define cfgOS_ALARM_NUM 0x0
#define cfgOS_MAX_PRIORITY 0x4
#define cfgOS_CC BCC2

/* Application Modes */


/* Task1 configuation */
#define Task1 0
#define Task1_ipriority 1
#define Task1_rpriority 1
			/* [] */
#define Task1_activation 2
#define Task1_stacksize 256
#define Task1_schedule FULL
#define Task1_autostart TRUE
#define Task1_appmode (INVALID_APPMODE | OSDEFAULTAPPMODE)
#define Task1_eventhandle INVALID_FLAG


/* Task2 configuation */
#define Task2 1
#define Task2_ipriority 2
#define Task2_rpriority 2
			/* [] */
#define Task2_activation 1
#define Task2_stacksize 256
#define Task2_schedule FULL
#define Task2_autostart FALSE
#define Task2_appmode (INVALID_APPMODE)
#define Task2_eventhandle INVALID_FLAG


/* Task3 configuation */
#define Task3 2
#define Task3_ipriority 2
#define Task3_rpriority 2
			/* [] */
#define Task3_activation 1
#define Task3_stacksize 256
#define Task3_schedule FULL
#define Task3_autostart FALSE
#define Task3_appmode (INVALID_APPMODE)
#define Task3_eventhandle INVALID_FLAG


/* Task4 configuation */
#define Task4 3
#define Task4_ipriority 4
#define Task4_rpriority 4
			/* [] */
#define Task4_activation 1
#define Task4_stacksize 256
#define Task4_schedule FULL
#define Task4_autostart FALSE
#define Task4_appmode (INVALID_APPMODE)
#define Task4_eventhandle INVALID_FLAG

#endif /* OSCFG_H_H */

