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
#include <mc9s12xep100.h>
/* ================================ MACROs    =============================== */
#define CPU_FREQUENCY 32000000
/* ================================ TYPEs     =============================== */

/* ================================ DATAs     =============================== */
LOCAL uint8* knl_tcb_sp[cfgOS_TASK_NUM]={0};
LOCAL FP    knl_tcb_dispatcher[cfgOS_TASK_NUM]={0};
LOCAL uint8 knl_system_stack[512];

/* ================================ FUNCTIONs =============================== */
LOCAL void portStartSystemTimer( void );
LOCAL void portActivateR(void);
LOCAL void portDispatchR(void);
LOCAL void l_dispatch0(void);

void TERMIO_PutChar(char c)
{
    while(!SCI0SR1_TDRE);       /* 等待发送数据寄存器（缓冲器）为空 */
	SCI0DRL = c;
}
LOCAL void portSciInit()
{
	SCI0BD = CPU_FREQUENCY/16/115200;   /* 设置SCI0波特率为115200 */
	SCI0CR1 = 0x00;       /* 设置SCI0为正常模式，八位数据位，无奇偶校验*/
	SCI0CR2 = 0x08;       /* 允许发送数据，禁止中断功能 */
}
LOCAL void portSystemClockInit(void)
{
	CRGINT = 0;                  //关中断
	CLKSEL_PLLSEL = 0;           //在未初始化PLL前不使用PLL的输出作为CPU时钟

  #if(CPU_FREQUENCY == 40000000)
	SYNR = 4;
  #elif(CPU_FREQUENCY == 32000000)
	SYNR = 3;
  #elif(CPU_FREQUENCY == 24000000)
	SYNR = 2;
  #endif

	REFDV = 1;                   //PLLCLK=2×OSCCLK×(SYNR+1)/(REFDV+1)＝64MHz ,fbus=32M
	PLLCTL_PLLON = 1;            //开PLL
	while (CRGFLG_LOCK == 0);    //等待PLL锁定频率
	CLKSEL_PLLSEL = 1;           //选择系统时钟由PLL产生
}
EXPORT  void portOsStartupHook(void)
{
	portSystemClockInit();
	portSciInit();
	portStartSystemTimer();
}
EXPORT imask_t knl_disable_int( void )
{
    asm psha;
    asm tpa;
    asm tab;
    asm sei;
    asm pula;
}

EXPORT void knl_enable_int( imask_t mask )
{
	(void)mask;
	asm psha;
	asm tba;
	asm tap;
	asm pula;
}

EXPORT void knl_setup_context(TaskType taskid)
{
	knl_tcb_sp[taskid] = knl_tcb_stack[taskid];
	knl_tcb_dispatcher[taskid] = portActivateR;
}

EXPORT void knl_force_dispatch(void)
{
	asm lds #knl_system_stack:512   /* Set system stack */

	knl_dispatch_disabled = 1;    /* Dispatch disable */

	knl_curtsk=INVALID_TASK;

	DISABLE_INTERRUPT();

	asm jmp l_dispatch0;
}
LOCAL void l_dispatch0(void)
{
l_dispatch1:
	DISABLE_INTERRUPT();
	if(INVALID_TASK == knl_schedtsk)
	{
		ENABLE_INTERRUPT();
		asm nop;
		asm nop;
		asm nop;
		asm nop;
		goto l_dispatch1;
	}
l_dispatch2:
	knl_curtsk=knl_schedtsk;
	knl_dispatch_disabled=0;    /* Dispatch enable */

	/* Context restore */
	asm LDAB  knl_curtsk
	asm CLRA
	asm LSLD
	asm TFR   D,X
	asm LDS   knl_tcb_sp,X
	// knl_tcb_dispatcher[knl_curtsk];
	asm LDAB  #3
	asm LDAA  knl_curtsk
	asm MUL
	asm TFR   D,Y
	asm JMP  [knl_tcb_dispatcher,Y]
}
//when task start to run
LOCAL void portActivateR(void)
{
    /* This is the most easiest Way to get Internal Resourse and
     * to make a task non-preemtable I think */
    GetInternalResource();
	ENABLE_INTERRUPT();
    knl_tcb_pc[knl_curtsk]();
}

//when task resume to run
LOCAL void portDispatchR(void)
{

    asm   pula;
    asm   staa	$15;	      /* restore PPAGE */
    asm   rti;
}
#pragma CODE_SEG __NEAR_SEG NON_BANKED
//dispatch entry, entered by "swi" instruction
//do preempt the current running task <knl_ctxtsk>,
//and then do dispatch the high ready task <knl_schedtsk>
interrupt 4 void knl_dispatch_entry(void)
{
    knl_dispatch_disabled=1;    /* Dispatch disable */
    //asm   ldd   knl_taskmode
    //asm   pshd;                 /* save knl_taskmode */
    asm   ldaa	$15
	asm   psha                  /* save ppage */
	asm LDAB  knl_curtsk
	asm CLRA
	asm LSLD
	asm TFR   D,X
	asm STS   knl_tcb_sp,X
	knl_tcb_dispatcher[knl_curtsk] = portDispatchR;
	knl_curtsk=INVALID_TASK;

	asm jmp l_dispatch0;
}

//default SystemTick ISR handler,which also is an example for other ISR
//the system counter whose id is 0 will be processed.
interrupt 7 void portSystemTimer(void)
{
    CRGFLG &=0xEF;			// clear the interrupt flag
    EnterISR();
	(void)SignalCounter(0);
    LeaveISR();
}
#pragma CODE_SEG DEFAULT


//mean start the SystemTick ISR timer.
//here the test board I use has a 16 MHZ oscillator
//modify it if you have a different board and want a different
//SystemTick ISR frequency
LOCAL void portStartSystemTimer( void )
{
    CRGINT_RTIE=1;       //enable real-time interrupt
    RTICTL = 0x70;       //period is 4.096ms
	//OSCCLK = 16 x 10E6
    //RTI ISR period =  1/OSCCLK * (0+1) * 2E(7+9)=0.004096s=4.096ms
}
