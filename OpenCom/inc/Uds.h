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
#ifndef UDS_H_H_HH_
#define UDS_H_H_HH_
/* ================================ INCLUDEs  =============================== */
#include "Uds_Cfg.h"
/* ================================ MACROs    =============================== */
// SID table
#define SID_DIAGNOSTIC_SESSION_CONTROL			0x10
#define SID_ECU_RESET							0x11
#define SID_CLEAR_DIAGNOSTIC_INFORMATION		0x14
#define SID_READ_DTC_INFORMATION				0x19
#define SID_READ_DATA_BY_IDENTIFIER				0x22
#define SID_READ_MEMORY_BY_ADDRESS				0x23
#define SID_READ_SCALING_DATA_BY_IDENTIFIER		0x24
#define SID_SECURITY_ACCESS						0x27
#define SID_COMMUNICATION_CONTROL               0x28
#define SID_READ_DATA_BY_PERIODIC_IDENTIFIER	0x2A
#define SID_DYNAMICALLY_DEFINE_DATA_IDENTIFIER	0x2C
#define SID_WRITE_DATA_BY_IDENTIFIER			0x2E
#define SID_INPUT_OUTPUT_CONTROL_BY_IDENTIFIER	0x2F
#define SID_ROUTINE_CONTROL						0x31
#define SID_WRITE_MEMORY_BY_ADDRESS				0x3D
#define SID_TESTER_PRESENT						0x3E
#define SID_NEGATIVE_RESPONSE					0x7F
#define SID_CONTROL_DTC_SETTING					0x85

// Session Mask List
// See ISO - 14229 (2006):
// So in fact Only 4 Session was supported according to Table 26
// Session 00,05-3F,So may be should build a Map for Session and its Mask
// here is the Map default implemented by OpenCom
// 	{
//		01 : UdsDefaultSession  (0), 			02 : UdsProgramSession       (1),
//		03 : UdsExtendedSession (2),			04 : UdsSafetySystemSession  (3),
//Hint: 40 : Your Session Want  (4),    and so on  see API udsIsNewSessionValid()
// 	}
#define UdsAllSession           0xFFFFU
#define UdsDefaultSession       0x0001U
#define UdsProgramSession       0x0002U
#define UdsExtendedSession      0x0004U
#define UdsSafetySystemSession  0x0008U
#define UdsSession0         0x0001U
#define UdsSession1         0x0002U
#define UdsSession2         0x0004U
#define UdsSession3         0x0008U
#define UdsSession4         0x0010U
#define UdsSession5         0x0020U
#define UdsSession6         0x0040U
#define UdsSession7         0x0080U
#define UdsSession8         0x0100U
#define UdsSession9         0x0200U
#define UdsSessionA         0x0400U
#define UdsSessionB         0x0800U
#define UdsSessionC         0x1000U
#define UdsSessionD         0x2000U
#define UdsSessionE         0x4000U
#define UdsSessionF         0x8000U

// Security Level Mask List
#define UdsUnSecurityLevel        0x0000U
#define UdsSecurityLevel0         0x0001U
#define UdsSecurityLevel1         0x0002U
#define UdsSecurityLevel2         0x0004U
#define UdsSecurityLevel3         0x0008U
#define UdsSecurityLevel4         0x0010U
#define UdsSecurityLevel5         0x0020U
#define UdsSecurityLevel6         0x0040U
#define UdsSecurityLevel7         0x0080U
#define UdsSecurityLevel8         0x0100U
#define UdsSecurityLevel9         0x0200U
#define UdsSecurityLevelA         0x0400U
#define UdsSecurityLevelB         0x0800U
#define UdsSecurityLevelC         0x1000U
#define UdsSecurityLevelD         0x2000U
#define UdsSecurityLevelE         0x4000U
#define UdsSecurityLevelF         0x8000U

/* ================================ TYPEs     =============================== */
typedef uint8 Uds_SessionType;
typedef uint8 Uds_SecurityLevelType;

typedef uint8 Uds_ServiceIdType;
// Each bit is a session
typedef uint16 Uds_SessionMaskType;
// Each bit is a level
typedef uint16 Uds_SecurityLevelMaskType;

typedef struct
{
	Uds_ServiceIdType sid;
	Uds_SessionMaskType sessionMask;
	Uds_SecurityLevelMaskType securityLevelMask;
}Uds_ServiceType;

typedef struct
{
	uint16 did;
	Uds_SessionMaskType sessionMask;
	Uds_SecurityLevelMaskType securityLevelMask;
	/*
	 * Data:   pointer to the buffer to store value of DID
	 * length: the length of the buffer "Data"
	 * Return:  (1) length of this DID for ReadDID
	 *          (2) E_OK,E_NOT_OK for WriteDID
	 *                E_OK: write OK
	 *                E_NOT_OK: write failed, such as condition is not OK
	 * IF DID's length > Data's length, please return 0 to indicate the
	 * UDS Server that the buffer is not enough. This may happen when
	 * Read many DIDs in one request.
	 */
	uint16 (*callout)(uint8* Data,uint16 length);
}Uds_DIDType;

typedef struct
{
	const Uds_ServiceType* sidList;
	uint8                  sidNbr;
	const Uds_DIDType*     rdidList;
	uint8                  rdidNbr;
	const Uds_DIDType*     wdidList;
	uint8                  wdidNbr;
}Uds_ConfigType;


typedef uint8 Uds_NrcType;

/* ================================ DATAs     =============================== */

/* ================================ FUNCTIONs =============================== */
IMPORT void Uds_RxIndication(PduIdType RxPduId,PduLengthType Length);
IMPORT void Uds_TxConformation(PduIdType RxPduId,StatusType status);

#endif
