
#ifndef STM32_USART2_H
#define STM32_USART2_H
/******************************************************************************/
#include "stm32f4xx.h"

/******************************************************************************/
#define USART2_BUFFER_SIZE 256
/******************************************************************************/
extern char snd2_buff[USART2_BUFFER_SIZE];
extern char rcv2_buff[USART2_BUFFER_SIZE];
extern unsigned long rcv2_cntr;
extern unsigned long rcv2_flag;
/******************************************************************************/
void USART2_Init(unsigned long bound);
void USART2_SendDMA(uint32_t len);
/******************************************************************************/


#endif


