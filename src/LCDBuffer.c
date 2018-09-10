// File: LCDBuffer.c
// Name: Brian Cheung and Sam Wang
// Date: 5/4/18
// Desc: This software writes to the screen buffer array
// Usage: Draw game objects and animations

#include "LCDBuffer.h"
#include "AstroParty.h"
#include "ST7735.h"
#include "math.h"
#include "GamePhysics.h"

#define HEIGHT 160
#define WIDTH 128

//fill screen buffer array with black
void clear(void){
	for(uint16_t i=0; i<20; i++){
		screenBuffer[i] = ST7735_BLACK;
	}
}

//draw image
void draw(int16_t x, int16_t y, const uint8_t *image, uint8_t w, uint8_t h){
	
	int16_t ogWidth = w, ogHeight = h;
	uint16_t startR = 0, skipC = 0;
	if(x>=WIDTH || (y-h+1)>=HEIGHT || (x+w)<=0 || y<0) return; //image is totally off the screen, do nothing
	if(w>WIDTH || h>HEIGHT) return; //image is too wide for the screen, do nothing
	
	if((x+w-1)>=WIDTH){             //image exceeds right of screen
	  w = WIDTH-x;
	}
	if((y-h+1)<0){                  //image exceeds top of screen
	  h = y+1;
	}
	if(x<0){                        //image exceeds left of screen
		w = w+x;
		skipC = -x;
		x = 0;
	}
	if(y>=HEIGHT){                  //image exceeds bottom of screen
		startR = ogHeight - (h-(y-HEIGHT+1));
	    y = HEIGHT-1;
	}

	uint16_t sbPixel=(WIDTH*(HEIGHT-(int16_t)y-1))+x;				//calculate corresponding screenBuffer coordinate of bottom left corner of image
	for(uint8_t r=startR; r<h; r++){											
		for(uint8_t c=0; c<w; c++){
			if(image[r*ogWidth+c+skipC]!=1){												//if not special pixel
				screenBuffer[sbPixel+c] = image[r*ogWidth+c+skipC];		//draw corresponding sprite pixel onto screenBuffer
			} else {																		//else draw background
				if(stage == Game){
					screenBuffer[sbPixel+c] = game.map[sbPixel+c];
				} else {
					screenBuffer[sbPixel+c] = 0x00;
				}
			}
		}
		sbPixel += WIDTH;															//next row
	}
}

//draw rectangle
void drawRect(int16_t x, int16_t y, uint8_t w, uint8_t h, uint8_t color) {
	int16_t ogWidth = w, ogHeight = h;
  
	if(x>=WIDTH || (y-h+1)>=HEIGHT || (x+w)<=0 || y<0) return; //image is totally off the screen, do nothing
	if(w>WIDTH || h>HEIGHT) return; //image is too wide for the screen, do nothing
	
	if((x+w-1)>=WIDTH){             //image exceeds right of screen
	  w = WIDTH-x;
	}
	if((y-h+1)<0){                  //image exceeds top of screen
	  h = y+1;
	}
	if(x<0){                        //image exceeds left of screen
	  w = w+x;
	  x = 0;
	}
	if(y>=HEIGHT){                  //image exceeds bottom of screen
	    h = h-(y-HEIGHT+1);
	    y = HEIGHT-1;
  	}
	
	uint16_t sbPixel=(WIDTH*(HEIGHT-(int16_t)y-1))+x;		//calculate corresponding screenBuffer coordinate of bottom left corner of rect
	for(uint8_t r=0; r<h; r++){											
		for(uint8_t c=0; c<w; c++){
			if(r==0 || r==ogHeight-1 || c==0 || c==ogWidth-1){	//if edge 
				screenBuffer[sbPixel+c] = color;		//draw corresponding sprite pixel onto screenBuffer
			} else {																		//else draw background
				screenBuffer[sbPixel+c] = game.map[sbPixel+c];
			}
		}
		sbPixel += WIDTH;															//next row
	}
}

//draw filled rectangle
void fillRect(int16_t x, int16_t y, uint8_t w, uint8_t h, uint8_t color) {  
	if(x>=WIDTH || (y-h+1)>=HEIGHT || (x+w)<=0 || y<0) return; //image is totally off the screen, do nothing
	if(w>WIDTH || h>HEIGHT) return; //image is too wide for the screen, do nothing
	
	if((x+w-1)>=WIDTH){             //image exceeds right of screen
	  w = WIDTH-x;
	}
	if((y-h+1)<0){                  //image exceeds top of screen
	  h = y+1;
	}
	if(x<0){                        //image exceeds left of screen
	  w = w+x;
	  x = 0;
	}
	if(y>=HEIGHT){                  //image exceeds bottom of screen
	    h = h-(y-HEIGHT+1);
	    y = HEIGHT-1;
	}
	
	uint16_t sbPixel=(WIDTH*(HEIGHT-(int16_t)y-1))+x;	//calculate corresponding screenBuffer coordinate of bottom left corner of rect
	for(uint8_t r=0; r<h; r++){											
		for(uint8_t c=0; c<w; c++){
				screenBuffer[sbPixel+c] = color;		//draw corresponding color onto screenBuffer
		}
		sbPixel += WIDTH;															//next row
	}
}

//fill rectangle with background
void eraseRect(int16_t x, int16_t y, uint8_t w, uint8_t h) {
	if(x>=WIDTH || (y-h+1)>=HEIGHT || (x+w)<=0 || y<0) return; //image is totally off the screen, do nothing
	if(w>WIDTH || h>HEIGHT) return; //image is too wide for the screen, do nothing
	
	if((x+w-1)>=WIDTH){             //image exceeds right of screen
	  w = WIDTH-x;
	}
	if((y-h+1)<0){                  //image exceeds top of screen
	  h = y+1;
	}
	if(x<0){                        //image exceeds left of screen
	  w = w+x;
	  x = 0;
	}
	if(y>=HEIGHT){                  //image exceeds bottom of screen
	    h = h-(y-HEIGHT+1);
	    y = HEIGHT-1;
	}
	
	uint16_t sbPixel=(WIDTH*(HEIGHT-(int16_t)y-1))+x;				//calculate corresponding screenBuffer coordinate of bottom left corner of region
	for(uint8_t r=0; r<h; r++){											
		for(uint8_t c=0; c<w; c++){
				screenBuffer[sbPixel+c] = game.map[sbPixel+c];
			}
			sbPixel += WIDTH;															//next row
		}
}

void drawPlayer(Player *player){	
	uint16_t sbPixel=(WIDTH*(HEIGHT-(int16_t)player->y-1))+player->x;				//calculate corresponding screenBuffer coordinate of bottom left corner of player
	for(uint8_t r=0; r<PLAYER_HEIGHT; r++){											
		for(uint8_t c=0; c<PLAYER_WIDTH; c++){
			if(player->sprites[player->dir][r*PLAYER_WIDTH+c]!=1){//if not transparent pixel
				if(player->sprites[player->dir][r*PLAYER_WIDTH+c]==0x07){
					screenBuffer[sbPixel+c] = player->color;		//draw player color onto screenBuffer
				} else {
					screenBuffer[sbPixel+c] = player->sprites[player->dir][r*PLAYER_WIDTH+c];		//draw corresponding sprite pixel onto screenBuffer
				}
			} else {																		//else draw background
				if(player->powerUp == Blades && blades.displayed == 1 && (r==0 || r==PLAYER_HEIGHT-1 || c==0 || c==PLAYER_WIDTH-1)){
					screenBuffer[sbPixel+c] = 0x1c;
				} else {
					screenBuffer[sbPixel+c] = game.map[sbPixel+c];
				}
			}
		}
		sbPixel += WIDTH;															//next row
	}
}

void drawBulletExplosion(Bullet *bullet){
	bullet->active = 0;
	bullet->destroyed = 1;
	draw(bullet->expX,bullet->expY, bullet->bulletExp, BULLET_EXP_W, BULLET_EXP_H);
}

void drawBullet(Bullet *bullet){
	uint16_t sbPixel=(WIDTH*(HEIGHT-(int16_t)bullet->y-1))+bullet->x;				//calculate corresponding screenBuffer coordinate of bottom left corner of bullet
	for(uint8_t r=0; r<BULLET_HEIGHT; r++){											
		for(uint8_t c=0; c<BULLET_WIDTH; c++){
			screenBuffer[sbPixel+c] = bullet->bulletSprite[r*BULLET_WIDTH+c];
		}
		sbPixel += WIDTH;															//next row
	}
}

void drawLaser(double x, double y, uint8_t w, uint8_t h){
	uint16_t sbPixel=(WIDTH*(HEIGHT-(int16_t)y-1))+x;				//calculate corresponding screenBuffer coordinate of bottom left corner of bullet
	for(uint8_t r=0; r<h; r++){											
		for(uint8_t c=0; c<w; c++){
			screenBuffer[sbPixel+c] = laserBeam.sprite[r*LASER_WIDTH+c];
		}
		sbPixel += WIDTH;															//next row
	}
}

//fill laser with background
void eraseLaser(void){		
	laserBeam.displayed = 0;
	while((fabs(laserBeam.startX-laserBeam.stopX)>1 || 
		fabs(laserBeam.startY-laserBeam.stopY)>1) && (laserBeam.startX < WIDTH && laserBeam.startX > 0 && laserBeam.startY > 0 && laserBeam.startY < HEIGHT)){
		eraseRect(laserBeam.startX,laserBeam.startY,LASER_WIDTH,LASER_HEIGHT);
		if(fabs(laserBeam.startX-laserBeam.stopX) > 1){	
				laserBeam.startX += laserBeam.dx;
				} 
		if(fabs(laserBeam.startY-laserBeam.stopY) > 1) {
			laserBeam.startY += laserBeam.dy;
		}
	}
}

//draw active walls
void drawWalls(void){
	for(uint8_t i=0; i<WALLS_N; i++){
		if(game.walls[i].active){
				if(!game.walls[i].destroyed){		//if wall has not been destroyed
					game.walls[i].displayed = 1;
					uint16_t sbPixel=(WIDTH*(HEIGHT-(int16_t)game.walls[i].y-1))+game.walls[i].x;				//calculate corresponding screenBuffer coordinate of bottom left corner of rect
					for(uint8_t r=0; r<WALL_SIZE; r++){											
						for(uint8_t c=0; c<WALL_SIZE; c++){
							if(r==0 || r==WALL_SIZE-1 || c==0 || c==WALL_SIZE-1){					//if not special pixel
								screenBuffer[sbPixel+c] = game.walls[i].colors[(r+c)%2];		//draw corresponding sprite pixel onto screenBuffer
							} else {																		//else draw background
								screenBuffer[sbPixel+c] = game.map[sbPixel+c];
							}
						}
						sbPixel += WIDTH;															//next row
					}
				} 
				else if(game.walls[i].destroyed && game.walls[i].displayed){		//if wall has been destroyed, but is still displayed, erase it
					game.walls[i].displayed = 0;
					eraseRect(game.walls[i].x,game.walls[i].y,WALL_SIZE,WALL_SIZE);
					particleGenerate(game.walls[i].x +WALL_SIZE/2,game.walls[i].y-WALL_SIZE/2);
				}
		}
	}
}

//draw active powerups
void drawPowerUps(void){
	for(uint8_t i=0; i<POWERUPS_N; i++){
		if(game.powerUps[i].active){
				game.powerUps[i].displayed = 1;
				uint16_t sbPixel=(WIDTH*(HEIGHT-(int16_t)game.powerUps[i].y-1))+game.powerUps[i].x;				//calculate corresponding screenBuffer coordinate of bottom left corner of rect
				for(uint8_t r=0; r<POWERUP_WIDTH; r++){											
					for(uint8_t c=0; c<POWERUP_HEIGHT; c++){
							screenBuffer[sbPixel+c] = game.powerUps[i].sprite[r*POWERUP_WIDTH+c];
					}
					sbPixel += WIDTH;															//next row
				}
		}
	}
}

void drawParticle(uint8_t x, uint8_t y){
	uint16_t sbPixel=(WIDTH*(HEIGHT-y-1))+x;
	screenBuffer[sbPixel] = 0xFF;	
}

void eraseParticle(uint8_t x, uint8_t y){
	uint16_t sbPixel=(WIDTH*(HEIGHT-y-1))+x;
	screenBuffer[sbPixel] = game.map[sbPixel];	
}
