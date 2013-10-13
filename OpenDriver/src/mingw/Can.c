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
#include "Can.h"
#include <windows.h>
#include <winsock2.h>
#include <Ws2tcpip.h>
#include <Os.h>
// Link with ws2_32.lib
#ifndef __GNUC__
#pragma comment(lib, "Ws2_32.lib")
#endif
/* ================================ MACROs    =============================== */

/* ================================ TYPEs     =============================== */
typedef struct
{
	Can_ControllerIdType CanControllerId;
	uint16          CanControllerBaudRate;
	uint16          CanControllerPropSeg;
	uint16          CanControllerSeg1;
	uint16          CanControllerSeg2;
	// Special data for implementation
	uint32          CanSocketServerPort;
}Can_ControllerConfigType;
typedef struct
{
	Can_ControllerConfigType CanControllerConfig[CAN_CONTROLLER_CNT];
}Can_ConfigType;
typedef struct
{
	Can_IdType 	id;
	uint8		length;
	uint8 		sdu[8];
	PduIdType   swPduHandle;
}Can_PduType2;
/* ================================ DATAs     =============================== */
LOCAL const Can_ConfigType canCongig =
{
	{
		{
			.CanControllerId = CAN_CTRL_0,
			.CanSocketServerPort = 8000, // 127.0.0.1:8000 CAN-BUS1 server
		},
//		{
//			.CanControllerId = CAN_CTRL_1,
//			.CanSocketServerPort = 9000, // 127.0.0.1:9000 CAN-BUS2 server
//		}
	}
};
LOCAL const Can_ConfigType* Can_Config_PC = &canCongig;
LOCAL HANDLE Can_CtrlRxThread[CAN_CONTROLLER_CNT] = {NULL,};
LOCAL HANDLE Can_CtrlTxThread[CAN_CONTROLLER_CNT] = {NULL,};
LOCAL HANDLE Can_CtrlTxMutex[CAN_CONTROLLER_CNT] = {NULL,};
LOCAL HANDLE Can_CtrlTxEvent[CAN_CONTROLLER_CNT] = {NULL,};
LOCAL struct sockaddr_in Can_SockAddr[CAN_CONTROLLER_CNT];
LOCAL Can_PduType2 Can_PduMsg[CAN_CONTROLLER_CNT];
LOCAL Can_StateTransitionType Can_CtrlState[CAN_CONTROLLER_CNT];
LOCAL uint32 Can_CtrlServerPort[CAN_CONTROLLER_CNT];
/* ================================ FUNCTIONs =============================== */
LOCAL void* Can_RxMainThread(const Can_ControllerConfigType* ctrlConfig);
LOCAL void* Can_TxMainThread(const Can_ControllerConfigType* ctrlConfig);
// Set-up the simulate environment for CAN communication
// Build Socket-links bind to local-host:8001 and local-host:8002
// So 2 Can-Nodes is simulated, and it's enough to support the run time
// environment for Com and NM.
EXPORT void Can_Init(const void* Config)
{
	int i;
	WSADATA wsaData;
	int iResult;
	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != NO_ERROR) {
		wprintf(L"WSAStartup function failed with error: %d\n", iResult);
		return;
	}
	// Initialize Can Controller
	for(i=0;i<CAN_CONTROLLER_CNT;i++)
	{
		Can_InitController(i,&(Can_Config_PC->CanControllerConfig[i]));
	}
}

EXPORT void Can_InitController(uint8 Controller,const void* Config)
{
	// The sockaddr_in structure specifies the address family,
	// IP address, and port of the server to be connected to.
	Can_SockAddr[Controller].sin_family = AF_INET;
	Can_SockAddr[Controller].sin_addr.s_addr = inet_addr("127.0.0.1");
	Can_SockAddr[Controller].sin_port = htons(Can_Config_PC->CanControllerConfig[Controller].CanSocketServerPort);

	Can_CtrlRxThread[Controller]=CreateThread( NULL, 0, ( LPTHREAD_START_ROUTINE ) Can_RxMainThread, \
						(void*)Config, CREATE_SUSPENDED, NULL );
	if(NULL == Can_CtrlRxThread[Controller])
	{
		printf("Failed at Can_InitController() to create the Rx Thread.\n");
	}
	else
	{  // So priority higher than Task
		SetThreadPriority( Can_CtrlRxThread[Controller], THREAD_PRIORITY_BELOW_NORMAL );
		SetThreadPriorityBoost( Can_CtrlRxThread[Controller], TRUE );
		SetThreadAffinityMask( Can_CtrlRxThread[Controller], 0x01 );
	}
	Can_CtrlTxThread[Controller]=CreateThread( NULL, 0, ( LPTHREAD_START_ROUTINE ) Can_TxMainThread, \
						(void*)Config, CREATE_SUSPENDED, NULL );
	if(NULL == Can_CtrlTxThread[Controller])
	{
		printf("Failed at Can_InitController() to create the Tx Thread.\n");
	}
	else
	{  // So priority higher than Task
		SetThreadPriority( Can_CtrlTxThread[Controller], THREAD_PRIORITY_BELOW_NORMAL );
		SetThreadPriorityBoost( Can_CtrlTxThread[Controller], TRUE );
		SetThreadAffinityMask( Can_CtrlTxThread[Controller], 0x01 );
	}

	Can_CtrlTxMutex[Controller] = CreateMutex( NULL, FALSE, NULL );
	if(NULL == Can_CtrlTxMutex[Controller])
	{
		printf("Failed at Can_InitController() to create the Tx Mutex.\n");
	}
	Can_CtrlTxEvent[Controller] = CreateEvent( NULL, FALSE, FALSE, NULL );
	if(NULL == Can_CtrlTxEvent[Controller])
	{
		printf("Failed at Can_InitController() to create the Tx Event.\n");
	}
//	(void)Can_SetControllerMode(Controller,CAN_T_START);
}

EXPORT Can_ReturnType Can_SetControllerMode(uint8 Controller,Can_StateTransitionType Transition)
{
	Can_ReturnType rv = CAN_OK;

	switch(Transition )
	{
		case CAN_T_START:
			Can_CtrlState[Controller] = Transition;
			ResumeThread(Can_CtrlRxThread[Controller]);
			ResumeThread(Can_CtrlTxThread[Controller]);
            break;
		case CAN_T_WAKEUP:
			Can_CtrlState[Controller] = Transition;
			ResumeThread(Can_CtrlRxThread[Controller]);
			ResumeThread(Can_CtrlTxThread[Controller]);
            break;
		case CAN_T_SLEEP:  //CAN258, CAN290
			Can_CtrlState[Controller] = Transition;
			ResumeThread(Can_CtrlRxThread[Controller]);
			SuspendThread(Can_CtrlTxThread[Controller]);
            break;
		case CAN_T_STOP:
			Can_CtrlState[Controller] = Transition;
			SuspendThread(Can_CtrlRxThread[Controller]);
			SuspendThread(Can_CtrlTxThread[Controller]);
            break;
		default:
            break;
	}
	Sleep(100); // Wait for OK.
	return rv;
}

EXPORT Can_ReturnType Can_Write(Can_HwHandleType Hth,const Can_PduType* PduInfo)
{
	int i;
	uint16 Controller = Hth; // find controller
	// Check parameter
	if(Controller >= CAN_CONTROLLER_CNT)
	{
		printf("ERROR:Hth out of range.\n");
		return CAN_NOT_OK;
	}
	if((CAN_T_START != Can_CtrlState[Controller])
		&& (CAN_T_WAKEUP != Can_CtrlState[Controller]))
	{
		printf("ERROR: CAN controller not started or waked up.\n");
		return CAN_NOT_OK;
	}
	WaitForSingleObject( Can_CtrlTxMutex[Controller], INFINITE );

	Can_PduMsg[Controller].swPduHandle = PduInfo->swPduHandle;
	Can_PduMsg[Controller].id = PduInfo->id;
	Can_PduMsg[Controller].length = PduInfo->length;
	for(i=0;i<Can_PduMsg[Controller].length;i++)
	{
		Can_PduMsg[Controller].sdu[i] = PduInfo->sdu[i];
	}
	SetEvent( Can_CtrlTxEvent[Controller] );
	ReleaseMutex( Can_CtrlTxMutex[Controller] );
	return CAN_OK;
}
EXPORT void Can_DeInit(void)
{
	int i;
	for(i=0;i<CAN_CONTROLLER_CNT;i++)
	{
		TerminateThread(Can_CtrlTxThread[i],0);
		TerminateThread(Can_CtrlRxThread[i],0);
	}
	WSACleanup();
}
LOCAL void* Can_TxMainThread(const Can_ControllerConfigType* Config)
{
	uint16 Controller = Config->CanControllerId;
	// This is simulation implementation, so Hth is the id of can control.
	SOCKET ConnectSocket;
	int ercd;
	void *pvObjectList[ 2 ];
	uint8 msg[64];
	int i;
	/* Going to block on the mutex that ensured exclusive access to the simulated
	interrupt objects, and the event that signals that a simulated interrupt
	should be processed. */
	pvObjectList[ 0 ] = Can_CtrlTxMutex[Controller];
	pvObjectList[ 1 ] = Can_CtrlTxEvent[Controller];
	// printf("Enter Tx Thread for Controller %d.\n",Controller);
	for(;;)
	{
		WaitForMultipleObjects( sizeof( pvObjectList ) / sizeof( void * ), pvObjectList, TRUE, INFINITE );
		Sleep(1); // Slow it down, let looks more like the True Controller.
		// Create a SOCKET for connecting to server
		ConnectSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (ConnectSocket == INVALID_SOCKET) {
			printf("socket function failed with error: %d\n", WSAGetLastError());
		}
		// Connect to server.
		ercd = connect(ConnectSocket, (SOCKADDR *) & Can_SockAddr[Controller], sizeof (SOCKADDR));
		if (ercd == SOCKET_ERROR) {
			printf("connect function failed with error: %d\n", WSAGetLastError());
			ercd = closesocket(ConnectSocket);
			if (ercd == SOCKET_ERROR){
				printf("closesocket function failed with error: %d\n", WSAGetLastError());
			}
		}
		msg[0] = (uint8)(Can_PduMsg[Controller].id>>24);
		msg[1] = (uint8)(Can_PduMsg[Controller].id>>16);
		msg[2] = (uint8)(Can_PduMsg[Controller].id>>8);
		msg[3] = (uint8)(Can_PduMsg[Controller].id);
		msg[4] = Can_PduMsg[Controller].length;
		if(msg[4] > 8)
		{
			msg[4] = 8;
		}
		for(i=0;i<msg[4];i++)
		{
			msg[5+i] = Can_PduMsg[Controller].sdu[i];
		}
		for(;i<8;i++)
		{
			msg[5+i] = 0x55;  // PADDING
		}
		msg[13] = (uint8)(Can_CtrlServerPort[Controller]>>24);
		msg[14] = (uint8)(Can_CtrlServerPort[Controller]>>16);
		msg[15] = (uint8)(Can_CtrlServerPort[Controller]>>8);
		msg[16] = (uint8)(Can_CtrlServerPort[Controller]);
		send(ConnectSocket,(const char*)msg,17,0);
		ercd = closesocket(ConnectSocket);
		if (ercd == SOCKET_ERROR){
			printf("closesocket function failed with error: %d\n", WSAGetLastError());
		}
		SuspendAllInterrupts();
		Can_TxConformation(Controller,Can_PduMsg[Controller].swPduHandle);
		ResumeAllInterrupts();
		ReleaseMutex( Can_CtrlTxMutex[Controller] );
	}
	return NULL;
}
LOCAL void* Can_RxMainThread(const Can_ControllerConfigType* Config)
{
	uint16 Controller = Config->CanControllerId;
	// the listening socket to be created
	SOCKET ListenSocket = INVALID_SOCKET;
	// Create a SOCKET for accepting incoming requests.
	SOCKET AcceptSocket;
	// The socket address to be passed to bind
	struct sockaddr_in service;
	int ercd;
	uint32 port = Config->CanSocketServerPort+1;  // try until find one.
	//----------------------
	// Create a SOCKET for listening for
	// incoming connection requests
	ListenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (ListenSocket == INVALID_SOCKET) {
		wprintf(L"socket function failed with error: %u\n", WSAGetLastError());
		WSACleanup();
		return NULL;
	}
	//----------------------
	// Create a SOCKET for listening for
	// incoming connection requests
	ListenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (ListenSocket == INVALID_SOCKET) {
		wprintf(L"socket function failed with error: %u\n", WSAGetLastError());
		return NULL;
	}
	//----------------------
	// The sockaddr_in structure specifies the address family,
	// IP address, and port for the socket that is being bound.
	service.sin_family = AF_INET;
	service.sin_addr.s_addr = inet_addr("127.0.0.1");
	do{
		//----------------------
		// Bind the socket.
		service.sin_port = htons(port);
		ercd = bind(ListenSocket, (SOCKADDR *) &service, sizeof (service));
		if (ercd == SOCKET_ERROR) {
			port ++;
			continue;
		}
		else
		{
			Can_CtrlServerPort[Controller] = port;
			printf("Listen to %d\n",(int)port);
			break; // OK
		}
	}while(TRUE);
	// Listen for incoming connection requests.
	// on the created socket
	if (listen(ListenSocket, 32) == SOCKET_ERROR) {
		wprintf(L"listen failed with error: %ld\n", WSAGetLastError());
		closesocket(ListenSocket);
		return NULL;
	}
	for(;;)
	{
		Can_IdType id;
		uint8 msg[64];
		AcceptSocket = accept(ListenSocket, NULL, NULL);
		if (AcceptSocket == INVALID_SOCKET) {
			wprintf(L"accept failed with error: %ld\n", WSAGetLastError());
			continue;
		}
		ercd = recv(AcceptSocket, (void*)msg, 64, 0);
		if ( ercd == 17 )
		{ // Rx ISR
			id = (((uint32)msg[0])<<24) + (((uint32)msg[1])<<16) + (((uint32)msg[2])<<8) + msg[3];
			SuspendAllInterrupts();
			if(CAN_T_SLEEP == Can_CtrlState[Controller])
			{
				Can_WakeupIndication(Controller);
			}
			else
			{
				Can_RxIndication(Controller,id,&msg[5],msg[4]);
			}
			ResumeAllInterrupts();
		}
		else
		{
			if((ercd != -1) && (ercd != 0))
			{
				printf("ERROR:Invalid CAN message length : %d.\n",ercd);
			}
			else
			{
				printf("ERROR: Something Wrong when receive: %d.\n",ercd);
			}
		}
		ercd = closesocket(AcceptSocket);
		if (ercd == SOCKET_ERROR){
			printf("RX:closesocket function failed with error: %d\n", WSAGetLastError());
		}
	}
	return NULL;
}
