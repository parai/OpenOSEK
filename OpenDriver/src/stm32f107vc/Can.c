#include "Com.h"
#include "hw_config.h"
#include "usb_lib.h"
__IO uint8_t PrevXferComplete = 1;

EXPORT void Can_Init(const void* Config)
{
    Set_System();
    //USB_Interrupts_Config();

    Set_USBClock();

    USB_Init();
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

void Delay(__IO uint32_t nCount)
{
  for(; nCount!= 0;nCount--);
}

#ifdef  USE_FULL_ASSERT
/*******************************************************************************
* Function Name  : assert_failed
* Description    : Reports the name of the source file and the source line number
*                  where the assert_param error has occurred.
* Input          : - file: pointer to the source file name
*                  - line: assert_param error line source number
* Output         : None
* Return         : None
*******************************************************************************/
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while(1)
  {
  }
}
#endif
