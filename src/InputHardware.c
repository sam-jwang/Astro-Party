// File: InputHardware.c
// Name: Brian Cheung and Sam Wang
// Date: 5/4/18
// Desc: This software configures hardware ports, SysTick Timer, and Edge-Triggered interrupts
// Usage: Initialize the hardware ports, SysTick Timer, and Edge-Triggered interrupts; Defines Interrupt Service Routines

#include "InputHardware.h"
#include "AstroParty.h"
#include "../inc/tm4c123gh6pm.h"
#include "ADC.h"

#define PF1       (*((volatile uint32_t *)0x40025008))
#define PF2       (*((volatile uint32_t *)0x40025010))
#define PF3       (*((volatile uint32_t *)0x40025020))



void Port_Init(void){
	volatile unsigned long delay;
	
	ADC_Init();
	
	//turn on clock for PF and PE
	SYSCTL_RCGCGPIO_R |= 0x30;
	delay = SYSCTL_RCGCGPIO_R;

	//initialize PF4-0
	GPIO_PORTF_LOCK_R = 0x4C4F434B;	//unlock Port F
	GPIO_PORTF_CR_R = 0x1F;
	GPIO_PORTF_AMSEL_R &= ~0x1F; //disable analog function PF4-0
	GPIO_PORTF_AFSEL_R &= ~0x1F; //regular function on PF4-0
	GPIO_PORTF_DIR_R &= ~0x11;  //input on PF4 and PF0
	GPIO_PORTF_DIR_R |= 0xE;		//output on PF3-1
	GPIO_PORTF_PUR_R |= 0x11;		//negative logic pull down resistor
	GPIO_PORTF_DEN_R |= 0x1F;   //enable digital on PF4-0
	
	//initialize PE3-0
	GPIO_PORTE_DIR_R &= ~0xF; 
	GPIO_PORTE_AFSEL_R |= 0xF;
	GPIO_PORTE_AMSEL_R |= 0xF;
	GPIO_PORTE_DEN_R |= 0xF;
	
	
}

void SysTick_Init(void){
	//off
	NVIC_ST_CTRL_R &= ~(0x01);
	//set start
	NVIC_ST_RELOAD_R = 2666667; //30Hz
	//clear current
	NVIC_ST_CURRENT_R= 0;
	NVIC_SYS_PRI3_R = (NVIC_SYS_PRI3_R&0x00FFFFFF)|0x40000000; // 8) priority 2
	//enable Systick with core clock
	NVIC_ST_CTRL_R = 0x7;
}

//SysTick ISR performs different tasks at different game stages
uint8_t PEStatus;	//flag
uint8_t portE;		//mailbox
uint8_t PFStatus;	//flag
uint8_t portF;		//mailbox
uint8_t ADCStatus;//flag
uint32_t ADCValue;	//mailbox
void SysTick_Handler(void){ // every 25 ms
	switch (stage){
		case StartScreen : {
			PF1^=0x02;						//toggle a heartbeat
			PF1^=0x02;						//toggle a heartbeat
			PFStatus = 1;
			portF = GPIO_PORTF_DATA_R;
			PF1^=0x02;						//toggle a heartbeat
			break;
		}
		case Menu : {
			PF1^=0x02;						//toggle a heartbeat
			PF1^=0x02;						//toggle a heartbeat
			ADCStatus = 1;
			ADCValue = ADC_In();
			PFStatus = 1;
			portF = GPIO_PORTF_DATA_R;
			PF1^=0x02;						//toggle a heartbeat
			break;
		}
		case Game : {
			//update particle life
			for(uint8_t i=0; i<PARTICLES_N;i++){
				if(particles[i].active && particles[i].time!=0){
					particles[i].time--;
				}
			}
			//update powerUpCooldowns
			for(uint8_t i=0;i<POWERUPS_N;i++){
				if(game.powerUps[i].powerUpCooldown != 0){
					game.powerUps[i].powerUpCooldown--;
				}
			}
			if(blades.displayed){
				if(blades.bladeTime!=0){
					blades.bladeTime--;
				} else {
					blades.displayed = 0;
					(blades.player)->powerUp = None;
				}
			}
			//update player shotCooldown
			if(p1.shotCooldown != 0){
				p1.shotCooldown--;
			}
			if(p2.shotCooldown != 0){
				p2.shotCooldown--;
			}
			PF1^=0x02;						//toggle a heartbeat
			PF1^=0x02;						//toggle a heartbeat
			PEStatus = 1;
			portE = GPIO_PORTE_DATA_R;
			PF1^=0x02;						//toggle a heartbeat
			//return from interrupt
			break;
		}
		case Win : {
			break;
		}
	};
}

//Edge-Triggered Interrupts
volatile unsigned long FallingEdges = 0;
void EdgeCounter_Init(void){       
	SYSCTL_RCGCGPIO_R |= 0x00000020; // (a) activate clock for port F
	FallingEdges = 0;             // (b) initialize count and wait for clock
	GPIO_PORTF_DIR_R &= ~0x10;    // (c) make PF4 in (built-in button)
	GPIO_PORTF_AFSEL_R &= ~0x10;  //     disable alt funct on PF4
	GPIO_PORTF_DEN_R |= 0x10;     //     enable digital I/O on PF4
	GPIO_PORTF_PCTL_R &= ~0x000F0000; //  configure PF4 as GPIO
	GPIO_PORTF_AMSEL_R &= ~0x10;  //    disable analog functionality on PF4
	GPIO_PORTF_PUR_R |= 0x10;     //     enable weak pull-up on PF4
	GPIO_PORTF_IS_R &= ~0x10;     // (d) PF4 is edge-sensitive
	GPIO_PORTF_IBE_R &= ~0x10;    //     PF4 is not both edges
	GPIO_PORTF_IEV_R &= ~0x10;    //     PF4 falling edge event
	GPIO_PORTF_ICR_R = 0x10;      // (e) clear flag4
	GPIO_PORTF_IM_R |= 0x10;      // (f) arm interrupt on PF4
	NVIC_PRI7_R = (NVIC_PRI7_R&0xFF00FFFF)|0x00000000; // (g) priority 0
	NVIC_EN0_R = 0x40000000;      // (h) enable interrupt 30 in NVIC

}

//interrupt sets pause game flag
void GPIOPortF_Handler(void){
	GPIO_PORTF_ICR_R = 0x10;      // acknowledge flag4
	FallingEdges = FallingEdges + 1;
	game.paused ^= 1;

}
