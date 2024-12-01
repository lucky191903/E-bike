
#include "MyProject.h"


/************************************************
电流采样函数

本程序仅供学习，引用代码请标明出处
使用教程：https://blog.csdn.net/loop222/article/details/119220638
创建日期：20221010
作    者：loop222 @郑州
************************************************/
/******************************************************************************/
int pinA,pinB,pinC;
float gain_a,gain_b,gain_c;
float offset_ia,offset_ib,offset_ic;
/******************************************************************************/
void Current_calibrateOffsets(void);
/******************************************************************************/
void LowsideCurrentSense(float _shunt_resistor, float _gain, int _pinA, int _pinB, int _pinC)
{
	float volts_to_amps_ratio;
	
	pinA = _pinA;
	pinB = _pinB;
	pinC = _pinC;
	
	volts_to_amps_ratio = 1.0f /_shunt_resistor / _gain; // volts to amps
	
	gain_a = volts_to_amps_ratio;
	gain_b =-volts_to_amps_ratio;
	gain_c =-volts_to_amps_ratio;
	
	//printf("gain_a:%.2f,gain_b:%.2f,gain_c:%.2f.\r\n",gain_a,gain_b,gain_c);  //必须屏蔽，这句话打印会导致下面的函数执行不正常，一下午没有找到原因
}
/******************************************************************************/
void LowsideCurrentSense_Init(void)
{
	ADC_Common_Init();          //初始化ADC的引脚和通用配置，设置中断
	ADC1_DMA_Init();            //配置DMA模式，转换温度检测和vbus，温度检测在simpleFOC中没有使用，开始转换
	ADC2_TRGO_Init();           //配置注入组中断，检测m0_phB，等待TIM1触发
	ADC3_TRGO_Init();           //配置注入组中断，检测m0_phC，等待TIM1触发
	
	TIM_Cmd(TIM1, ENABLE);      //使能TIM1，触发AD注入组转换
	
	delay_ms(10);
	Current_calibrateOffsets(); //检测偏置电压，也就是电流0A时的运放输出电压值，理论值=1.65V
}
/******************************************************************************/
// Function finding zero offsets of the ADC
void Current_calibrateOffsets(void)
{
	int i;
	
	offset_ia=0;
	offset_ib=0;
	offset_ic=0;
	// read the adc voltage 1000 times ( arbitrary number )
	for(i=0; i<1000; i++)
	{
		if(_isset(pinA)) offset_ia += m0_phA;
		if(_isset(pinB)) offset_ib += m0_phB;
		if(_isset(pinC)) offset_ic += m0_phC;
		delay_ms(1);
	}
	// calculate the mean offsets
	if(_isset(pinA)) offset_ia = offset_ia/1000;
	if(_isset(pinB)) offset_ib = offset_ib/1000;
	if(_isset(pinC)) offset_ic = offset_ic/1000;
	
	printf("offset_ia:%.4f,offset_ib:%.4f,offset_ic:%.4f.\r\n",offset_ia,offset_ib,offset_ic);
}
/******************************************************************************/
// read all three phase currents (if possible 2 or 3)
PhaseCurrent_s getPhaseCurrents(void)
{
	PhaseCurrent_s current;
	
	current.a = (!_isset(pinA)) ? 0 : (m0_phA - offset_ia)*gain_a; // amps
	current.b = (!_isset(pinB)) ? 0 : (m0_phB - offset_ib)*gain_b; // amps
	current.c = (!_isset(pinC)) ? 0 : (m0_phC - offset_ic)*gain_c; // amps
	
	return current;
}
/******************************************************************************/





