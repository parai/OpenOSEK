// This file only for test purpose.
#include <stdio.h>
#include "Can.h"
#include <windows.h>

extern void Can_Send(void);
int main(int argc,char* argv[])
{
	Can_PduType pdu;
	pdu.id =0x731;
	pdu.length = 5;
	pdu.sdu = "Hello";
	pdu.swPduHandle = 0;
	Can_Init(NULL);
	printf("Send msg1.\n");
	Can_Write(0,&pdu);
	Sleep(1000);
	pdu.sdu = "World";
	pdu.length = 5;
	printf("Send msg2.\n");
	Can_Write(0,&pdu);
	while(1);
	return 0;
}
