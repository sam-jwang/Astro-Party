// File: Sound.c
// Name: Brian Cheung and Sam Wang
// Date: 5/4/18
// Desc: This software configures the sound effects
// Usage: Play specific sound effects

#include "Sound.h"
#include "Sounds.h"
#include "DAC.h"
#include "Timer0.h"



void Sound_Init(void){
	DAC_Init();
	Timer0_Init(10000);
};

void Sound_Shoot(void){
	Timer0A_SoundSet(&pew[0],PEW_N);
};

void Sound_NoShoot(void){
	Timer0A_SoundSet(&pewNoAmmo[0],PEW_NO_AMMO_N);
};

void Sound_Explosion1(void){
	Timer0A_SoundSet(&kaboom[0],KABOOM_N);
};

void Sound_Explosion2(void){
	Timer0A_SoundSet(&kapow[0],KAPOW_N);
};

void Sound_P1Win(void){
	Timer0A_SoundSet(&P1Win[0],P1_WIN_N);
};

void Sound_P2Win(void){
	Timer0A_SoundSet(&P2Win[0],P2_WIN_N);
};

void Sound_Win(void){
	Timer0A_SoundSet(&Win[0],WIN_N);
};
