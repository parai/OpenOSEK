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
#ifndef _STD_TYPES_H_
#define _STD_TYPES_H_

/* ================================ INCLUDEs  =============================== */
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

/* ================================ MACROs    =============================== */
#define STD_NULL ((void *) 0)
#define STD_TRUE  (1)
#define STD_FALSE (0)

#define STD_ON   1
#define STD_OFF  0

#ifndef NULL
#define NULL ((void*)0)
#endif

#define NULL_FP   ((FP)0)

#ifndef TRUE
#define TRUE  1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#define EXPORT
#define IMPORT extern
#define LOCAL  static

#ifndef MIN
#define MIN(_x,_y) (((_x) < (_y)) ? (_x) : (_y))
#endif
#ifndef MAX
#define MAX(_x,_y) (((_x) > (_y)) ? (_x) : (_y))
#endif
#ifndef ABS
#define ABS(_x) (((_x) > 0) ? (_x) : (-_x))
#endif

/* Values for Std_ReturnType */
#define E_OK            0
#define E_NOT_OK        1  


/* ================================ TYPEs     =============================== */
typedef unsigned char       boolean;
typedef signed char         sint8;        
typedef unsigned char       uint8;
typedef signed short        sint16;       
typedef unsigned short      uint16;       
typedef signed long         sint32;       
typedef unsigned long       uint32;
typedef unsigned long long  uint64; /* Yeah, This type may not by supported by 16 bit cpu */
typedef unsigned int        uint8_least;  
typedef unsigned int        uint16_least; 
typedef unsigned long       uint32_least; 
typedef signed int          sint8_least;  
typedef signed int          sint16_least; 
typedef signed long         sint32_least; 
typedef float               float32; 
typedef double              float64; 
typedef uint8               Std_ReturnType;

typedef void (*FP)(void);       /* function pointer type */

typedef unsigned int UINT;
typedef   signed int SINT;

/* ================================ DATAs     =============================== */

/* ================================ FUNCTIONs =============================== */

#endif /* _STD_TYPES_H_ */


