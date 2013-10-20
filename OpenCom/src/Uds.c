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
// Wa haha, something copy from Arccore.

/* ================================ MACROs    =============================== */
#define cfgUDS_Q_NUM 2
#define cfgUDS_Q_TIMEOUT 100  // ms
#define cfgUdsMainTaskTick   10 //ms
#define msToUdsTick(time) ((time + cfgUdsMainTaskTick - 1)/cfgUdsMainTaskTick)

#define udsGetSerivceData(__i) (ComRxIPDUConfig[udsRte.pduId].pdu.SduDataPtr[__i])
#define udsSetResponseCode(__i,__v) {ComTxIPDUConfig[udsRte.pduId].pdu.SduDataPtr[__i] = (uint8)(__v);}
#define udsGetResponseBuffer(__i) (&ComTxIPDUConfig[udsRte.pduId].pdu.SduDataPtr[__i])

#define cfgP2Server   5000  //ms

#define udsSetAlarm(__tick)			\
do{										\
	udsRte.timer = __tick + 1;	\
}while(0)
#define udsSignalAlarm()				\
do{										\
	if(udsRte.timer > 1)		\
	{									\
		udsRte.timer --;		\
	}									\
}while(0)
#define udsCancelAlarm()	{ udsRte.timer = 0;}
#define udsIsAlarmTimeout() ( 1u == udsRte.timer )
#define udsIsAlarmStarted() ( 0u != udsRte.timer )

#define UDS_E_GENERALREJECT								((Uds_NrcType)0x10)
#define UDS_E_BUSYREPEATREQUEST							((Uds_NrcType)0x21)
#define UDS_E_CONDITIONSNOTCORRECT						((Uds_NrcType)0x22)
#define UDS_E_REQUESTSEQUENCEERROR						((Uds_NrcType)0x24)
#define UDS_E_REQUESTOUTOFRANGE							((Uds_NrcType)0x31)
#define UDS_E_SECUTITYACCESSDENIED						((Uds_NrcType)0x33)
#define UDS_E_GENERALPROGRAMMINGFAILURE					((Uds_NrcType)0x72)
#define UDS_E_SUBFUNCTIONNOTSUPPORTEDINACTIVESESSION	((Uds_NrcType)0x7E)
#define UDS_E_RPMTOOHIGH								((Uds_NrcType)0x81)
#define UDS_E_RPMTOLOW									((Uds_NrcType)0x82)
#define UDS_E_ENGINEISRUNNING							((Uds_NrcType)0x83)
#define UDS_E_ENGINEISNOTRUNNING						((Uds_NrcType)0x84)
#define UDS_E_ENGINERUNTIMETOOLOW						((Uds_NrcType)0x85)
#define UDS_E_TEMPERATURETOOHIGH						((Uds_NrcType)0x86)
#define UDS_E_TEMPERATURETOOLOW							((Uds_NrcType)0x87)
#define UDS_E_VEHICLESPEEDTOOHIGH						((Uds_NrcType)0x88)
#define UDS_E_VEHICLESPEEDTOOLOW						((Uds_NrcType)0x89)
#define UDS_E_THROTTLE_PEDALTOOHIGH						((Uds_NrcType)0x8A)
#define UDS_E_THROTTLE_PEDALTOOLOW						((Uds_NrcType)0x8B)
#define UDS_E_TRANSMISSIONRANGENOTINNEUTRAL				((Uds_NrcType)0x8C)
#define UDS_E_TRANSMISSIONRANGENOTINGEAR				((Uds_NrcType)0x8D)
#define UDS_E_BRAKESWITCH_NOTCLOSED						((Uds_NrcType)0x8F)
#define UDS_E_SHIFTERLEVERNOTINPARK						((Uds_NrcType)0x90)
#define UDS_E_TORQUECONVERTERCLUTCHLOCKED				((Uds_NrcType)0x91)
#define UDS_E_VOLTAGETOOHIGH							((Uds_NrcType)0x92)
#define UDS_E_VOLTAGETOOLOW								((Uds_NrcType)0x93)

#define UDS_E_POSITIVERESPONSE							((Uds_NrcType)0x00)
#define UDS_E_SERVICENOTSUPPORTED						((Uds_NrcType)0x11)
#define UDS_E_SUBFUNCTIONNOTSUPPORTED					((Uds_NrcType)0x12)
#define UDS_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT		((Uds_NrcType)0x13)
#define UDS_E_RESPONSEPENDING							((Uds_NrcType)0x78)
#define UDS_E_SERVICENOTSUPPORTEDINACTIVESESSION		((Uds_NrcType)0x7F)

// Misc definitions
#define SUPPRESS_POS_RESP_BIT		((uint8)0x80)
#define SID_RESPONSE_BIT			((uint8)0x40)
#define VALUE_IS_NOT_USED			((uint8)0x00)

/* ================================ TYPEs     =============================== */
typedef struct
{
	PduIdType     pduId;
	PduLengthType rxLength;
	TickType      timer; // For Timeout monitor
}Uds_RequestQItemType;
typedef struct
{
	Uds_RequestQItemType queue[cfgUDS_Q_NUM];
	uint8 counter;
}Uds_RequestQueueType;
typedef struct
{
	PduIdType         pduId;   		/* used to refer the request service data */
	PduLengthType     rxLength;		/* record the length of the PDU <pduId> */
	PduLengthType     txLength;		/* record the length of the response for the service <pduId> */
	Uds_SessionType   session;		/* record current session */
	Uds_ServiceIdType currentSid;	/* record current service id */
	uint8             sidIndex;		/* sid index to refer UdsConfig.sidList[] */
	Uds_SecurityLevelMaskType securityLevel;	/* record current unsecured level, each bit is a special level.*/
	boolean           suppressPosRspMsg;		/* TODO: */
	TickType      	  timer;		/* Timer used for error recover or P2Server */
	volatile enum
	{
		Uds_stIdle = 0,
		Uds_stServiceRequested,
		Uds_stServiceInProcess,
		Uds_stSentingResponse,
		Uds_stServiceFinshed
	}state;						/* UDS State Machine */
	// Uds_RequestQueueType Q;		/* TODO: Queue to cache the consecutive request service in case that UDS State is not Idle */
}Uds_RTEType;
/* ================================ DATAs     =============================== */
IMPORT const Com_IPDUConfigType ComRxIPDUConfig[];
IMPORT const Com_IPDUConfigType ComTxIPDUConfig[];
IMPORT const Uds_ConfigType UdsConfig;
LOCAL Uds_RTEType udsRte;

/* ================================ FUNCTIONs =============================== */
LOCAL void udsHandleServiceRequest(void);
LOCAL void udsCreateAndSendNrc(Uds_NrcType Nrc);
LOCAL void udsProcessingDone(Uds_NrcType Ncr);
LOCAL boolean udsLookupSid(void);
LOCAL boolean udsCheckSessionLevel(void);
LOCAL boolean udsIsNewSessionValid(Uds_SessionType Session);
LOCAL Uds_SessionType udsSessionMap(Uds_SessionType Session);
LOCAL void udsSessionControlFnc(void);
LOCAL void udsSecurityAccessFnc(void);
LOCAL void udsSelectServiceFunction(void);
LOCAL uint8 udsPrepareSeed(uint8* seed);

EXPORT void Uds_Init(void)
{
	memset(&udsRte,0,sizeof(udsRte));
}
EXPORT void Uds_TxConformation(PduIdType RxPduId,StatusType status)
{
	if((Uds_stSentingResponse == udsRte.state) && (E_OK == status))
	{
		udsRte.state = Uds_stIdle;
	}
	else
	{
		devTrace(tlError,"Error In Uds_TxConformation state = %d\n",udsRte.state);// UDS state machine error
		Uds_Init();  // Reset as Error
	}
}
EXPORT void Uds_RxIndication(PduIdType RxPduId,PduLengthType Length)
{
	if(Uds_stIdle == udsRte.state)
	{
		udsRte.state = Uds_stServiceRequested;
		udsRte.pduId = RxPduId;
		udsRte.rxLength = Length;
	}
	else
	{
		// TODO:
//		if(udsRte.Q.counter < cfgUDS_Q_NUM)
//		{
//			udsRte.Q.queue[udsRte.Q.counter].pduId = RxPduId;
//			udsRte.Q.queue[udsRte.Q.counter].rxLength = Length;
//			udsRte.Q.queue[udsRte.Q.counter].timer  = msToUdsTick(cfgUDS_Q_TIMEOUT);
//			udsRte.Q.counter ++;
//		}
	}
}
LOCAL void udsSendResponse(boolean send)
{
	if(TRUE == send)
	{
		CanTp_Transmit(udsRte.pduId,udsRte.txLength);
		udsRte.state = Uds_stSentingResponse;
	}
	else
	{
		CanTp_ReleaseRxBuffer(udsRte.pduId);
		udsRte.state = Uds_stIdle;  // Goto Idle as Error.
	}
}
LOCAL void udsCreateAndSendNrc(Uds_NrcType Nrc)
{
	if ( (Nrc == UDS_E_SERVICENOTSUPPORTED) ||
		 (Nrc == UDS_E_SUBFUNCTIONNOTSUPPORTED) ||
		 (Nrc == UDS_E_REQUESTOUTOFRANGE) ) {   /** @req DCM001 */
		udsSetResponseCode(0,SID_NEGATIVE_RESPONSE);
		udsSetResponseCode(1,udsRte.currentSid);
		udsSetResponseCode(2,Nrc);
		udsRte.txLength = 3;
		udsSendResponse(TRUE);
	}
	else {
		devTrace(tlUds,"Goto IDLE as Error %x but without response.\n",Nrc);
		udsSendResponse(FALSE);
	}
}
LOCAL void udsProcessingDone(Uds_NrcType Ncr)
{
	if (UDS_E_POSITIVERESPONSE == Ncr) {
		if (False == udsRte.suppressPosRspMsg) {	/** @req DCM200 */ /** @req DCM231 */
			/** @req DCM222 */
			udsSetResponseCode(0,udsRte.currentSid | SID_RESPONSE_BIT)
			udsSendResponse(True);
		}
		else {
			udsSendResponse(False);
		}
	}
	else {
		udsCreateAndSendNrc(Ncr);	/** @req DCM228 */
	}
}
LOCAL boolean udsLookupSid(void)
{
	uint8 i;
	boolean find = FALSE;
	for(i=0;i<UdsConfig.sidNbr;i++)
	{
		if(udsRte.currentSid == UdsConfig.sidList[i].sid)
		{
			find = TRUE;
			udsRte.sidIndex = i;
			break;
		}
	}
	return find;
}
LOCAL boolean udsCheckSessionLevel(void)
{
	if( (UdsConfig.sidList[udsRte.sidIndex].sessionMask&(1<<udsRte.session)) != 0u )
	{
		return True;
	}
	else
	{
		return False;
	}
}
LOCAL boolean udsCheckSecurityLevel(void)
{
	if(0u == UdsConfig.sidList[udsRte.sidIndex].securityLevelMask)
	{	// This sid is unsecured
		return True;
	}
	else if( (UdsConfig.sidList[udsRte.sidIndex].securityLevelMask&(udsRte.securityLevel)) != 0u )
	{
		return True;
	}
	else
	{
		return False;
	}
}
LOCAL boolean udsIsNewSessionValid(Uds_SessionType Session)
{
	boolean isValid = False;
	if(0u == Session)
	{	// ISOSAEReserved
		// False
	}
	else if(Session <= 4)
	{   // 01 - 04
		isValid = True;
	}
	else if(Session <= 0x3Fu)
	{	// ISOSAEReserved
		// False
	}
	else if(Session <= 0x5F)
	{	// vehicleManufacturerSpecific
		// TODO: Add Special process of your session in this range, see the Session Mask List
	}
	else if(Session <= 0x7E)
	{	// systemSupplierSpecific
		// TODO: Add Special process of your session in this range, see the Session Mask List
	}
	return isValid;
}
// Parm: Session is the value from UDS request message
LOCAL Uds_SessionType udsSessionMap(Uds_SessionType Session)
{
	Uds_SessionType newS;
	if(0u == Session)
	{	// ISOSAEReserved
		// False
	}
	else if(Session <= 4)
	{   // 01 - 04
		newS = Session -1;
	}
	else if(Session <= 0x3Fu)
	{	// ISOSAEReserved
		// False
	}
	else if(Session <= 0x5F)
	{	// vehicleManufacturerSpecific
		// TODO: Add Special process of your session in this range, see the Session Mask List
	}
	else if(Session <= 0x7E)
	{	// systemSupplierSpecific
		// TODO: Add Special process of your session in this range, see the Session Mask List
	}
	return newS;
}
/* @UDS Service: 0x10 */
LOCAL void udsSessionControlFnc(void)
{
	if(2u == udsRte.rxLength)
	{
		Uds_SessionType session = udsGetSerivceData(1);
		devTrace(tlUds,"Info:Session Control %2x.\n",session);
		if(True == udsIsNewSessionValid(session))
		{
			// TODO: here should askApplicationForSessionPermission
			udsRte.session = udsSessionMap(session);
			// Create Response
			udsSetResponseCode(1,session);
			udsRte.txLength = 2;
			udsProcessingDone(UDS_E_POSITIVERESPONSE);
		}
		else
		{
			udsProcessingDone(UDS_E_SUBFUNCTIONNOTSUPPORTED);
		}
	}
	else
	{
		udsProcessingDone(UDS_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT);
	}
}
/* This Service is an example, it should be re-implemented
 * according to the requirement of Application
 * @In: seed buffer
 * @Return: the length of the seed */
LOCAL uint8 udsPrepareSeed(uint8* seed)
{
#if 0
	seed[0] = 0xDE; // Example key 0xDEADBEEF
	seed[1] = 0xAD;
	seed[2] = 0xBE;
	seed[3] = 0xEF;
 	return 4;
#else
 	int i;
 	for(i=0;i<128;i++)
 	{
 		seed[i] = i;
 	}
 	return 128;
#endif
}

/* @UDS Service: 0x27 */
LOCAL void udsSecurityAccessFnc(void)
{
	static boolean isSeedRequested = False;
	static Uds_SecurityLevelType securityLevelRequested = 0u;
	Uds_NrcType nrc = UDS_E_POSITIVERESPONSE;
	if(2u <= udsRte.rxLength)
	{
		if(0x01u == udsGetSerivceData(1))	// Request Seed
		{
			if(3u == udsRte.rxLength)
			{
				isSeedRequested = True;
				securityLevelRequested = udsGetSerivceData(2);
				// Create Response
				udsSetResponseCode(1u,0x01);
				udsRte.txLength = 2 + udsPrepareSeed(udsGetResponseBuffer(2));
				nrc = UDS_E_POSITIVERESPONSE;
			}
			else
			{
				nrc = UDS_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT;
			}
		}
		else if(0x02u == udsGetSerivceData(1)) // Send key
		{
			if(True == isSeedRequested)
			{
			}
			else
			{
				nrc = UDS_E_REQUESTSEQUENCEERROR;
			}

		}
		else
		{
			// TODO: many sub - function not supported by me. add the process here
			nrc = UDS_E_SUBFUNCTIONNOTSUPPORTED;
		}
	}
	else
	{
		nrc = UDS_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT;
	}
	udsProcessingDone(nrc);
}

LOCAL void udsSelectServiceFunction(void)
{
	switch(udsRte.currentSid)
	{
		case SID_DIAGNOSTIC_SESSION_CONTROL:
			udsSessionControlFnc();
			break;
		case SID_SECURITY_ACCESS:
			udsSecurityAccessFnc();
			break;
		default:
			break;
	}
}

LOCAL void udsHandleServiceRequest(void)
{
	udsRte.currentSid = udsGetSerivceData(0);
	if(TRUE == udsLookupSid())
	{	// Start to Process it
		if(TRUE == udsCheckSessionLevel())
		{
			if(TRUE == udsCheckSecurityLevel())
			{
				udsRte.suppressPosRspMsg = False; // TODO: ???
				udsSelectServiceFunction();
			}
			else
			{
				udsCreateAndSendNrc(UDS_E_SECUTITYACCESSDENIED);
			}
		}
		else
		{
			udsCreateAndSendNrc(UDS_E_SERVICENOTSUPPORTEDINACTIVESESSION);
		}
	}
	else
	{
		udsCreateAndSendNrc(UDS_E_SERVICENOTSUPPORTED);	/** @req DCM197 */
	}
}

EXPORT void Uds_MainTask(void)
{
	switch(udsRte.state)
	{
		case Uds_stServiceRequested:
		{   // Easy the implementation, don't care the request is valid or not. maybe not safe.
			udsSetAlarm(msToUdsTick(cfgP2Server));
			udsHandleServiceRequest();
			break;
		}
		default:
		{
			if(udsIsAlarmStarted())
			{
				udsSignalAlarm();
				if(udsIsAlarmTimeout())
				{
					Uds_Init();  // Reset As P2Server Time-out
					devTrace(tlUds,"Info:Uds P2Server Time out.\n");
				}
			}
			break;
		}
	}
}

TASK(TaskUdsMain)
{
	Uds_MainTask();
	TerminateTask();
}
