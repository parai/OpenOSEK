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
#include "Nm.h"
#include "Dll.h"
/* ================================ MACROs    =============================== */
#define rx_limit 4
#define tx_limit 8

// Alarm Management
#define nmSetAlarm(Timer)				\
do										\
{										\
	NM_ControlBlock[NetId].Alarm._##Timer = 1+NM_ControlBlock[NetId].nmDirectNMParams._##Timer;	\
}while(0)

// signal the alarm to process one step/tick forward
#define nmSingalAlarm(Timer)								\
do															\
{															\
	 if(NM_ControlBlock[NetId].Alarm._##Timer > 1)			\
	 {														\
		(NM_ControlBlock[NetId].Alarm._##Timer)--;			\
	 }														\
}while(0)

#define nmIsAlarmTimeout(Timer)  (1 == NM_ControlBlock[NetId].Alarm._##Timer)

#define nmIsAlarmStarted(Timer)	 (0 != NM_ControlBlock[NetId].Alarm._##Timer)

#define nmCancelAlarm(Timer)					\
do												\
{												\
	NM_ControlBlock[NetId].Alarm._##Timer = 0;	\
}while(0)

#define nmSendMessage()					\
do{										\
	StatusType ercd;					\
	ercd = D_WindowDataReq(NetId,&(NM_ControlBlock[NetId].nmTxPdu),8);		\
	if(ercd != E_OK)					\
	{									\
		nmSetAlarm(TTx); /* re-Transmit after TTx */						\
	}									\
}while(0)

#define nmDebug(str) //printf(str)
/* ================================ TYPEs     =============================== */
typedef struct
{
	NMType nmType;
	ScalingParamType nmScalingParams;
	struct
	{
		uint8 normal[32];
		uint8 normalExtended[32];
		uint8 limphome[32];
	}nmConfig;
	struct
	{
		uint8 normal[32];
		uint8 normalExtended[32];
		uint8 limphome[32];
	}nmCMask;
	struct
	{
		struct{
			SignallingMode SMode;
			TaskType TaskId;
			EventMaskType EMask;
		}normal;
		struct{
			SignallingMode SMode;
			TaskType TaskId;
			EventMaskType EMask;
		}normalExtended;
		struct{
			SignallingMode SMode;
			TaskType TaskId;
			EventMaskType EMask;
		}limphome;
	}nmIndDeltaConfig;
	struct{
		NetworkStatusType SMask;
		NetworkStatusType NetworkStatus;
		SignallingMode SMode;
		TaskType TaskId;
		EventMaskType EMask;
		union
		{
			uint32 w;
			struct
			{
				unsigned stable : 1;
			}W;
		}nmMerker; // TODO: what's the meaning?
	}nmStatus;
	NMStateType nmState;
	struct
	{
		NodeIdType NodeId;
		TickType _TTx;
		TickType _TTyp;
		TickType _TMax;
		TickType _TError;
		TickType _TWbs;
	}nmDirectNMParams;
	uint8 nmRxCount;
	uint8 nmTxCount;
	uint8 nmActive;
	struct{
		TickType _TTx;
		TickType _TTyp;
		TickType _TMax;
		TickType _TError;
		TickType _TWbs;
	}Alarm;
	NMPduType nmTxPdu;
}NM_ControlBlockType;
/* ================================ DATAs     =============================== */
EXPORT NM_ControlBlockType NM_ControlBlock[cfgNM_NET_NUM];
/* ================================ FUNCTIONs =============================== */
// See NM_Cfg.c
IMPORT void NMInit(NetIdType NetId);

LOCAL void nmInitReset(NetIdType NetId);
LOCAL void nmAddToConfig(NetIdType NetId,ConfigKindName ConfigKind,NodeIdType NodeId);
LOCAL void nmRemoveFromConfig(NetIdType NetId,ConfigKindName ConfigKind,NodeIdType NodeId);
LOCAL void nmNormalStandard(NetIdType NetId,NMPduType* NMPDU);
LOCAL NodeIdType nmDetermineLS(NodeIdType S,NodeIdType R,NodeIdType L);
LOCAL boolean nmIsMeSkipped(NodeIdType S,NodeIdType R,NodeIdType D);
LOCAL void nmBusSleep(NetIdType NetId);
LOCAL void nmNormalTwbsMain(NetIdType NetId);
LOCAL void nmNormalPrepSleepMain(NetIdType NetId);
LOCAL void nmLimphomeMain(NetIdType NetId);
LOCAL void nmNormalMain(NetIdType NetId);

EXPORT void InitNMType(NetIdType NetId,NMType nmType)
{
	NM_ControlBlock[NetId].nmType = nmType;
}

EXPORT void InitNMScaling(NetIdType NetId,ScalingParamType ScalingParams)
{
	NM_ControlBlock[NetId].nmScalingParams = ScalingParams;
}
EXPORT void InitCMaskTable(NetIdType NetId,ConfigKindName ConfigKind,ConfigRefType CMask)
{
	switch(ConfigKind)
	{
		case NM_ckNormal:
			(void)memcpy(NM_ControlBlock[NetId].nmCMask.normal,CMask,32);
			break;
		case NM_ckNormalExtended:
			(void)memcpy(NM_ControlBlock[NetId].nmCMask.normalExtended,CMask,32);
			break;
		case NM_ckLimphome:
			(void)memcpy(NM_ControlBlock[NetId].nmCMask.limphome,CMask,32);
			break;
		default:
			break;
	}
}

EXPORT void InitTargetConfigTable(NetIdType NetId,ConfigKindName ConfigKind,ConfigRefType TargetConfig)
{
	switch(ConfigKind)
	{
		case NM_ckNormal:
			(void)memcpy(NM_ControlBlock[NetId].nmConfig.normal,TargetConfig,32);
			break;
		case NM_ckNormalExtended:
			(void)memcpy(NM_ControlBlock[NetId].nmConfig.normalExtended,TargetConfig,32);
			break;
		case NM_ckLimphome:
			(void)memcpy(NM_ControlBlock[NetId].nmConfig.limphome,TargetConfig,32);
			break;
		default:
			break;
	}
}

EXPORT void InitIndDeltaConfig(NetIdType NetId,ConfigKindName ConfigKind,SignallingMode SMode,	\
				TaskType TaskId,EventMaskType EMask)
{
	switch(ConfigKind)
	{
		case NM_ckNormal:
			NM_ControlBlock[NetId].nmIndDeltaConfig.normal.SMode = SMode;
			NM_ControlBlock[NetId].nmIndDeltaConfig.normal.TaskId = TaskId;
			NM_ControlBlock[NetId].nmIndDeltaConfig.normal.EMask = EMask;
			break;
		case NM_ckNormalExtended:
			NM_ControlBlock[NetId].nmIndDeltaConfig.normalExtended.SMode = SMode;
			NM_ControlBlock[NetId].nmIndDeltaConfig.normalExtended.TaskId = TaskId;
			NM_ControlBlock[NetId].nmIndDeltaConfig.normalExtended.EMask = EMask;
			break;
		case NM_ckLimphome:
			NM_ControlBlock[NetId].nmIndDeltaConfig.limphome.SMode = SMode;
			NM_ControlBlock[NetId].nmIndDeltaConfig.limphome.TaskId = TaskId;
			NM_ControlBlock[NetId].nmIndDeltaConfig.limphome.EMask = EMask;
			break;
		default:
			break;
	}
}

EXPORT void InitSMaskTable(NetIdType NetId,StatusRefType SMask)
{
	NM_ControlBlock[NetId].nmStatus.SMask = *SMask;
}
EXPORT void InitTargetStatusTable(NetIdType NetId,StatusRefType TargetStatus)
{
	NM_ControlBlock[NetId].nmStatus.nmMerker.w = 0;
	NM_ControlBlock[NetId].nmStatus.NetworkStatus = *TargetStatus;
}
EXPORT void InitIndDeltaStatus(NetIdType NetId,SignallingMode SMode,TaskType TaskId,EventMaskType EMask)
{
	NM_ControlBlock[NetId].nmStatus.SMode = SMode;
	NM_ControlBlock[NetId].nmStatus.TaskId = TaskId;
	NM_ControlBlock[NetId].nmStatus.EMask = EMask;
}

EXPORT StatusType InitConfig(NetIdType NetId)
{

	return E_OK;
}
EXPORT StatusType GetConfig(NetIdType NetId,ConfigRefType Config,ConfigKindName ConfigKind)
{
	switch(ConfigKind)
	{
		case NM_ckNormal:
			(void)memcpy(Config,NM_ControlBlock[NetId].nmConfig.normal,32);
			break;
		case NM_ckNormalExtended:
			(void)memcpy(Config,NM_ControlBlock[NetId].nmConfig.normalExtended,32);
			break;
		case NM_ckLimphome:
			(void)memcpy(Config,NM_ControlBlock[NetId].nmConfig.limphome,32);
			break;
		default:
			break;
	}
	return E_OK;
}


EXPORT void InitDirectNMParams(NetIdType NetId,NodeIdType NodeId,TickType TimerTyp,TickType TimerMax, 	\
								TickType TimerError,TickType TimerWaitBusSleep,TickType TimerTx)
{
	NM_ControlBlock[NetId].nmDirectNMParams.NodeId = NodeId;
	NM_ControlBlock[NetId].nmDirectNMParams._TTyp = TimerTyp;
	NM_ControlBlock[NetId].nmDirectNMParams._TMax = TimerMax;
	NM_ControlBlock[NetId].nmDirectNMParams._TError = TimerError;
	NM_ControlBlock[NetId].nmDirectNMParams._TWbs = TimerWaitBusSleep;
	NM_ControlBlock[NetId].nmDirectNMParams._TTx = TimerTx;
}

// see nm253.pdf Figure 30 Start-up of the network
EXPORT StatusType StartNM(NetIdType NetId)
{
	StatusType ercd = E_OK;
	// step 1:
	NM_ControlBlock[NetId].nmTxPdu.OpCode.B.SleepInd = 0;
	NM_ControlBlock[NetId].nmTxPdu.OpCode.B.SleepAck = 0;
	// step 2: enter NMInit
	NM_ControlBlock[NetId].nmState= NM_stInit;
	NMInit(NetId);
	NM_ControlBlock[NetId].nmTxPdu.Source = NM_ControlBlock[NetId].nmDirectNMParams.NodeId;
	NM_ControlBlock[NetId].nmActive = TRUE;
	// step 3: config.limphome  := 0
	nmRemoveFromConfig(NetId,NM_ckLimphome,NM_ControlBlock[NetId].nmDirectNMParams.NodeId);
	// step 4:
	NM_ControlBlock[NetId].nmRxCount = 0;
	NM_ControlBlock[NetId].nmTxCount = 0;
	D_Online(NetId);
	nmInitReset(NetId);
	return ercd;
}

EXPORT StatusType GotoMode(NetIdType NetId,NMModeName NewMode)
{
	StatusType ercd = E_OK;
	if(NewMode == NM_BusSleep)
	{
		NM_ControlBlock[NetId].nmStatus.NetworkStatus.W.bussleep = 1;
		switch(NM_ControlBlock[NetId].nmState)
		{
			case NM_stNormal:
			{
				if(NM_ControlBlock[NetId].nmStatus.NetworkStatus.W.NMactive)
				{
					// NMNormal
				}
				else
				{
					nmDebug("GotoMode_BusSleep,Enter NormalPrepSleep.\n");
					NM_ControlBlock[NetId].nmState = NM_stNormalPrepSleep;
				}
				break;
			}
			case NM_stNormalPrepSleep:
				NM_ControlBlock[NetId].nmState = NM_stNormal;
				break;
			default:
				ercd = E_NOT_OK; // Wrong transition requirement during this state
				break;
		}
	}
	else if(NewMode == NM_Awake)
	{
		NM_ControlBlock[NetId].nmStatus.NetworkStatus.W.bussleep = 0;
		switch(NM_ControlBlock[NetId].nmState)
		{
			case NM_stTwbsNormal:
			{
				nmInitReset(NetId);
				break;
			}
			default:
				ercd = E_NOT_OK;
				break;
		}
	}

	return ercd;
}
EXPORT void NM_TxConformation(NetIdType NetId)
{
	NM_ControlBlock[NetId].nmTxCount = 0;
	switch(NM_ControlBlock[NetId].nmState)
	{
		case NM_stNormal:
		{
			if(NM_ControlBlock[NetId].nmTxPdu.OpCode.B.Ring)
			{
				nmCancelAlarm(TTyp);
				nmCancelAlarm(TMax);
				nmSetAlarm(TMax);
				nmDebug("TxConform,Cancel TTyp Set TMax.\n");
				if(NM_ControlBlock[NetId].nmTxPdu.OpCode.B.SleepInd)
				{
					if(NM_ControlBlock[NetId].nmStatus.NetworkStatus.W.bussleep)
					{
						NM_ControlBlock[NetId].nmTxPdu.OpCode.B.SleepAck = 1;
						NM_ControlBlock[NetId].nmState = NM_stNormalPrepSleep;
						nmDebug("sleep.ind=1,set sleep.ack=1 and enter NormalPrepSleep state.\n");
					}
				}
			}
			break;
		}
		case NM_stNormalPrepSleep:
		{	// 2 NMInitBusSleep
			if(NM_ControlBlock[NetId].nmTxPdu.OpCode.B.Ring)
			{
				D_Offline(NetId);
				nmSetAlarm(TWbs);
				NM_ControlBlock[NetId].nmState = NM_stTwbsNormal;
				nmDebug("sleep.ack=1, set TWbs and enter TwbsNormal state.\n");
			}
			break;
		}
		default:
			break;
	}
}
EXPORT void NM_RxIndication(NetIdType NetId,NMPduType* NMPDU)
{
//	printf("S=0x%x,D=0x%x,OpCode=0x%x\n",NMPDU->Source,NMPDU->Destination,NMPDU->OpCode.b);
	switch(NM_ControlBlock[NetId].nmState)
	{
		case NM_stNormal:
		case NM_stNormalPrepSleep:
		{
			nmNormalStandard(NetId,NMPDU);
			if(NMPDU->OpCode.B.SleepAck)
			{
				if(NM_ControlBlock[NetId].nmStatus.NetworkStatus.W.bussleep)
				{ // 2 NMInitBusSleep
					D_Offline(NetId);
					nmSetAlarm(TWbs);
					NM_ControlBlock[NetId].nmState = NM_stTwbsNormal;
					nmDebug("sleep.ack=1, set TWbs and enter TwbsNormal state.\n");
				}
			}
			// Special process for NM_stNormalPrepSleep only
			if(NM_stNormalPrepSleep == NM_ControlBlock[NetId].nmState)
			{
				if(NMPDU->OpCode.B.SleepInd)
				{
				}
				else
				{
					NM_ControlBlock[NetId].nmState = NM_stNormal;
				}
			}
			break;
		}
		case NM_stTwbsNormal:
		{
			if(NMPDU->OpCode.B.SleepInd)
			{  	// = 1
			}
			else
			{	// = 0
				nmCancelAlarm(TWbs);
				nmInitReset(NetId);
			}
			break;
		}
		default:
			break;
	}
}
EXPORT void NM_WakeupIndication(NetIdType NetId)
{
	if(NM_stBusSleep == NM_ControlBlock[NetId].nmState)
	{
		//OK Wakeup
		StartNM(NetId);
	}
}

LOCAL void nmInitReset(NetIdType NetId)
{
	NM_ControlBlock[NetId].nmState= NM_stInitReset;
	//config.present = own station
	nmAddToConfig(NetId,NM_ckNormal,NM_ControlBlock[NetId].nmDirectNMParams.NodeId);
	//logical successor := own station
	NM_ControlBlock[NetId].nmTxPdu.Destination = NM_ControlBlock[NetId].nmDirectNMParams.NodeId;
	NM_ControlBlock[NetId].nmRxCount += 1;
	// Initialise the NMPDU(Data,OpCode)
	NM_ControlBlock[NetId].nmTxPdu.OpCode.b = 0;
	memset(NM_ControlBlock[NetId].nmTxPdu.RingData,0,sizeof(RingDataType));
	// Cancel all Alarm
	nmCancelAlarm(TTx);
	nmCancelAlarm(TTyp);
	nmCancelAlarm(TMax);
	nmCancelAlarm(TWbs);
	nmCancelAlarm(TError);
	if(NM_ControlBlock[NetId].nmStatus.NetworkStatus.W.NMactive)
	{
		NM_ControlBlock[NetId].nmTxCount += 1;
		// Send A alive message
		NM_ControlBlock[NetId].nmTxPdu.OpCode.b = NM_MaskAlive;
		nmSendMessage();
		nmDebug("nmInitReset:Send Alive,");
	}
	if((NM_ControlBlock[NetId].nmTxCount <= tx_limit)	&&
		(NM_ControlBlock[NetId].nmRxCount <= rx_limit))
	{
		nmSetAlarm(TTyp);
		NM_ControlBlock[NetId].nmState = NM_stNormal;
		nmDebug("Set TTyp, enter Normal state.\n");
	}
	else
	{
		nmSetAlarm(TError);
		NM_ControlBlock[NetId].nmState = NM_stLimphome;
		nmDebug("Set TError, Enter Limphome state.\n");
	}
}
LOCAL void nmNormalStandard(NetIdType NetId,NMPduType* NMPDU)
{
	NM_ControlBlock[NetId].nmRxCount = 0;
	if(NMPDU->OpCode.B.Limphome)
	{
		// add sender to config.present
		nmAddToConfig(NetId,NM_ckLimphome,NMPDU->Source);
		nmRemoveFromConfig(NetId,NM_ckNormal,NMPDU->Source);
		nmDebug("A limphome message received!");
	}
	else
	{
		// add sender to config.present
		nmAddToConfig(NetId,NM_ckNormal,NMPDU->Source);
		nmRemoveFromConfig(NetId,NM_ckLimphome,NMPDU->Source);
		// determine logical successor
		NM_ControlBlock[NetId].nmTxPdu.Destination = nmDetermineLS(NMPDU->Source,	\
									NM_ControlBlock[NetId].nmDirectNMParams.NodeId,	\
										NM_ControlBlock[NetId].nmTxPdu.Destination);
		if(NMPDU->OpCode.B.Ring)
		{
			nmCancelAlarm(TTyp);
			nmCancelAlarm(TMax);
			nmDebug("RxIndication Ring,Cancel TTyp and TMax, ");
			if((NMPDU->Destination == NM_ControlBlock[NetId].nmDirectNMParams.NodeId) // to me
				|| (NMPDU->Destination == NMPDU->Source)) // or D = S
			{
				nmDebug("To me, Set TTyp.\n");
				nmSetAlarm(TTyp);
			}
			else
			{
				nmDebug("Not to me, Set TMax.\n");
				nmSetAlarm(TMax);
				if(nmIsMeSkipped(NMPDU->Source,NM_ControlBlock[NetId].nmDirectNMParams.NodeId,NMPDU->Destination))
				{
					nmDebug("Me is skipped, send Alive.\n");
					if(NM_ControlBlock[NetId].nmStatus.NetworkStatus.W.NMactive)
					{
						NM_ControlBlock[NetId].nmTxPdu.OpCode.b= NM_MaskAlive;
						NM_ControlBlock[NetId].nmTxPdu.Destination= NM_ControlBlock[NetId].nmDirectNMParams.NodeId;
						if(NM_ControlBlock[NetId].nmStatus.NetworkStatus.W.bussleep)
						{
							NM_ControlBlock[NetId].nmTxPdu.OpCode.B.SleepInd = 1;
						}
						nmSendMessage();
					}
				}
			}
		}
	}
}
LOCAL NodeIdType nmDetermineLS(NodeIdType S,NodeIdType R,NodeIdType L)
{
	NodeIdType newL = L;
	if(L==R)
	{
		newL = S;
	}
	else
	{
		if(L<R)
		{
			if(S<L)
			{
				newL = S; // SLR
			}
			else
			{
				if(S<R)
				{
					//LSR
				}
				else
				{
					newL = S; //LRS
				}
			}
		}
		else
		{
			if(S<L)
			{
				if(S<R)
				{
					//SRL
				}
				else
				{
					newL = S; // RSL
				}
			}
			else
			{
				//RLS
			}
		}
	}
	return newL;
}
LOCAL boolean nmIsMeSkipped(NodeIdType S,NodeIdType R,NodeIdType D)
{
	boolean isSkipped = FALSE;
	if(D<R)
	{
		if(S<D)
		{
			// not skipped //SDR
		}
		else
		{
			if(S<R)
			{
				isSkipped = TRUE; // DRS
			}
			else
			{
				// not skipped //DSR
			}
		}
	}
	else
	{
		if(S<D)
		{
			if(S<R)
			{
				isSkipped = TRUE; //SRD
			}
			else
			{
				//RSD
			}
		}
		else
		{
			isSkipped = TRUE; // RDS
		}
	}
	return isSkipped;
}
LOCAL void nmAddToConfig(NetIdType NetId,ConfigKindName ConfigKind,NodeIdType NodeId)
{
	ConfigRefType Config = NULL;
	switch(ConfigKind)
	{
		case NM_ckNormal:
			Config = NM_ControlBlock[NetId].nmConfig.normal;
			break;
		case NM_ckNormalExtended:
			Config = NM_ControlBlock[NetId].nmConfig.normalExtended;
			break;
		case NM_ckLimphome:
			Config = NM_ControlBlock[NetId].nmConfig.limphome;
			break;
		default:
			break;
	}
	if(NULL != Config)
	{
		uint8 byte    = NodeId/8;
		uint8 bit     = NodeId%8;
		Config[byte] |= (1u<<bit);
	}
}

LOCAL void nmRemoveFromConfig(NetIdType NetId,ConfigKindName ConfigKind,NodeIdType NodeId)
{
	ConfigRefType Config = NULL;
	switch(ConfigKind)
	{
		case NM_ckNormal:
			Config = NM_ControlBlock[NetId].nmConfig.normal;
			break;
		case NM_ckNormalExtended:
			Config = NM_ControlBlock[NetId].nmConfig.normalExtended;
			break;
		case NM_ckLimphome:
			Config = NM_ControlBlock[NetId].nmConfig.limphome;
			break;
		default:
			break;
	}
	if(NULL != Config)
	{
		uint8 byte    = NodeId/8;
		uint8 bit     = NodeId%8;
		Config[byte] &= (~(1u<<bit));
	}
}
LOCAL void nmNormalMain(NetIdType NetId)
{
	if(nmIsAlarmStarted(TTyp))
	{
		nmSingalAlarm(TTyp);
		if(nmIsAlarmTimeout(TTyp))
		{
			nmCancelAlarm(TTyp);
			nmCancelAlarm(TMax);
			nmSetAlarm(TMax);
			nmDebug("TTyp Timeout, Set TMax,");
			if(NM_ControlBlock[NetId].nmStatus.NetworkStatus.W.NMactive)
			{
				NM_ControlBlock[NetId].nmTxPdu.OpCode.b = NM_MaskRing;
				if(NM_ControlBlock[NetId].nmStatus.NetworkStatus.W.bussleep)
				{
					NM_ControlBlock[NetId].nmTxPdu.OpCode.B.SleepInd = 1;
				}
				NM_ControlBlock[NetId].nmTxCount ++;
				nmDebug("Send Ring.\n");
				nmSendMessage();
			}
			else
			{
				nmDebug("NMactive = False.\n");
			}
			if(NM_ControlBlock[NetId].nmTxCount > tx_limit)
			{
				nmDebug("TxCounter > tx_limit, enter Limphome state.Set TError.\n");
				NM_ControlBlock[NetId].nmState = NM_stLimphome;
				nmSetAlarm(TError);
			}
			else
			{
				if(NM_ControlBlock[NetId].nmStatus.nmMerker.W.stable)
				{
					NM_ControlBlock[NetId].nmStatus.NetworkStatus.W.configurationstable = 1;
				}
				else
				{
					NM_ControlBlock[NetId].nmStatus.nmMerker.W.stable = 1;
				}
			}
		}
	}

	if(nmIsAlarmStarted(TMax))
	{
		nmSingalAlarm(TMax);
		if(nmIsAlarmTimeout(TMax))
		{
			nmDebug("TMax Timeout, Do NmInitReset.\n");
			nmCancelAlarm(TMax);
			nmInitReset(NetId);
		}
	}
}
LOCAL void nmLimphomeMain(NetIdType NetId)
{
	if(nmIsAlarmStarted(TError))
	{
		nmSingalAlarm(TError);
		if(nmIsAlarmTimeout(TError))
		{
			D_Online(NetId);
			NM_ControlBlock[NetId].nmTxPdu.OpCode.b = NM_MaskLimphome;
			if(NM_ControlBlock[NetId].nmStatus.NetworkStatus.W.bussleep)
			{
				nmSetAlarm(TMax);
				NM_ControlBlock[NetId].nmTxPdu.OpCode.B.SleepInd =1;
				NM_ControlBlock[NetId].nmState = NM_stLimphomePrepSleep;
			}
			else
			{
				nmSetAlarm(TError);
			}
			if(NM_ControlBlock[NetId].nmStatus.NetworkStatus.W.NMactive)
			{
				nmSendMessage();
			}
		}
	}
}
LOCAL void nmNormalPrepSleepMain(NetIdType NetId)
{
	if(nmIsAlarmStarted(TTyp))
	{
		nmSingalAlarm(TTyp);
		if(nmIsAlarmTimeout(TTyp))
		{
			nmCancelAlarm(TTyp);
			if(NM_ControlBlock[NetId].nmStatus.NetworkStatus.W.NMactive)
			{
				// Send ring message with set sleep.ack bit
				NM_ControlBlock[NetId].nmTxPdu.OpCode.b = NM_MaskRing|NM_MaskSI|NM_MaskSA;
				nmSendMessage();
			}
		}
	}
}
LOCAL void nmBusSleep(NetIdType NetId)
{
	D_Init(NetId,BusSleep);
	NM_ControlBlock[NetId].nmState = NM_stBusSleep;
}
LOCAL void nmNormalTwbsMain(NetIdType NetId)
{
	if(nmIsAlarmStarted(TWbs))
	{
		nmSingalAlarm(TWbs);
		if(nmIsAlarmTimeout(TWbs))
		{
			nmCancelAlarm(TWbs);
			nmBusSleep(NetId);
		}
	}
}
EXPORT void NM_MainTask(void)
{
	NetIdType NetId;
	for(NetId= 0; NetId < cfgNM_NET_NUM; NetId ++)
	{
		if(nmIsAlarmStarted(TTx))
		{
			nmSingalAlarm(TTx);
			if(nmIsAlarmTimeout(TTx))
			{
				nmCancelAlarm(TTx);
				nmSendMessage();
				continue; // skip the process of state
			}
		}
		switch(NM_ControlBlock[NetId].nmState)
		{
			case NM_stNormal:
				nmNormalMain(NetId);
				break;
			case NM_stLimphome:
				nmLimphomeMain(NetId);
				break;
			case NM_stNormalPrepSleep:
				nmNormalPrepSleepMain(NetId);
				break;
			case NM_stTwbsNormal:
				nmNormalTwbsMain(NetId);
				break;
			default:
				break;
		}
	}
}
TASK(TaskNmMain)
{
	NM_MainTask();
	TerminateTask();
}
