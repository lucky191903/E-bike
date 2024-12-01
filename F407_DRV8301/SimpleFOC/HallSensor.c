
#include "MyProject.h"


/************************************************
电机驱动板405
A(PB4)、B(PB5)和Z(PC9)接hall信号，三个信号之间随便接，不用区分
=================================================
使用教程：https://blog.csdn.net/loop222/article/details/129819526
         《SimpleFOC移植STM32(八) —— 霍尔电机》
创建日期：20230321
作    者：loop222 @郑州
************************************************/
/***************************************************************************/
// seq 1 > 5 > 4 > 6 > 2 > 3 > 1     000 001 010 011 100 101 110 111
signed char const ELECTRIC_SECTORS[8] = { -1,  0,  4,  5,  2,  1,  3 , -1 };
/***************************************************************************/
uint8_t  A_active,B_active,C_active;
uint8_t  hall_state;
/***************************************************************************/
Direction  direction=UNKNOWN;
Direction  old_direction=UNKNOWN;
signed char electric_sector=0;
long  electric_rotations=0;
long  pulse_timestamp=0;
long  pulse_diff=0;
long  cpr;
/***************************************************************************/
void updateState(void);
float getAngle(void);
/***************************************************************************/
void Hall_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	EXTI_InitTypeDef EXTI_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOC, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4|GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;      //输入模式
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_25MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; //GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;//普通输入模式
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_25MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; //GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOB, EXTI_PinSource4);  //PB4
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOB, EXTI_PinSource5);  //PB5
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOC, EXTI_PinSource9);  //PC9
	
	EXTI_InitStructure.EXTI_Line = EXTI_Line4;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);
	
	NVIC_InitStructure.NVIC_IRQChannel = EXTI4_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	EXTI_InitStructure.EXTI_Line = EXTI_Line5|EXTI_Line9;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);
	
	NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	cpr=pole_pairs*6;
}
/***************************************************************************/
void EXTI4_IRQHandler(void)
{
	if(EXTI_GetITStatus(EXTI_Line4) == SET)
	{
		EXTI_ClearITPendingBit(EXTI_Line4); //清除中断标志位
		A_active= GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_4);
		updateState();
	}
}
/***************************************************************************/
void EXTI9_5_IRQHandler(void)
{
	if(EXTI_GetITStatus(EXTI_Line5) == SET)
	{
		EXTI_ClearITPendingBit(EXTI_Line5);//清除中断标志位
		B_active= GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_5);
		updateState();
	}
	
	if(EXTI_GetITStatus(EXTI_Line9) == SET)
	{
		EXTI_ClearITPendingBit(EXTI_Line9);//清除中断标志位
		C_active= GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_9);
		updateState();
	}
}
/***************************************************************************/
void updateState(void)
{
	signed char new_electric_sector;
	long  new_pulse_timestamp = _micros();
	uint8_t new_hall_state = C_active + (B_active << 1) + (A_active << 2);
	
	if(new_hall_state == hall_state)return;
  hall_state = new_hall_state;
	new_electric_sector = ELECTRIC_SECTORS[hall_state];  //根据hall状态判断所在扇区
	
  if (new_electric_sector - electric_sector > 3) {
    //underflow
    direction = CCW;
    electric_rotations += direction;
  } else if (new_electric_sector - electric_sector < (-3)) {
    //overflow
    direction = CW;
    electric_rotations += direction;
  } else {
    direction = (new_electric_sector > electric_sector)? CW : CCW;
  }
  electric_sector = new_electric_sector;
	
  // glitch avoidance #2 changes in direction can cause velocity spikes.  Possible improvements needed in this area
  if(direction == old_direction)
	{
    // not oscilating or just changed direction
		pulse_diff = new_pulse_timestamp - pulse_timestamp;
  }
	else
	{
		pulse_diff = 0;
  }
	
  pulse_timestamp = new_pulse_timestamp;
  //total_interrupts++;   //没有用到，所以屏蔽
  old_direction = direction;
  //if (onSectorChange != nullptr) onSectorChange(electric_sector);
	
}
/***************************************************************************/
/*
float hall_getSensorAngle(void)
{
	return getAngle();
}*/
/***************************************************************************/
/*
float hall_getMechanicalAngle(void)
{
  return ((float)((electric_rotations * 6 + electric_sector) % cpr) / (float)cpr) * _2PI ;
}*/
/***************************************************************************/
float getVelocity(void)
{
  if(pulse_diff == 0 || ((long)(_micros() - pulse_timestamp) > pulse_diff) ) { // last velocity isn't accurate if too old
    return 0;
  }
	else {
    float vel = direction * (_2PI / (float)cpr) / (pulse_diff / 1000000.0f);
    // quick fix https://github.com/simplefoc/Arduino-FOC/issues/192
    if(vel < -1000 || vel > 1000)  vel = 0.0f;   //if velocity is out of range then make it zero
    return vel;
  }
}
/***************************************************************************/
float getAngle(void)
{
  return ((float)(electric_rotations * 6 + electric_sector) / (float)cpr) * _2PI ;
}
/***************************************************************************/
