#include "Os.h"
#include "Can.h"
void StartupHook(void)
{
	printf("In StartupHook()\n");
	Can_Init(NULL);
}
TASK(Task1)
{
	Can_PduType pdu;
	pdu.id =0x731;
	pdu.length = 5;
	pdu.sdu = "Hello";
	pdu.swPduHandle = 0;
	Can_Write(0,&pdu);
	TerminateTask();
}
