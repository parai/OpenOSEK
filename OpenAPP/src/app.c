#include "Os.h"
#include "Can.h"
#include "Nm.h"
void StartupHook(void)
{
	Can_Init(NULL);
	StartNM(0);
}

TASK(TaskKeyMonitor)
{
	char chr;
	for(;;)
	{
		chr = (char)getchar();
		if('s' == chr)
		{
			GotoMode(0,NM_BusSleep);
			printf("Goto Bus Sleep.\n");
		}
		else if('w' == chr)
		{
			GotoMode(0,NM_Awake);
			printf("Goto Awake.\n");
		}
	}
	TerminateTask();
}
