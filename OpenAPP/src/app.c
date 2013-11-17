#include "Os.h"
#include "Com.h"
void StartupHook(void)
{
	Can_Init(NULL);
	StartNM(0);
	CanTp_Init();
}
extern void CanTp_Print(void);
TASK(TaskKeyMonitor)
{
#if defined(__GNUCC__) || defined(WIN32) 
	char chr;
	printf("TaskKeyMonitor is running\n");
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
		else if('T' == chr)
		{
			TalkNM(0);
			printf("TalkNM(0).\n");
		}
		else if('S' == chr)
		{
			SilentNM(0);
			printf("SilentNM(0).\n");
		}
		else if('k' == chr)
		{
			StopNM(0);
			printf("StopNM\n");
		}
		else if('r' == chr)
		{
			StartNM(0);
			printf("StartNM\n");
		}
		else if('p' == chr)
		{
			// print the info of each module
			CanTp_Print();
		}
	}
    printf("TaskKeyMonitor is Running.\n");
#endif    
    for(;;);  // Should always be Idle.
	TerminateTask();
}

TASK(TaskIdle)
{
	printf("TaskIdle is running\n");
	for(;;);
	devAssert(False,"System Panic as TaskIdle returned\n");
}

