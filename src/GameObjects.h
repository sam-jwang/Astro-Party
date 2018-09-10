#ifndef GAMEOBJECTS_H
#define GAMEOBJECTS_H
#include <stdint.h>

#define PARTICLES_N 100
typedef struct Particle {
	double x, y, dx, dy;
	double angle;
	const uint8_t *sprite;
	uint8_t active;
	uint8_t displayed;
	uint8_t time;
} Particle;

enum PowerUpEnum {None,Reverse,Laser,Blades};

#define POWERUP_WIDTH 9
#define POWERUP_HEIGHT 9

typedef __packed struct PowerUp {
	double x, y;
	enum PowerUpEnum powerUp;
	const uint8_t *sprite;
	uint8_t active;
	uint8_t displayed;
	uint16_t powerUpCooldown;
} PowerUp;

#define BULLET_WIDTH 2
#define BULLET_HEIGHT 2
#define BULLET_EXP_W 9
#define BULLET_EXP_H 9
typedef struct Bullet {
	double x, y, dx, dy, expX, expY;
	const uint8_t *bulletSprite;
	const uint8_t *bulletExp;
	uint8_t active;
	uint8_t destroyed;
} Bullet;

#define PLAYER_WIDTH 9
#define PLAYER_HEIGHT 9
#define PLAYER1_COLOR 0x07
#define PLAYER2_COLOR 0xE4
#define PLAYER_AMMO 3
typedef struct Player {
	double x, y, dx, dy;
	uint8_t dir;
	uint16_t angle;
	uint8_t color;
	const uint8_t *sprites[16];
	Bullet ammo[PLAYER_AMMO];
	uint8_t shotCooldown;
	uint8_t alive;
	uint8_t kills;
	enum PowerUpEnum powerUp;
} Player;

typedef struct BladesPU {
	Player *player;
	uint8_t displayed;
	uint8_t bladeTime;
} BladesPU;

#define LASER_WIDTH 3
#define LASER_HEIGHT 3
typedef struct LaserBeamPU {
	double startX, startY,stopX,stopY,dx,dy;
	const uint8_t *sprite;
	uint8_t displayed;
} LaserBeamPU;

#define WALL_SIZE 10
#define WALL_COLOR1 0xFF
#define WALL_COLOR2 0xC7
#define WALL_COLOR3 0x5c

typedef __packed struct Wall {
	uint8_t active;
	uint16_t x,y;
	uint8_t destructible;
	uint8_t displayed;
	uint8_t destroyed;
	uint8_t colors[2];
} Wall;

#define WALLS_N 48
#define POWERUPS_N 3

typedef __packed struct GameState {
	uint8_t paused;
	uint8_t clockwise;
	uint8_t kills;
	const uint8_t *map;
	Wall walls[WALLS_N];
	PowerUp powerUps[POWERUPS_N];
} GameState;

#endif
