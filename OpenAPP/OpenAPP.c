#include "osek_os.h"

TASK(Task0)
{
	//ActivateTask(3);
    printf("Task0 is running.\n");
    //ChainTask(Task2);
    TerminateTask();
}
TASK(Task1)
{
	//ActivateTask(Task0);
    printf("Task1 is running.\n");
    TerminateTask();
}
TASK(Task2)
{
	ActivateTask(Task3);
    printf("Task2 is running.\n");
    TerminateTask();
}
TASK(Task3)
{
static int called = FALSE;
    if(called == FALSE)
    {
    	ActivateTask(Task2);
    	ActivateTask(Task2);
    	ActivateTask(Task1);
    	ActivateTask(Task1);
    	called = TRUE;
	}
    printf("Task3 is running.\n");
    TerminateTask();
}
