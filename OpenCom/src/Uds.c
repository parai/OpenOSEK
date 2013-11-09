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
#define udsGetServiceBuffer(__i) (&ComRxIPDUConfig[udsRte.pduId].pdu.SduDataPtr[__i])
#define udsSetResponseCode(__i,__v) {ComTxIPDUConfig[udsRte.pduId].pdu.SduDataPtr[__i] = (uint8)(__v);}
#define udsGetResponseBuffer(__i) (&ComTxIPDUConfig[udsRte.pduId].pdu.SduDataPtr[__i])
#define udsGetResponseBufferLength() (ComTxIPDUConfig[udsRte.pduId].pdu.SduLength)
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
LOCAL boolean udsIsSeedRequested = False;
LOCAL Uds_SecurityLevelType udsSecurityLevelRequested = 0u;

/* ================================ FUNCTIONs =============================== */
LOCAL void udsHandleServiceRequest(void);
LOCAL void udsCreateAndSendNrc(Uds_NrcType Nrc);
LOCAL void udsProcessingDone(Uds_NrcType Ncr);
LOCAL boolean udsLookupSid(void);
LOCAL boolean udsCheckSessionLevel(Uds_SessionMaskType sessionMask);
LOCAL boolean udsCheckSecurityLevel(Uds_SecurityLevelMaskType securityLevelMask);
LOCAL boolean udsIsNewSessionValid(Uds_SessionType Session);
LOCAL Uds_SessionType udsSessionMap(Uds_SessionType Session);
LOCAL void udsSessionControlFnc(void);
LOCAL void udsSecurityAccessFnc(void);
LOCAL void udsCommunicationControlFnc(void);
LOCAL void udsTesterPresentFnc(void);
LOCAL void udsRDIDFnc(void);
LOCAL void udsWDIDFnc(void);
LOCAL void udsSelectServiceFunction(void);
LOCAL uint8 udsPrepareSeed(uint8* seed);
LOCAL boolean udsCompareKey(uint8* key,uint8 length);

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
		devTrace(tlError,"UDS Server is Currently Busy.\n");
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
			devTrace(tlUds,"Suppressed Response!\n");
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
LOCAL boolean udsCheckSessionLevel(Uds_SessionMaskType sessionMask)
{
	boolean isOk = False;
	if( (sessionMask&(1<<udsRte.session)) != 0u )
	{
		isOk = True;
	}
	return isOk;
}
LOCAL boolean udsCheckSecurityLevel(Uds_SecurityLevelMaskType securityLevelMask)
{
	boolean isOk = False;
	if(UdsUnSecurityLevel == securityLevelMask)
	{
		isOk = True;
	}
	else if( (securityLevelMask&(udsRte.securityLevel)) != 0u )
	{
		isOk = True;
	}
	else
	{
	}
	return isOk;
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
		uint8 subFnc = udsGetSerivceData(1); //
		if((0x80U&subFnc) != 0)
		{
			udsRte.suppressPosRspMsg = True;
			subFnc = subFnc&0x7F;
		}
		devTrace(tlUds,"Info:Session Control %2x.\n",subFnc);
		if(True == udsIsNewSessionValid(subFnc))
		{
			// TODO: here should askApplicationForSessionPermission
//			if(udsRte.session == udsSessionMap(subFnc))
//			{ //TODO
//			}
			udsRte.session = udsSessionMap(subFnc);
			// Create Response
			udsSetResponseCode(1,subFnc);
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
	seed[0] = 0xDE; // Example key 0xDEADBEEF
	seed[1] = 0xAD;
	seed[2] = 0xBE;
	seed[3] = 0xEF;
 	return 4;
}
LOCAL boolean udsCompareKey(uint8* key,uint8 length)
{
	if( 	(4 == length) &&
			(key[0] == 0xFE) && (key[1] == 0xEB) && (key[2] == 0xDA) && (key[3] == 0xED))
	{
		return True;
	}
	return False;
}

/* @UDS Service: 0x27 */
LOCAL void udsSecurityAccessFnc(void)
{
	Uds_NrcType nrc = UDS_E_POSITIVERESPONSE;
	if(2u <= udsRte.rxLength)
	{
		uint8 subFnc = udsGetSerivceData(1);
		if((0x80U&subFnc) != 0u)
		{
			udsRte.suppressPosRspMsg = True;
			subFnc = subFnc&0x7F;
		}
		if(0x01u == subFnc)	// Request Seed
		{
			if(3u == udsRte.rxLength)
			{
				udsSecurityLevelRequested = udsGetSerivceData(2);
				if(udsCheckSecurityLevel(1<<udsSecurityLevelRequested))
				{
					udsSetResponseCode(1u,0x01);
					udsRte.txLength = 2 + 4 /* TODO key length */;
					memset(udsGetResponseBuffer(2),0,4);
					nrc = UDS_E_POSITIVERESPONSE;
				}
				else
				{
					udsIsSeedRequested = True;
					// Create Response
					udsSetResponseCode(1u,0x01);
					udsRte.txLength = 2 + udsPrepareSeed(udsGetResponseBuffer(2));
					nrc = UDS_E_POSITIVERESPONSE;
				}
			}
			else
			{
				nrc = UDS_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT;
			}
		}
		else if(0x02u == subFnc) // Send key
		{
			if(True == udsIsSeedRequested)
			{
				if(True == udsCompareKey(udsGetServiceBuffer(2),udsRte.rxLength-2))
				{
					udsRte.securityLevel = (1<<udsSecurityLevelRequested);
					udsIsSeedRequested = False;
					// Create Response
					udsSetResponseCode(1u,0x02);
					udsRte.txLength = 2;
					nrc = UDS_E_POSITIVERESPONSE;
				}
				else
				{
					nrc = UDS_E_CONDITIONSNOTCORRECT;
				}
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
/* @UDS Service: 0x28 */
LOCAL void udsCommunicationControlFnc(void)
{
	Uds_NrcType nrc = UDS_E_POSITIVERESPONSE;
	if(3u == udsRte.rxLength)
	{
		uint8 subFnc = udsGetSerivceData(1); //
		if((0x80U&subFnc) != 0)
		{
			udsRte.suppressPosRspMsg = True;
			subFnc = subFnc&0x7F;
		}
		// TODO: Add Special Process For Your App here
		switch(subFnc)
		{
			case 0x00: //enableRxAndTx
				if(0x02 == udsGetSerivceData(2))  // CTP is NWMCP
				{	// This is just a example
					TalkNM(0);
				}
				else
				{
					nrc = UDS_E_CONDITIONSNOTCORRECT;
				}
				break;
			case 0x01: //enableRxAndDisableTx
				if(0x02 == udsGetSerivceData(2))  // CTP is NWMCP
				{	// This is just a example
					SilentNM(0);
				}
				else
				{
					nrc = UDS_E_CONDITIONSNOTCORRECT;
				}
				break;
			case 0x02: //disableRxAndEnableTx
				nrc = UDS_E_SUBFUNCTIONNOTSUPPORTED;
				break;
			case 0x03: //disableRxAndTx
				nrc = UDS_E_SUBFUNCTIONNOTSUPPORTED;
				break;
			default:
				nrc = UDS_E_SUBFUNCTIONNOTSUPPORTED;
				break;
		}
		if(UDS_E_POSITIVERESPONSE == nrc)
		{
			udsRte.txLength = 2;
			udsSetResponseCode(1,udsGetSerivceData(1));
		}
	}
	else
	{
		nrc = UDS_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT;
	}
	udsProcessingDone(nrc);
}
/* @UDS Service: 0x3e */
LOCAL void udsTesterPresentFnc(void)
{
	Uds_NrcType nrc = UDS_E_POSITIVERESPONSE;
	if(2u == udsRte.rxLength)
	{
		uint8 subFnc = udsGetSerivceData(1); //
		if((0x80U&subFnc) != 0)
		{
			udsRte.suppressPosRspMsg = True;
			subFnc = subFnc&0x7F;
		}
		if(0x00 == subFnc)
		{
			udsSetAlarm(msToUdsTick(cfgP2Server));
			udsSetResponseCode(1,0x00);
			udsRte.txLength = 2;
		}
		else
		{
			nrc = UDS_E_SUBFUNCTIONNOTSUPPORTED;
		}
	}
	else
	{
		nrc = UDS_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT;
	}
	udsProcessingDone(nrc);
}
/* @UDS Service: 0x22 */
LOCAL void udsRDIDFnc(void)
{
	Uds_NrcType nrc = UDS_E_POSITIVERESPONSE;
	if(3u <= udsRte.rxLength)
	{
		uint8 didNbr = (udsRte.rxLength-1)/2;
		uint8 i,j;
		udsRte.txLength = 1;
		for(i=0;i<didNbr;i++)
		{
			uint16 did = ((uint16)udsGetSerivceData(1+i*2)<<8) + udsGetSerivceData(2+i*2);
			for(j=0;j<UdsConfig.rdidNbr;j++)
			{
				if(did == UdsConfig.rdidList[j].did)
				{
					break;
				}
			}
			if(j < UdsConfig.rdidNbr )
			{
				if(True == udsCheckSessionLevel(UdsConfig.rdidList[j].sessionMask))
				{
					if(True == udsCheckSecurityLevel(UdsConfig.rdidList[j].securityLevelMask) )
					{
						uint16 rlen;
						udsSetResponseCode(udsRte.txLength,did>>8);
						udsSetResponseCode(udsRte.txLength+1,did);
						udsRte.txLength += 2;
						rlen = UdsConfig.rdidList[j].callout(udsGetResponseBuffer(udsRte.txLength),
								udsGetResponseBufferLength()-udsRte.txLength);
						if(rlen != 0u)
						{
							udsRte.txLength += rlen;
						}
						else
						{
							nrc = UDS_E_REQUESTOUTOFRANGE;
							break;
						}
					}
					else
					{
						nrc = UDS_E_SECUTITYACCESSDENIED;
						break;
					}
				}
				else
				{
					nrc = UDS_E_SERVICENOTSUPPORTEDINACTIVESESSION;
					break;
				}
			}
			else
			{
				nrc = UDS_E_REQUESTOUTOFRANGE;
				break;
			}
		}
	}
	else
	{
		nrc = UDS_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT;
	}
	udsProcessingDone(nrc);
}
/* @UDS Service: 0x22 */
LOCAL void udsWDIDFnc(void)
{
	Uds_NrcType nrc = UDS_E_POSITIVERESPONSE;
	if(3u < udsRte.rxLength)
	{
		uint16 did = ((uint16)udsGetSerivceData(1)<<8) + udsGetSerivceData(2);
		uint8 i;
		for(i=0;i<UdsConfig.wdidNbr;i++)
		{
			if(did == UdsConfig.wdidList[i].did)
			{
				break;
			}
		}
		if(i < UdsConfig.wdidNbr )
		{
			if(True == udsCheckSessionLevel(UdsConfig.wdidList[i].sessionMask))
			{
				if(True == udsCheckSecurityLevel(UdsConfig.wdidList[i].securityLevelMask) )
				{
					StatusType ercd;
					ercd = UdsConfig.wdidList[i].callout(udsGetServiceBuffer(3),udsRte.rxLength-3);
					if(E_OK == ercd)
					{
						udsSetResponseCode(1,did>>8);
						udsSetResponseCode(2,did);
						udsRte.txLength = 3;
					}
					else
					{
						nrc = UDS_E_GENERALPROGRAMMINGFAILURE;
					}
				}
				else
				{
					nrc = UDS_E_SECUTITYACCESSDENIED;
				}
			}
			else
			{
				nrc = UDS_E_SERVICENOTSUPPORTEDINACTIVESESSION;
			}
		}
		else
		{
			nrc = UDS_E_REQUESTOUTOFRANGE;
		}
	}
	else
	{
		nrc = UDS_E_INCORRECTMESSAGELENGTHORINVALIDFORMAT;
	}
	udsProcessingDone(nrc);
}
/* @UDS Service: 0x31 */
LOCAL void udsRCFnc(void)
{
	Uds_NrcType nrc = UDS_E_POSITIVERESPONSE;
	if(4u <= udsRte.rxLength)
	{
		uint8 i;
		uint8 subFnc = udsGetSerivceData(1);
		uint16 rcid = ((uint16)udsGetSerivceData(2)<<8) + udsGetSerivceData(3);
		if((0x80U&subFnc) != 0)
		{
			udsRte.suppressPosRspMsg = True;
			subFnc = subFnc&0x7F;
		}
		for(i=0;i<UdsConfig.rcNbr;i++)
		{
			if(UdsConfig.rcList[i].id == rcid)
			{
				break;
			}
		}
		if(i<UdsConfig.rcNbr)
		{
			uint16 ercd = 0;
			switch(subFnc)
			{
				case 0x01:
					ercd = UdsConfig.rcList[i].startRC(udsGetServiceBuffer(4),
								udsRte.rxLength-4,udsGetResponseBuffer(4));
					break;
				case 0x02:
					ercd = UdsConfig.rcList[i].stopRC(udsGetServiceBuffer(4),
								udsRte.rxLength-4,udsGetResponseBuffer(4));
					break;
				case 0x03:
					ercd = UdsConfig.rcList[i].requestResultRC(udsGetServiceBuffer(4),
								udsRte.rxLength-4,udsGetResponseBuffer(4));
					break;
				default:
					nrc = UDS_E_SUBFUNCTIONNOTSUPPORTED;
					break;
			}
			if((subFnc>=0x01) && (subFnc<=0x03))
			{
				if(0 == ercd)
				{
					nrc = UDS_E_CONDITIONSNOTCORRECT;
				}
				else
				{
					udsSetResponseCode(1,subFnc);
					udsSetResponseCode(2,rcid>>8);
					udsSetResponseCode(3,rcid);
					udsRte.txLength = 4 + ercd -1;
				}
			}
		}
		else
		{
			nrc = UDS_E_REQUESTOUTOFRANGE;
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
		case SID_COMMUNICATION_CONTROL:
			udsCommunicationControlFnc();
			break;
		case SID_TESTER_PRESENT:
			udsTesterPresentFnc();
			break;
		case SID_READ_DATA_BY_IDENTIFIER:
			udsRDIDFnc();
			break;
		case SID_WRITE_DATA_BY_IDENTIFIER:
			udsWDIDFnc();
			break;
		case SID_ROUTINE_CONTROL:
			udsRCFnc();
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
		if(TRUE == udsCheckSessionLevel(UdsConfig.sidList[udsRte.sidIndex].sessionMask))
		{
			if(TRUE == udsCheckSecurityLevel(UdsConfig.sidList[udsRte.sidIndex].securityLevelMask))
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
