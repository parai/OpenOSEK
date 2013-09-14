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
EXPORT AlarmType knl_ccb_head[cfgOS_COUNTER_NUM];
EXPORT TickType  knl_ccb_value[cfgOS_COUNTER_NUM];
EXPORT AlarmType knl_acb_next[cfgOS_ALARM_NUM];
EXPORT AlarmType knl_acb_prev[cfgOS_ALARM_NUM];
EXPORT TickType  knl_acb_value[cfgOS_ALARM_NUM];
EXPORT TickType  knl_acb_period[cfgOS_ALARM_NUM];

/* ================================ FUNCTIONs =============================== */
StatusType SetRelAlarm ( AlarmType AlarmID , TickType Increment ,TickType Cycle )
{
	StatusType ercd = E_OK;
	CounterType counter;
	OS_EXT_VALIDATE((AlarmID < cfgOS_ALARM_NUM),E_OS_ID);
	OS_STD_VALIDATE((ALARM_STOPPED == knl_acb_value[AlarmID]),E_OS_STATE);
	counter = knl_acb_counter[AlarmID];
	OS_EXT_VALIDATE((knl_ccb_max[counter] >= Increment),E_OS_VALUE);
	OS_EXT_VALIDATE((knl_ccb_max[counter] >= Cycle),E_OS_VALUE);
	OS_EXT_VALIDATE(((knl_ccb_min[counter] <= Cycle)),E_OS_VALUE);
	BEGIN_DISABLE_INTERRUPT();
	knl_acb_value[AlarmID] = knl_add_ticks(knl_ccb_value[counter],Increment,knl_ccb_max[counter]*2);
	knl_acb_period[AlarmID] = Cycle;
	knl_alarm_insert(AlarmID);
	END_DISABLE_INTERRUPT();
OS_VALIDATE_ERROR_EXIT()
	return ercd;
}

StatusType SetAbsAlarm ( AlarmType AlarmID , TickType Start ,TickType Cycle )
{
	StatusType ercd = E_OK;
	CounterType counter;
	OS_EXT_VALIDATE((AlarmID < cfgOS_ALARM_NUM),E_OS_ID);
	OS_STD_VALIDATE((ALARM_STOPPED == knl_acb_value[AlarmID]),E_OS_STATE);
	counter = knl_acb_counter[AlarmID];
	OS_EXT_VALIDATE((knl_ccb_max[counter] >= Start),E_OS_VALUE);
	OS_EXT_VALIDATE((knl_ccb_max[counter] >= Cycle),E_OS_VALUE);
	OS_EXT_VALIDATE(((knl_ccb_min[counter] <= Cycle)),E_OS_VALUE);
	BEGIN_DISABLE_INTERRUPT();
	knl_acb_value[AlarmID] = Start;
	knl_acb_period[AlarmID] = Cycle;
	knl_alarm_insert(AlarmID);
	END_DISABLE_INTERRUPT();
OS_VALIDATE_ERROR_EXIT()
	return ercd;
}

StatusType CancelAlarm ( AlarmType AlarmID )
{
    StatusType ercd = E_OK;
    OS_EXT_VALIDATE((AlarmID < cfgOS_ALARM_NUM),E_OS_ID);
	OS_STD_VALIDATE((ALARM_STOPPED != knl_acb_value[AlarmID]),E_OS_NOFUNC);

	BEGIN_DISABLE_INTERRUPT();
	knl_alarm_remove(AlarmID);
	END_DISABLE_INTERRUPT();
OS_VALIDATE_ERROR_EXIT()
	return ercd;
}
EXPORT StatusType SignalCounter(CounterType counter)
{
	AlarmType alarm;
	BEGIN_CRITICAL_SECTION();
	/* here I see the difference between arccore and nxtOSEK.
	   in nxtOSEK, max allowed value allowed for counter and alarm are both the MaxAllowedValue*2,
	   as it has implemented the TicksPerBase. So the method used by nxtOSEK is preferred.
	*/
	knl_ccb_value[counter] = knl_add_ticks(knl_ccb_value[counter],1,knl_ccb_max[counter]*2);
	alarm = knl_ccb_head[counter];
	while(INVALID_ALARM != alarm)
	{
		if(knl_diff_tick(knl_ccb_value[counter],knl_acb_value[alarm],knl_ccb_max[counter]*2)
				> knl_ccb_max[counter])
		{
			break;
		}
		else
		{
			AlarmType next = knl_acb_next[alarm];
			knl_acb_action[alarm]();
			knl_alarm_remove(alarm);
			if(0 != knl_acb_period[alarm])
			{
				knl_acb_value[alarm] = knl_add_ticks(knl_ccb_value[counter],	\
														knl_acb_period[alarm],	\
														knl_ccb_max[counter]*2);
				knl_alarm_insert(alarm);
			}
			alarm = next;
		}
	}
	END_CRITICAL_SECTION();
	return E_OK;
}
EXPORT void knl_alarm_counter_init(void)
{
	uint8 i;
	for(i=0;i<cfgOS_COUNTER_NUM;i++)
	{
		knl_ccb_head[i] = INVALID_ALARM;
		knl_ccb_value[i] = 0;
	}
	for(i=0;i<cfgOS_ALARM_NUM;i++)
	{
		knl_acb_next[i] = INVALID_ALARM;
		knl_acb_prev[i] = INVALID_ALARM;
		knl_acb_value[i] = ALARM_STOPPED;
	}
	for(i=0;i<cfgOS_ALARM_NUM;i++)
	{
		if((knl_acb_mode[i]&knl_appmode) != 0u)
		{
			(void)SetRelAlarm(i,knl_acb_time[i],knl_acb_cycle[i]);
		}
	}
}

//add the alarm value by incr
EXPORT TickType knl_add_ticks(TickType almval,TickType incr,TickType maxval2)
{
    if(incr <= (maxval2 - almval))
    {
        return (almval+incr);
    }
    else
    {
        return (incr - (maxval2 + 1  - almval));
    }
}

//calculate the difference between counter current value and alarm
//next expiry value.
//if the diff < maxval, it really means the alarm has expiried and
//should be processed.
EXPORT TickType knl_diff_tick(TickType curval, TickType almval, TickType maxval2)
{
	if (curval >= almval) {
		return(curval - almval);
	}
	else {
		return(maxval2 - almval + curval + 1);
	}
}

//Insert the alarm <alarm> at the proper position at the alarm queue of
//the counter <counter>.
//The alarm queue is sorted by the alarm next expire value, from min to max
//NOTE:the overflowed alarm next expire value(whose time < knl_ccb_value[counter]) should treat
//bigger than the non-overflowed one.
EXPORT void knl_alarm_insert(AlarmType alarm)
{

	AlarmType prev,next;
	CounterType counter = knl_acb_counter[alarm];
	prev =  INVALID_ALARM;
	next =  knl_ccb_head[counter];

	if( knl_acb_value[alarm] < knl_ccb_value[counter] )
	{   /* It's an overflowed Alarm,So Skip all the non-overflowed one*/
		while( (next != INVALID_ALARM) &&
			   (knl_acb_value[next] > knl_ccb_value[counter] ))
		{
			prev = next;
			next = knl_acb_next[next];
		}
	}
	while( (next != INVALID_ALARM ) &&
		   (knl_acb_value[next] < knl_acb_value[alarm]))
	{   /*  Find untill next's value is bigger that alarm's value,
		 *  That is just the place to insert alarm*/
		prev = next;
		next = knl_acb_next[next];
	}
	if(INVALID_ALARM == prev)
	{                           /* Should insert it At Head */
#if 0
		if(INVALID_ALARM == next)
		{
			knl_ccb_head[counter] = alarm;
			knl_acb_next[alarm] = INVALID_ALARM;
			knl_acb_prev[alarm] = INVALID_ALARM;
		}
		else
#endif  // can see it's ok to comment out the code above
		{
			knl_ccb_head[counter] = alarm;
			knl_acb_next[alarm] = next;
			knl_acb_prev[alarm] = INVALID_ALARM;
		}
	}
	else if(INVALID_ALARM == next)
	{                           /* Should insert it At Tail */
		knl_acb_next[alarm] = INVALID_ALARM;
		knl_acb_prev[alarm] = prev;
		knl_acb_next[prev] = alarm;
	}
	else
	{                           /* Should insert it between prev,next */
		knl_acb_next[alarm] = next;
		knl_acb_prev[alarm] = prev;
		knl_acb_next[prev] = alarm;
		knl_acb_prev[next] = alarm;
	}
}

//remove the alarm from list
EXPORT void knl_alarm_remove(AlarmType alarm)
{
	AlarmType prev,next;
	CounterType counter = knl_acb_counter[alarm];
	prev =  knl_acb_prev[alarm];
	next =  knl_acb_next[alarm];
	if(INVALID_ALARM == prev)
	{	// remove alarm at head;
		knl_ccb_head[counter] = next;
		if(INVALID_ALARM != next)
		{
			knl_acb_prev[next] = INVALID_ALARM;
		}
	}
	else if(INVALID_ALARM == next)
	{	// remove alarm at tail;
		knl_acb_next[prev] = INVALID_ALARM;
	}
	else
	{
		knl_acb_next[prev] = next;
		knl_acb_prev[next] = prev;
	}
}

