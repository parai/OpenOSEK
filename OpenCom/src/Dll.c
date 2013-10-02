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
#include "Dll.h"
#include "Can.h"
/* ================================ MACROs    =============================== */

/* ================================ TYPEs     =============================== */
typedef struct
{
	uint32 WindowMask;
	uint32 IdBase;
	uint32 SourceId;
	uint8 DataLengthTx;
	uint8 DataLengthRx;
}DLL_NMWindowType;
/* ================================ DATAs     =============================== */
LOCAL DLL_NMWindowType DLL_NMWindow[cfgNM_NET_NUM];
LOCAL uint8 DLL_NetOnline[cfgNM_NET_NUM];
/* ================================ FUNCTIONs =============================== */

EXPORT void D_Init(NetIdType NetId,RoutineRefType InitRoutine)
{
	if(InitRoutine != NULL)
	{
		InitRoutine(NetId);
	}
}

EXPORT void D_DefineWindow(NetIdType NetId,uint32 WindowMask,uint32 IdBase,		\
			PduIdType SourceId,uint8 DataLengthTx,uint8 DataLengthRx)
{
	DLL_NMWindow[NetId].WindowMask = WindowMask;
	DLL_NMWindow[NetId].IdBase = IdBase;
	DLL_NMWindow[NetId].SourceId = SourceId; // TODO: ??
	DLL_NMWindow[NetId].DataLengthTx = DataLengthTx;
	DLL_NMWindow[NetId].DataLengthRx = DataLengthRx;
}

EXPORT void D_Online(NetIdType NetId)
{
	DLL_NetOnline[NetId] = TRUE;
}

EXPORT void D_Offline(NetIdType NetId)
{
	DLL_NetOnline[NetId] = FALSE;
}

EXPORT StatusType D_WindowDataReq(NetIdType NetId,NMPduType* NMPDU,uint8 DataLengthTx)
{
	StatusType ercd = E_OK;
	if(0 == NetId)
	{	// send on CAN_CTRL_0
		Can_PduType pdu;
		pdu.id = (NMPDU->Source+DLL_NMWindow[NetId].IdBase)&(DLL_NMWindow[NetId].WindowMask);
		pdu.length = DataLengthTx;
		pdu.swPduHandle = DLL_NMWindow[NetId].SourceId;
		pdu.sdu = &(NMPDU->Destination);
		ercd = Can_Write(CAN_CTRL_0,&pdu);
	}
	return ercd;
}


EXPORT void Can_TxConformation(PduIdType TxHandle)
{
	uint8 NetId;
	for(NetId=0;NetId<cfgNM_NET_NUM;NetId++)
	{
		if(DLL_NMWindow[NetId].SourceId == TxHandle)
		{
			NM_TxConformation(0);
		}
	}
}
