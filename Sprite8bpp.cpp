//
// sprite8bpp.cpp
//

#include "Sprite8bpp.h"

void Sprite8bpp::fillRect(int32_t x, int32_t y, int32_t w, int32_t h, uint8_t color, bool blend) {
	if (!_created ) return;

	if (x < 0) { w += x; x = 0; }

	if ((x < 0) || (y < 0) || (x >= _iwidth) || (y >= _iheight)) return;
	if ((x + w) > _iwidth)  w = _iwidth  - x;
	if ((y + h) > _iheight) h = _iheight - y;
	if ((w < 1) || (h < 1)) return;

	int32_t yp = _iwidth * y + x;

	if (!blend) {
		while (h--) {
			memset(_img8 + yp, color, w);
			yp += _iwidth;
		}
	} else {
		while (h--) {
			uint8_t *p = _img8 + yp;
			for (int i = 0; i < w; i++) {
				*p++ |= color;
			}
			yp += _iwidth;
		}
	}
}

void Sprite8bpp::fill(uint8_t color) {
    memset(_img8, color, _iwidth * _iheight);
}

void Sprite8bpp::blendAlphaBitmap(int32_t x, int32_t y, int32_t w, int32_t h, const uint8_t *bitmap, int ratio) {
	if (x < 0) { w += x; x = 0; }

	if ((x < 0) || (y < 0) || (x >= _iwidth) || (y >= _iheight)) return;
	if ((x + w) > _iwidth)  w = _iwidth  - x;
	if ((y + h) > _iheight) h = _iheight - y;
	if ((w < 1) || (h < 1)) return;

	int offset = _iwidth * y + x;
	const uint8_t *p = bitmap;

	while (h--) {
		uint8_t *q = _img8 + offset;
		for (int i = 0; i < w; i++) {
			uint8_t b = pgm_read_byte(p++);
			uint16_t d = *q;
			if (d == 0) {
				*q++ = b;
			} else {
				d = (b >= d) ? b + d / ratio : b / ratio + d; // TODO
				if (d > 255) {
					d = 255;
				}
				*q++ = (uint8_t)d;
			}
		}
		offset += _iwidth;
	}

}

void Sprite8bpp::flush(uint16_t *palette) {
	if (palette == NULL) {
		pushSprite(0, 0);
		return;
	}

	int x = 0;
	int y = 0;
	int w = _dwidth;
	int h = _dheight;

	startWrite();
	inTransaction = true;

	// setAddrWindow(x, y, x + w - 1, y + h - 1);
	setWindow(x, y, x + w - 1, y + h - 1);
	uint16_t lineBuf[w];
	uint8_t *data = _img8 + x + y * w;

	int dh = h;
	while (dh--) {
		int len = w;
		uint8_t* p = data;
		uint16_t* q = lineBuf;

		while(len--) {
			*q++ = palette[*p++];
		}

		pushColors(lineBuf, w, true); // swapByte = true
		data += w;
	}

	CS_H;
	inTransaction = false;
	endWrite();
}

