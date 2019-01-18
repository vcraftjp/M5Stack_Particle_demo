//
// particle.h
//
#pragma once

#include <Arduino.h>

typedef int16_t coord_t; // LCD width(320) x SCALE(100) < 32768

//
// Particle class
//
class Particle {
public:
	static const int SCALE = 100; // for integer operation

	Particle() { Particle(wallWidth / 2, wallHeight / 2); }
	Particle(coord_t x, coord_t y);

	void setPosition(coord_t _x, coord_t _y) { x = _x * SCALE; y = _y * SCALE; }
	void setVelocity(coord_t _vx, coord_t _vy) { vx = _vx; vy = _vy; }
	void setBounce(coord_t b) { bounce = b; }
	void setAccel(coord_t accelX, coord_t accelY) { vx += accelX; vy += accelY; }
	coord_t getX() { return x / SCALE; }
	coord_t getY() { return y / SCALE; }

	bool update();

	static void setWallSize(coord_t w, coord_t h) { wallWidth = w; wallHeight = h; }
	static void setParticleRadius(coord_t r) { radius = r; }

protected:
	coord_t x;
	coord_t y;
	coord_t vx; // velocity
	coord_t vy;
	coord_t bounce; // 0 ... SCALE(1.0)
	static coord_t wallWidth;
	static coord_t wallHeight;
	static coord_t radius;

public:
	static int heatAmount;
	static int minHeatingVelocity;
	static int maxVelocity;

	bool trimInWall(int32_t& n, coord_t wallSize);
};

int getCosX(int r, int deq);
int getSinY(int r, int deq);
