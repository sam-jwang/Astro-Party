// File: GamePhysics.c
// Name: Brian Cheung and Sam Wang
// Date: 5/4/18
// Desc: This software is the physics engine and contains the game actions
// Usage: Update and move game objects; collision detection

#include "GamePhysics.h"
#include "AstroParty.h"
#include "math.h"
#include "LCDBuffer.h"
#include "Sound.h"
#include "ST7735.h"
#include "RandomGenerate.h"

#define HEIGHT 160
#define WIDTH 128
#define ACCEL 2.2
#define MAX_SPEED 2.5
#define ANG_VEL 12
#define BULLET_SPEED 5
#define PI 3.14159265359
#define PI8 PI/8 //angle = pi/8
#define DT 0.033333 //delta time at 30Hz
#define SHOT_COOLDOWN 10
#define POWERUP_COOLDOWN 300
#define BLADE_TIME 150



BladesPU blades = {0,0,BLADE_TIME};

void rotatePlayer(Player *player){
	if(game.clockwise){
		if(player->angle == 0) {
			player->angle = 360 - ANG_VEL;		//0 wraps around to 360 - ANG_VEL
		} else{
			player->angle = (player->angle-ANG_VEL)%360;	
		}
	} else {
		player->angle = (player->angle+ANG_VEL)%360;
	}
	player->dir = (player->angle)/22.5;
}

void acceleratePlayer(Player *player){
	double theta = PI8*player->dir;		//player angle
	double dx = cos(theta)*MAX_SPEED;		//desired dx
	double dy = -sin(theta)*MAX_SPEED;	//desired dy
	
	//accelerate player until desired dx is reached
	if(fabs(player->dx - dx) < DT*ACCEL){
		player->dx = dx;
	}	else if(player->dx < dx){
		player->dx += DT*ACCEL;
	} else if(player->dx > dx){
		player->dx -= DT*ACCEL;
	}
	
	//accelerate player until desired dy is reached	
	if(fabs(player->dy - dy) < DT*ACCEL){
		player->dy = dy;
	}	else if(player->dy < dy){
		player->dy += DT*ACCEL;
	} else if(player->dy > dy){
		player->dy -= DT*ACCEL;
	}	
}

void movePlayer(Player *player){
	powerUpCD(player);
	//erase previous player image
	eraseRect(player->x,player->y,PLAYER_WIDTH,PLAYER_HEIGHT);
	//update player x and y position w/ dx and dy
	double newX = player->x + player->dx;
	double newY = player->y + player->dy;
	
	//player may destroy destructible walls and player on contact
	if(player->powerUp == Blades && blades.displayed==1){
		for(uint8_t i=0; i<WALLS_N; i++){
				//detect collision with destructible wall
				if(game.walls[i].destructible){
					if (newX < game.walls[i].x + WALL_SIZE &&
						newX + PLAYER_WIDTH > game.walls[i].x &&
						newY > game.walls[i].y - WALL_SIZE &&
						newY - PLAYER_HEIGHT < game.walls[i].y){
							game.walls[i].destroyed = 1;
						}
				}
		}
		//detect collision with other player
		if(player == &p1){
			if (newX < p2.x + PLAYER_WIDTH &&
				newX + PLAYER_WIDTH > p2.x &&
				newY > p2.y - PLAYER_HEIGHT &&
				newY - PLAYER_WIDTH < p2.y) {
					p2.alive = 0;
					p1.kills += 1;
					Sound_Explosion2();
				}		
		} else {
			if (newX < p1.x + PLAYER_WIDTH &&
				newX + PLAYER_WIDTH > p1.x &&
				newY > p1.y - PLAYER_HEIGHT &&
				newY - PLAYER_HEIGHT < p1.y) {
					p1.alive = 0;
					p2.kills += 1;
					Sound_Explosion1();
				}
		}		
		
	}
	
	//player collision with wall
	uint8_t colX=0, colY=0;
	for(uint8_t i=0; i<WALLS_N; i++){
		if(!(game.walls[i].destroyed)){
			//detect collision with new position on specific wall
			if (newX < game.walls[i].x + WALL_SIZE &&
				newX + PLAYER_WIDTH > game.walls[i].x &&
				newY > game.walls[i].y - WALL_SIZE &&
				newY - PLAYER_HEIGHT < game.walls[i].y) {
					//collision detected
					//detect collision with new x position
					if (newX < game.walls[i].x + WALL_SIZE &&
						newX + PLAYER_WIDTH > game.walls[i].x &&
						player->y > game.walls[i].y - WALL_SIZE &&
						player->y - PLAYER_HEIGHT < game.walls[i].y) {
							//collision in x direction
							colX = 1;
						}
					//detect collision with new y position
					if (player->x < game.walls[i].x + WALL_SIZE &&
						player->x + PLAYER_WIDTH > game.walls[i].x &&
						newY > game.walls[i].y - WALL_SIZE &&
						newY - PLAYER_HEIGHT < game.walls[i].y) {
							//collision in y direction
							colY = 1;
						}
				}
		}
	}
	
	//player collision with boundary
	uint8_t right = (newX+PLAYER_WIDTH-1)>=WIDTH-1, //exceeds right of screen
			left = newX<0+1, 												//exceeds left of screen
			top = (newY-PLAYER_HEIGHT+1)<0+1,				//exceeds top of screen
			bottom = newY>=HEIGHT-1;								//exceeds bottom of screen
	uint8_t sum = right+left+top+bottom+(colX+colY);
	
	if(sum==2){	//collision with 2 edges
		//collision detected in both directions, player doesn't move
		drawPlayer(player);
		return;
	}
	if(right){             		//exceeds right of screen
		colX=1;
	}
	if(left){                 //exceeds left of screen
		colX=1;
	}
	if(top){                  //exceeds top of screen
		colY=1;
	}
	if(bottom){               //exceeds bottom of screen
		colY=1;
	}
	
	//move player if collcion in direction is not detected
	if(!colX){
		player->x = newX;
	} else{
		player->dx = 0;
	}
	if(!colY){
		player->y = newY;
	} else{
		player->dy = 0;
	}

	//draw new player position
	drawPlayer(player);
	
}

void shootBullet(Player *player){
	//detect if player has powerUp
	switch (player->powerUp){
		case Reverse : {
			break;
		}
		case Laser : {
			shootLaser(player);
			break;
		}
		case Blades : {
			blades.player = player;
			blades.displayed = 1;
			blades.bladeTime = BLADE_TIME;
		}
		case None : {
			//shoot bullet if there exists an inactive bullet and shotCooldown == 0
			if(player->shotCooldown != 0) {
				return;
			}
			
			for(uint8_t i=0; i<PLAYER_AMMO+1; i++){
				if(i>PLAYER_AMMO){
					//no inactive bullets
					Sound_NoShoot();
				}
				else if(player->ammo[i].active){
					Sound_Shoot();
					double theta = PI8*player->dir;		//player angle
					player->ammo[i].active = 1;				//activate bullet
					
					//start bullet at player's origin
					player->ammo[i].x = player->x + PLAYER_WIDTH/2;
					player->ammo[i].y = player->y - PLAYER_HEIGHT/2;
					
					//calculate bullet velocity
					double dx = cos(theta)*BULLET_SPEED,
								dy = -sin(theta)*BULLET_SPEED;
					player->ammo[i].dx = dx;			
					player->ammo[i].dy = dy;	
					
					//direction offset from player sprite
					double newX = player->ammo[i].x + dx/4, 
								newY = player->ammo[i].y + dy/4;
					while(newX <= player->x + PLAYER_WIDTH &&
						newX + BULLET_WIDTH >= player->x &&
						newY >= player->y - PLAYER_HEIGHT &&
						newY - BULLET_HEIGHT <= player->y &&
						newX < WIDTH && newX > 0 && newY > 0 && newY < HEIGHT){
							newX += dx/4;
							newY += dy/4;
						}	
					player->ammo[i].x = newX;
					player->ammo[i].y = newY;
					if(newX < WIDTH-1 && newX > 1 && newY > 1 && newY < HEIGHT-1){
						(bulletCD(&(player->ammo[i]),player));
						drawBullet(&(player->ammo[i]));
					} else {
						return;
					}
				}
			}
		}
	}
	player->shotCooldown = SHOT_COOLDOWN;		//reset shotCooldown

}

//move bullets with dx and dy
//linear bullet collision detection
void moveBullets(Player *player){
	//update bullet x and y position w/ dx and dy
	for(uint8_t i=0; i<3; i++){
		if(player->ammo[i].active){
			//erase previous active bullets
			eraseRect(player->ammo[i].x,player->ammo[i].y,BULLET_WIDTH,BULLET_HEIGHT);
			
			//calculate line between old bullet position and new bullet position
			double dx, dy, oldX, oldY, newX, newY, 
						 colDx, colDy, colX,colY, endX, endY;
			oldX = player->ammo[i].x;
			oldY = player->ammo[i].y;
			newX = player->ammo[i].x + player->ammo[i].dx;
			newY = player->ammo[i].y + player->ammo[i].dy;
			
			//start at leftmost point
			if(oldX>newX){
				colX = newX;
				colY = newY;
				endX = oldX;
				endY = oldY;
			} else {
				colX = oldX;
				colY = oldY;
				endX = newX;
				endY = newY;
			}
			//calculate slope from left point, dx>=0
			dx = endX-colX;
			dy = endY-colY;
			double m;
			//calculate slope 
			if(fabs(dx)>=0.1 && fabs(dy)>=0.1){		//if dy!=0 and dx!=0 within 0.1 error
				m = dy/dx;
				if(dy>0){
					if(fabs(m)<1 && fabs(m-1)>=0.1){
						colDx = 1;		//Xn+1 = Xn + 1
						colDy = m;		//Yn+1 = Yn + m
					}else if(fabs(m)>1 && fabs(m-1)>=0.1){	//m!=1 within 0.1 error
						colDx = 1/m;		//Xn+1 = Xn + 1/m
						colDy = 1;			//Yn+1 = Yn + 1
					} else {		//m=1
						colDx = 1;		//Xn+1 = Xn + 1
						colDy = 1;		//Yn+1 = Yn + 1
					}
				} else {					//dy<0
					if(fabs(m)<1 && fabs(m-1)>=0.1){	//m!=1 within 0.1 error
						colDx = 1;		//Xn+1 = Xn + 1
						colDy = m;		//Yn+1 = Yn + m
					}else if(fabs(m)>1 && fabs(m-1)>=0.1){
						colDx = -1/m;		//Xn+1 = Xn + 1/m
						colDy = -1;			//Yn+1 = Yn - 1
					} else {		//m=-1
						colDx = 1;		//Xn+1 = Xn + 1
						colDy = -1;		//Yn+1 = Yn - 1
					}
				}
			} else if(fabs(dx)<=0.1){	//dx = 0
				endX = colX;
				if(dy>0){
					colDx = 0;		//Xn+1 = Xn
					colDy = 1;		//Yn+1 = Yn + 1
				} else{
					colDx = 0;		//Xn+1 = Xn
					colDy = -1;		//Yn+1 = Yn - 1
				}
			} else {	//dy = 0	
				endY = colY;				
				colDx = 1;		//Xn+1 = Xn + 1
				colDy = 0;		//Yn+1 = Yn
			}			
			//separate dx and dy because dx might be reached but not dy
			while(colX<endX || fabs(colY)<fabs(endY)){
				player->ammo[i].x = colX;
				player->ammo[i].y = colY;
				//bullet collision detection
				if(bulletCD(&(player->ammo[i]),player)) return;
				if(colX<endX){	
				colX += colDx;
				} 
				if(fabs(colY)<fabs(endY)) {
				colY += colDy;
				}
			}
			
			//update new bullet position
			player->ammo[i].x = newX;
			player->ammo[i].y = newY;
			//bullet collision detection
			if(bulletCD(&(player->ammo[i]),player)) return;
			drawBullet(&(player->ammo[i]));

		} 
		if(player->ammo[i].destroyed){
			player->ammo[i].destroyed = 0;
			//erase explosion
			eraseRect(player->ammo[i].expX,player->ammo[i].expY,BULLET_EXP_W,BULLET_EXP_H);

		}
	}
}

//player collision with projectile
uint8_t playerCD(double x, double y, uint8_t w, uint8_t h, Player *player){
	//player 1 collision		
	if (player == &p2 && x < p1.x + PLAYER_WIDTH &&
		x + w > p1.x &&
		y > p1.y - PLAYER_HEIGHT &&
		y - h < p1.y) {
			p1.alive = 0;
			p2.kills += 1;
			Sound_Explosion1();
			return 1;
		}
	//player 2 collision		
	if (player == &p1 && x < p2.x + PLAYER_WIDTH &&
		x + w > p2.x &&
		y > p2.y - PLAYER_HEIGHT &&
		y - h < p2.y) {
			p2.alive = 0;
			p1.kills += 1;
			Sound_Explosion2();
			return 1;
		}	
	return 0;
}

//bullet collision with wall
uint8_t wallBulletCD(int16_t x, int16_t y, uint8_t w, uint8_t h){
	for(uint8_t i=0; i<WALLS_N; i++){
		if(game.walls[i].destructible && !(game.walls[i].destroyed)){
			//detect collision with new position on specific destructible wall
			if (x <= game.walls[i].x + WALL_SIZE &&
					x + w >= game.walls[i].x &&
					y >= game.walls[i].y - WALL_SIZE &&
					y - h < game.walls[i].y) {
						game.walls[i].destroyed = 1;
						return 1;
			}
		} else if(!game.walls[i].destructible){
				//detect collision with new position on specific wall
				if (x <= game.walls[i].x + WALL_SIZE &&
						x + w >= game.walls[i].x &&
						y >= game.walls[i].y - WALL_SIZE &&
						y - h <= game.walls[i].y) {
							return 1;
				}
		}
	}
	return 0;	//no collision detected with wall
}

//bullet collision with walls, boundary, and other player
uint8_t bulletCD(Bullet *bullet, Player *player){
	//wall collision
	if(wallBulletCD(bullet->x,bullet->y,BULLET_WIDTH,BULLET_HEIGHT)){
		bullet->expX = bullet->x - BULLET_EXP_W/2;
		bullet->expY = bullet->y + BULLET_EXP_H/2;
		drawBulletExplosion(bullet);
		return 1;	//collision detected
	}
	
	//bullet boundary collision
	uint8_t right = (bullet->x+BULLET_WIDTH-1)>=WIDTH, //exceeds right of screen
			left = bullet->x<0, 												//exceeds left of screen
			top = (bullet->y-BULLET_HEIGHT+1)<0,				//exceeds top of screen
			bottom = bullet->y>=HEIGHT;								//exceeds bottom of screen
	
	uint8_t sum = right+left+top+bottom;
	
	if(sum>0){	//bullet out of bounds
		if(right){             		//exceeds right of screen
			bullet->expX = WIDTH - BULLET_EXP_W/2;
			bullet->expY = bullet->y + BULLET_EXP_H/2;
			drawBulletExplosion(bullet);
		}
		else if(left){                 //exceeds left of screen
			bullet->expX = 0 - BULLET_EXP_W/2;
			bullet->expY = bullet->y + BULLET_EXP_H/2;
			drawBulletExplosion(bullet);
		}
		else if(top){                  //exceeds top of screen
			bullet->expX = bullet->x - BULLET_EXP_W/2;
			bullet->expY = 0 + BULLET_EXP_H/2;
			drawBulletExplosion(bullet);		
		}
		else if(bottom){               //exceeds bottom of screen
			bullet->expX = bullet->x - BULLET_EXP_W/2;
			bullet->expY = HEIGHT + BULLET_EXP_H/2;
			drawBulletExplosion(bullet);			
		}
		return 1; //collision detected
	}
	
	//player collision
	if(playerCD(bullet->x,bullet->y,BULLET_WIDTH,BULLET_HEIGHT,player)){
		bullet->expX = bullet->x - BULLET_EXP_W/2;
		bullet->expY = bullet->y + BULLET_EXP_H/2;
		drawBulletExplosion(bullet);
		return 1;	//collision detected
	}
	
	return 0;	//no collision detected
	
}

void spawnPowerUps(void){
	for(uint8_t i=0; i<POWERUPS_N; i++){
		if(!game.powerUps[i].active){
			if(game.powerUps[i].powerUpCooldown == 0){		//if powerUpCooldown == 0
				game.powerUps[i].active = 1;
			}  
		}
	}
	drawPowerUps();
}

//powerUp collision with player
uint8_t powerUpCD(Player *player){
	for(uint8_t i=0;i<POWERUPS_N;i++){
		if(game.powerUps[i].active && game.powerUps[i].displayed){
			//collision with active and displayed powerup
			if (game.powerUps[i].x < player->x + PLAYER_WIDTH &&
				game.powerUps[i].x + POWERUP_WIDTH > player->x &&
				game.powerUps[i].y > player->y - PLAYER_HEIGHT &&
				game.powerUps[i].y - POWERUP_HEIGHT < player->y) {
					if(game.powerUps[i].powerUp == Reverse){
						game.clockwise ^= 1;
					} else {
						//set player powerup
						player->powerUp = game.powerUps[i].powerUp;
					}
					//erase powerUp and deactivate
					game.powerUps[i].active = 0;
					game.powerUps[i].displayed = 0;
					game.powerUps[i].powerUpCooldown = POWERUP_COOLDOWN;
					eraseRect(game.powerUps[i].x,game.powerUps[i].y,POWERUP_WIDTH,POWERUP_HEIGHT);
					return 1;
			}
		}
	}
	return 0;
}

//draw laser and collsion detection
void shootLaser(Player *player){
	player->powerUp = None;
	double dx, dy, startX, startY, stopX, stopY;
	double theta = PI8*player->dir;		//player angle
	//calculate laser velocity
	dx = cos(theta)*BULLET_SPEED;
	dy = -sin(theta)*BULLET_SPEED;
	//start laser at player's origin
	startX = player->x + PLAYER_WIDTH/2;
	startY = player->y - PLAYER_HEIGHT/2;
	
	//direction offset from player sprite
	startX = startX + dx/4, 
	startY = startY + dy/4;
	while(startX <= player->x + PLAYER_WIDTH &&
		startX + BULLET_WIDTH >= player->x &&
		startY >= player->y - PLAYER_HEIGHT &&
		startY - BULLET_HEIGHT <= player->y &&
		startX < WIDTH && startX > 0 && startY > 0 && startY < HEIGHT){
			startX += dx/4;
			startY += dy/4;
		}	
	stopX = startX;
	stopY = startY;
	//while in bounds, increment endX and endY
	do{
		stopX += dx;
		stopY += dy;
	} while(stopX < WIDTH && stopX > 0 && stopY > 0 && stopY < HEIGHT);
	stopX -= dx;
	stopY -= dy;
	double tempX,tempY;
	//start at leftmost point
	if(startX>stopX){
		tempX = startX;
		tempY = startY;
		startX = stopX;
		startY = stopY;
		stopX = tempX;
		stopY = tempY;
	}
	//calculate slope from left point, dx>=0
	dx = stopX-startX;
	dy = stopY-startY;
	double m;
	//calculate slope 
	if(fabs(dx)>=0.1 && fabs(dy)>=0.1){		//if dy!=0 and dx!=0 within 0.1 error
		m = dy/dx;
		if(dy>0){
			if(fabs(m)<1 && fabs(m-1)>=0.1){
				dx = 1;		//Xn+1 = Xn + 1
				dy = m;		//Yn+1 = Yn + m
			}else if(fabs(m)>1 && fabs(m-1)>=0.1){	//m!=1 within 0.1 error
				dx = 1/m;		//Xn+1 = Xn + 1/m
				dy = 1;			//Yn+1 = Yn + 1
			} else {		//m=1
				dx = 1;		//Xn+1 = Xn + 1
				dy = 1;		//Yn+1 = Yn + 1
			}
		} else {					//dy<0
			if(fabs(m)<1 && fabs(m-1)>=0.1){	//m!=1 within 0.1 error
				dx = 1;		//Xn+1 = Xn + 1
				dy = m;		//Yn+1 = Yn + m
			}else if(fabs(m)>1 && fabs(m-1)>=0.1){
				dx = -1/m;		//Xn+1 = Xn + 1/m
				dy = -1;			//Yn+1 = Yn - 1
			} else {		//m=-1
				dx = 1;		//Xn+1 = Xn + 1
				dy = -1;		//Yn+1 = Yn - 1
			}
		}
	} else if(fabs(dx)<=0.1){	//dx = 0
		stopX = startX;
		if(dy>0){
			dx = 0;		//Xn+1 = Xn
			dy = 1;		//Yn+1 = Yn + 1
		} else{
			dx = 0;		//Xn+1 = Xn
			dy = -1;		//Yn+1 = Yn - 1
		}
	} else {	//dy = 0	
		stopY = startY;				
		dx = 1;		//Xn+1 = Xn + 1
		dy = 0;		//Yn+1 = Yn
	}	
	laserBeam.startX = startX;
	laserBeam.startY = startY;
	laserBeam.stopX = stopX;
	laserBeam.stopY = stopY;
	laserBeam.dx = dx;
	laserBeam.dy = dy;

	//separate dx and dy because dx might be reached but not dy
	while((fabs(startX-stopX)>1 || fabs(startY-stopY)>1) && (startX < WIDTH && startX > 0 && startY > 0 && startY < HEIGHT)){
		drawLaser(startX,startY,LASER_WIDTH,LASER_HEIGHT);
		laserCD(startX,startY,LASER_WIDTH,LASER_HEIGHT, player);
		if(fabs(startX-stopX) > 1){	
			startX += dx;
		} 
		if(fabs(startY-stopY) > 1) {
			startY += dy;
		}
	}
	laserBeam.displayed = 1;
			
}

//laser collision with walls and other player
void laserCD(double x, double y,uint8_t w, uint8_t h, Player *player){
	//wall collision
	for(uint8_t i=0; i<WALLS_N; i++){
		if(game.walls[i].destructible && !(game.walls[i].destroyed)){
			//detect collision with new position on specific destructible wall
			if (x <= game.walls[i].x + WALL_SIZE &&
				x + w >= game.walls[i].x &&
				y >= game.walls[i].y - WALL_SIZE &&
				y - h < game.walls[i].y) {
					game.walls[i].destroyed = 1;
			}	
		}
	}
	playerCD(x,y,w,h,player);
}

//generate particles
#define PARTICLES_G_N	15
void particleGenerate(double x, double y){
	uint8_t j =0;
	for(uint8_t i=0;i<PARTICLES_G_N;i++){
		while(j<PARTICLES_N && particles[j].active == 1){
			j++;
		}
		particles[j].active = 1;
		particles[j].x = x;
		particles[j].y = y;
		particles[j].angle = randomGenerate(0,359)*PI/180;			//convert to theta please
		particles[j].time = randomGenerate(20,60);
		uint8_t speed = randomGenerate(1,4);
		particles[j].dx = cos(particles[j].angle)*speed;
		particles[j].dy = -sin(particles[j].angle)*speed;
	}
}

//move active particles
void moveParticles(void){
	for(uint8_t i=0;i<PARTICLES_N;i++){
		if(particles[i].active){
			//erase previous particle
			eraseParticle(particles[i].x,particles[i].y);
			if(particles[i].time == 0){
				particles[i].active = 0;
			} else {
				//move particle
				particles[i].x += particles[i].dx;
				particles[i].y += particles[i].dy;
				if(particles[i].x < WIDTH && particles[i].x > 0 && particles[i].y > 0 && particles[i].y < HEIGHT){
					//draw particle if in bounds
					drawParticle(particles[i].x,particles[i].y);
				} else {
					particles[i].active = 0;
				}
			}
			
		}
	}
}
