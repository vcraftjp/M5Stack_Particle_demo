//
// particle.cpp
//

#include "particle.h"

coord_t Particle::wallWidth;
coord_t Particle::wallHeight;
coord_t Particle::radius = 0;
int Particle::heatAmount = 0;
int Particle::minHeatingVelocity = 0;
int Particle::maxVelocity = 0;


Particle::Particle(coord_t _x, coord_t _y) {
	setPosition(_x, _y);
	setVelocity(0, 0);
	setBounce(0);
}

bool Particle::update() {
	bool bounced = false;
	int _x = (int)x + vx;
	int _y = (int)y + vy;
	int v = 0;
	if (trimInWall(_x, wallWidth)) {
		vx = -vx * bounce / SCALE;
		v = abs(vx);
	}
	if (trimInWall(_y, wallHeight)) {
		vy = -vy * bounce / SCALE;
		v = abs(vy);
	}
	if (v >= minHeatingVelocity) {
		heatAmount += v / SCALE;
		bounced = true;
	}
	v = min(abs(vx), abs(vy));
	if (v > maxVelocity) {
		maxVelocity = v;
	}
	x = (coord_t)_x;
	y = (coord_t)_y;
	return bounced;
}

bool Particle::trimInWall(int32_t& n, coord_t wallSize) {
	if (n < radius * SCALE) {
		n = radius * SCALE;
		return true;
	} else if (n >= (wallSize - radius) * SCALE) {
		n =  (wallSize - radius) * SCALE;
		return true;
	}
	return false;
}

#define COS_SIN_DIV 15
static const int8_t cos_sin_table[] = {
	100, 0, 97, 26, 87, 50, 71, 71, 50, 87, 26, 97, 0, 100, -26, 97, -50, 87, -71, 71, -87, 50, -97, 26,
	-100, 0, -97, -26, -87, -50, -71, -71, -50, -87, -26, -97, 0, -100, 26, -97, 50, -87, 71, -71, 87, -50, 97, -26
};

int getCosX(int r, int deq) {
	return r * cos_sin_table[(deq / COS_SIN_DIV) * 2] / 100;
}

int getSinY(int r, int deq) {
	return r * cos_sin_table[(deq / COS_SIN_DIV) * 2 + 1] / 100;
}
