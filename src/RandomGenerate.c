// File: InputHardware.c
// Name: Brian Cheung and Sam Wang
// Date: 5/4/18
// Desc: This software configures the random number generator
// Usage: Seed the random number generator and generate a random number

#include "RandomGenerate.h"
#include "random.h"
#include "../inc/tm4c123gh6pm.h"

void Seed(void){
	Random_Init(NVIC_ST_CURRENT_R);
}

uint32_t randomGenerate(uint32_t min, uint32_t max){
	return ((Random32()>>24)%(max-min))+min;
}
