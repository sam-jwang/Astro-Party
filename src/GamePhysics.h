#ifndef GAMEPHYSICS_H
#define GAMEPHYSICS_H
#include <stdint.h>
#include "GameObjects.h"

void rotatePlayer(Player *player);
void acceleratePlayer(Player *player);
void movePlayer(Player *player);

void shootLaser(Player *player);
void shootBullet(Player *player);
void moveBullets(Player *player);

uint8_t playerCD(double x, double y, uint8_t w, uint8_t h, Player *player);
uint8_t wallBulletCD(int16_t x, int16_t y, uint8_t w, uint8_t h);
uint8_t bulletCD(Bullet *bullet, Player *player);
uint8_t powerUpCD(Player *player);
void laserCD(double x, double y,uint8_t w, uint8_t h, Player *player);

void spawnPowerUps(void);
void particleGenerate(double x, double y);
void moveParticles(void);

#endif
