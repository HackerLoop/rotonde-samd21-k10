/*
 * k10.c
 *
 * Created: 21/01/2016 21:44:38
 *  Author: GroundLayer
 */ 

#include "modules/drivers/env_temp.h"
#include "modules/drivers/k10.h"
#include "string.h"

static struct usart_module K10_uart;
static unsigned char buffer[60];
static char buffer2[60];
static bool buffer_received = false;

bool K10_init(void){
	struct usart_config usart_conf;
	
	usart_get_config_defaults(&usart_conf);
	usart_conf.mux_setting = USART_RX_3_TX_2_XCK_3;
	usart_conf.pinmux_pad0 = PINMUX_UNUSED;
	usart_conf.pinmux_pad1 = PINMUX_UNUSED;
	usart_conf.pinmux_pad2 = PINMUX_PA18C_SERCOM1_PAD2;
	usart_conf.pinmux_pad3 = PINMUX_PA19C_SERCOM1_PAD3;
	usart_conf.baudrate    = 9600;

	while (usart_init(&K10_uart, SERCOM1, &usart_conf) != STATUS_OK ) {
		//add a timeout if blocked here
	}
	usart_register_callback(&K10_uart, K10_buffer_received , USART_CALLBACK_BUFFER_RECEIVED);
	usart_enable_callback(&K10_uart, USART_CALLBACK_BUFFER_RECEIVED);

	usart_enable(&K10_uart);
	
	usart_read_buffer_job(&K10_uart ,buffer , 60);
	return true;
	
}

void calib(int cmd) {
	switch (cmd) {
		case 0:
		{
			unsigned char calib[] = "Cal,clean\r";
			usart_write_buffer_wait(&K10_uart , calib , sizeof(calib));
		}
		break;
		case 1:
		{
			unsigned char calib[] = "Cal,dry\r";
			usart_write_buffer_wait(&K10_uart , calib , sizeof(calib));
		}
		break;
		case 2:
		{
			unsigned char calib[] = "Cal,low,12880\r";
			usart_write_buffer_wait(&K10_uart , calib , sizeof(calib));
		}
		break;
		case 3:
		{
			unsigned char calib[] = "Cal,high,80000\r";
			usart_write_buffer_wait(&K10_uart , calib , sizeof(calib));
		}
		break;
	}
}

bool K10_read_conductivity(float* EC , float* TDS , float* S , float* SG){
	
	
	if(buffer_received == false){
		return false;
	}
	float temp = ENV_TEMP_get_temp();
	char calib[8];
	sprintf( calib , "T,%4.2f\r" , temp);
	usart_write_buffer_wait(&K10_uart , (unsigned char*) calib , 8);
	strncpy(buffer2 , (char*) buffer , 60);
	buffer_received = false;

	usart_read_buffer_job(&K10_uart ,buffer , 60);
	
	char* start;
	char* end;
	start = strchr(buffer2 , '\r');
	if(start == NULL || strlen(start) < 40){
		return false;
	}
	
	end = strchr(start+1 , '\r');
	if(end == NULL || end - start < 17){
		return false;
	}
	
	int i = 0;
	for(int j = 0 ; j < end - start ; j++){
		if(start[j] == ','){
			i++;
		}
	}
	
	if(i != 3){
		return false;
	}
	
	char data[end - start];
	strncpy(data , start , end - start);
	char* token = strtok(data, ",");
	if(token == NULL){
		return false;
	}
	*EC = atof(token);
	token = strtok(NULL, ",");
	if(token == NULL){
		return false;
	}
	*TDS = atof(token);
	
	token = strtok(NULL, ",");
	if(token == NULL){
		return false;
	}
	*S = atof(token);
	
	token = strtok(NULL, "\r");
	if(token == NULL){
		return false;
	}
	*SG = atof(token);

	return true;
}

void K10_buffer_received(struct usart_module *const module){
	buffer_received = true;
}

bool K10_recover_data(char* string , int terminator, float* data){
	char* end_data;
	end_data = strchr(string , terminator);
	if(end_data == NULL){
		return false;
	}
	char temp[end_data - string];
	strncpy(temp , string , end_data - string);
	*data = atof(temp);
	string = end_data;
	return true;
}