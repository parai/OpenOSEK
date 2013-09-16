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
#define cfgOS_ERRORHOOK 0
#define cfgOS_PRETASKHOOK 0
#define cfgOS_POSTTASKHOOK 0
#define cfgOS_SHUTDOWDHOOK 0
#define cfgOS_STARTUPHOOK 0

#define cfgOS_TASK_NUM 2
#define cfgOS_FLAG_NUM 0
#define cfgOS_S_RES_NUM 1
#define cfgOS_I_RES_NUM 0
#define cfgOS_COUNTER_NUM 0
#define cfgOS_ALARM_NUM 0
#define cfgOS_MAX_PRIORITY 2
#define cfgOS_CC BCC1

/* Application Modes */

/* ====================== Tasks ====================== */

/* Task1 configuation */
#define Task1 0
#define Task1_ipriority 1
#define Task1_rpriority 1
			/* [] */
#define Task1_activation 1
#define Task1_stacksize 200
#define Task1_autostart TRUE
#define Task1_schedule FULL
#define Task1_appmode (INVALID_APPMODE | OSDEFAULTAPPMODE)
#define Task1_eventhandle INVALID_FLAG

/* Task2 configuation */
#define Task2 1
#define Task2_ipriority 2
#define Task2_rpriority 2
			/* [] */
#define Task2_activation 1
#define Task2_stacksize 200
#define Task2_autostart FALSE
#define Task2_schedule FULL
#define Task2_appmode (INVALID_APPMODE)
#define Task2_eventhandle INVALID_FLAG

/* ====================== Events ====================== */

/* ====================== Resources ====================== */

/* ====================== Counters  ====================== */
#define SystemTimer 0
#define SystemTimer_maxallowedvalue 32767
#define SystemTimer_ticksperbase 1
#define SystemTimer_mincycle 1


/* ======================= Alarms  ======================= */

#endif /* OSCFG_H_H */

