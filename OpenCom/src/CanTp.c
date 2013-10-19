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
#define cfgCanTpMainTaskPeriod 10
#define msToCanTpTick(__ms) (__ms/cfgCanTpMainTaskPeriod)
// Configuration Time(in ms)
// This CanTp is just for UDS only, and just has one Channel
#define N_As  1
#define N_Bs  200 // important
#define N_Cs  xx   // same with the period of CanTpMainTask
#define N_Ar  1
#define N_Br  xx
#define N_Cr  200  // important
#define N_STmin 10
#define N_BS    8

//see ISO 15765-2 2004
#define N_PCI_MASK  0x30
#define N_PCI_SF    0x00
#define N_PCI_FF    0x10
#define N_PCI_CF    0x20
#define N_PCI_FC    0x30
#define N_PCI_SF_DL 0x07
//Flow Control Status Mask
#define N_PCI_FS    0x0F
//Flow Control Status
#define N_PCI_CTS   0x00
#define N_PCI_WT    0x01	// TODO: not supported for receiving
#define N_PCI_OVFLW 0x02

#define N_PCI_SN   0x0F

#define N_SF_MAX_LENGTH   7

#define tpSetAlarm(__tp,__tick)			\
do{										\
	cantpRte[__tp].timer = __tick + 1;	\
}while(0)
#define tpSignalAlarm(__tp,__tick)		\
do{										\
	if(cantpRte[__tp].timer > 1)		\
	{									\
		cantpRte[__tp].timer --;		\
	}									\
}while(0)
#define tpCancelAlarm(__tp)	{ cantpRte[__tp].timer = 0;}
#define tpIsAlarmTimeout(__tp) ( 1u == cantpRte[__tp].timer )
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
	PduLengthType index;  // For Rx and Tx
	PduLengthType length; // For Rx and Tx
	TickType timer;
	uint8 BS;		//Block Size
	uint8 SN;		//Sequence Number
	volatile enum
	{
		CanTp_stIdle = 0,
		CanTp_stReceiving,     // For Rx
		CanTp_stSentFC,
		CanTp_stBusy,          // To say the rx buffer of handle is in used by UDS,locked
		CanTp_stStartToSent,   // For Tx
		CanTp_stSenting,
		CanTp_stSentFinished,  // Wait For TxConform
	}state;
}cantpRteType; // RTE

/* ================================ DATAs     =============================== */
IMPORT const Com_IPDUConfigType ComRxIPDUConfig[];
IMPORT const Com_IPDUConfigType ComTxIPDUConfig[];
LOCAL cantpRteType cantpRte[cfgCOM_TPIPDU_NUM];
/* ================================ FUNCTIONs =============================== */
LOCAL void canTpReceiveSF(PduIdType RxPduId,uint8 pos);
LOCAL void CanTp_ReceivingMain(PduIdType RxPduId);
LOCAL void canTpSendSF(PduIdType RxPduId);
LOCAL void canTpSentFC(PduIdType RxPduId);
LOCAL void canTpReceiveFF(PduIdType RxPduId,uint8 pos);
LOCAL void CanTp_StartToSendMain(PduIdType TxPduId);

EXPORT void CanTp_Init(void)
{
	memset(&cantpRte,0,sizeof(cantpRte));
}
EXPORT void CanTp_ReleaseRxBuffer(PduIdType RxPduId)
{
	if(CanTp_stBusy == cantpRte[RxPduId].state)
	{
		cantpRte[RxPduId].state = CanTp_stIdle;
	}
	else
	{
		devTrace(tlError,"Error In CanTp_ReleaseRxBuffer state[%d] = %d.\n",RxPduId,cantpRte[RxPduId].state);
	}
}
EXPORT void CanTp_TxConformation(PduIdType TxPduId)
{
	if(CanTp_stSentFinished == cantpRte[TxPduId].state )
	{
		cantpRte[TxPduId].state = CanTp_stIdle;
		Uds_TxConformation(TxPduId);
	}
	else
	{
		devTrace(tlError,"Error In CanTp_TxConformation state[%d] = %d\n",TxPduId,cantpRte[TxPduId].state);
	}
}
EXPORT void CanTp_RxIndication( PduIdType RxPduId, const PduInfoType *CanTpRxPduPtr )
{
	uint8 index = cantpRte[RxPduId].Q.counter;
	if( index < N_BS)
	{
		memcpy(cantpRte[RxPduId].Q.queue[index].data,CanTpRxPduPtr->SduDataPtr,CanTpRxPduPtr->SduLength);
		cantpRte[RxPduId].Q.queue[index].length = CanTpRxPduPtr->SduLength;
		cantpRte[RxPduId].Q.counter = index + 1;
	}
}

EXPORT Std_ReturnType CanTp_Transmit( PduIdType TxSduId, PduLengthType Length)
{
	Std_ReturnType ercd = E_OK;
	if(CanTp_stBusy == cantpRte[TxSduId].state)
	{
		cantpRte[TxSduId].state = CanTp_stStartToSent;
		cantpRte[TxSduId].length = Length;
	}
	else
	{
		ercd = E_NOT_OK;
	}
	return ercd;
}

LOCAL void canTpReceiveSF(PduIdType RxPduId,uint8 pos)
{
	uint8 length = cantpRte[RxPduId].Q.queue[pos].data[0]&N_PCI_SF_DL;
	memcpy(ComRxIPDUConfig[RxPduId].pdu.SduDataPtr,&(cantpRte[RxPduId].Q.queue[pos].data[1]),length);
	cantpRte[RxPduId].state = CanTp_stBusy;
	Uds_RxIndication(RxPduId,length);
}

LOCAL void canTpSentFC(PduIdType RxPduId)
{
	Can_ReturnType ercd;
	Can_PduType pdu;
	uint8 data[8];
	pdu.id  = ComTxIPDUConfig[RxPduId].id;
	if(cantpRte[RxPduId].length < ComTxIPDUConfig[RxPduId].pdu.SduLength)
	{
		data[0] = N_PCI_FC|N_PCI_CTS;
	}
	data[1] = N_BS;
	data[2] = N_STmin;
	pdu.sdu = data;
	pdu.length = 3;
	pdu.swPduHandle = RxPduId;

	// Note, Can_Write will push the data to Transmit CAN message box, and return.
	// It always do a Can_TxConformation after the return from Can_Write,
	// otherwise, Your CAN controller speed is too fast.
	ercd = Can_Write(ComTxIPDUConfig[RxPduId].controller,&pdu);
	if(CAN_OK == ercd)
	{
		cantpRte[RxPduId].state = CanTp_stReceiving;
	}
	else
	{
		cantpRte[RxPduId].state = CanTp_stSentFC;
	}
}

LOCAL void canTpReceiveFF(PduIdType RxPduId,uint8 pos)
{
	PduLengthType length = cantpRte[RxPduId].Q.queue[pos].data[0]&0x0F;
	length += (length << 8) + cantpRte[RxPduId].Q.queue[pos].data[1];
	cantpRte[RxPduId].length = length;
	memcpy(ComRxIPDUConfig[RxPduId].pdu.SduDataPtr,&(cantpRte[RxPduId].Q.queue[pos].data[2]),6);
	cantpRte[RxPduId].index = 6; // 6 bytes already received by FF
	canTpSentFC(RxPduId);
	cantpRte[RxPduId].BS = N_BS;
	cantpRte[RxPduId].SN = 1;
}

LOCAL void canTpReceiveCF(PduIdType RxPduId,uint8 pos)
{
	if(cantpRte[RxPduId].SN == (cantpRte[RxPduId].Q.queue[pos].data[0]&N_PCI_SN))
	{
		uint8 size;
		cantpRte[RxPduId].SN ++ ;
		if(cantpRte[RxPduId].SN > 15) { cantpRte[RxPduId].SN = 0; }
		size = cantpRte[RxPduId].length - cantpRte[RxPduId].index;
		if( size > 7 ) { size = 7; }
		memcpy(ComRxIPDUConfig[RxPduId].pdu.SduDataPtr + cantpRte[RxPduId].index,
				&(cantpRte[RxPduId].Q.queue[pos].data[1]),size);
		cantpRte[RxPduId].index += size;
		if(cantpRte[RxPduId].index >= cantpRte[RxPduId].length)
		{
			cantpRte[RxPduId].state = CanTp_stBusy;
			Uds_RxIndication(RxPduId,cantpRte[RxPduId].length);
		}
		else
		{
			cantpRte[RxPduId].BS --;
			if(0 == cantpRte[RxPduId].BS)
			{
				canTpSentFC(RxPduId);
			}
		}
	}
	else
	{	// Sequence Number Wrong
		cantpRte[RxPduId].state = CanTp_stIdle;
		devTrace(tlError,"ERROR: CanTp[%d] Sequence Number Wrong,Abort Current Receiving.",RxPduId);
	}
}

LOCAL void CanTp_ReceivingMain(PduIdType RxPduId)
{
	uint8 i;
	for(i=0;(i<cantpRte[RxPduId].Q.counter)&&(CanTp_stReceiving == cantpRte[RxPduId].state);i++)
	{
		if( N_PCI_SF == (cantpRte[RxPduId].Q.queue[i].data[0]&N_PCI_MASK))
		{
			canTpReceiveSF(RxPduId,i);
		}
		else if(N_PCI_FF == (cantpRte[RxPduId].Q.queue[i].data[0]&N_PCI_MASK))
		{
			canTpReceiveFF(RxPduId,i);
		}
		else if(N_PCI_CF == (cantpRte[RxPduId].Q.queue[i].data[0]&N_PCI_MASK))
		{
			canTpReceiveCF(RxPduId,i);
		}
	}
	if(i < cantpRte[RxPduId].Q.counter)  // as in busy state
	{
		cantpRte[RxPduId].Q.counter -= i;
		memcpy(cantpRte[RxPduId].Q.queue,&(cantpRte[RxPduId].Q.queue[i]),sizeof(CanTp_QItemType)*(cantpRte[RxPduId].Q.counter));
		devTrace(tlCanTp,'Info:CanTp[%d] Q is not empty but transit to state Busy.\n',RxPduId);
	}
	else
	{
		cantpRte[RxPduId].Q.counter = 0; // Empty it
	}
}
LOCAL void canTpSendSF(PduIdType TxPduId)
{
	Can_ReturnType ercd;
	Can_PduType pdu;
	uint8 data[8];
	uint8 i;
	pdu.id  = ComTxIPDUConfig[TxPduId].id;
	data[0] = N_PCI_SF|cantpRte[TxPduId].length;
	for(i=0;i<cantpRte[TxPduId].length;i++)
	{
		data[1+i] = ComTxIPDUConfig[TxPduId].pdu.SduDataPtr[i];
	}
	pdu.sdu = data;
	pdu.length = cantpRte[TxPduId].length+1;
	pdu.swPduHandle = TxPduId;

	// Note, Can_Write will push the data to Transmit CAN message box, and return.
	// It always do a Can_TxConformation after the return from Can_Write,
	// otherwise, Your CAN controller speed is too fast.
	ercd = Can_Write(ComTxIPDUConfig[TxPduId].controller,&pdu);
	if(CAN_OK == ercd)
	{
		cantpRte[TxPduId].state = CanTp_stSentFinished;
	}
	else
	{
		// Failed, Redo
	}
}
LOCAL void CanTp_StartToSendMain(PduIdType TxPduId)
{
	if(cantpRte[TxPduId].length <= N_SF_MAX_LENGTH)
	{
		canTpSendSF(TxPduId);
	}
}
void CanTp_TaskMain(void)
{
	uint8 i;
	SuspendAllInterrupts();
	for(i=0;i<cfgCOM_TPIPDU_NUM;i++)
	{
		switch(cantpRte[i].state)
		{
			case CanTp_stIdle:
			{
				if(0 != cantpRte[i].Q.counter)  // has data in queue
				{
					cantpRte[i].state = CanTp_stReceiving;
					CanTp_ReceivingMain(i);
				}
				break;
			}
			case CanTp_stReceiving:
			{
				CanTp_ReceivingMain(i);
				break;
			}
			case CanTp_stSentFC:
			{
				canTpSentFC(i);
				break;
			}
			case CanTp_stStartToSent:
			{
				CanTp_StartToSendMain(i);
				break;
			}
			default:
				break;
		}
	}
	ResumeAllInterrupts();
}
TASK(TaskCanTpMain)
{
	CanTp_TaskMain();
	TerminateTask();
}
