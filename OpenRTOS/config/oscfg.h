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

#define cfgOS_TASK_NUM 4
#define cfgOS_EVENT_NUM 4
#define cfgOS_S_RES_NUM 2
#define cfgOS_I_RES_NUM 1
#define cfgOS_COUNTER_NUM 1
#define cfgOS_ALARM_NUM 2
#define cfgOS_MAX_PRIORITY 3
#define cfgOS_CC BCC2

/* Application Modes */
#define AppMode1 0x2

/* ====================== Tasks ====================== */

/* Task0 configuation */
#define Task0 0
#define Task0_ipriority 0
#define Task0_rpriority 0
			/* [] */
#define Task0_activation 3
#define Task0_stacksize 256
#define Task0_autostart FALSE
#define Task0_schedule FULL
#define Task0_appmode (INVALID_APPMODE)

/* Task1 configuation */
#define Task1 1
#define Task1_ipriority 1
#define Task1_rpriority 1
			/* [] */
#define Task1_activation 3
#define Task1_stacksize 200
#define Task1_autostart FALSE
#define Task1_schedule FULL
#define Task1_appmode (INVALID_APPMODE)

/* Task2 configuation */
#define Task2 2
#define Task2_ipriority 2
#define Task2_rpriority 2
			/* [] */
#define Task2_activation 3
#define Task2_stacksize 200
#define Task2_autostart FALSE
#define Task2_schedule FULL
#define Task2_appmode (INVALID_APPMODE)

/* Task3 configuation */
#define Task3 3
#define Task3_ipriority 3
#define Task3_rpriority 3
			/* [] */
#define Task3_activation 3
#define Task3_stacksize 200
#define Task3_autostart TRUE
#define Task3_schedule FULL
#define Task3_appmode (INVALID_APPMODE | OSDEFAULTAPPMODE)

/* ====================== Events ====================== */
#define Event1 0x0
#define Event2 0x0
#define Event3 0x0
#define Event5 0x0

/* ====================== Resources ====================== */
#define ResourceCAN0 0 /* property = STANDARD */
#define ResourceCAN0_priority 0

#define ResourceCAN1 1 /* property = STANDARD */
#define ResourceCAN1_priority 0

#define ResourceStdOut 0 /* property = INTERNAL */
#define ResourceStdOut_priority 0


/* ====================== Counters  ====================== */
#define SystemTimer 0
#define SystemTimer_maxallowedvalue 1000000
#define SystemTimer_ticksperbase 1
#define SystemTimer_mincycle 1


/* ======================= Alarms  ======================= */
#define WakeTask0 0
#define WakeTask0_counter SystemTimer
#define WakeTask0_time 0
#define WakeTask0_cycle 100
#define WakeTask0_appmode (INVALID_APPMODE | OSDEFAULTAPPMODE | AppMode1)
#define WakeTask0_Action ActivateTask
#define WakeTask0_Task Task0

#define WakeTask1 1
#define WakeTask1_counter SystemTimer
#define WakeTask1_time 50
#define WakeTask1_cycle 100
#define WakeTask1_appmode (INVALID_APPMODE | OSDEFAULTAPPMODE)
#define WakeTask1_Action ActivateTask
#define WakeTask1_Task Task1


#endif /* OSCFG_H_H */

