//
// Audio.h
//
#pragma once

#include <M5Stack.h>

//
// Audio class
//
class Audio {
public:
	Audio();
    void setVolume(int _volume);
	void setSampleRate(int _sampleRate);
	void play(const uint8_t *wave, int length, int duration = 0); // duration(ms), -1=infinite
	static void stop();

protected:
	void init();
};
