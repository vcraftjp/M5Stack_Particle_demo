//
// util.h
//
#pragma once

#include <M5Stack.h>

#ifndef _countof
#define _countof(a) (sizeof(a) / sizeof(a[0]))
#endif

//
// FPS class
//
class FPS {
public:
	FPS(int _period = 1000) {
		period = _period;
		tick = millis();
		fps = 0;
		count = 0;
	}

	int getFPS(bool updated = true) {
		count++;
		int elapse = (int)(millis() - tick);
		if (elapse >= period) {
			tick = millis();
			fps = count;
			count = 0;
		} else if (updated) {
			return 0;
		}
		return fps;
	}

private:
	unsigned long tick;
	int period;
	int count;
	int fps;
};

//
// HSV to RGB
//
#define MAX_SV 255
uint32_t hsvToRGB(int h, int s, int v, uint8_t *rgbArray = NULL);
uint32_t rgbToColor32(uint8_t r, uint8_t g, uint8_t b);
uint16_t rgbToColor16(uint8_t r, uint8_t g, uint8_t b);
uint16_t color32To16(uint32_t color);
