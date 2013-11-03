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
#ifndef _PORTABLE_H_
#define _PORTABLE_H_
/* ================================ INCLUDEs  =============================== */

/* ================================ MACROs    =============================== */
#define CPU_FREQUENCY 64000000

#define OpenOSEKStartupHook() portOsStartupHook()
/*
 * Interrupt enable/disable
 */
#define DISABLE_INTERRUPT()	 { __asm("cpsid   i");}
#define ENABLE_INTERRUPT()	 { __asm("cpsie   i");}

/*
 * Start/End interrupt disable section
 */
#define BEGIN_DISABLE_INTERRUPT()	{ imask_t _primask_ = knl_disable_int()
#define END_DISABLE_INTERRUPT()	knl_enable_int(_primask_); }

/*
 * Start/End critical section
 */
#define BEGIN_CRITICAL_SECTION()	{ imask_t _primask_ = knl_disable_int()
#define END_CRITICAL_SECTION()	    if ( knl_curtsk != knl_schedtsk         \
                                     && (0u == knl_taskindp)                \
                                     && (0u == knl_dispatch_disabled)       \
   /* Pre-Interrupt Enabled */       && (0u == (_primask_&0x01u))) {        \
        knl_dispatch();                                                     \
    }                                                                       \
    knl_enable_int(_primask_); }

/*
 * Start task dispatcher
 */ // See core_cm3.h
#define SCB_ICSR_ADDRESS    (0xE000E000+0x0D00+0x04)
#define knl_dispatch()     { (*(volatile unsigned long*)(SCB_ICSR_ADDRESS)) = (1u<<28); }

#define knl_isr_dispatch() knl_dispatch()

/* ================================ TYPEs     =============================== */
/* interrupr mask type.determined by CPU */
typedef unsigned int imask_t;

/* ================================ DATAs     =============================== */

/* ================================ FUNCTIONs =============================== */
IMPORT imask_t knl_disable_int( void );
IMPORT void knl_enable_int( imask_t mask );
IMPORT void knl_force_dispatch(void);
IMPORT void knl_setup_context(TaskType taskid);
IMPORT void portOsStartupHook(void);
IMPORT void portNVICSetPriority(sint32 IRQn,uint32 priority);

#endif /* _PORTABLE_H_ */
