//
// Audio.cpp
//

#include "Audio.h"

#define MAX_VOLUME 10

static int volume;
static int sampleRate;
static const uint8_t *wave;
static int waveIndex;
static int waveLength;
static int writeBytes;
static int totalBytes;

static volatile SemaphoreHandle_t timerSemaphore;
static hw_timer_t *timer = NULL;

void IRAM_ATTR onTimer() {
	if (waveIndex >= 0) {
		M5.Speaker.write(pgm_read_byte(wave + waveIndex) / (MAX_VOLUME + 1 - volume));
		waveIndex++;
		writeBytes++;
		if (waveIndex >= waveLength) {
			if (totalBytes) {
				if (totalBytes == -1) { // infinite loop
					waveIndex = 0;
				} else if (writeBytes < totalBytes) {
					waveIndex = 0;
				} else {
					Audio::stop();
				}
			} else {
				Audio::stop();
			}
		}
	}
	xSemaphoreGiveFromISR(timerSemaphore, NULL);
}

Audio::Audio() {
	setVolume(MAX_VOLUME);
	setSampleRate(48000);
	waveIndex = -1;
}

void Audio::setVolume(int _volume) {
	volume = min(_volume, 10);
}

void Audio::setSampleRate(int _sampleRate) {
	if (_sampleRate != sampleRate) {
		sampleRate = _sampleRate;
		if (timer) {
			timerAlarmWrite(timer, 1000000 / sampleRate, true);
		}
	}
}

void Audio::init() {
	timerSemaphore = xSemaphoreCreateBinary();

	timer = timerBegin(0, 80, true);
	timerAttachInterrupt(timer, &onTimer, true);
	timerAlarmWrite(timer, 1000000 / sampleRate, true);
	timerAlarmEnable(timer);
}

void Audio::play(const uint8_t *_wave, int _length, int _duration) {
	wave = _wave;
	waveLength = _length;
	waveIndex = 0;
	writeBytes = 0;
	totalBytes = _duration;
	if (_duration > 0) {
		totalBytes = _duration * (sampleRate / 1000);
	}
	if (!timer) {
		init();
	}
}

void Audio::stop() {
	waveIndex = -1;
	M5.Speaker.write(0);
}
