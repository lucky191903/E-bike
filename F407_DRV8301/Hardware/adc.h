
#ifndef __STM32_ADC_H
#define __STM32_ADC_H
/******************************************************************************/

#include "stm32f4xx.h"


/******************************************************************************/
extern  uint16_t adc1_value[32];
extern  float m0_phA,m0_phB,m0_phC;
/******************************************************************************/
void ADC_Common_Init(void);
void ADC1_DMA_Init(void);
void ADC2_TRGO_Init(void);
void ADC3_TRGO_Init(void);
float get_vbus_voltage(void);
/******************************************************************************/


#endif


