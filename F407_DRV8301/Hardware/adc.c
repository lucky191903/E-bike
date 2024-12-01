
#include "stm32f4xx.h"


/****************************************************************************/
uint16_t  adc1_value[32];
float  m0_phA,m0_phB,m0_phC;
/****************************************************************************/
// PC0/Channel_10==M0_SO1==注入转换T1_TRGO，PC3/Channel_13==M1_SO1==规则转换T8_TRGO，
// PC1/Channel_11==M0_SO2==注入转换T1_TRGO，PC2/Channel_12==M1_SO2==规则转换T8_TRGO，
void ADC_Common_Init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
  NVIC_InitTypeDef   NVIC_InitStructure;
	ADC_CommonInitTypeDef  ADC_CommonInitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC|RCC_AHB1Periph_GPIOA, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6;  //PA4-M1_temp/PA5-AUX_temp/PA6-Vbus_S
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_5;  //PC5-M0_temp
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	
	NVIC_InitStructure.NVIC_IRQChannel = ADC_IRQn;
//	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
//	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	//ADC Common通用配置，只配置一次适用三个ADC外设
	ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;                    //独立模式
	ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div4;                 //4分频21MHz
	ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;     //DMA禁止选项(对于多个ADC模式)
	ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles;//采样延时
	ADC_CommonInit(&ADC_CommonInitStructure);
}
/****************************************************************************/
void ADC1_DMA_Init(void)
{
  NVIC_InitTypeDef  NVIC_InitStructure;
	DMA_InitTypeDef   DMA_InitStructure;
	ADC_InitTypeDef   ADC_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
	
	NVIC_InitStructure.NVIC_IRQChannel=DMA2_Stream0_IRQn;                            
	NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	//DMA2_Stream0 Channel_0 对应ADC1
	DMA_InitStructure.DMA_Channel = DMA_Channel_0;  
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&(ADC1->DR);       //ADC地址
	DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)&adc1_value;          //内存地址
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;                 //外设到内存
	DMA_InitStructure.DMA_BufferSize = 4;                                   //传输长度
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;        //外设递增关闭
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;                 //内存递增开启
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;//数据宽度16bit
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;                         //循环模式
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;                     //优先级高
	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;                  //DMA FIFO 关闭
	DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;      
	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
	DMA_Init(DMA2_Stream0, &DMA_InitStructure);
	DMA_Cmd(DMA2_Stream0, ENABLE);                                          //使能DMA
	
	DMA_ClearFlag(DMA2_Stream0, DMA_IT_TC);
  DMA_ITConfig(DMA2_Stream0, DMA_IT_TC, ENABLE);   //使能DMA中断
	
	
	//ADC1 regular channel 12 configuration 
	ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;                      //12bit
	ADC_InitStructure.ADC_ScanConvMode = ENABLE;                                //扫描模式
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;                         //禁止连续转换，转换完毕进入中断
	ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None; //禁止外部触发
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;                      //ADC数据右对齐
	ADC_InitStructure.ADC_NbrOfConversion = 4;                                  //4个通道
	ADC_Init(ADC1, &ADC_InitStructure);
	
	ADC_RegularChannelConfig(ADC1, ADC_Channel_15,1, ADC_SampleTime_84Cycles);  //M0_temp
	ADC_RegularChannelConfig(ADC1, ADC_Channel_4, 2, ADC_SampleTime_84Cycles);  //M1_temp
	ADC_RegularChannelConfig(ADC1, ADC_Channel_5, 3, ADC_SampleTime_84Cycles);  //AUX_temp
	ADC_RegularChannelConfig(ADC1, ADC_Channel_6, 4, ADC_SampleTime_84Cycles);  //Vbus
	
	ADC_DMARequestAfterLastTransferCmd(ADC1, ENABLE);  // Enable DMA request after last transfer (Single-ADC mode)
	ADC_DMACmd(ADC1, ENABLE);       //ADC使能DMA
	ADC_Cmd(ADC1, ENABLE);
	ADC_SoftwareStartConv(ADC1);    //软件触发，ADC1开始在后台转换，永不停止
}
/****************************************************************************/
//DMA转换完毕，重新再开始，防止ADC错位
void DMA2_Stream0_IRQHandler(void) 
{
	if(DMA_GetITStatus(DMA2_Stream0, DMA_IT_TCIF0) == SET)  
	{
		DMA_ClearITPendingBit(DMA2_Stream0, DMA_IT_TCIF0); 
		ADC_SoftwareStartConv(ADC1);
	}
}
/****************************************************************************/
void ADC2_TRGO_Init(void)
{
	ADC_InitTypeDef  ADC_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC2, ENABLE);
	
	ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;                        //12bit 
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;                                 //模式
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;                           //单次转换
	ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_Rising;
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T8_TRGO;        //外部触发 T8_TRGO
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;                        //ADC数据右对齐
	ADC_InitStructure.ADC_NbrOfConversion = 1;                                    //规则组只有1个通道
	ADC_Init(ADC2, &ADC_InitStructure);
	ADC_DMARequestAfterLastTransferCmd(ADC2, DISABLE);
	ADC_EOCOnEachRegularChannelCmd(ADC2, DISABLE);
	ADC_RegularChannelConfig(ADC2, ADC_Channel_13, 1, ADC_SampleTime_3Cycles);   //channel_13,检测m1_phB
	
	ADC_InjectedSequencerLengthConfig(ADC2,1);                                    //注入组只有一个通道
	ADC_InjectedChannelConfig(ADC2, ADC_Channel_10, 1, ADC_SampleTime_3Cycles);  //channel_10,检测m0_phB
	ADC_SetInjectedOffset(ADC2, ADC_InjectedChannel_1, 0);
	ADC_ExternalTrigInjectedConvEdgeConfig(ADC2, ADC_ExternalTrigInjecConvEdge_Rising);
	ADC_ExternalTrigInjectedConvConfig(ADC2, ADC_ExternalTrigInjecConv_T1_TRGO);  //外部触发 T1_TRGO
	ADC_AutoInjectedConvCmd(ADC2, DISABLE);
	ADC_InjectedDiscModeCmd(ADC2, DISABLE);
	
//	ADC_ITConfig(ADC2, ADC_IT_EOC, ENABLE);     //打开规则组完成中断
	ADC_ITConfig(ADC2, ADC_IT_JEOC, ENABLE);    //打开注入组完成中断
	ADC_Cmd(ADC2, ENABLE);
}
/****************************************************************************/
void ADC3_TRGO_Init(void)
{
	ADC_InitTypeDef  ADC_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC3, ENABLE);
	
	ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;                        //12bit 
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;                                 //模式
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;                           //单次转换
	ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_Rising;
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T8_TRGO;        //外部触发 T8_TRGO
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;                        //ADC数据右对齐
	ADC_InitStructure.ADC_NbrOfConversion = 1;                                    //规则组只有1个通道
	ADC_Init(ADC3, &ADC_InitStructure);
	ADC_DMARequestAfterLastTransferCmd(ADC3, DISABLE);
	ADC_EOCOnEachRegularChannelCmd(ADC3, DISABLE);
	ADC_RegularChannelConfig(ADC3, ADC_Channel_12, 1, ADC_SampleTime_3Cycles);   //channel_12,检测m1_phC
	
	ADC_InjectedSequencerLengthConfig(ADC3,1);                                    //注入组只有一个通道
	ADC_InjectedChannelConfig(ADC3, ADC_Channel_11, 1, ADC_SampleTime_3Cycles);  //channel_11,检测m0_phC
	ADC_SetInjectedOffset(ADC3, ADC_InjectedChannel_1, 0);
	ADC_ExternalTrigInjectedConvEdgeConfig(ADC3, ADC_ExternalTrigInjecConvEdge_Rising);
	ADC_ExternalTrigInjectedConvConfig(ADC3, ADC_ExternalTrigInjecConv_T1_TRGO);  //外部触发 T1_TRGO
	ADC_AutoInjectedConvCmd(ADC3, DISABLE);
	ADC_InjectedDiscModeCmd(ADC3, DISABLE);
	
//	ADC_ITConfig(ADC3, ADC_IT_EOC, ENABLE);     //打开规则组完成中断
	ADC_ITConfig(ADC3, ADC_IT_JEOC, ENABLE);    //打开注入组完成中断
	ADC_Cmd(ADC3, ENABLE);
}
/*****************************************************************************/
//tim1配置为中央对齐模式，一个周期update两次，每(2+1)次触发一次，所以进入中断的频率为16KHz，
void ADC_IRQHandler(void)		//ADC中断
{
	if(((ADC2->SR & ADC_SR_JEOC) == ADC_SR_JEOC)&&((ADC3->SR & ADC_SR_JEOC) == ADC_SR_JEOC))
	{
		ADC2->SR = ~ADC_SR_JEOC;
		ADC3->SR = ~ADC_SR_JEOC;
		if((TIM1->CR1 & TIM_CR1_DIR)==0)   //=0为递增计数,上臂为低下臂为高,此时采样
		{
			m0_phB = (float)ADC2->JDR1 *3.3f/4096;
			m0_phC = (float)ADC3->JDR1 *3.3f/4096;
		}
	}
}
/****************************************************************************/
#define  VBUS_S_DIVIDER_RATIO  18.7273f    //电源电压检测，采用39K+2.2K电阻分压
float get_vbus_voltage(void)
{
	float  vbus=(float)adc1_value[3]*3.3f*VBUS_S_DIVIDER_RATIO/4096;
	return vbus;
}
/****************************************************************************/

