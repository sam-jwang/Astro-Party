# Astro Party

## Overview

University of Texas EE319K Game Design Lab

We recreated a popular iOS mobile game called [Astro Party](https://itunes.apple.com/us/app/astro-party/id904693943?mt=8) on the EK-TM4C123GXL Microcontroller.
Astro Party is a 1v1 game in which each player controls their spaceship and shoot each other. 

### Basic Rules
* Each ship is always accelerating in the direction it is currently facing
* Each ship can only rotate one direction with a button press (clockwise or counter-clockwise)
* Each ship has a maximum of 3 bullets (bullets reload after active ones explode)
* Power-Ups temporarily give players new abilities
* A round ends when one of the ships is destroyed
* A point is earned by destroying the opposing ship
* The first to reach the point cap is the winner

## Features

### Software:
* Two-player game
* Physics Engine
  * Collision Detection
    * Hitbox Collision Detection
    * Linear Per-pixel Collision Detection
  * Velocity 
  * Acceleration
* Gameplay
  * Destructible Walls
  * Power-Ups
    * Laser
    * Blades
    * Reverse Rotation
  * Particles
* Gameplay modes
  * Adjustable Game Length (1,3 or 5 kills)
  * 3 Unique Maps
* Graphics Rendering
  * Virtual Buffer
  * Layered Graphics
* Edge-Triggered Interrupts
  * Pause game

### Hardware:
* Three Onboard Buttons (reset, start, and select)
* Four Buttons (two for each player)
  * Shoot
  * Rotate ship
* ADC Slider
  * Navigate game menu (settings)
  * Volume control
* 8-bit DAC to Audio Output
  * Sound effects
* Sitronix ST7735R LCD

## Demo

[<img width="300" alt="demo" src="http://img.youtube.com/vi/q_EWK4ydNm0/0.jpg">](https://youtu.be/q_EWK4ydNm0)

## Screenshots

<img width="295" alt="screenshot9" src="https://github.com/b-cheung/Astro-Party/blob/master/screenshots/Astro%20Party%20Screenshot%209.png"><img width="295" alt="screenshot7" src="https://github.com/b-cheung/Astro-Party/blob/master/screenshots/Astro%20Party%20Screenshot%207.png"><img width="295" alt="screenshot4" src="https://github.com/b-cheung/Astro-Party/blob/master/screenshots/Astro%20Party%20Screenshot%204.png"><img width="295" alt="screenshot2" src="https://github.com/b-cheung/Astro-Party/blob/master/screenshots/Astro%20Party%20Screenshot%202.png"><img width="295" alt="screenshot6" src="https://github.com/b-cheung/Astro-Party/blob/master/screenshots/Astro%20Party%20Screenshot%206.png"><img width="295" alt="screenshot5" src="https://github.com/b-cheung/Astro-Party/blob/master/screenshots/Astro%20Party%20Screenshot%205.png"><img width="295" alt="screenshot1" src="https://github.com/b-cheung/Astro-Party/blob/master/screenshots/Astro%20Party%20Screenshot%201.png"><img width="295" alt="screenshot8" src="https://github.com/b-cheung/Astro-Party/blob/master/screenshots/Astro%20Party%20Screenshot%208.png"><img width="295" alt="screenshot3" src="https://github.com/b-cheung/Astro-Party/blob/master/screenshots/Astro%20Party%20Screenshot%203.png">

## EE319K SuperFinals Game Competition (3rd Place team):

[<img width="300" alt="competition" src="http://img.youtube.com/vi/ogT-apOq7TE/1.jpg">](https://youtu.be/ogT-apOq7TE)

## Built With

* [C](https://en.wikipedia.org/wiki/C_(programming_language)) - Programming language
* [Keil uVision IDE](http://www2.keil.com/mdk5/uvision/) - IDE for embedded software development

## Authors

* **Brian Cheung** - Game engine, physics engine, animations, hardware
* **Samuel Wang** - Sound module, sprites, hardware

## License

This project is licensed under the MIT License - see the [LICENSE.md](LICENSE.md) file for details

## Acknowledgments

* We do not own any of the Astro Party UI desgns, concepts, logos, etc. Rusty Moyher owns these UI designs, concepts, logos, etc.
* Code snippets and boilerplate code from:

  "Embedded Systems: Real Time Interfacing to Arm Cortex M Microcontrollers", ISBN: 978-1463590154, Jonathan Valvano, copyright (c) 2015 

  belong to Jonathan Valvano. I do not take credit for code snippets and boilerplate code from the textbook.
