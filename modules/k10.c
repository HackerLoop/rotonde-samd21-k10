/*
 * k10.c
 *
 * Created: 21/01/2016 21:57:07
 *  Author: GroundLayer
 */ 

#include "modules/drivers/k10.h"
#include "modules/modules/k10.h"

#include "registrar/registrar.h"
#include "tasks/modules.h"

#define MODULE_NAME "K10"
#define MODULE_DATA_SIZE 4
#define MODULE_DATA1 "EC"
#define MODULE_DATA2 "TDS"
#define MODULE_DATA3 "S"
#define MODULE_DATA4 "SG"

#define CALIB_NAME "K10_CALIB"
#define CALIB_N_DATA 1
#define CALIB_CMD "cmd"

data_name K10_names[] = { MODULE_DATA1 , MODULE_DATA2 , MODULE_DATA3 ,  MODULE_DATA4};
data_type K10_types[] = { FLOAT , FLOAT , FLOAT,FLOAT};

data_name calib_names[] = {CALIB_CMD};
data_type calib_types[] = {INT};

void K10_tick(void);
definition *K10_def;

void calib_action(instance);
definition *K10_calib;

void K10_module_init(void){
	K10_init();
	K10_def = register_definition(EVENT , MODULE_NAME , K10_names ,K10_types , MODULE_DATA_SIZE , NULL, K10_tick , 1000);
	K10_calib = register_definition(ACTION, CALIB_NAME, calib_names, calib_types, CALIB_N_DATA, calib_action, NULL, 0);
}

void K10_tick(void) {
	float EC , TDS , S, SG;
	
	if(K10_read_conductivity(&EC , &TDS , &S , &SG)) {
		instance inst = create_instance(MODULE_NAME);
		if(inst.def != NULL && inst.values != NULL){
			(*(float*) inst.values[0]) = EC;
			(*(float*) inst.values[1]) = TDS;
			(*(float*) inst.values[2]) = S;
			(*(float*) inst.values[3]) = SG;
			portBASE_TYPE xStatus = xQueueSendToBack(module_out, &inst , 10 / portTICK_RATE_MS );
			if(xStatus != pdPASS){
				release_instance(inst);
			}
		}
	}
}

void calib_action(instance inst) {
	int cmd = *((int *)inst.values[0]);
	
	calib(cmd);
}