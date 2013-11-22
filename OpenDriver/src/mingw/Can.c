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
#include <winsock2.h>
#include <Ws2tcpip.h>
#include <windows.h>
#include <osek_os.h>
// Link with ws2_32.lib
#ifndef __GNUC__
#pragma comment(lib, "Ws2_32.lib")
#endif
/* ================================ MACROs    =============================== */
#define cCanMsgBoxSz 32
/* ================================ TYPEs     =============================== */
typedef struct
{
	Can_ControllerIdType CanControllerId;
	uint32          CanControllerBaudRate;
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
	boolean isTxIsr;
	boolean isRxIsr;
	boolean isWkupIsr;
}CanISRRegType;

typedef struct
{
	Can_IdType 	id;
	uint8      dlc;
	uint8      data[8];
	PduIdType   swPduHandle;
	enum{
		canMsgBoxIdle = 0,
		canMsgBoxBusy,
		canMsgBoxTxed,
		canMsgBoxRxed
	}state;
}CanMsgBoxType;

typedef struct
{
	CanMsgBoxType rxMsg[cCanMsgBoxSz];
	CanMsgBoxType txMsg[cCanMsgBoxSz];
	HANDLE        rxThread;
	HANDLE        txThread;
	HANDLE		  mutex;
	HANDLE        txEvent;
	CanISRRegType IRQ;
	uint32 port;			// Port listen on
	enum
	{
		eCanUnInit = 0,
		eCanStarted,
		eCanStopped,
		eCanSleep,
		eCanWakeup
	}state;
}CanRTE_Type;
/* ================================ DATAs     =============================== */
LOCAL const Can_ConfigType canCongig =
{
	{
		{
			/*.CanControllerId = */CAN_CTRL_0,
			/* .CanControllerBaudRate = */ 125000,
			/* .CanControllerPropSeg = */ 1,
			/* .CanControllerSeg1 = */ 13,
			/* .CanControllerSeg2 = */ 2,
			/* .CanSocketServerPort = */8000, // 127.0.0.1:8000 CAN-BUS1 server
		},
//		{
//			.CanControllerId = CAN_CTRL_1,
//			.CanSocketServerPort = 9000, // 127.0.0.1:9000 CAN-BUS2 server
//		}
	}
};
LOCAL const Can_ConfigType* Can_Config_PC = &canCongig;
LOCAL CanRTE_Type canRte[CAN_CONTROLLER_CNT];
/* ================================ FUNCTIONs =============================== */
LOCAL void* Can_RxMainThread(const Can_ControllerConfigType* ctrlConfig);
LOCAL void* Can_TxMainThread(const Can_ControllerConfigType* ctrlConfig);
LOCAL void Can0_IRQnHandle(void);
LOCAL void Can1_IRQnHandle(void);
// Set-up the simulate environment for CAN communication
// Build Socket-links bind to local-host:8001 and local-host:8002
// So 2 Can-Nodes is simulated, and it's enough to support the run time
// environment for Com and NM.
EXPORT void Can_Init(const void* Config)
{
	int i;
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);
	memset(&canRte,0u,sizeof(canRte));
	knl_install_isr(eIRQnCan0,Can0_IRQnHandle);
	knl_install_isr(eIRQnCan1,Can1_IRQnHandle);
	// Initialize Can Controller
	for(i=0;i<CAN_CONTROLLER_CNT;i++)
	{
		Can_InitController(i,&(Can_Config_PC->CanControllerConfig[i]));
	}
}

EXPORT void Can_InitController(uint8 Controller,const void* Config)
{
	canRte[Controller].txEvent = CreateEvent( NULL, FALSE, FALSE, NULL );
	canRte[Controller].mutex = CreateMutex( NULL, FALSE, NULL );
	canRte[Controller].rxThread = portCreSecondaryThread(Can_RxMainThread,Config);
	canRte[Controller].txThread = portCreSecondaryThread(Can_TxMainThread,Config);
}

EXPORT Can_ReturnType Can_SetControllerMode(uint8 Controller,Can_StateTransitionType Transition)
{
	switch(Transition)
	{
		case CAN_T_START:
			canRte[Controller].state = eCanStarted;
			break;
		case CAN_T_STOP:
			canRte[Controller].state = eCanStopped;
			break;
		case CAN_T_SLEEP:
			canRte[Controller].state = eCanSleep;
			break;
		case CAN_T_WAKEUP:
			canRte[Controller].state = eCanWakeup;
			break;
		default:
			assert(False);
			break;
	}
	return CAN_OK;
}

EXPORT Can_ReturnType Can_Write(Can_HwHandleType Hth,const Can_PduType* PduInfo)
{
	int i,I;
	uint16 Controller = Hth; // find controller
	// Check parameter
	assert(Controller < CAN_CONTROLLER_CNT);

	if((eCanStarted != canRte[Controller].state)
		&& (eCanWakeup != canRte[Controller].state))
	{
		printf("ERROR: CAN controller not started or waked up.\n");
		return CAN_NOT_OK;
	}
	for(I=0;I<cCanMsgBoxSz;I++)
	{
		if(canMsgBoxIdle == canRte[Controller].txMsg[I].state)
		{
			break;
		}
	}
	if(I>=cCanMsgBoxSz)
	{
		return CAN_BUSY;
	}
	portEnterCriticalSection();

#if(0 /*tlCan*/ > cfgDEV_TRACE_LEVEL) // The test says that don't send it too fast, better 10ms / 1 Frame
	{
		int i;
		printf("Send[0x%X] = [",(unsigned int)PduInfo->id);
		for(i=0;i<PduInfo->length;i++)
		{
			printf("0x%-2X,",(unsigned int)PduInfo->sdu[i]);
		}
		printf("]\n");
	}
#endif
	canRte[Controller].txMsg[I].swPduHandle = PduInfo->swPduHandle;
	canRte[Controller].txMsg[I].id = PduInfo->id;
	canRte[Controller].txMsg[I].dlc = PduInfo->length;
	assert(PduInfo->length<=8);
	for(i=0;i<PduInfo->length;i++)
	{
		canRte[Controller].txMsg[I].data[i] = PduInfo->sdu[i];
	}
	canRte[Controller].txMsg[I].state = canMsgBoxBusy;
	SetEvent( canRte[Controller].txEvent );
	portExitCriticalSection();
	return CAN_OK;
}
EXPORT void Can_DeInit(void)
{
	int i;
	for(i=0;i<CAN_CONTROLLER_CNT;i++)
	{
		TerminateThread(canRte[i].rxThread,0);
		TerminateThread(canRte[i].txThread,0);
		CloseHandle(canRte[i].txEvent);
		CloseHandle(canRte[i].mutex);
	}
	WSACleanup();
}
LOCAL void* Can_TxMainThread(const Can_ControllerConfigType* Config)
{
	uint16 Controller = Config->CanControllerId;
	struct sockaddr_in sockaddr;
	// This is simulation implementation, so Hth is the id of can control.
	SOCKET ConnectSocket;
	int ercd;
	uint8 msg[64];
	int i,I;
	void *pvObjectList[ 2 ];
	pvObjectList[ 0 ] = canRte[Controller].mutex;
	pvObjectList[ 1 ] = canRte[Controller].txEvent;
	// printf("Enter Tx Thread for Controller %d.\n",Controller);

	// Creat IP address
	sockaddr.sin_family = AF_INET;
	sockaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	sockaddr.sin_port = (u_short)htons(Can_Config_PC->CanControllerConfig[Controller].CanSocketServerPort);
	for(;;)
	{
		Sleep(1);
		WaitForMultipleObjects( sizeof( pvObjectList ) / sizeof( void * ), pvObjectList, True, INFINITE );
		// Create a SOCKET for connecting to server
		ConnectSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (ConnectSocket == INVALID_SOCKET) {
			printf("socket function failed with error: %d\n", WSAGetLastError());
		}
		// Connect to server.
		ercd = connect(ConnectSocket, (SOCKADDR *) & sockaddr, sizeof (SOCKADDR));
		if (ercd == SOCKET_ERROR) {
			printf("connect function failed with error: %d\n", WSAGetLastError());
			ercd = closesocket(ConnectSocket);
			if (ercd == SOCKET_ERROR){
				printf("closesocket function failed with error: %d\n", WSAGetLastError());
			}
		}
		for(I=0;I<cCanMsgBoxSz;I++)
		{
			if(canMsgBoxBusy == canRte[Controller].txMsg[I].state)
			{
				break;
			}
		}
		if(I < cCanMsgBoxSz)
		{
			msg[0] = (uint8)(canRte[Controller].txMsg[I].id>>24);
			msg[1] = (uint8)(canRte[Controller].txMsg[I].id>>16);
			msg[2] = (uint8)(canRte[Controller].txMsg[I].id>>8);
			msg[3] = (uint8)(canRte[Controller].txMsg[I].id);
			msg[4] = canRte[Controller].txMsg[I].dlc;
			if(msg[4] > 8)
			{
				msg[4] = 8;
			}
			for(i=0;i<msg[4];i++)
			{
				msg[5+i] = canRte[Controller].txMsg[I].data[i];
			}
			for(;i<8;i++)
			{
				msg[5+i] = 0x55;  // PADDING
			}
			msg[13] = (uint8)(canRte[Controller].port>>24);
			msg[14] = (uint8)(canRte[Controller].port>>16);
			msg[15] = (uint8)(canRte[Controller].port>>8);
			msg[16] = (uint8)(canRte[Controller].port);
			send(ConnectSocket,(const char*)msg,17,0);
			ercd = closesocket(ConnectSocket);
			if (ercd == SOCKET_ERROR){
				printf("closesocket function failed with error: %d\n", WSAGetLastError());
			}
			canRte[Controller].txMsg[I].state = canMsgBoxTxed; // Request Tx ISR
			canRte[Controller].IRQ.isTxIsr = True;
			ReleaseMutex(canRte[Controller].mutex);
			portGenerateSimulatedInterrupt(eIRQnCan0+Controller);
		}
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
	int I;
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
		service.sin_port = (u_short)htons(port);
		ercd = bind(ListenSocket, (SOCKADDR *) &service, sizeof (service));
		if (ercd == SOCKET_ERROR) {
			port ++;
			continue;
		}
		else
		{
			canRte[Controller].port = port;
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
			WaitForSingleObject(canRte[Controller].mutex,INFINITE);
			id = (((uint32)msg[0])<<24) + (((uint32)msg[1])<<16) + (((uint32)msg[2])<<8) + msg[3];
			if(eCanSleep == canRte[Controller].state)
			{
				canRte[Controller].IRQ.isWkupIsr = True;
			}
			else
			{
				for(I=0;I<cCanMsgBoxSz;I++)
				{
					if(canMsgBoxIdle == canRte[Controller].rxMsg[I].state)
					{
						break;
					}
				}
				if(I < cCanMsgBoxSz)
				{
					canRte[Controller].rxMsg[I].id = id;
					canRte[Controller].rxMsg[I].dlc = msg[4];
					memcpy(canRte[Controller].rxMsg[I].data,&msg[5],msg[4]);
					canRte[Controller].rxMsg[I].state = canMsgBoxRxed;
				}
				else
				{
					printf("Can Message Box Full, Message Lost!\n");
				}
				canRte[Controller].IRQ.isRxIsr = True;
			}
			ReleaseMutex(canRte[Controller].mutex);
			portGenerateSimulatedInterrupt(eIRQnCan0+Controller);
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
		Sleep(0);
	}
	return NULL;
}

LOCAL void Can_IRQnHandle(int Controller)
{
	int I;
	EnterISR();
	if(canRte[Controller].IRQ.isTxIsr)
	{
		for(I=0;I<cCanMsgBoxSz;I++)
		{
			if(canMsgBoxTxed == canRte[Controller].txMsg[I].state)
			{
				Can_TxConformation(Controller,canRte[Controller].txMsg[I].swPduHandle);
				canRte[Controller].txMsg[I].state = canMsgBoxIdle;
			}
		}
		canRte[Controller].IRQ.isTxIsr = False;
	}
	if(canRte[Controller].IRQ.isRxIsr)
	{
		for(I=0;I<cCanMsgBoxSz;I++)
		{
			if(canMsgBoxRxed == canRte[Controller].rxMsg[I].state)
			{
				Can_RxIndication(Controller,canRte[Controller].rxMsg[I].id,
						canRte[Controller].rxMsg[I].data,canRte[Controller].rxMsg[I].dlc);
				canRte[Controller].rxMsg[I].state = canMsgBoxIdle;
			}
		}
		canRte[Controller].IRQ.isRxIsr = False;
	}
	if(canRte[Controller].IRQ.isWkupIsr)
	{
		Can_WakeupIndication(Controller);
		canRte[Controller].IRQ.isWkupIsr = False;
	}
	LeaveISR();
}

LOCAL void Can0_IRQnHandle(void)
{
	Can_IRQnHandle(CAN_CTRL_0);
}
LOCAL void Can1_IRQnHandle(void)
{
	//Can_IRQnHandle(CAN_CTRL_1);
}
