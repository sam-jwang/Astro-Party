// File: DAC.c
// Name: Brian Cheung and Sam Wang
// Date: 5/4/18
// Desc: This software configures DAC 
// Usage: Initialize and output to the DAC

#include "DAC.h"
#include "../inc/tm4c123gh6pm.h"
#include "ADC.h"



double audioScale(double input);

// **************DAC_Init*********************
// Initialize 4-bit DAC, called once 
// Input: none
// Output: none
void DAC_Init(void){   
	volatile unsigned long delay;
	SYSCTL_RCGCGPIO_R |= 0x2;
	delay = SYSCTL_RCGCGPIO_R; 
	
	GPIO_PORTB_AMSEL_R &= ~0xFF; //disable analog function on PB7-0
	GPIO_PORTB_AFSEL_R &= ~0xFF; //regular function on PB7-0
	GPIO_PORTB_DR8R_R |= 0xFF;
	GPIO_PORTB_DIR_R |= 0xFF;    //outputs on PB7-0
	GPIO_PORTB_DEN_R |= 0xFF;    //enable digital on PB7-0
}

// **************DAC_Out*********************
// output to DAC
// Input: 4-bit data, 0 to 15 
// Input=n is converted to n*3.3V/15
// Output: none
void DAC_Out(uint8_t data){
	double scalar;
	scalar = audioScale(ADC_In());
	data = data*scalar;
	GPIO_PORTB_DATA_R = data;
}

double audioScale(double input){
	return 1- input/4095;
}
