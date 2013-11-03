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
    /* Program status register (PSR) */
    #define TS_PSR_N               0x80000000        /* Condition flag Negative */
    #define TS_PSR_Z               0x40000000        /* Zero */
    #define TS_PSR_C               0x20000000        /* Carry */
    #define TS_PSR_V               0x10000000        /* Overflow */
    #define TS_PSR_T               0x01000000        /* Thumb mode */

    /* Interrupt  mask register (Primask) */
    #define TS_PMK_D               0x1               /* Interrupt disable */
    #define TS_PMK_E               0x0               /* Interrupt enable */

    /* Except  mask register (Faultmask) */
    #define TS_FMK_D               0x1               /* Except disable */
    #define TS_FMK_E               0x0               /* Except enable */

    /* Control register (Control) */
    #define TS_CTL_MSP             0x0               /* Select MSP */
    #define TS_CTL_PSP             0x2               /* Select PSP */
    #define TS_CTL_SVC             0x0               /* Select supervisor mode */
    #define TS_CTL_USR             0x1               /* Select user mode */
  
    EXTERN knl_curtsk, knl_schedtsk, knl_taskindp
    EXTERN knl_dispatch_disabled,SignalCounter
    EXTERN knl_system_stack
    EXTERN knl_tcb_dispatcher,knl_tcb_sp
    SECTION .text:CODE
    THUMB
    PUBLIC knl_enable_int
//EXPORT void knl_enable_int( imask_t imask );    
knl_enable_int:
    mrs     r1, primask
    msr     primask, r0
    mov     r0, r1
    bx      lr

    SECTION .text:CODE
    THUMB
    PUBLIC knl_disable_int
//EXPORT imask_t knl_disable_int( void );    
knl_disable_int:
    mrs     r0, primask
    ldr     r1, =TS_PMK_D
    msr     primask, r1
    bx      lr
    
    SECTION .text:CODE
    THUMB
    PUBLIC  knl_activate_r
    EXTERN  portActivateR
knl_activate_r:
    mov r3, #TS_PSR_T     //{ //return from exception&interrupr firstly
    ldr r2, =portActivateR
    push {r2,r3}
    subs  SP,SP,#24
    bx lr                 //}

    SECTION .text:CODE
    THUMB
    PUBLIC  knl_dispatch_r
knl_dispatch_r:
     /* start to restore task's context */
    pop     {r4-r11}
    cpsie   i                                    /* just before schedule to next task */
    bx      lr                                   /* lr is EXC_RETURN */

    SECTION .text:CODE
    THUMB
    PUBLIC  knl_force_dispatch_impl
knl_force_dispatch_impl:
    /* Interrupt is disabled,during SVC mode */
    ldr     sp, =(knl_system_stack + 512)  /* Set system stack, see portable.c*/

    ldr     r12, =knl_dispatch_disabled
    ldr     r0, =1
    strb    r0, [r12]                            /* Dispatch disable */

    ldr     r4, =knl_curtsk                      /* R4 = &knl_curtsk */
    ldr     r0, =0xFF
    strb    r0, [r4]                             /* knl_curtsk = INVALID_TASK */
    cpsie   i                                    /* Interrupt enable */
    b       l_dispatch0

    PUBLIC knl_dispatch_entry, knl_dispatch_ret_int
knl_dispatch_entry:
knl_dispatch_ret_int:
    /* Interrupt is disabled,during SVC mode */
    ldr     r12, =knl_dispatch_disabled
    ldr     r0, =1
    strb    r0, [r12]                            /* Dispatch disable */
    cpsie   i                                    /* Interrupt enable */
    push    {r4-r11}
    ldr     r4, =knl_curtsk                      /* R4 = &knl_curtsk */
    ldrb    r8, [r4]                             /* R8 = curtsk */
    ldr     r7,=knl_tcb_sp
    str     sp, [r7,r8, lsl #2]                 /* Save 'ssp' to TCB */
    ldr     r12,=knl_dispatch_r
    ldr     r7, = knl_tcb_dispatcher
    str     r12, [r7,r8, lsl #2]                 /* Save 'dispatcher' to TCB */
    ldr     r0, =0xFF
    strb    r0, [r4]                             /* curtsk = INVALID_TASK */

l_dispatch0:    
l_dispatch1:             /* Judge if goto 'schedtsk'or'low_pow' */
    /* During interrupt enable */
    ldr     r5, = knl_schedtsk              /* R5 = knl_schedtsk */
    ldrb    r5, [r5]
    cpsid   i
    cmp     r5, #0xFF                     /* Is there 'schedtsk'? */
    bne     l_dispatch2
    /* wait untill a task is ready again. */
    cpsie   i                                    /* Interrupt enable */
    nop
    nop
    nop
    b       l_dispatch1

l_dispatch2:                                     /* Switch to 'schedtsk' */
    /* During interrupt disable */
    strb    r5, [r4]                             /* curtsk = schedtsk */
    ldr     r8, = knl_tcb_sp
    ldr     sp, [r8, r5,lsl #2]    /* Restore 'ssp' from TCB */

    ldr     r12, =knl_dispatch_disabled
    ldr     r0, =0
    strb    r0, [r12]                            /* Dispatch enable */
    
    ldr     r8, = knl_tcb_dispatcher
    ldr     r0, [r8, r5,lsl #2]    /* Restore 'dispatcher' from TCB */
    bx      r0

    PUBLIC EnterISR2
EnterISR2:    
    ldr     r1, =knl_taskindp              /* Enter task independent part */
    ldrb    r2, [r1]
    add     r3, r2, #1  /* knl_taskindp++ */
    strb    r3, [r1]
    push    {r1, r2}           /* Here I say wonderful Idea, do you know the reason? */
    cpsie   i     /* Enable Interrupr */
    bx      lr

    PUBLIC ExitISR2
ExitISR2:
    pop     {r1,r2}          /* here same as knl_taskindp--, do you know why? */
    strb    r2, [r1]                             /* Leave task independent part */
    pop     {lr}
    cpsid   i

    ldr     r0, =knl_taskindp              /* Is it a nesting interrupt? */
    ldrb    r0, [r0]
    cmp     r0, #0
    
    bne     l_nodispatch

    ldr     r0, =knl_dispatch_disabled    /* Is it during dispatch disable? */
    ldrb    r0, [r0]
    cmp     r0, #0
    bne     l_nodispatch

    ldr     r0, = knl_curtsk                /* Is dispatch required? */
    ldrb    r0,[r0]
    ldr     r1, = knl_schedtsk
    ldrb    r1,[r1]
    cmp     r0, r1
    bne     knl_dispatch_ret_int            /* To dispatch processing */

l_nodispatch:
    cpsie   i
    bx      lr

    PUBLIC knl_system_timer
    EXTERN SignalCounter
/* This is an ISR example for SystemTick.
 * The Entry Code for ISR must be Assemble.
 * The Way to change it to C code is hasn't been found.
 */    
                            //EXPORT knl_system_timer
knl_system_timer:
                            //{
    push {lr}
    bl EnterISR2             //EnterISR2();

    mov r0,#0
    bl SignalCounter     //(void)IncrementCounter(0);
    b  ExitISR2              //ExitISR2();
                            //}                            
    END
