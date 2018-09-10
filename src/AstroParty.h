#ifndef ASTROPARTY_H
#define ASTROPARTY_H
#include <stdint.h>
#include "GameObjects.h"

enum GameStageEnum {StartScreen,Menu,Game,Win};
extern enum GameStageEnum stage;

extern GameState game;
extern Player p1;
extern Player p2;

extern uint8_t screenBuffer[20480];
extern LaserBeamPU laserBeam;
extern BladesPU blades;
extern Particle particles[PARTICLES_N];

#endif
