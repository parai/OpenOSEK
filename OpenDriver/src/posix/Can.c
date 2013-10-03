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
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <arpa/inet.h>
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
LOCAL pthread_t Can_CtrlRxThread[CAN_CONTROLLER_CNT] = {0};
LOCAL pthread_t Can_CtrlTxThread[CAN_CONTROLLER_CNT] = {0};
LOCAL pthread_mutex_t Can_CtrlTxMutex[CAN_CONTROLLER_CNT] = {PTHREAD_MUTEX_INITIALIZER};
LOCAL unsigned int Can_CtrlTxEvent[CAN_CONTROLLER_CNT] = {0};
LOCAL struct sockaddr_in Can_SockAddr[CAN_CONTROLLER_CNT];
LOCAL Can_PduType2 Can_PduMsg[CAN_CONTROLLER_CNT];
LOCAL Can_StateTransitionType Can_CtrlState[CAN_CONTROLLER_CNT];
LOCAL uint32 Can_CtrlServerPort[CAN_CONTROLLER_CNT];
/* ================================ FUNCTIONs =============================== */
IMPORT void Can_TxConformation(PduIdType TxHandle);
IMPORT void Can_RxIndication(Can_ControllerIdType Controller,Can_IdType canid,uint8* data,uint8 length);
LOCAL void* Can_RxMainThread(const Can_ControllerConfigType* ctrlConfig);
LOCAL void* Can_TxMainThread(const Can_ControllerConfigType* ctrlConfig);
// Set-up the simulate environment for CAN communication
// Build Socket-links bind to local-host:8001 and local-host:8002
// So 2 Can-Nodes is simulated, and it's enough to support the run time
// environment for Com and NM.
EXPORT void Can_Init(const void* Config)
{
	int i;
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
	pthread_attr_t portThreadAttr;
	/* No need to join the threads. */
	pthread_attr_init( &portThreadAttr );
	pthread_attr_setdetachstate( &portThreadAttr, PTHREAD_CREATE_DETACHED );
	Can_SockAddr[Controller].sin_family = AF_INET;
	Can_SockAddr[Controller].sin_addr.s_addr = inet_addr("127.0.0.1");
	Can_SockAddr[Controller].sin_port = htons(Can_Config_PC->CanControllerConfig[Controller].CanSocketServerPort);

	pthread_mutex_init(&Can_CtrlTxMutex[Controller],NULL);

	pthread_create(&Can_CtrlRxThread[Controller],&portThreadAttr,
						Can_RxMainThread,(void*)&(Can_Config_PC->CanControllerConfig[Controller]));
	pthread_create(&Can_CtrlTxThread[Controller],&portThreadAttr,
						Can_TxMainThread,(void*)&(Can_Config_PC->CanControllerConfig[Controller]));
}

EXPORT Can_ReturnType Can_SetControllerMode(uint8 Controller,Can_StateTransitionType Transition)
{
	Can_ReturnType rv = CAN_OK;

	Can_CtrlState[Controller] = Transition;

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
	pthread_mutex_lock(&Can_CtrlTxMutex[Controller]);

	Can_PduMsg[Controller].swPduHandle = PduInfo->swPduHandle;
	Can_PduMsg[Controller].id = PduInfo->id;
	Can_PduMsg[Controller].length = PduInfo->length;
	for(i=0;i<Can_PduMsg[Controller].length;i++)
	{
		Can_PduMsg[Controller].sdu[i] = PduInfo->sdu[i];
	}
	Can_CtrlTxEvent[Controller] = TRUE;
	pthread_mutex_unlock(&Can_CtrlTxMutex[Controller]);
	return CAN_OK;
}
EXPORT void Can_DeInit(void)
{
	int i;
	for(i=0;i<CAN_CONTROLLER_CNT;i++)
	{

	}
}
LOCAL void* Can_TxMainThread(const Can_ControllerConfigType* Config)
{
	uint16 Controller = Config->CanControllerId;
	// This is simulation implementation, so Hth is the id of can control.
	int ConnectSocket;
	int ercd;
	uint8 msg[64];
	int i;
	// printf("Enter Tx Thread for Controller %d.\n",Controller);
	for(;;)
	{
		pthread_mutex_lock(&Can_CtrlTxMutex[Controller]);
		if(TRUE == Can_CtrlTxEvent[Controller])
		{
			// Create a SOCKET for connecting to server
			ConnectSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
			if (ConnectSocket == -1) {
				printf("socket function failed with error\n");
			}
			// Connect to server.
			ercd = connect(ConnectSocket, (__CONST_SOCKADDR_ARG ) & Can_SockAddr[Controller], sizeof (struct sockaddr));
			if (ercd == -1) {
				printf("connect function failed with error\n");
				ercd = close(ConnectSocket);
				if (ercd == -1){
					printf("closesocket function failed with error\n");
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
				msg[5+i] = 0;
			}
			msg[13] = (uint8)(Can_CtrlServerPort[Controller]>>24);
			msg[14] = (uint8)(Can_CtrlServerPort[Controller]>>16);
			msg[15] = (uint8)(Can_CtrlServerPort[Controller]>>8);
			msg[16] = (uint8)(Can_CtrlServerPort[Controller]);
			send(ConnectSocket,(const char*)msg,17,0);
			close(ConnectSocket);
			Can_TxConformation(Can_PduMsg[Controller].swPduHandle);
			Can_CtrlTxEvent[Controller] = FALSE;
		}
		pthread_mutex_unlock(&Can_CtrlTxMutex[Controller]);
	}
	return NULL;
}
LOCAL void* Can_RxMainThread(const Can_ControllerConfigType* Config)
{
	uint16 Controller = Config->CanControllerId;
	// the listening socket to be created
	int ListenSocket = -1;
	// Create a SOCKET for accepting incoming requests.
	int AcceptSocket;
	// The socket address to be passed to bind
	struct sockaddr_in service;
	int ercd;
	uint32 port = Config->CanSocketServerPort+1;  // try until find one.
	//----------------------
	// Create a SOCKET for listening for
	// incoming connection requests
	ListenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (ListenSocket == -1) {
		printf("socket function failed with error\n");
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
		ercd = bind(ListenSocket, (struct sockaddr*) &service, sizeof (service));
		if (ercd == -1) {
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
	if (listen(ListenSocket, 32) == -1) {
		printf("listen failed with error.\n");
		close(ListenSocket);
		return NULL;
	}
	for(;;)
	{
		Can_IdType id;
		uint8 msg[64];
		AcceptSocket = accept(ListenSocket, NULL, NULL);
		if (AcceptSocket == -1) {
			//printf("accept failed with error.\n");
			continue;
		}
		ercd = recv(AcceptSocket, (void*)msg, 64, 0);
		if ( ercd == 17 )
		{ // Rx ISR
			id = (((uint32)msg[0])<<24) + (((uint32)msg[1])<<16) + (((uint32)msg[2])<<8) + msg[3];
			Can_RxIndication(Controller,id,&msg[5],msg[4]);
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
		ercd = close(AcceptSocket);
		if (ercd == -1){
			printf("RX:closesocket function failed with error\n");
		}
	}
	return NULL;
}
