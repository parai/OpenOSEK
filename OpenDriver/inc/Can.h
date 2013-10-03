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
#ifndef CAN_H_H_H_H
#define CAN_H_H_H_H
/* ================================ INCLUDEs  =============================== */
#include "ComStack_Types.h"
/* ================================ MACROs    =============================== */

/* ================================ TYPEs     =============================== */
typedef enum {
	CAN_CTRL_0,
//	CAN_CTRL_1,
	//CAN_CTRL_2, If you have this comment it out.
	//CAN_CTRL_3,
	//CAN_CTRL_4,
	CAN_CONTROLLER_CNT
}Can_ControllerIdType;

typedef uint32 Can_IdType;
typedef uint16 Can_HwHandleType;
typedef struct{
	/* the CAN ID, 29 or 11-bit  */
	Can_IdType 	id;
	/* Length, max 8 bytes  */
	uint8		length;
	/* data ptr  */
	uint8 		*sdu;
	/* private data for CanIf,just save and use for callback */
	PduIdType   swPduHandle;
} Can_PduType;

typedef enum {
	CAN_OK,
	CAN_NOT_OK,
	CAN_BUSY
} Can_ReturnType;

typedef enum {
	CAN_T_START,
	CAN_T_STOP,
	CAN_T_SLEEP,
	CAN_T_WAKEUP
} Can_StateTransitionType;

/* ================================ DATAs     =============================== */

/* ================================ FUNCTIONs =============================== */
IMPORT void Can_Init(const void* Config);
IMPORT void Can_DeInit();
IMPORT void Can_InitController(uint8 Controller,const void* Config);
IMPORT Can_ReturnType Can_SetControllerMode(uint8 Controller,Can_StateTransitionType Transition);
IMPORT void Can_EnableControllerInterrupts(uint8 Controller);
IMPORT void Can_DisableControllerInterrupts(uint8 Controller);
IMPORT Can_ReturnType Can_CheckWakeup(uint8 Controller);
IMPORT Can_ReturnType Can_Write(Can_HwHandleType Hth,const Can_PduType* PduInfo);

#endif /* CAN_H_H_H_H */

