/*
 * k10.h
 *
 * Created: 21/01/2016 21:44:46
 *  Author: GroundLayer
 */ 


#ifndef K10_H_
#define K10_H_

#include "asf.h"

bool K10_init(void);
void calib(int cmd);
bool K10_read_conductivity(float* EC , float* TDS , float* S , float* SG);
void K10_buffer_received(struct usart_module *const module);
bool K10_recover_data(char* string , int terminator, float* data);




#endif /* K10_H_ */