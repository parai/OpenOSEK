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
#ifndef _DEBUG_H_
#define _DEBUG_H_
/* ================================ INCLUDEs  =============================== */
#include "Std_Types.h"

/* ================================ PRE-PROCESSs  =========================== */
#ifndef USE_DEBUG
#warning The macro USE_DEBUG hasn't been defined,the default value STD_OFF will be used.
#define USE_DEBUG STD_OFF
#endif

#ifndef DEBUG_LVL
#warning The macro DEBUG_LVL hasn't been defined,the default value DEBUG_MEDIUM will be used.
#define DEBUG_LVL		DEBUG_MEDIUM
#endif

/* ================================ MACROs    =============================== */
#define DEBUG_LOW		1
#define DEBUG_MEDIUM	2
#define DEBUG_HIGH		3
#define DEBUG_NONE		4

#ifdef  USE_DEBUG

#define DEBUG(_level,message) DEBUG_PRINT0(_level,message)

#define DEBUG_PRINT0(_level,arg0)    \
	do { \
		if(_level>=DEBUG_LVL) { \
			printf(arg0); \
		}; \
	} while(0);
#define DEBUG_PRINT1(_level,format,arg1)    \
	do { \
		if(_level>=DEBUG_LVL) { \
			printf (format,arg1); \
		}; \
	} while(0);	
#define DEBUG_PRINT2(_level,format,arg1,arg2)    \
	do { \
		if(_level>=DEBUG_LVL) { \
			printf (format,arg1,arg2); \
		}; \
	} while(0);
#define DEBUG_PRINT3(_level,format,arg1,arg2,arg3)    \
	do { \
		if(_level>=DEBUG_LVL) { \
			printf (format,arg1,arg2,args3); \
		}; \
	} while(0);
#define DEBUG_PRINT4(_level,format,arg1,arg2,arg3,arg4)    \
	do { \
		if(_level>=DEBUG_LVL) { \
			printf (format,arg1,arg2,args3,arg4); \
		}; \
	} while(0);
#define DEBUG_PRINT5(_level,format,arg1,arg2,arg3,arg4,arg5)    \
	do { \
		if(_level>=DEBUG_LVL) { \
			printf (format,arg1,arg2,args3,arg4,arg5); \
		}; \
	} while(0);	

#else  /* USE_DEBUG */ 
#define DEBUG(_level,message)    
#define DEBUG_PRINT0(_level,arg0)
#define DEBUG_PRINT1(_level,format,arg1)
#define DEBUG_PRINT2(_level,format,arg1,arg2)
#define DEBUG_PRINT3(_level,format,arg1,arg2,arg3)
#define DEBUG_PRINT4(_level,format,arg1,arg2,arg3,arg4)
#define DEBUG_PRINT5(_level,format,arg1,arg2,arg3,arg4,arg5)
#endif /* USE_DEBUG */ 

/* ================================ TYPEs     =============================== */

/* ================================ DATAs     =============================== */

/* ================================ FUNCTIONs =============================== */


#endif /* _DEBUG_H_ */
