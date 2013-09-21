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
#ifndef NM_H_H_H_H
#define NM_H_H_H_H
/* ================================ INCLUDEs  =============================== */
#include "Nm.h"
/* ================================ MACROs    =============================== */

/* ================================ TYPEs     =============================== */

/* ================================ DATAs     =============================== */

/* ================================ FUNCTIONs =============================== */
/* @ nm253.pdf 5.2.1 P108 :: Requirements of the data link layer */
IMPORT void D_Init(NetIdType NetId,RoutineRefType InitRoutine);
IMPORT void D_GetLayerStatus(NetIdType NetId,uint32* status);
IMPORT void D_Offline(NetIdType NetId);
IMPORT void D_Online(NetIdType NetId);
IMPORT void D_DefineWindow(NetIdType NetId,uint32 WindowMask,uint32 IdBase,		\
			PduIdType SourceId,uint8 DataLengthTx,uint8 DataLengthRx);
IMPORT void D_WindowDataReq(NetIdType NetId,uint8* NMPDU,uint8 DataLengthTx);

