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
#include "stm32f10x.h"
/* ================================ MACROs    =============================== */
#define CPU_FREQUENCY 72000000
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
EXPORT uint8 knl_system_stack[1024];

/* ================================ FUNCTIONs =============================== */
LOCAL void portStartSystemTimer( void );

// for printf
EXPORT int putchar( int ch )
{
  /* Place your implementation of fputc here */
  /* e.g. write a character to the USART */
  USART_SendData(USART2, (uint8_t) ch);

  /* Loop until the end of transmission */
  while (USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET)
  {}

  return ch;
}

LOCAL void Usart_Init(void)
{
    USART_InitTypeDef USART_InitStructure;
    GPIO_InitTypeDef GPIO_InitStructure;
    
    /* USARTx configured as follow:
        - BaudRate = 115200 baud  
        - Word Length = 8 Bits
        - One Stop Bit
        - No parity
        - Hardware flow control disabled (RTS and CTS signals)
        - Receive and transmit enabled
    */
    USART_InitStructure.USART_BaudRate = 115200;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    
    /* Enable GPIO clock */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD | RCC_APB2Periph_AFIO, ENABLE);

    /* Enable the USART2 Pins Software Remapping */
    GPIO_PinRemapConfig(GPIO_Remap_USART2, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

    /* Configure USART Tx as alternate function push-pull */
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOD, &GPIO_InitStructure);

    /* Configure USART Rx as input floating */
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
    GPIO_Init(GPIOD, &GPIO_InitStructure);

    /* USART configuration */
    USART_Init(USART2, &USART_InitStructure);

    /* Enable USART */
    USART_Cmd(USART2, ENABLE);
}
EXPORT  void portOsStartupHook(void)
{
    Usart_Init();

    SysTick_Config(CPU_FREQUENCY / 1000);
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

