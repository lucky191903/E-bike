#ifndef __DELAY_H
#define __DELAY_H 			   
#include <sys.h>	  


/******************************************************************************/
void systick_CountInit(void);
uint32_t _micros(void);
void delay_us(unsigned long nus);
void delay_ms(unsigned long nms);
/******************************************************************************/

#endif




