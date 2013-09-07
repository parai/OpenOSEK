#include "osek_os.h"

TASK(Task0)
{
    printf("Task0 is running.\n");
    ChainTask(Task2);
}
TASK(Task1)
{
	ActivateTask(Task0);
    printf("Task1 is running.\n");
    TerminateTask();
}
TASK(Task2)
{
	ActivateTask(Task1);
    printf("Task2 is running.\n");
    TerminateTask();
}
TASK(Task3)
{
	ActivateTask(Task2);
    printf("Task3 is running.\n");
    TerminateTask();
}
