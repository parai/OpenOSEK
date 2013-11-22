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
#ifndef _DEBUG_H__H_H_H
#define _DEBUG_H__H_H_H

/* ================================ INCLUDEs  =============================== */
#include <assert.h>
/* ================================ MACROs    =============================== */
#define cfgDEV_TRACE_ON     1
// the bigger value has higher level
#define cfgDEV_TRACE_LEVEL  15
// Suggest Level
#define tlError      100
#define tlWarning    90

// Component
#define tlGen        10
#define tlCan        20
#define tlCanTp      30
#define tlUds        40
#define tlOs         0
#define tlPort       0
#if	(defined( __GNUC__ )||defined( WIN32 )) && (cfgDEV_TRACE_ON == 1)
#define devTrace(__level,...) 	\
	if(__level > cfgDEV_TRACE_LEVEL)	printf(__VA_ARGS__)
#define devAction(__level,__action)	\
	if(__level > cfgDEV_TRACE_LEVEL)	(__action)
#define devAssert(__isTrue,...)		\
	if(!(__isTrue))					\
	{								\
		printf(__VA_ARGS__);		\
		for(;;);					\
	}
#else
//#if	(defined( __GNUC__ )||defined( WIN32 ))
#define devTrace(__level,...)
#define devAction(__level,__action)
#define devAssert(__isTrue,...)
//#else
//void devTrace(int __level,char* fmt,...);
//#define devAction(__level,__action)
//void devAssert(int __level,char* fmt,...);
//#endif
#endif

/* ================================ TYPEs     =============================== */

/* ================================ DATAs     =============================== */

/* ================================ FUNCTIONs =============================== */
#endif
