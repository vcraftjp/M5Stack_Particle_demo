//
// util.cpp
//

#include "util.h"

uint32_t hsvToRGB(int h, int s, int v, uint8_t *rgbArray) {
	uint8_t max = v * 255 / MAX_SV;
	uint8_t min = max - (s * max / MAX_SV);
	uint8_t d = max - min;
	uint8_t r = 0;
	uint8_t g = 0;
	uint8_t b = 0;

	h %= 360;
	switch (h / 60) {
		case 0:
			r = max;
			g = h * d / 60 + min;
			b = min;
			break;
		case 1:
			r = (120 - h) * d / 60 + min;
			g = max;
			b = min;
			break;
		case 2:
			r = min;
			g = max;
			b = (h - 120) * d / 60 + min;
			break;
		case 3:
			r = min;
			g = (240 - h) * d / 60 + min;
			b = max;
			break;
		case 4:
			r =  (h - 240) * d / 60 + min;
			g = min;
			b = max;
			break;
		case 5:
			r = max;
			g = min;
			b = (360 - h) * d / 60 + min;
			break;
	}
	if (rgbArray) {
		rgbArray[0] = r;
		rgbArray[1] = g;
		rgbArray[2] = b;
	}
	return rgbToColor32(r, g, b);
}

uint32_t rgbToColor32(uint8_t r, uint8_t g, uint8_t b) {
	return (uint32_t)r << 16 | (uint32_t)g << 8 | b;
}

uint16_t rgbToColor16(uint8_t r, uint8_t g, uint8_t b) {
	return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}

uint16_t color32To16(uint32_t color) {
	return rgbToColor16((color >> 16) & 0xFF, (color >> 8) & 0xFF, color & 0xFF);

}
