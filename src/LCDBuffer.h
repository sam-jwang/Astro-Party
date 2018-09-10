#ifndef LCDBUFFER_H
#define LCDBUFFER_H
#include <stdint.h>
#include "GameObjects.h"



void clear(void);

void draw(int16_t x, int16_t y, const uint8_t *image, uint8_t w, uint8_t h);
void drawRect(int16_t x, int16_t y, uint8_t w, uint8_t h, uint8_t color);
void fillRect(int16_t x, int16_t y, uint8_t w, uint8_t h, uint8_t color);

void eraseRect(int16_t x, int16_t y, uint8_t w, uint8_t h);
void drawPlayer(Player *player);
void drawBulletExplosion(Bullet *bullet);
void drawBullet(Bullet *bullet);
void drawLaser(double x, double y, uint8_t w, uint8_t h);
void eraseLaser(void);
void drawWalls(void);
void drawPowerUps(void);
void drawParticle(uint8_t x, uint8_t y);
void eraseParticle(uint8_t x, uint8_t y);

#endif
