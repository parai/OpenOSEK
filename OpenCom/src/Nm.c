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

LOCAL void NMInitReset(NetIdType NetId);
LOCAL void nmAddToConfig(NetIdType NetId,ConfigKindName ConfigKind,NodeIdType NodeId);
LOCAL void nmRemoveFromConfig(NetIdType NetId,ConfigKindName ConfigKind,NodeIdType NodeId);

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
	NMInitReset(NetId);
	return ercd;
}
LOCAL void NMInitReset(NetIdType NetId)
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
		StatusType ercd;

		NM_ControlBlock[NetId].nmTxCount += 1;
		// Send A alive message
		NM_ControlBlock[NetId].nmTxPdu.OpCode.b = NM_MaskAlive;
		ercd = D_WindowDataReq(NetId,&(NM_ControlBlock[NetId].nmTxPdu),8);
		if(ercd != E_OK)
		{
			nmSetAlarm(TTx); // re-Transmit after TTx
		}
	}
	if((NM_ControlBlock[NetId].nmTxCount <= tx_limit)	&&
		(NM_ControlBlock[NetId].nmRxCount <= rx_limit))
	{
		nmSetAlarm(TTyp);
		NM_ControlBlock[NetId].nmState = NM_stNormal;
	}
	else
	{
		nmSetAlarm(TError);
		NM_ControlBlock[NetId].nmState = NM_stLimphome;
	}
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
				if(NM_ControlBlock[NetId].nmTxPdu.OpCode.B.SleepInd)
				{
					if(NM_ControlBlock[NetId].nmStatus.NetworkStatus.W.bussleep)
					{
						NM_ControlBlock[NetId].nmTxPdu.OpCode.B.SleepAck = 1;
						NM_ControlBlock[NetId].nmState = NM_stNormalPrepSleep;
					}
				}
			}
			break;
		}
		default:
			break;
	}

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
			nmCancelAlarm(TMax);
			nmSetAlarm(TMax);
			if(NM_ControlBlock[NetId].nmStatus.NetworkStatus.W.NMactive)
			{
				StatusType ercd;
				NM_ControlBlock[NetId].nmTxPdu.OpCode.b = NM_MaskRing;
				if(NM_ControlBlock[NetId].nmStatus.NetworkStatus.W.bussleep)
				{
					NM_ControlBlock[NetId].nmTxPdu.OpCode.B.SleepInd = 1;
				}
				NM_ControlBlock[NetId].nmTxCount ++;
				ercd = D_WindowDataReq(NetId,&(NM_ControlBlock[NetId].nmTxPdu),8);
				if(ercd != E_OK)
				{
					nmSetAlarm(TTx); // re-Transmit after TTx
				}
			}
			if(NM_ControlBlock[NetId].nmTxCount > tx_limit)
			{
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
	else if(nmIsAlarmStarted(TMax))
	{
		nmSingalAlarm(TMax);
		if(nmIsAlarmTimeout(TMax))
		{
			NMInitReset(NetId);
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
				StatusType ercd;
				ercd = D_WindowDataReq(NetId,&(NM_ControlBlock[NetId].nmTxPdu),8);
				if(ercd != E_OK)
				{
					nmSetAlarm(TTx); // re-Transmit after TTx
				}
			}
		}
	}
}
TASK(TaskNmMain)
{
	NetIdType NetId;
	for(NetId= 0; NetId < cfgNM_NET_NUM; NetId ++)
	{
		switch(NM_ControlBlock[NetId].nmState)
		{
			case NM_stNormal:
				nmNormalMain(NetId);
				break;
			case NM_stLimphome:
				nmLimphomeMain(NetId);
				break;
			default:
				break;
		}

	}
	TerminateTask();
}
