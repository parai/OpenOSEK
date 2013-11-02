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
#define CPU_FREQUENCY 64000000
// Regeister Address
#define rWDT_MR      (0x400E1450U+0x04)

#define rEEFC_FMR    (0x400E0A00U+0x00)

#define rPMC_PCER    (0x400E0400U+0x10)
#define rCKGR_MOR    (0x400E0400U+0x20)
#define rPMC_MCKR    (0x400E0400U+0x30)
#define rPMC_SR      (0x400E0400U+0x68)
#define rCKGR_PLLAR  (0x400E0400U+0x28)

#define rPIO_PER     (0x400e0e00+0x00)
#define rPIO_PDR     (0x400e0e00+0x04)
#define rPIO_PSR     (0x400e0e00+0x08)

#define rUART0_CR    (0x400e0600+0x00)         /* Control Register                */
#define rUART0_MR    (0x400e0600+0x04)         /* Mode Register                   */
#define rUART0_IER   (0x400e0600+0x08)         /* Interrupt Enable Register       */
#define rUART0_IDR   (0x400e0600+0x0c)         /* Interrupt Disable Registe       */
#define rUART0_IMR   (0x400e0600+0x10)         /* Interrupt Mask Register         */
#define rUART0_SR    (0x400e0600+0x14)         /* Channel Status Register         */
#define rUART0_RHR   (0x400e0600+0x18)         /* Receiver Holding Register       */
#define rUART0_THR   (0x400e0600+0x1c)         /* Transmitter Holding Register    */
#define rUART0_BRGR  (0x400e0600+0x20)         /* Baud Rate Generator Register    */
#define rPERIPH_PTCR (0x400e0600+0x120)

#define rSysTickCtrl    (0xE000E000+0x0010+0x00)
#define rSysTickLoad    (0xE000E000+0x0010+0x04)
#define rSysTickVal     (0xE000E000+0x0010+0x08)
#define rSysTickCalib   (0xE000E000+0x0010+0x0C)

#define rSCB_SHP        (0xE000E000+0x0D00+0x18)
#define rNVIC_IP        (0xE000E000+0x0100+0x300)
// Register Input/Output
#define rOut(_address,_value) { (*(volatile unsigned long*)(_address)) = (_value); }
#define rIn(_address)  (*(volatile unsigned long*)(_address))

/* ================================ TYPEs     =============================== */

/* ================================ DATAs     =============================== */
EXPORT uint8* knl_tcb_sp[cfgOS_TASK_NUM]={0};
EXPORT FP    knl_tcb_dispatcher[cfgOS_TASK_NUM]={0};
EXPORT uint8 knl_system_stack[512];

/* ================================ FUNCTIONs =============================== */
LOCAL void portStartSystemTimer( void );

// for printf
EXPORT int putchar( int c )
{
	/* Wait for the transmitter to be ready */
    if('\n' == c)
    {
        while ( 0u == (rIn(rUART0_SR) & (1u<<9))) ;
        rOut(rUART0_THR,'\r');
    }
	while ( 0u == (rIn(rUART0_SR) & (1u<<9))) ;
	rOut(rUART0_THR,c);
	return c;
}
EXPORT void portNVICSetPriority(sint32 IRQn,uint32 priority)
{
	if(IRQn < 0)
	{	/* set Priority for Cortex-M3 System Interrupts */
		*(uint8*)( rSCB_SHP+(((uint32)(IRQn) & 0xF)-4) ) = ((priority << (8 - 4)) & 0xff);
	}
	else
	{	/* set Priority for device specific Interrupts  */
		*(uint8*)( rNVIC_IP+IRQn ) = ((priority << (8 - 4)) & 0xff);
	}
}
EXPORT  void portOsStartupHook(void)
{
    //=========== See SAM3S.h
    // Disable Watch Dog, WDT_MR 
    rOut(rWDT_MR,1u<<15);

    /* Set 3 FWS for Embedded Flash Access */
    rOut(rEEFC_FMR,(0xFu<<8)& (3<<8));

    //=========== Init System Clock To 64MHZ
    // See pmc_init() of uTenux
    rOut(rCKGR_MOR,0x01370809);         /*crystal_init*/
    while(0u == (rIn(rPMC_SR)&0x01));	/*wait_stabilized*/
    rOut(rCKGR_PLLAR,0x200f0103);		/* set_pllar */
    while(0u == (rIn(rPMC_SR)&0x02));	/* set_pllar_delay */
    rOut(rPMC_MCKR,0x00000001);			/* set_mck */
    while(0u == (rIn(rPMC_SR)&0x08));	/* set_mckr_delay */
    rOut(rPMC_MCKR,0x00000002);			/* enable_plla */
    while(0u == (rIn(rPMC_SR)&0x08));	/* enable_plla_delay */

    //========== Uart0 initialize see uart_init() of uTenux
    rOut(rPIO_PDR,(0x01 << 9 | 0x01 << 10 ));  	/* set to peripheral mode for UART0 transmit/receive */
    rOut(rPMC_PCER,1u<<8);						/* UART0 clock enable */
    rOut(rUART0_CR,0xCu);						/* Asynchronous Mode,115200bps, 8bit, non-parity, 1 stop bit */
    rOut(rUART0_IDR,0xFFFFFFFF);
    rOut(rUART0_BRGR,0x22);						/* CD = MCLK/(baud*16) = 34.00(115200bps)   MCLK=64MHz */
    rOut(rUART0_MR,0x0800);
    rOut(rPERIPH_PTCR,0x0800);				    /* Disable DMA channel */
    rOut(rUART0_CR,0x50);						/* Enable receiver and transmitter */

    portStartSystemTimer();
}

IMPORT void knl_activate_r(void);
EXPORT void knl_setup_context(TaskType taskid)
{
	knl_tcb_sp[taskid] = knl_tcb_stack[taskid];
	knl_tcb_dispatcher[taskid] = knl_activate_r;
}

EXPORT void knl_force_dispatch(void)
{
	__asm("cpsie   i");
    __asm("svc     0");
}

//when task start to run
EXPORT void portActivateR(void)
{
    /* This is the most easiest Way to get Internal Resourse and
     * to make a task non-preemtable I think */
    GetInternalResource();
	ENABLE_INTERRUPT();
    knl_tcb_pc[knl_curtsk]();
}

//mean start the SystemTick ISR timer.
//here the test board I use has a 16 MHZ oscillator
//modify it if you have a different board and want a different
//SystemTick ISR frequency
LOCAL void portStartSystemTimer( void )
{
	rOut(rSysTickLoad,(CPU_FREQUENCY/1000)-1);	/* set reload register */
	portNVICSetPriority(-1,1u<<4-1);			/* set Priority for Cortex-M0 System Interrupts */
	rOut(rSysTickVal,0);
	rOut(rSysTickCtrl,(1u<<2)|(1u<<1)|(1u<<0)); /* Enable SysTick IRQ and SysTick Timer */
}

