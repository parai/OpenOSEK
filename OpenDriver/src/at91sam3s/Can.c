#include "Com.h"

EXPORT void Can_Init(const void* Config)
{

}

EXPORT void Can_InitController(uint8 Controller,const void* Config)
{

}

EXPORT Can_ReturnType Can_SetControllerMode(uint8 Controller,Can_StateTransitionType Transition)
{
	return E_OK;
}

EXPORT Can_ReturnType Can_Write(Can_HwHandleType Hth,const Can_PduType* PduInfo)
{
#if(tlCan > cfgDEV_TRACE_LEVEL) // The test says that don't send it too fast, better 10ms / 1 Frame
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
	return E_OK;
}
