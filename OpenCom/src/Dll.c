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
#include "Com.h"

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
IMPORT const Com_IPDUConfigType ComRxIPDUConfig[];
IMPORT const Com_IPDUConfigType ComTxIPDUConfig[];
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


EXPORT void Can_TxConformation(Can_ControllerIdType Controller,PduIdType TxHandle)
{
	NetIdType NetId;
	if(CAN_CTRL_0 == Controller)
	{
		NetId = 0;
	}
	else
	{
		return;
	}
	if(DLL_NMWindow[NetId].SourceId == TxHandle)
	{
		NM_TxConformation(NetId);
	}
	else if(TRUE == DLL_NetOnline[NetId])
	{
		if((ComTxIPDUConfig[TxHandle].controller == Controller) && (ComTxIPDUConfig[TxHandle].pdu.SduLength > 8))
		{
			CanTp_TxConformation(TxHandle);
		}
		else
		{
			//Com
		}
	}
}

EXPORT void Can_RxIndication(Can_ControllerIdType Controller,Can_IdType canid,uint8* data,uint8 length)
{
	NetIdType NetId;
	if(CAN_CTRL_0 == Controller)
	{
		NetId = 0;
	}
	else
	{
		return;
	}

	if((DLL_NMWindow[NetId].IdBase <= canid) && ((DLL_NMWindow[NetId].IdBase+0xFF) >= canid))
	{	// This is NM message
		if(8 == length)
		{
			NMPduType nmPdu;
			uint8 i;
			nmPdu.Source = canid-DLL_NMWindow[NetId].IdBase;
			nmPdu.Destination = data[0];
			nmPdu.OpCode.b = data[1];
			for(i=0;i<6;i++)
			{
				nmPdu.RingData[i] = data[2+i];
			}
			NM_RxIndication(NetId,&nmPdu);
		}
	}
	else if(TRUE == DLL_NetOnline[NetId])
	{
		//may be for com or uds
		PduIdType i;
		PduInfoType pdu;
		for(i=0;i<cfgCOM_RxIPDU_NUM;i++)
		{
			if((ComRxIPDUConfig[i].id == canid) && (ComRxIPDUConfig[i].controller == Controller))
			{
				pdu.SduDataPtr = data;
				pdu.SduLength = length;
				if(ComRxIPDUConfig[i].pdu.SduLength > 8)
				{
					CanTp_RxIndication(i,&pdu);
				}
				else
				{
					//Com Rx
				}
			}
		}
	}
	else
	{
		// Offline
	}
}

EXPORT void Can_WakeupIndication(Can_ControllerIdType Controller)
{
	if(CAN_CTRL_0 == Controller)
	{
		NM_WakeupIndication(0);
	}
}
