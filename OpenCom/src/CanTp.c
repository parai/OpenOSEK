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
// Configuration Time(in ms)
// This CanTp is just for UDS only, and just has one Channel
#define N_As  1
#define N_Bs  50  // important
#define N_Cs  10
#define N_Ar  1
#define N_Br  49
#define N_Cr  50  // important
#define N_STmin 10
#define N_BS    8

//see ISO 15765-2 2004
#define N_PCI_MASK  0x30
#define N_PCI_SF    0x00
#define N_PCI_FF    0x10
#define N_PCI_CF    0x20
#define N_PCI_FC    0x00
#define N_PCI_SF_DL 0x07

//
/* ================================ TYPEs     =============================== */
typedef struct
{
	uint8 data[8];
	uint8 length;
	PduIdType handle;
}CanTp_QItemType;

typedef struct
{
	CanTp_QItemType queue[N_BS];
	uint8 counter; // must < N_BS
}CanTp_QueueType;

typedef struct
{
	CanTp_QueueType Q; // For Rx only
	PduLengthType index;
	TickType timer;
	PduIdType handle;
	volatile enum
	{
		CanTp_stIdle = 0,
		CanTp_stReceiving, // For Rx
		CanTp_stSenting,   // For Tx
		CanTp_stBusy       // To say the rx buffer of handle is in used by UDS,locked
	}state;
}CanTp_RTEType; // RTE

/* ================================ DATAs     =============================== */
LOCAL CanTp_RTEType CanTp_RTE;
/* ================================ FUNCTIONs =============================== */

LOCAL void CanTp_ReceivingMain(void);

EXPORT void CanTp_Init(void)
{
	uint8 i;
	memset(&CanTp_RTE,0,sizeof(CanTp_RTEType));
	CanTp_RTE.handle = INVALID_PDU_ID;
}
EXPORT void CanTp_ReleaseRxBuffer(PduIdType CanTpRxPduId)
{
	if((CanTp_stBusy == CanTp_RTE.state) && (CanTp_RTE.handle == CanTpRxPduId))
	{
		CanTp_RTE.state = CanTp_stIdle;
		CanTp_RTE.handle = INVALID_PDU_ID;
	}
}
IMPORT const Com_IPDUConfigType ComRxIPDUConfig[];
EXPORT void CanTp_RxIndication( PduIdType CanTpRxPduId, const PduInfoType *CanTpRxPduPtr )
{
	uint8 index = CanTp_RTE.Q.counter;
	if( index < N_BS)
	{
		memcpy(CanTp_RTE.Q.queue[index].data,CanTpRxPduPtr->SduDataPtr,CanTpRxPduPtr->SduLength);
		CanTp_RTE.Q.queue[index].length = CanTpRxPduPtr->SduLength;
		CanTp_RTE.Q.counter = index + 1;
	}
}
LOCAL void canTpReceiveSF(uint8 pos)
{
	uint8 length = CanTp_RTE.Q.queue[pos].data[0]&N_PCI_SF_DL;
	memcpy(ComRxIPDUConfig[CanTp_RTE.handle].pdu.SduDataPtr,&(CanTp_RTE.Q.queue[pos].data[1]),length);
	CanTp_RTE.state = CanTp_stBusy;
	Uds_RxIndication(CanTp_RTE.handle,length);
}
LOCAL void CanTp_ReceivingMain(void)
{
	uint8 i;
	for(i=0;(i<CanTp_RTE.Q.counter)&&(CanTp_stReceiving == CanTp_RTE.state);i++)
	{
		if(CanTp_RTE.handle != CanTp_RTE.Q.queue[i].handle)
		{ //TODO: this is a shit, I just want to implement an easy TP for UDS only.
		  // So Really Sorry. Only One RX PDU's size is allowed to be bigger than 8 bytes.
			CanTp_RTE.handle = CanTp_RTE.Q.queue[i].handle; // New Request
			CanTp_RTE.index = 0;
		}
		if( N_PCI_SF == (CanTp_RTE.Q.queue[i].data[i]&N_PCI_MASK))
		{
			canTpReceiveSF(i);
		}
	}
	if(i < CanTp_RTE.Q.counter)  // as in busy state
	{
		CanTp_RTE.Q.counter -= i;
		memcpy(CanTp_RTE.Q.queue,&(CanTp_RTE.Q.queue[i]),sizeof(CanTp_QItemType)*(CanTp_RTE.Q.counter));
	}
	else
	{
		CanTp_RTE.Q.counter = 0; // Empty it
	}
}
void CanTp_TaskMain(void)
{
	uint8 i;
	SuspendAllInterrupts();
	switch(CanTp_RTE.state)
	{
		case CanTp_stIdle:
		{
			if(0 != CanTp_RTE.Q.counter)  // has data in queue
			{
				CanTp_RTE.state = CanTp_stReceiving;
				CanTp_ReceivingMain();
			}
			break;
		}
		case CanTp_stReceiving:
		{
			CanTp_ReceivingMain();
			break;
		}
		case CanTp_stSenting:
		{
			break;
		}
		default:
			break;
	}
	ResumeAllInterrupts();
}
TASK(TaskCanTpMain)
{
	CanTp_TaskMain();
	TerminateTask();
}
