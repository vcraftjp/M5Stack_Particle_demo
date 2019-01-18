//
// Sprite8bpp.h
//
#pragma once

#include <M5Stack.h>

class Sprite8bpp : public TFT_eSprite {
public:
	Sprite8bpp(TFT_eSPI *tft) : TFT_eSprite(tft) {
		setColorDepth(8);
	}
	void fillRect(int32_t x, int32_t y, int32_t w, int32_t h, uint8_t color, bool blend = false);
	void fillRect(int32_t x, int32_t y, int32_t w, int32_t h, uint16_t color, bool blend = false) {
		fillRect(x, y, w, h, color16to8(color), blend);
	}
	void fill(uint8_t color);

	void blendAlphaBitmap(int32_t x, int32_t y, int32_t w, int32_t h, const uint8_t *bitmap, int ratio = 3);

	void flush(uint16_t *palette = NULL);

	inline void startWrite() {
		#if defined (SPI_HAS_TRANSACTION) && defined (SUPPORT_TRANSACTIONS) && !defined(ESP32_PARALLEL)
			if (locked) {locked = false; SPI.beginTransaction(SPISettings(SPI_FREQUENCY, MSBFIRST, SPI_MODE0));}
		#endif
		CS_L;
	}
	inline void endWrite() {
		#if defined (SPI_HAS_TRANSACTION) && defined (SUPPORT_TRANSACTIONS) && !defined(ESP32_PARALLEL)
			if(!inTransaction) {if (!locked) {locked = true; SPI.endTransaction();}}
		#endif
		CS_H;
	}
};
