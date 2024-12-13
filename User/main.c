#include "stm32f10x.h"                  // Device header

#include "Config.h"
#include "Delay.h"
#include "Cmd.h"

#include "LED.h"
#include "OLED.h"
#include "PWM.h"
#include "Timer.h"
#include "Serial.h"

#include "Key.h"
#include "Encoder.h"
#include "On.h"

#include "string.h"
#include <math.h>

uint16_t i;
uint8_t sin_table[TABLE_SIZE];

uint8_t g_amplitude = 100; //幅度
uint8_t g_frequency = 1;   //频率
uint16_t index = 0;		   //正弦表索引
uint8_t g_running = 0;	   //运行状态


void GenerateSineTable(uint8_t amplitude){
	for(int i = 0; i < TABLE_SIZE; i++){
		sin_table[i] = (uint8_t)(sin(2 * PI * i / TABLE_SIZE) * amplitude + amplitude); //范围0-200
	}
}

/**
 * @brief 根据上位机命令进行任务处理
 * 
 * @param cmd 
 */
// void ProcessCommand(const char* cmd){
// 	if(strcmp(cmd,"START") == 0){
// 		g_running = 1;
// 		Serial_SendString("START_OK\r\n");
// 	}else if(strcmp(cmd,"STOP") == 0){
// 		g_running = 0;
// 		Serial_SendString("STOP_OK\r\n");
// 	}else if(strcmp(cmd,"AMPL+") == 0){
// 		g_amplitude += 10;
// 		if(g_amplitude > 200){
// 			g_amplitude = 200;
// 		}
// 		GenerateSineTable(g_amplitude);
// 		Serial_SendString("AMPL+_OK\r\n");
// 	}else if(strcmp(cmd,"AMPL-") == 0){
// 		g_amplitude -= 10;
// 		if(g_amplitude > 200){
// 			g_amplitude = 0;
// 		}
// 		GenerateSineTable(g_amplitude);
// 		Serial_SendString("AMPL-_OK\r\n");
// 	}else if(strcmp(cmd,"FREQ+") == 0){
// 		g_frequency += 1;
// 		if(g_frequency > 10){
// 			g_frequency = 10;
// 		}
// 		Serial_SendString("FREQ+_OK\r\n");
// 	}else if(strcmp(cmd,"freq-") == 0){
// 		g_frequency -= 1;
// 		if(g_frequency > 10){
// 			g_frequency = 1;
// 		}
// 		Serial_SendString("FREQ-_OK\r\n");
// 	}	
// }

void ProcessCommand(const char* cmd){
	for(int i = 0; i < 7; i++){
		if(strcmp(cmd,commands[i].cmd) == 0){
			commands[i].func();
			return;
		}
	}
	Serial_SendString("ERRORCMD\r\n");
}

/**
 * @brief 设置定时器中断 更新波形的幅度 
 * 
 */
void TIM2_IRQHandler(void)
{
	if (TIM_GetITStatus(TIM2, TIM_IT_Update) == SET)
	{
		if(g_running){
			PWM_SetCompare(sin_table[index]);
			index = (index + 1) % TABLE_SIZE;
		}

		TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
	}
}

/**
 * @brief 串口接收中断
 * 
 */
void USART1_IRQHandler(void)
{
	static uint8_t RxState = 0;
	static uint8_t pRxPacket = 0;
	if (USART_GetITStatus(USART1, USART_IT_RXNE) == SET)
	{
		uint8_t RxData = USART_ReceiveData(USART1);
		
		if (RxState == 0)
		{
			if (RxData == '@' && Serial_RxFlag == 0)
			{
				RxState = 1;
				pRxPacket = 0;
			}
		}
		else if (RxState == 1)
		{
			if (RxData == '\r')
			{
				RxState = 2;
			}
			else
			{
				Serial_RxPacket[pRxPacket] = RxData;
				pRxPacket ++;
			}
		}
		else if (RxState == 2)
		{
			if (RxData == '\n')
			{
				RxState = 0;
				Serial_RxPacket[pRxPacket] = '\0';
				Serial_RxFlag = 1;
			}
		}
		
		// if(Serial_RxFlag == 1){
		// 	if(strcmp(Serial_RxPacket,"START") == 0){
		// 		g_running = 1;
		// 		Serial_SendString("START_OK\r\n");
		// 		Serial_RxFlag = 0;
		// 	}
		// }

		USART_ClearITPendingBit(USART1, USART_IT_RXNE);
	}
}

int main(void)
{
	/*模块初始化*/
	On_Init();
	LED_Init();
	OLED_Init();
	PWM_Init();
	Key_Init();
	Encoder_Init();

	GenerateSineTable(g_amplitude);
	Timer_Init(2000/g_frequency,720);
	Serial_Init();

	OLED_ShowString(1,1,"ampl:");
	OLED_ShowString(2,1,"freq:");
	OLED_ShowString(3,1,"run:");
	OLED_ShowString(4,1,"cmd:");

	while (1)
	{
		OLED_ShowNum(1,6,g_amplitude,5);
		OLED_ShowNum(2,6,g_frequency,5);

		OLED_ShowString(3,5,"   ");
		if(g_running){
			OLED_ShowString(3,5,"on");
		}else{
			OLED_ShowString(3,5,"off");
		}

		if(Serial_RxFlag == 1){
			OLED_ShowString(4,5,"        ");
			OLED_ShowString(4,5,Serial_RxPacket);

			ProcessCommand(Serial_RxPacket);
			Serial_RxFlag = 0;
		}		
	}
}
