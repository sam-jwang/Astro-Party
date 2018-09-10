// File: Timer0.c
// Name: Brian Cheung and Sam Wang
// Date: 5/4/18
// Desc: This software initializes Timer0, sets the sound effect, and outputs to the DAC
// Usage: Use TIMER0 in 32-bit periodic mode to request interrupts at a periodic rate

/* This example accompanies the book
   "Embedded Systems: Real Time Interfacing to Arm Cortex M Microcontrollers",
   ISBN: 978-1463590154, Jonathan Valvano, copyright (c) 2015
  Program 7.5, example 7.6

 Copyright 2015 by Jonathan W. Valvano, valvano@mail.utexas.edu
    You may use, edit, run or distribute this file
    as long as the above copyright notice remains
 THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 VALVANO SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL,
 OR CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 For more information about my classes, my research, and my books, see
 http://users.ece.utexas.edu/~valvano/
 */
#include "Timer0.h"
#include "DAC.h"
#include "tm4c123gh6pm.h"



//void (*PeriodicTask0)(void);   // user function
static uint8_t const *soundPt;
static uint8_t const *arraySize;
static uint32_t pd;

// ***************** Timer0_Init ****************
// Activate TIMER0 interrupts to run user task periodically
// Inputs:  task is a pointer to a user function
//          period in units (1/clockfreq)
// Outputs: none
void Timer0_Init(/*void(*task)(void),*/ uint32_t period){
  SYSCTL_RCGCTIMER_R |= 0x01;   // 0) activate TIMER0
  //  PeriodicTask0 = task;          // user function
  TIMER0_CTL_R = 0x00000000;    // 1) disable TIMER0A during setup
  TIMER0_CFG_R = 0x00000000;    // 2) configure for 32-bit mode
  TIMER0_TAMR_R = 0x00000002;   // 3) configure for periodic mode, default down-count settings
  TIMER0_TAILR_R = period-1;    // 4) reload value
  pd = period - 1;
  TIMER0_TAPR_R = 0;            // 5) bus clock resolution
  TIMER0_ICR_R = 0x00000001;    // 6) clear TIMER0A timeout flag
  TIMER0_IMR_R = 0x00000001;    // 7) arm timeout interrupt
  NVIC_PRI4_R = (NVIC_PRI4_R&0x00FFFFFF)|0x20000000; // 8) priority 1
  // interrupts enabled in the main program after all devices initialized
  // vector number 35, interrupt number 19
  NVIC_EN0_R = 1<<19;           // 9) enable IRQ 19 in NVIC
  //  TIMER0_CTL_R = 0x00000001;    // 10) enable TIMER0A
}


void Timer0A_SoundSet(const uint8_t *pt, uint32_t size){
  TIMER0_TAILR_R = pd;										//reload
  TIMER0_CTL_R = 0x00000001;							//enable timer0
  soundPt = pt;														//pointer to sound effect array
  arraySize = size + soundPt;							//size
	
}

void Timer0A_Handler(void){
  TIMER0_ICR_R = TIMER_ICR_TATOCINT;// acknowledge TIMER0A timeout
	DAC_Out(*soundPt);											//sound
	soundPt++;															//increment pointer in sound effect array		
	if(soundPt == arraySize){
		DAC_Out(0);
		TIMER0_CTL_R = 0x00000000;						//set when go through array
	}
}
