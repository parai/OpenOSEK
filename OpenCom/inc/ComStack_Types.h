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
#ifndef COMSTACK_TYPES_H_
#define COMSTACK_TYPES_H_

#define ECUC_SW_MAJOR_VERSION   1
#define ECUC_SW_MINOR_VERSION   0
#define ECUC_SW_PATCH_VERSION   0

#include "Std_Types.h"


/* Zero-based integer number */
/* The size of this global type depends on the maximum */
/* number of PDUs used within one software module. */
/* Example : */
/* If  no software module deals with more PDUs that */
/* 256, this type can be set to uint8. */
/* If at least one software module handles more than */
/* 256 PDUs, this type must globally be set to uint16. */

/* In order to be able to perform table-indexing within a software */
/* module, variables of this type shall be zero-based and consecutive. */
/* There might be several ranges of PduIds in a module, one for each type of */
/* operation performed within that module (e.g. sending and receiving). */
typedef uint16 PduIdType;
#define INVALID_PDU_ID   0xFFFFu
typedef uint16 PduLengthType;

typedef struct {
	uint8 *SduDataPtr;			 /* payload */
	PduLengthType SduLength;	 /* length of SDU */
} PduInfoType;

typedef enum {
	TP_DATACONF,
	TP_DATARETRY,
	TP_CONFPENDING,
	TP_NORETRY,
} TpDataStateType;

typedef struct {
	TpDataStateType TpDataState;
	PduLengthType TxTpDataCnt;
} RetryInfoType;

typedef enum {
	BUFREQ_OK=0,
	BUFREQ_NOT_OK,
	BUFREQ_BUSY,
	BUFREQ_OVFL
} BufReq_ReturnType;

/* 0x00--0x1e General return types */
/* 0x1f--0x3c Error notif, CAN */
/* 0x3d--0x5a Error notif, LIN */
/* more */
typedef uint8 NotifResultType;

#define NTFRSLT_OK						0x00
#define NTFRSLT_E_NOT_OK				0x01
#define NTFRSLT_E_CANCELATION_NOT_OK	0x0C
#define NTFRSLT_E_WRONG_SN 				0x05
#define NTFRSLT_E_NO_BUFFER 			0x09

typedef uint8 BusTrcvErrorType;


#define BUSTRCV_NO_ERROR	0x00
#define BUSBUSTRCV_E_ERROR	0x01


#define COMSTACKTYPE_AR_MINOR_VERSION		1
#define COMSTACKTYPE_AR_MAJOR_VERSION		0
#define COMSTACKTYPE_AR_PATCH_VERSION		0

typedef uint8 NetworkHandleType;

#endif /*COMSTACK_TYPES_H_*/
