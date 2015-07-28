#include <stdio.h>
#include <math.h>
#include "main.h"
#include "utils.h"

TIM_HandleTypeDef 	TIM_Handle;
uint32_t						TockValue;


void TIM_Tick_Tock_Init(void)
{
	__TIM5_CLK_ENABLE();	
	TIM_Handle.Instance = TIM5;
	TIM_Handle.Init.Period = 0xFFFFFFFF;
	TIM_Handle.Init.Prescaler = 83;
	TIM_Handle.Init.ClockDivision = 0;
	TIM_Handle.Init.CounterMode = TIM_COUNTERMODE_UP;	
	HAL_TIM_Base_Init(&TIM_Handle);	
	HAL_TIM_Base_Start(&TIM_Handle);	
}


void Tick(void)
{
	__HAL_TIM_SetCounter(&TIM_Handle,0);
}

void Tock(void)
{
	TockValue=__HAL_TIM_GetCounter(&TIM_Handle);
	printf("Tiempo transcurrido: %u uS\n",TockValue);
}
