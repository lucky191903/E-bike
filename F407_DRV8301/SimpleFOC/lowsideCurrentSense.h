#ifndef INLINE_CS_LIB_H
#define INLINE_CS_LIB_H

#include "foc_utils.h" 


/******************************************************************************/
void LowsideCurrentSense(float _shunt_resistor, float _gain, int _pinA, int _pinB, int _pinC);
void LowsideCurrentSense_Init(void);
PhaseCurrent_s getPhaseCurrents(void);
/******************************************************************************/


#endif

