// File: ADC.c
// Name: Brian Cheung and Sam Wang
// Date: 5/4/18
// Desc: This software configures ADC0
// Usage: Initialize, sample, and convert ADC0 data

#include "ADC.h"
#include "TExaS.h"
#include "../inc/tm4c123gh6pm.h"



// ADC initialization function 
// Input: none
// Output: none
// measures from PD2, analog channel 5
void ADC_Init(void){ 
	volatile unsigned long delay;
	
	SYSCTL_RCGCGPIO_R |= 0x8;		//PD2 Init
	delay = SYSCTL_RCGCGPIO_R;
	GPIO_PORTD_DIR_R &= ~0x4; 
	GPIO_PORTD_AFSEL_R |= 0x4;
	GPIO_PORTD_AMSEL_R |= 0x4;
	GPIO_PORTD_DEN_R &= ~0x4;
	
	SYSCTL_RCGCADC_R |=	0x1;		//ADC specific Init
	delay = SYSCTL_RCGCADC_R;
	delay = SYSCTL_RCGCADC_R;
	delay = SYSCTL_RCGCADC_R;
	delay = SYSCTL_RCGCADC_R;
	delay = SYSCTL_RCGCADC_R;
	delay = SYSCTL_RCGCADC_R;
	delay = SYSCTL_RCGCADC_R;
	delay = SYSCTL_RCGCADC_R;
	ADC0_PC_R = 0x1;            // 7) configure for 125K 
	ADC0_SSPRI_R = 0x123;       // 8) Seq 3 is highest priority
	ADC0_ACTSS_R &= ~0x8;       // 9) disable sample sequencer 3
	ADC0_EMUX_R &= ~0xF000;     // 10) seq3 is software trigger
	ADC0_SSMUX3_R = (ADC0_SSMUX3_R&0xFFFFFFF0)+5;  // 11) Ain5 (PD2)
	ADC0_SSCTL3_R = 0x6;        // 12) no TS0 D0, yes IE0 END0
	ADC0_IM_R &= ~0x8;          // 13) disable SS3 interrupts
	ADC0_ACTSS_R |= 0x8;        // 14) enable sample sequencer 3
	
}

//------------ADC_In------------
// Busy-wait Analog to digital conversion
// Input: none
// Output: 12-bit result of ADC conversion
// measures from PD2, analog channel 5
uint32_t ADC_In(void){  
	uint32_t data;
	ADC0_PSSI_R = 0x0008;            
	while((ADC0_RIS_R&0x08)==0){};   
	data = ADC0_SSFIFO3_R&0xFFF; 
	ADC0_ISC_R = 0x0008; 
	return data;
}

uint8_t menuScale(uint32_t input){
	if(input <= 1364){
		return 2;
	} else if(input > 1364 && input <= 2728){
		return 1;
	} else {
		return 0;
	}
}
