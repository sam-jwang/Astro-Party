// File: AstroParty.c
// Name: Brian Cheung and Sam Wang
// Date: 5/4/18
// Desc: This software is the main game engine
// Usage: Handles game inputs, events, and outputs to LCD

#include "AstroParty.h"
#include "ST7735.h"
#include "PLL.h"
#include "InputHardware.h"
#include "ADC.h"
#include "Sound.h"
#include "LCDBuffer.h"
#include "Images.h"
#include "GamePhysics.h"
#include "RandomGenerate.h"

#define HEIGHT 160
#define WIDTH 128

void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
void Delay100ms(uint32_t count); // time delay in 0.1 seconds
void startScreen(void);
void menu(void);
void changeMenuItem(void);
void selectMenuItem(void);
void gameInit(void);
void playerInit(void);
void cornerMapInit(void);
void hallMapInit(void);
void cacheMapInit(void);
void powerUpInit(void);
void update(void);
void pauseGame(void);
void win(void);

//import mailboxes and flags
extern uint8_t PEStatus;
extern uint8_t portE;
extern uint8_t PFStatus;
extern uint8_t portF;
extern uint8_t ADCStatus;
extern uint32_t ADCValue;



//X and Y positions of players and powerups
const double P1_X[3] = {11.0,11.0,6.0};
const double P1_Y[3] = {20.0,20.0,15.0};
const double P2_X[3] = {108.0,108.0,113.0};
const double P2_Y[3] = {147.0,147.0,153.0};

const double PU0_X[3] = {60.0,89.0,30.0};
const double PU0_Y[3] = {84.0,48.0,120.0};
const double PU1_X[3] = {60.0,13.0,105.0};
const double PU1_Y[3] = {84.0,146.0,20.0};
const double PU2_X[3] = {60.0,6.0,113.0};
const double PU2_Y[3] = {84.0,153.0,15.0};

uint32_t randomInd;

enum MenuEnum {KillsMenuItem,MapMenuItem,StartMenuItem};
enum GameStageEnum stage;
enum MenuEnum menuItem;

uint8_t killsIndex = 0;
uint8_t kills[3] = {1,3,5};
const uint8_t *killsBtns[3] = {Kill_1,Kill_3,Kill_5};

uint8_t mapIndex = 0;
const uint8_t *mapBtns[3] = {Level_Corner,Level_Hall,Level_Cache};


//create game variables
GameState game;
Player p1;
Player p2;

uint8_t screenBuffer[20480];
LaserBeamPU laserBeam = {0,0,0,0,0,0,Laser_Beam_Sprite,0};
Particle particles[PARTICLES_N];

uint8_t startGame = 0 ;

int main(void){
	//initialize hardware
	PLL_Init(Bus80MHz);       // Bus clock is 80 MHz 
	Port_Init();
	Sound_Init();
	Output_Init();  					//init LCD
	
	stage = StartScreen;
	ST7735_DrawBitmap16(0, 159, Astro_Start_Menu, 128,160);
	
	SysTick_Init();
	EnableInterrupts();
	
	startScreen();
	
	stage = Menu;
	menu();
		
	gameInit();
	stage = Game;
	
	EdgeCounter_Init();

	while(p1.kills!=game.kills && p2.kills!=game.kills){
		update();
		ST7735_DrawBitmap8(0, 159, screenBuffer, 128,160);
	}
	
	stage = Win;
	win();
}

//stay on start screen until start button(PF4) is pressed
void startScreen(void){
	while(PFStatus == 0){}
	while((portF&0x10) == 0x10){
	}
	PFStatus= 0 ;
}

#define MENU_BTN_X 30
#define SPRITE_X 12

//stay on menu screen until start button is pressed
void menu(void){
	//draw buttons
	ST7735_FillScreen(0);
	clear();
	draw(WIDTH-(MENU_BTN_X+KILLS_BTN_WIDTH),140,killsBtns[killsIndex],KILLS_BTN_WIDTH,KILLS_BTN_HEIGHT);
	draw(WIDTH-(MENU_BTN_X+LEVEL_BTN_WIDTH),85,mapBtns[mapIndex],LEVEL_BTN_WIDTH,LEVEL_BTN_HEIGHT);
	draw(WIDTH-(MENU_BTN_X+START_BTN_WIDTH),30,Start,START_BTN_WIDTH,START_BTN_HEIGHT);
	ST7735_DrawBitmap8(0, 159, screenBuffer, 128,160);
	
	//poll for input change 
	while(!startGame){
		while(ADCStatus == 0){}
		ADCStatus = 0;
		while((portF & 0x1) == 0x0){}
		changeMenuItem();
		
		uint32_t count = 1000000;
		while(count!=0){
			count--;
		}
		if((portF & 0x1) == 0x00){
			selectMenuItem();
		}
	}
}

//update highlighted menu item
void changeMenuItem(void){
	if(menuScale(ADCValue) == 0){
		//erase other sprites
		fillRect(WIDTH-(SPRITE_X+PLAYER_WIDTH),75,PLAYER_WIDTH,PLAYER_HEIGHT,0x00);
		fillRect(WIDTH-(SPRITE_X+PLAYER_WIDTH),25,PLAYER_WIDTH,PLAYER_HEIGHT,0x00);
		//draw sprite
		draw(WIDTH-(SPRITE_X+PLAYER_WIDTH),130,P1_8,PLAYER_WIDTH,PLAYER_HEIGHT);
		menuItem = KillsMenuItem;
	} else if(menuScale(ADC_In()) == 1){
		//erase other sprites
		fillRect(WIDTH-(SPRITE_X+PLAYER_WIDTH),130,PLAYER_WIDTH,PLAYER_HEIGHT,0x00);
		fillRect(WIDTH-(SPRITE_X+PLAYER_WIDTH),25,PLAYER_WIDTH,PLAYER_HEIGHT,0x00);
		//move sprite
		draw(WIDTH-(SPRITE_X+PLAYER_WIDTH),75,P1_8,PLAYER_WIDTH,PLAYER_HEIGHT);
		menuItem = MapMenuItem;
	} else if(menuScale(ADC_In()) == 2){
		//erase other sprites
		fillRect(WIDTH-(SPRITE_X+PLAYER_WIDTH),75,PLAYER_WIDTH,PLAYER_HEIGHT,0x00);
		fillRect(WIDTH-(SPRITE_X+PLAYER_WIDTH),130,PLAYER_WIDTH,PLAYER_HEIGHT,0x00);
		//draw sprite
		draw(WIDTH-(SPRITE_X+PLAYER_WIDTH),25,P1_8,PLAYER_WIDTH,PLAYER_HEIGHT);
		menuItem = StartMenuItem;
	}
	ST7735_DrawBitmap8(0, 159, screenBuffer, 128,160);
}

//perform action according to menu item selected
void selectMenuItem(void){
	if(PFStatus){
		PFStatus = 0;
		if((portF&0x1) == 0x0){
			switch(menuItem) {
				case KillsMenuItem : {
					//change kill count
					killsIndex = (killsIndex+1)%3;
					draw(WIDTH-(MENU_BTN_X+KILLS_BTN_WIDTH),140,killsBtns[killsIndex],KILLS_BTN_WIDTH,KILLS_BTN_HEIGHT);
					break;
				}
				case MapMenuItem : {
					//change map
					mapIndex = (mapIndex+1)%3;
					draw(WIDTH-(MENU_BTN_X+LEVEL_BTN_WIDTH),85,mapBtns[mapIndex],LEVEL_BTN_WIDTH,LEVEL_BTN_HEIGHT);
					break;
				}
				case StartMenuItem : {
					//start game
					startGame = 1;
					break;
				}
			};
		}
		ST7735_DrawBitmap8(0, 159, screenBuffer, 128,160);
	}
}

//initialize game variables
void gameInit(void){
	clear();
	
	playerInit();
	
	Seed();
	randomInd = randomGenerate(0,2);
	
	if(mapIndex == 0){
		cornerMapInit();
	} else if (mapIndex == 1){
		hallMapInit();
	} else {
		cacheMapInit();
	}
	
	powerUpInit();
	
	//draw game objects
	draw(0,159,game.map,128,160);
	drawPlayer(&p1);										
	drawPlayer(&p2);										
	drawWalls();
	drawPowerUps();

}

//initialize players
void playerInit(void){
	p1 = (Player){.x = P1_X[mapIndex],.y = P1_Y[mapIndex],.dx = 0.0,.dy = 0.0,
								.dir = 0,.angle = 0,
								.sprites = {P1_0,P1_1,P1_2,P1_3,P1_4,P1_5,P1_6,P1_7,P1_8,P1_9,P1_10,P1_11,P1_12,P1_13,P1_14,P1_15},
								.color = PLAYER1_COLOR,
								.ammo = {{.x = 0.0,.y = 0.0,.dx = 0.0,.dy = 0.0,.expX = 0.0,.expY = 0.0,.bulletSprite = Bullet_Sprite,.bulletExp = Bullet_Explosion,.active = 0,.destroyed = 0},
												{.x = 0.0,.y = 0.0,.dx = 0.0,.dy = 0.0,.expX = 0.0,.expY = 0.0,.bulletSprite = Bullet_Sprite,.bulletExp = Bullet_Explosion,.active = 0,.destroyed = 0},
												{.x = 0.0,.y = 0.0,.dx = 0.0,.dy = 0.0,.expX = 0.0,.expY = 0.0,.bulletSprite = Bullet_Sprite,.bulletExp = Bullet_Explosion,.active = 0,.destroyed = 0}},
								.shotCooldown = 0,
								.alive = 1,
								.kills = 0,
								.powerUp = None
	};
	p2 = (Player){.x = P2_X[mapIndex],.y = P2_Y[mapIndex],.dx = 0.0,.dy = 0.0,
								.dir = 8,.angle = 180,
								.sprites = {P1_0,P1_1,P1_2,P1_3,P1_4,P1_5,P1_6,P1_7,P1_8,P1_9,P1_10,P1_11,P1_12,P1_13,P1_14,P1_15},
								.color = PLAYER2_COLOR,
								.ammo = {{.x = 0.0,.y = 0.0,.dx = 0.0,.dy = 0.0,.expX = 0.0,.expY = 0.0,.bulletSprite = Bullet_Sprite,.bulletExp = Bullet_Explosion,.active = 0,.destroyed = 0},
												{.x = 0.0,.y = 0.0,.dx = 0.0,.dy = 0.0,.expX = 0.0,.expY = 0.0,.bulletSprite = Bullet_Sprite,.bulletExp = Bullet_Explosion,.active = 0,.destroyed = 0},
												{.x = 0.0,.y = 0.0,.dx = 0.0,.dy = 0.0,.expX = 0.0,.expY = 0.0,.bulletSprite = Bullet_Sprite,.bulletExp = Bullet_Explosion,.active = 0,.destroyed = 0}},
								.shotCooldown = 0,
								.alive = 1,
								.kills = 0,
								.powerUp = None
	};
}

//initialize corner map
void cornerMapInit(void){
	game = (GameState) {.paused = 0,
											.clockwise = 1,
											.kills = kills[killsIndex],
											.map = Corner_Map,
											.walls = {{.active = 1,.x = 44,.y = 74,.destructible = 1,.displayed = 0,.destroyed = 0,.colors = {WALL_COLOR1,WALL_COLOR2}},
																{.active = 1,.x = 44,.y = 84,.destructible = 1,.displayed = 0,.destroyed = 0,.colors = {WALL_COLOR1,WALL_COLOR2}},
																{.active = 1,.x = 44,.y = 94,.destructible = 1,.displayed = 0,.destroyed = 0,.colors = {WALL_COLOR1,WALL_COLOR2}},
																{.active = 1,.x = 74,.y = 74,.destructible = 1,.displayed = 0,.destroyed = 0,.colors = {WALL_COLOR1,WALL_COLOR2}},
																{.active = 1,.x = 74,.y = 84,.destructible = 1,.displayed = 0,.destroyed = 0,.colors = {WALL_COLOR1,WALL_COLOR2}},
																{.active = 1,.x = 74,.y = 94,.destructible = 1,.displayed = 0,.destroyed = 0,.colors = {WALL_COLOR1,WALL_COLOR2}},
																{.active = 1,.x = 54,.y = 64,.destructible = 1,.displayed = 0,.destroyed = 0,.colors = {WALL_COLOR1,WALL_COLOR2}},
																{.active = 1,.x = 64,.y = 64,.destructible = 1,.displayed = 0,.destroyed = 0,.colors = {WALL_COLOR1,WALL_COLOR2}},
																{.active = 1,.x = 54,.y = 104,.destructible = 1,.displayed = 0,.destroyed = 0,.colors = {WALL_COLOR1,WALL_COLOR2}},
																{.active = 1,.x = 64,.y = 104,.destructible = 1,.displayed = 0,.destroyed = 0,.colors = {WALL_COLOR1,WALL_COLOR2}},

																{.active = 1,.x = 24,.y = 64,.destructible = 0,.displayed = 0,.destroyed = 0,.colors = {WALL_COLOR3,WALL_COLOR3}},
																{.active = 1,.x = 34,.y = 64,.destructible = 0,.displayed = 0,.destroyed = 0,.colors = {WALL_COLOR3,WALL_COLOR3}},
																{.active = 1,.x = 44,.y = 64,.destructible = 0,.displayed = 0,.destroyed = 0,.colors = {WALL_COLOR3,WALL_COLOR3}},
																{.active = 1,.x = 44,.y = 54,.destructible = 0,.displayed = 0,.destroyed = 0,.colors = {WALL_COLOR3,WALL_COLOR3}},
																{.active = 1,.x = 44,.y = 44,.destructible = 0,.displayed = 0,.destroyed = 0,.colors = {WALL_COLOR3,WALL_COLOR3}},
																{.active = 1,.x = 74,.y = 44,.destructible = 0,.displayed = 0,.destroyed = 0,.colors = {WALL_COLOR3,WALL_COLOR3}},
																{.active = 1,.x = 74,.y = 54,.destructible = 0,.displayed = 0,.destroyed = 0,.colors = {WALL_COLOR3,WALL_COLOR3}},
																{.active = 1,.x = 74,.y = 64,.destructible = 0,.displayed = 0,.destroyed = 0,.colors = {WALL_COLOR3,WALL_COLOR3}},
																{.active = 1,.x = 84,.y = 64,.destructible = 0,.displayed = 0,.destroyed = 0,.colors = {WALL_COLOR3,WALL_COLOR3}},
																{.active = 1,.x = 94,.y = 64,.destructible = 0,.displayed = 0,.destroyed = 0,.colors = {WALL_COLOR3,WALL_COLOR3}},
																{.active = 1,.x = 24,.y = 104,.destructible = 0,.displayed = 0,.destroyed = 0,.colors = {WALL_COLOR3,WALL_COLOR3}},
																{.active = 1,.x = 34,.y = 104,.destructible = 0,.displayed = 0,.destroyed = 0,.colors = {WALL_COLOR3,WALL_COLOR3}},
																{.active = 1,.x = 44,.y = 104,.destructible = 0,.displayed = 0,.destroyed = 0,.colors = {WALL_COLOR3,WALL_COLOR3}},
																{.active = 1,.x = 44,.y = 114,.destructible = 0,.displayed = 0,.destroyed = 0,.colors = {WALL_COLOR3,WALL_COLOR3}},
																{.active = 1,.x = 44,.y = 124,.destructible = 0,.displayed = 0,.destroyed = 0,.colors = {WALL_COLOR3,WALL_COLOR3}},
																{.active = 1,.x = 74,.y = 104,.destructible = 0,.displayed = 0,.destroyed = 0,.colors = {WALL_COLOR3,WALL_COLOR3}},
																{.active = 1,.x = 74,.y = 114,.destructible = 0,.displayed = 0,.destroyed = 0,.colors = {WALL_COLOR3,WALL_COLOR3}},
																{.active = 1,.x = 74,.y = 124,.destructible = 0,.displayed = 0,.destroyed = 0,.colors = {WALL_COLOR3,WALL_COLOR3}},
																{.active = 1,.x = 84,.y = 104,.destructible = 0,.displayed = 0,.destroyed = 0,.colors = {WALL_COLOR3,WALL_COLOR3}},
																{.active = 1,.x = 94,.y = 104,.destructible = 0,.displayed = 0,.destroyed = 0,.colors = {WALL_COLOR3,WALL_COLOR3}},
																
																{.active = 0,.x = 0,.y = 0,.destructible = 0,.displayed = 0,.destroyed = 0,.colors = {WALL_COLOR3,WALL_COLOR3}},
																{.active = 0,.x = 0,.y = 0,.destructible = 0,.displayed = 0,.destroyed = 0,.colors = {WALL_COLOR3,WALL_COLOR3}},
																{.active = 0,.x = 0,.y = 0,.destructible = 0,.displayed = 0,.destroyed = 0,.colors = {WALL_COLOR3,WALL_COLOR3}},
																{.active = 0,.x = 0,.y = 0,.destructible = 0,.displayed = 0,.destroyed = 0,.colors = {WALL_COLOR3,WALL_COLOR3}},
																{.active = 0,.x = 0,.y = 0,.destructible = 0,.displayed = 0,.destroyed = 0,.colors = {WALL_COLOR3,WALL_COLOR3}},
																{.active = 0,.x = 0,.y = 0,.destructible = 0,.displayed = 0,.destroyed = 0,.colors = {WALL_COLOR3,WALL_COLOR3}},
																{.active = 0,.x = 0,.y = 0,.destructible = 0,.displayed = 0,.destroyed = 0,.colors = {WALL_COLOR3,WALL_COLOR3}},
																{.active = 0,.x = 0,.y = 0,.destructible = 0,.displayed = 0,.destroyed = 0,.colors = {WALL_COLOR3,WALL_COLOR3}},
																{.active = 0,.x = 0,.y = 0,.destructible = 0,.displayed = 0,.destroyed = 0,.colors = {WALL_COLOR3,WALL_COLOR3}},
																{.active = 0,.x = 0,.y = 0,.destructible = 0,.displayed = 0,.destroyed = 0,.colors = {WALL_COLOR3,WALL_COLOR3}},
																{.active = 0,.x = 0,.y = 0,.destructible = 0,.displayed = 0,.destroyed = 0,.colors = {WALL_COLOR3,WALL_COLOR3}},
																{.active = 0,.x = 0,.y = 0,.destructible = 0,.displayed = 0,.destroyed = 0,.colors = {WALL_COLOR3,WALL_COLOR3}},
																{.active = 0,.x = 0,.y = 0,.destructible = 0,.displayed = 0,.destroyed = 0,.colors = {WALL_COLOR3,WALL_COLOR3}},
																{.active = 0,.x = 0,.y = 0,.destructible = 0,.displayed = 0,.destroyed = 0,.colors = {WALL_COLOR3,WALL_COLOR3}},
																{.active = 0,.x = 0,.y = 0,.destructible = 0,.displayed = 0,.destroyed = 0,.colors = {WALL_COLOR3,WALL_COLOR3}},
																{.active = 0,.x = 0,.y = 0,.destructible = 0,.displayed = 0,.destroyed = 0,.colors = {WALL_COLOR3,WALL_COLOR3}},
																{.active = 0,.x = 0,.y = 0,.destructible = 0,.displayed = 0,.destroyed = 0,.colors = {WALL_COLOR3,WALL_COLOR3}},
																{.active = 0,.x = 0,.y = 0,.destructible = 0,.displayed = 0,.destroyed = 0,.colors = {WALL_COLOR3,WALL_COLOR3}}}
												};
	
}

//initialize hall map
void hallMapInit(void){	
	game = (GameState) {.paused = 0,
											.clockwise = 1,
											.kills = kills[killsIndex],
											.map = Hall_Map,
											.walls = {{.active = 1,.x = 35,.y = 10,.destructible = 1,.displayed = 0,.destroyed = 0,.colors = {WALL_COLOR1,WALL_COLOR2}},
																{.active = 1,.x = 35,.y = 20,.destructible = 1,.displayed = 0,.destroyed = 0,.colors = {WALL_COLOR1,WALL_COLOR2}},
																{.active = 1,.x = 35,.y = 30,.destructible = 1,.displayed = 0,.destroyed = 0,.colors = {WALL_COLOR1,WALL_COLOR2}},
																{.active = 1,.x = 35,.y = 40,.destructible = 1,.displayed = 0,.destroyed = 0,.colors = {WALL_COLOR1,WALL_COLOR2}},
																{.active = 1,.x = 35,.y = 128,.destructible = 1,.displayed = 0,.destroyed = 0,.colors = {WALL_COLOR1,WALL_COLOR2}},
																{.active = 1,.x = 35,.y = 138,.destructible = 1,.displayed = 0,.destroyed = 0,.colors = {WALL_COLOR1,WALL_COLOR2}},
																{.active = 1,.x = 35,.y = 148,.destructible = 1,.displayed = 0,.destroyed = 0,.colors = {WALL_COLOR1,WALL_COLOR2}},
																{.active = 1,.x = 35,.y = 158,.destructible = 1,.displayed = 0,.destroyed = 0,.colors = {WALL_COLOR1,WALL_COLOR2}},
																{.active = 1,.x = 83,.y = 10,.destructible = 1,.displayed = 0,.destroyed = 0,.colors = {WALL_COLOR1,WALL_COLOR2}},
																{.active = 1,.x = 83,.y = 20,.destructible = 1,.displayed = 0,.destroyed = 0,.colors = {WALL_COLOR1,WALL_COLOR2}},
																{.active = 1,.x = 83,.y = 30,.destructible = 1,.displayed = 0,.destroyed = 0,.colors = {WALL_COLOR1,WALL_COLOR2}},
																{.active = 1,.x = 83,.y = 40,.destructible = 1,.displayed = 0,.destroyed = 0,.colors = {WALL_COLOR1,WALL_COLOR2}},
																{.active = 1,.x = 83,.y = 128,.destructible = 1,.displayed = 0,.destroyed = 0,.colors = {WALL_COLOR1,WALL_COLOR2}},
																{.active = 1,.x = 83,.y = 138,.destructible = 1,.displayed = 0,.destroyed = 0,.colors = {WALL_COLOR1,WALL_COLOR2}},
																{.active = 1,.x = 83,.y = 148,.destructible = 1,.displayed = 0,.destroyed = 0,.colors = {WALL_COLOR1,WALL_COLOR2}},
																{.active = 1,.x = 83,.y = 158,.destructible = 1,.displayed = 0,.destroyed = 0,.colors = {WALL_COLOR1,WALL_COLOR2}},

																{.active = 1,.x = 35,.y = 49,.destructible = 0,.displayed = 0,.destroyed = 0,.colors = {WALL_COLOR3,WALL_COLOR3}},
																{.active = 1,.x = 35,.y = 59,.destructible = 0,.displayed = 0,.destroyed = 0,.colors = {WALL_COLOR3,WALL_COLOR3}},
																{.active = 1,.x = 35,.y = 69,.destructible = 0,.displayed = 0,.destroyed = 0,.colors = {WALL_COLOR3,WALL_COLOR3}},
																{.active = 1,.x = 35,.y = 79,.destructible = 0,.displayed = 0,.destroyed = 0,.colors = {WALL_COLOR3,WALL_COLOR3}},
																{.active = 1,.x = 35,.y = 89,.destructible = 0,.displayed = 0,.destroyed = 0,.colors = {WALL_COLOR3,WALL_COLOR3}},
																{.active = 1,.x = 35,.y = 99,.destructible = 0,.displayed = 0,.destroyed = 0,.colors = {WALL_COLOR3,WALL_COLOR3}},
																{.active = 1,.x = 35,.y = 109,.destructible = 0,.displayed = 0,.destroyed = 0,.colors = {WALL_COLOR3,WALL_COLOR3}},
																{.active = 1,.x = 35,.y = 119,.destructible = 0,.displayed = 0,.destroyed = 0,.colors = {WALL_COLOR3,WALL_COLOR3}},
																{.active = 1,.x = 83,.y = 49,.destructible = 0,.displayed = 0,.destroyed = 0,.colors = {WALL_COLOR3,WALL_COLOR3}},
																{.active = 1,.x = 83,.y = 59,.destructible = 0,.displayed = 0,.destroyed = 0,.colors = {WALL_COLOR3,WALL_COLOR3}},
																{.active = 1,.x = 83,.y = 69,.destructible = 0,.displayed = 0,.destroyed = 0,.colors = {WALL_COLOR3,WALL_COLOR3}},
																{.active = 1,.x = 83,.y = 79,.destructible = 0,.displayed = 0,.destroyed = 0,.colors = {WALL_COLOR3,WALL_COLOR3}},
																{.active = 1,.x = 83,.y = 89,.destructible = 0,.displayed = 0,.destroyed = 0,.colors = {WALL_COLOR3,WALL_COLOR3}},
																{.active = 1,.x = 83,.y = 99,.destructible = 0,.displayed = 0,.destroyed = 0,.colors = {WALL_COLOR3,WALL_COLOR3}},
																{.active = 1,.x = 83,.y = 109,.destructible = 0,.displayed = 0,.destroyed = 0,.colors = {WALL_COLOR3,WALL_COLOR3}},
																{.active = 1,.x = 83,.y = 119,.destructible = 0,.displayed = 0,.destroyed = 0,.colors = {WALL_COLOR3,WALL_COLOR3}},
																
																{.active = 0,.x = 0,.y = 0,.destructible = 0,.displayed = 0,.destroyed = 0,.colors = {WALL_COLOR3,WALL_COLOR3}},
																{.active = 0,.x = 0,.y = 0,.destructible = 0,.displayed = 0,.destroyed = 0,.colors = {WALL_COLOR3,WALL_COLOR3}},
																{.active = 0,.x = 0,.y = 0,.destructible = 0,.displayed = 0,.destroyed = 0,.colors = {WALL_COLOR3,WALL_COLOR3}},
																{.active = 0,.x = 0,.y = 0,.destructible = 0,.displayed = 0,.destroyed = 0,.colors = {WALL_COLOR3,WALL_COLOR3}},
																{.active = 0,.x = 0,.y = 0,.destructible = 0,.displayed = 0,.destroyed = 0,.colors = {WALL_COLOR3,WALL_COLOR3}},
																{.active = 0,.x = 0,.y = 0,.destructible = 0,.displayed = 0,.destroyed = 0,.colors = {WALL_COLOR3,WALL_COLOR3}},
																{.active = 0,.x = 0,.y = 0,.destructible = 0,.displayed = 0,.destroyed = 0,.colors = {WALL_COLOR3,WALL_COLOR3}},
																{.active = 0,.x = 0,.y = 0,.destructible = 0,.displayed = 0,.destroyed = 0,.colors = {WALL_COLOR3,WALL_COLOR3}},
																{.active = 0,.x = 0,.y = 0,.destructible = 0,.displayed = 0,.destroyed = 0,.colors = {WALL_COLOR3,WALL_COLOR3}},
																{.active = 0,.x = 0,.y = 0,.destructible = 0,.displayed = 0,.destroyed = 0,.colors = {WALL_COLOR3,WALL_COLOR3}},
																{.active = 0,.x = 0,.y = 0,.destructible = 0,.displayed = 0,.destroyed = 0,.colors = {WALL_COLOR3,WALL_COLOR3}},
																{.active = 0,.x = 0,.y = 0,.destructible = 0,.displayed = 0,.destroyed = 0,.colors = {WALL_COLOR3,WALL_COLOR3}},
																{.active = 0,.x = 0,.y = 0,.destructible = 0,.displayed = 0,.destroyed = 0,.colors = {WALL_COLOR3,WALL_COLOR3}},
																{.active = 0,.x = 0,.y = 0,.destructible = 0,.displayed = 0,.destroyed = 0,.colors = {WALL_COLOR3,WALL_COLOR3}},
																{.active = 0,.x = 0,.y = 0,.destructible = 0,.displayed = 0,.destroyed = 0,.colors = {WALL_COLOR3,WALL_COLOR3}},
																{.active = 0,.x = 0,.y = 0,.destructible = 0,.displayed = 0,.destroyed = 0,.colors = {WALL_COLOR3,WALL_COLOR3}}}
											};
}

//initialize cache map
void cacheMapInit(void){	
	game = (GameState) {.paused = 0,
											.clockwise = 1,
											.kills = kills[killsIndex],
											.map = Cache_Map,
											.walls = {{.active = 1,.x = 1,.y = 30,.destructible = 1,.displayed = 0,.destroyed = 0,.colors = {WALL_COLOR1,WALL_COLOR2}},
																{.active = 1,.x = 11,.y = 30,.destructible = 1,.displayed = 0,.destroyed = 0,.colors = {WALL_COLOR1,WALL_COLOR2}},
																{.active = 1,.x = 21,.y = 30,.destructible = 1,.displayed = 0,.destroyed = 0,.colors = {WALL_COLOR1,WALL_COLOR2}},
																{.active = 1,.x = 21,.y = 20,.destructible = 1,.displayed = 0,.destroyed = 0,.colors = {WALL_COLOR1,WALL_COLOR2}},
																{.active = 1,.x = 21,.y = 10,.destructible = 1,.displayed = 0,.destroyed = 0,.colors = {WALL_COLOR1,WALL_COLOR2}},
																{.active = 1,.x = 1,.y = 138,.destructible = 1,.displayed = 0,.destroyed = 0,.colors = {WALL_COLOR1,WALL_COLOR2}},
																{.active = 1,.x = 11,.y = 138,.destructible = 1,.displayed = 0,.destroyed = 0,.colors = {WALL_COLOR1,WALL_COLOR2}},
																{.active = 1,.x = 21,.y = 138,.destructible = 1,.displayed = 0,.destroyed = 0,.colors = {WALL_COLOR1,WALL_COLOR2}},
																{.active = 1,.x = 21,.y = 148,.destructible = 1,.displayed = 0,.destroyed = 0,.colors = {WALL_COLOR1,WALL_COLOR2}},
																{.active = 1,.x = 21,.y = 158,.destructible = 1,.displayed = 0,.destroyed = 0,.colors = {WALL_COLOR1,WALL_COLOR2}},
																{.active = 1,.x = 97,.y = 10,.destructible = 1,.displayed = 0,.destroyed = 0,.colors = {WALL_COLOR1,WALL_COLOR2}},
																{.active = 1,.x = 97,.y = 20,.destructible = 1,.displayed = 0,.destroyed = 0,.colors = {WALL_COLOR1,WALL_COLOR2}},
																{.active = 1,.x = 97,.y = 30,.destructible = 1,.displayed = 0,.destroyed = 0,.colors = {WALL_COLOR1,WALL_COLOR2}},
																{.active = 1,.x = 107,.y = 30,.destructible = 1,.displayed = 0,.destroyed = 0,.colors = {WALL_COLOR1,WALL_COLOR2}},
																{.active = 1,.x = 117,.y = 30,.destructible = 1,.displayed = 0,.destroyed = 0,.colors = {WALL_COLOR1,WALL_COLOR2}},
																{.active = 1,.x = 97,.y = 158,.destructible = 1,.displayed = 0,.destroyed = 0,.colors = {WALL_COLOR1,WALL_COLOR2}},
																{.active = 1,.x = 97,.y = 148,.destructible = 1,.displayed = 0,.destroyed = 0,.colors = {WALL_COLOR1,WALL_COLOR2}},
																{.active = 1,.x = 97,.y = 138,.destructible = 1,.displayed = 0,.destroyed = 0,.colors = {WALL_COLOR1,WALL_COLOR2}},
																{.active = 1,.x = 107,.y = 138,.destructible = 1,.displayed = 0,.destroyed = 0,.colors = {WALL_COLOR1,WALL_COLOR2}},
																{.active = 1,.x = 117,.y = 138,.destructible = 1,.displayed = 0,.destroyed = 0,.colors = {WALL_COLOR1,WALL_COLOR2}},
																{.active = 1,.x = 60,.y = 20,.destructible = 1,.displayed = 0,.destroyed = 0,.colors = {WALL_COLOR1,WALL_COLOR2}},
																{.active = 1,.x = 60,.y = 30,.destructible = 1,.displayed = 0,.destroyed = 0,.colors = {WALL_COLOR1,WALL_COLOR2}},
																{.active = 1,.x = 60,.y = 40,.destructible = 1,.displayed = 0,.destroyed = 0,.colors = {WALL_COLOR1,WALL_COLOR2}},
																{.active = 1,.x = 60,.y = 128,.destructible = 1,.displayed = 0,.destroyed = 0,.colors = {WALL_COLOR1,WALL_COLOR2}},
																{.active = 1,.x = 60,.y = 138,.destructible = 1,.displayed = 0,.destroyed = 0,.colors = {WALL_COLOR1,WALL_COLOR2}},
																{.active = 1,.x = 60,.y = 148,.destructible = 1,.displayed = 0,.destroyed = 0,.colors = {WALL_COLOR1,WALL_COLOR2}},
																{.active = 1,.x = 11,.y = 84,.destructible = 1,.displayed = 0,.destroyed = 0,.colors = {WALL_COLOR1,WALL_COLOR2}},
																{.active = 1,.x = 21,.y = 84,.destructible = 1,.displayed = 0,.destroyed = 0,.colors = {WALL_COLOR1,WALL_COLOR2}},
																{.active = 1,.x = 97,.y = 84,.destructible = 1,.displayed = 0,.destroyed = 0,.colors = {WALL_COLOR1,WALL_COLOR2}},
																{.active = 1,.x = 107,.y = 84,.destructible = 1,.displayed = 0,.destroyed = 0,.colors = {WALL_COLOR1,WALL_COLOR2}},
																{.active = 1,.x = 50,.y = 108,.destructible = 1,.displayed = 0,.destroyed = 0,.colors = {WALL_COLOR1,WALL_COLOR2}},
																{.active = 1,.x = 60,.y = 108,.destructible = 1,.displayed = 0,.destroyed = 0,.colors = {WALL_COLOR1,WALL_COLOR2}},
																{.active = 1,.x = 70,.y = 108,.destructible = 1,.displayed = 0,.destroyed = 0,.colors = {WALL_COLOR1,WALL_COLOR2}},
																{.active = 1,.x = 50,.y = 60,.destructible = 1,.displayed = 0,.destroyed = 0,.colors = {WALL_COLOR1,WALL_COLOR2}},
																{.active = 1,.x = 60,.y = 60,.destructible = 1,.displayed = 0,.destroyed = 0,.colors = {WALL_COLOR1,WALL_COLOR2}},
																{.active = 1,.x = 70,.y = 60,.destructible = 1,.displayed = 0,.destroyed = 0,.colors = {WALL_COLOR1,WALL_COLOR2}},
																{.active = 1,.x = 40,.y = 69,.destructible = 1,.displayed = 0,.destroyed = 0,.colors = {WALL_COLOR1,WALL_COLOR2}},
																{.active = 1,.x = 40,.y = 79,.destructible = 1,.displayed = 0,.destroyed = 0,.colors = {WALL_COLOR1,WALL_COLOR2}},
																{.active = 1,.x = 40,.y = 89,.destructible = 1,.displayed = 0,.destroyed = 0,.colors = {WALL_COLOR1,WALL_COLOR2}},
																{.active = 1,.x = 40,.y = 99,.destructible = 1,.displayed = 0,.destroyed = 0,.colors = {WALL_COLOR1,WALL_COLOR2}},
																{.active = 1,.x = 80,.y = 69,.destructible = 1,.displayed = 0,.destroyed = 0,.colors = {WALL_COLOR1,WALL_COLOR2}},
																{.active = 1,.x = 80,.y = 79,.destructible = 1,.displayed = 0,.destroyed = 0,.colors = {WALL_COLOR1,WALL_COLOR2}},
																{.active = 1,.x = 80,.y = 89,.destructible = 1,.displayed = 0,.destroyed = 0,.colors = {WALL_COLOR1,WALL_COLOR2}},
																{.active = 1,.x = 80,.y = 99,.destructible = 1,.displayed = 0,.destroyed = 0,.colors = {WALL_COLOR1,WALL_COLOR2}},

																{.active = 1,.x = 60,.y = 10,.destructible = 0,.displayed = 0,.destroyed = 0,.colors = {WALL_COLOR3,WALL_COLOR3}},
																{.active = 1,.x = 60,.y = 158,.destructible = 0,.displayed = 0,.destroyed = 0,.colors = {WALL_COLOR3,WALL_COLOR3}},
																{.active = 1,.x = 117,.y = 84,.destructible = 0,.displayed = 0,.destroyed = 0,.colors = {WALL_COLOR3,WALL_COLOR3}},
																{.active = 1,.x = 1,.y = 84,.destructible = 0,.displayed = 0,.destroyed = 0,.colors = {WALL_COLOR3,WALL_COLOR3}}}
										};
}

//initialize powerups
void powerUpInit(void){
	if(mapIndex == 0){
		game.powerUps[0].x = PU0_X[randomInd%3];
		game.powerUps[0].y = PU0_Y[randomInd%3];
		game.powerUps[1].x = PU0_X[(1+randomInd)%3];
		game.powerUps[1].y = PU0_Y[(1+randomInd)%3];
		game.powerUps[2].x = PU0_X[(2+randomInd)%3];
		game.powerUps[2].y = PU0_Y[(2+randomInd)%3];
	} else if(mapIndex == 1){
		game.powerUps[0].x = PU1_X[randomInd%3];
		game.powerUps[0].y = PU1_Y[randomInd%3];
		game.powerUps[1].x = PU1_X[(1+randomInd)%3];
		game.powerUps[1].y = PU1_Y[(1+randomInd)%3];
		game.powerUps[2].x = PU1_X[(2+randomInd)%3];
		game.powerUps[2].y = PU1_Y[(2+randomInd)%3];
	} else {
		game.powerUps[0].x = PU2_X[randomInd%3];
		game.powerUps[0].y = PU2_Y[randomInd%3];
		game.powerUps[1].x = PU2_X[(1+randomInd)%3];
		game.powerUps[1].y = PU2_Y[(1+randomInd)%3];
		game.powerUps[2].x = PU2_X[(2+randomInd)%3];
		game.powerUps[2].y = PU2_Y[(2+randomInd)%3];
	}
	
	
	game.powerUps[0].powerUp = Reverse;
	game.powerUps[0].sprite = Reverse_Sprite;
	game.powerUps[0].active = 1;
	game.powerUps[0].displayed = 0;
	game.powerUps[0].powerUpCooldown = 0;
	
	game.powerUps[1].powerUp = Laser;
	game.powerUps[1].sprite = Laser_Sprite;
	game.powerUps[1].active = 1;
	game.powerUps[1].displayed = 0;
	game.powerUps[1].powerUpCooldown = 0;
	
	game.powerUps[2].powerUp = Blades;
	game.powerUps[2].sprite = Blades_Sprite;
	game.powerUps[2].active = 1;
	game.powerUps[2].displayed = 0;
	game.powerUps[2].powerUpCooldown = 0;
}

void resetP1( struct Player *p1 ) {
	p1->x = P1_X[mapIndex];
	p1->y = P1_Y[mapIndex];
	p1->dx = 0.0;
	p1->dy = 0.0;
	p1->dir = 0;
	p1->angle = 0;
	for(int i=0; i<3; i++) {
		p1->ammo[i].x = 0;
		p1->ammo[i].y = 0;
		p1->ammo[i].dx = 0;
		p1->ammo[i].dy = 0;
		p1->ammo[i].active = 0;
	}
	p1->shotCooldown = 0;
	p1->alive = 1;
	p1->powerUp = None;
	return;
}

void resetP2(struct Player *p2 ) {
	p2->x = P2_X[mapIndex];
	p2->y = P2_Y[mapIndex];
	p2->dx = 0.0;
	p2->dy = 0.0;
	p2->dir = 8;
	p2->angle = 180;
	for(int i=0; i<3; i++) {
		p2->ammo[i].x = 0;
		p2->ammo[i].y = 0;
		p2->ammo[i].dx = 0;
		p2->ammo[i].dy = 0;
		p2->ammo[i].active = 0;
	}
	p2->shotCooldown = 0;
	p2->alive = 1;
	p2->powerUp = None;
	return;
}

void resetGame(void){
	Seed();
	randomInd++;
	
	resetP1(&p1);
	resetP2(&p2);
	
	if(mapIndex == 0){
		cornerMapInit();
	
	} else if(mapIndex == 1){
		hallMapInit();
	} else {
		cacheMapInit();
	}
	
	powerUpInit();
	draw(0,159,game.map,128,160);
	drawPlayer(&p1);										
	drawPlayer(&p2);										
	drawWalls();
	drawPowerUps();
}

void update(void){
	moveParticles();
	if(laserBeam.displayed){
		eraseLaser();
	}
	if(game.paused){
		pauseGame();
	}
	if(PEStatus){
		if(portE&0x1){
		rotatePlayer(&p1);
		}
		if((portE&0x2)>>1){
			shootBullet(&p1);	
		}
		if((portE&0x4)>>2){
		rotatePlayer(&p2);
		}
		if((portE&0x8)>>3){
			shootBullet(&p2);	
		}
		PEStatus = 0;
	}	
	acceleratePlayer(&p1);
	acceleratePlayer(&p2);
	moveBullets(&p1);
	moveBullets(&p2);
	if(!p1.alive || !p2.alive){
		//display score?
		ST7735_DrawBitmap8(0, 159, screenBuffer, 128,160);
		uint32_t wait = 1000000;
		while(wait!=0){
			wait--;
		}
		resetGame();
		return;
	}
	movePlayer(&p1);
	movePlayer(&p2);
	spawnPowerUps();
	drawWalls();

}

void pauseGame(void){
	while(game.paused){
	}
}

void win(void){
	//determine winner
	if(p1.kills == game.kills){
		Sound_P1Win();
		draw(20, 155, Win_P1, 21, 150);
		draw(87, 155, Win_P1_180, 21, 150);
		ST7735_DrawBitmap8(0, 159, screenBuffer, 128,160);
		p1.x = 60;
		p1.y = 84;
		//p1 animation
		while(1){	
			rotatePlayer(&p1);
			drawPlayer(&p1);		
			ST7735_DrawBitmap8(0, 159, screenBuffer, 128,160);
		}
		
	} else {
		Sound_P2Win();
		draw(20, 155, Win_P2, 20, 150);
		draw(87, 155, Win_P2_180, 20, 150);
		ST7735_DrawBitmap8(0, 159, screenBuffer, 128,160);
		p2.x = 60;
		p2.y = 84;
		//p2 animation
		while(1){	
			rotatePlayer(&p2);
			drawPlayer(&p2);
			ST7735_DrawBitmap8(0, 159, screenBuffer, 128,160);
		}
	}	
}
