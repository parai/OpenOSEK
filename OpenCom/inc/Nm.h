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
#include "Os.h"
#include "ComStack_Types.h"
#include "NM_Cfg.h"
/* ================================ MACROs    =============================== */
// Mask bit for ScalingParamType
// If 1 the function is supported, 0 not
#define NM_BusSleepSupport             0x01
#define NM_StatusDeltaSupport          0x02
#define NM_NormalConfigDeltaSupport    0x04
#define NM_LimphomeConfigDeltaSupport  0x08
#define NM_RingdataSupport             0x10

// Macros for OpCode
#define NM_MaskAlive   	 0x01
#define NM_MaskRing   	 0x02
#define NM_MaskLimphome  0x04
#define NM_MaskSI        0x10   // sleep ind
#define NM_MaskSA        0x20   // sleep ack

/* ================================ TYPEs     =============================== */
/* @ nm253.pdf 4.3 P89 */
typedef uint8 NodeIdType;
typedef uint8 NetIdType;
typedef void (*RoutineRefType)(NetIdType NetId);
// import EventMaskType from OS
typedef enum{
	SignalInvalid = 0,
	SignalActivation,
	SignalEvent
}SignallingMode;
// import StatusType / TaskRefType / TickType from OS
// -- Private Types
typedef enum
{
	NM_DIRECT,
	NM_INDIRECT
}NMType;
typedef uint8 ScalingParamType; // 8 bit is enough

/* @ nm253.pdf 4.4.2 P92 */
typedef uint8* ConfigRefType;
typedef enum
{
	NM_ckNormal,                  // supported by direct and indirect NM
	NM_ckNormalExtended,          // only supported by indirect NM
	NM_ckLimphome                 // only supported by direct NM
}ConfigKindName;
typedef ConfigRefType ConfigHandleType;
typedef union
{
	uint32 w;
	struct{
		unsigned NMactive:1;
		unsigned bussleep:1;
		unsigned configurationstable:1;
	}W;
}NetworkStatusType;

typedef NetworkStatusType* StatusRefType;
/* @ nm253.pdf 4.4.3.1 P98 */
typedef enum
{
	NM_BusSleep,
	NM_Awake
}NMModeName;
typedef enum
{
	NM_stOff,
	NM_stInit,
	NM_stInitReset,
	NM_stNormal,
	NM_stNormalPrepSleep,
	NM_stTwbsNormal,
	NM_stBusSleep,
	NM_stLimphome,
	NM_stLimphomePrepSleep,
	NM_stTwbsLimphome,
	NM_stOn // in fact if not Off then ON.
	// ...  and so on
}NMStateType;
typedef StatusRefType StatusHandleType;

/* @ nm253.pdf 4.4.5.3.1 P103 */
typedef uint8 RingDataType[6];
typedef RingDataType* RingDataRefType;

typedef struct
{
	uint8 Source;
	uint8 Destination;
	union
	{
		uint8 b;
		struct {
			uint8 Alive     :1;
			uint8 Ring      :1;
			uint8 Limphome  :1;
			uint8 reserved1 :1;
			uint8 SleepInd  :1;
			uint8 SleepAck  :1;
			uint8 reserved2 :2;
		}B;
	}OpCode;
	RingDataType RingData ;
}NMPduType;
/* ================================ DATAs     =============================== */

/* ================================ FUNCTIONs =============================== */
/* @ nm253.pdf 4.4.1.1 P89 System Generation Support */
IMPORT void InitNMType(NetIdType NetId,NMType nmType);
IMPORT void InitNMScaling(NetIdType NetId,ScalingParamType ScalingParams);
IMPORT void SelectHWRoutines(NetIdType NetId,RoutineRefType BusInit,RoutineRefType BusAwake,	\
				RoutineRefType BusSleep,RoutineRefType BusRestart,RoutineRefType BusShutdown);
IMPORT void InitCMaskTable(NetIdType NetId,ConfigKindName ConfigKind,ConfigRefType CMask);
IMPORT void InitTargetConfigTable(NetIdType NetId,ConfigKindName ConfigKind,ConfigRefType TargetConfig);
IMPORT void InitIndDeltaConfig(NetIdType NetId,ConfigKindName ConfigKind,SignallingMode SMode,	\
				TaskType TaskId,EventMaskType EMask);
IMPORT void InitSMaskTable(NetIdType NetId,StatusRefType SMask);
IMPORT void InitTargetStatusTable(NetIdType NetId,StatusRefType TargetStatus);
IMPORT void InitIndDeltaStatus(NetIdType NetId,SignallingMode SMode,TaskType TaskId,EventMaskType EMask);
/* @ nm253.pdf 4.4.2.3 P95 Services */
IMPORT StatusType InitConfig(NetIdType NetId);
IMPORT StatusType GetConfig(NetIdType NetId,ConfigRefType Config,ConfigKindName ConfigKind);
IMPORT StatusType CmpConfig(NetIdType NetId,ConfigRefType TestConfig,ConfigRefType RefConfig,ConfigRefType CMask);
IMPORT StatusType SelectDeltaConfig(NetIdType NetId,ConfigKindName ConfigKind,ConfigHandleType ConfigHandle,ConfigHandleType CMaskHandle);

IMPORT StatusType StartNM(NetIdType NetId);
IMPORT StatusType StopNM(NetIdType NetId);
IMPORT StatusType GotoMode(NetIdType NetId,NMModeName NewMode);
IMPORT StatusType GetStatus(NetIdType NetId,StatusRefType NetworkStatus);
IMPORT StatusType CmpStatus(NetIdType NetId,StatusRefType TestStatus,StatusRefType RefStatus,StatusRefType SMask);
IMPORT StatusType SelectDeltaStatus(NetIdType NetId,StatusHandleType StatusHandle,StatusHandleType SMaskHandle);

// FOR DIRECT NM
IMPORT void InitDirectNMParams(NetIdType NetId,NodeIdType NodeId,TickType TimerTyp,TickType TimerMax, 	\
								TickType TimerError,TickType TimerWaitBusSleep,TickType TimerTx);
IMPORT StatusType SilentNM(NetIdType);
IMPORT StatusType TalkNM(NetIdType);
IMPORT void InitIndRingData(NetIdType NetId,SignallingMode SMode,TaskType TaskId,EventMaskType EMask);
IMPORT void ReadRingData(NetIdType NetId,RingDataRefType RingData);
IMPORT void TransmitRingData(NetIdType NetId,RingDataRefType RingData);

// FOR INDIRECT NM
IMPORT void InitIndirectNMParams(NetIdType NetId,NodeIdType NodeId,TickType TOB,TickType TimerError,TickType TimerWaitBusSleep);
IMPORT void InitExtNodeMonitiring(NetIdType NetId,NodeIdType NodeId,uint8 DeltaInc,uint8 DeltaDec);


IMPORT void NM_TxConformation(NetIdType NetId);
IMPORT void NM_RxIndication(NetIdType NetId,NMPduType* NMPDU);
IMPORT void NM_WakeupIndication(NetIdType NetId);
EXPORT void NM_BusErrorIndication(NetIdType NetId);

IMPORT void NM_MainTask(void);
#endif
