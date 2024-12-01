

#ifndef MYPROJECT_H
#define MYPROJECT_H

/* Includes ------------------------------------------------------------------*/

#include <stdlib.h>
#include <stdio.h>
#include "usart2.h"
#include "delay.h"
#include "timer.h"
#include "adc.h"

#include "foc_utils.h"
#include "BLDCMotor.h"
#include "FOCMotor.h"
#include "lowpass_filter.h"
#include "pid.h"
#include "LowsideCurrentSense.h"
#include "CurrentSense.h"
#include "HallSensor.h"

#define M0_Disable   TIM_Cmd(TIM1, DISABLE);        //关闭M0输出

//霍尔信号
//M0的 A(PB4)、B(PB5)和Z(PC9)接hall信号，三个信号之间随便接，不用区分
//M1的 A(PB6)、B(PB7)和Z(PC15)接hall信号，三个信号之间随便接，不用区分

#endif

