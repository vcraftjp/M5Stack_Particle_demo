//
// M5Stack(Fire) Particle demo
//

#include <M5Stack.h>
#include <utility/MPU9250.h>
#include "Sprite8bpp.h"
#include "Audio.h"
#include "particle.h"
#include "util.h"
#include "bitmap_data.h"
#include "wave_data.h"

#define NEO_PIXEL

#ifdef NEO_PIXEL
#include <Adafruit_NeoPixel.h>
#define NEO_NUM_LEDS 10
#define NEO_DATA_PIN 15
#define NEOPIXEL_BRIGHTNESS 20
#endif

#define LCD_WIDTH 320
#define LCD_HEIGHT 240

#define INTERVAL_MS 20
#define SHRINK_MS 500

#define MAX_PARTICLES 24
#define WALL_WIDTH LCD_WIDTH
#define WALL_HEIGHT LCD_HEIGHT
#define RADIUS 12
#define MIN_BOUNCE 75
#define MAX_BOUNCE 90
#define INIT_DIST 64
#define SHRINK_SPEED (20 * Particle::SCALE)
#define SHRINK_MARGIN 4
#define ACCEL_SCALE 800

#define MIN_HUE 240
#define MAX_HUE (60 + 360)
#define HEAT_FRAMES 10
#define MIN_HEATING_VELOCITY 400
#define HEATUP_TH 100
#define HEATUP_RATE 200
#define MAX_HEATUP_STEP 4
#define COOLDOWN_TH 50
#define HEAT_STEP 10
#define MAX_HEAT ((MAX_HUE - MIN_HUE) / HEAT_STEP)
#define FREEZE_VELOCITY 220
#define FREEZE_WAIT 5

// #define DEMO

const int8_t particleCountTable[] = {
#ifndef DEMO
	2, 4, 6, 8, 12, 24
#else
	8, 24, 2, 12, 4, 6
#endif
};

Particle *particles[MAX_PARTICLES];
uint8_t particleCount;

Sprite8bpp img = Sprite8bpp(&M5.Lcd);
uint16_t palette[256];

enum { RESTART, SHRINK, MOVING };
uint8_t state = RESTART;

int heatFrame;
int heat;
int freeze;
bool isSound = false;
#ifdef NEO_PIXEL
bool showNeoPixel = true;
#endif

MPU9250 IMU;
FPS fps;
Audio audio;

#ifdef NEO_PIXEL
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NEO_NUM_LEDS, NEO_DATA_PIN, NEO_GRB + NEO_KHZ800);
#endif

void createPalette(int hue) {
	const int th = 144;
	const int min_sat = MAX_SV / 2;
	for (int i = 0; i < 256; i++) {
		int s = (i < th) ? MAX_SV : MAX_SV + ((i - th) * min_sat + (th - i) * MAX_SV) / (256 - th);
		int v = (i < th) ? i * MAX_SV / th : MAX_SV;
		palette[i] = color32To16(hsvToRGB(hue, s, v));
		// Serial.printf("%d: 0x%04X (s=%d)\r\n", i, palette[i], s);
	}
#ifdef NEO_PIXEL
	if (showNeoPixel) {
		uint32_t color = hsvToRGB(hue, MAX_SV, MAX_SV);
		for (int i = 0; i < NEO_NUM_LEDS; i++) {
			pixels.setPixelColor(i, color);
		}
		pixels.show();
		pixels.show(); // TODO: LED #0 color incorrect
	}
#endif
}

#ifdef DEMO
int particleCountIndex = 0;
#endif

void createParticles() {
#ifndef DEMO
	particleCount = particleCountTable[random(_countof(particleCountTable))];
#else
	particleCount = particleCountTable[particleCountIndex];
	if (++particleCountIndex >= _countof(particleCountTable)) {
		particleCountIndex = 0;
	}
#endif
	for (int i = 0; i < particleCount; i++) {
		int deq = 360 / particleCount * i;
		int x = getCosX(INIT_DIST, deq);
		int y = getSinY(INIT_DIST, deq);
		particles[i]->setPosition(LCD_WIDTH / 2 + x, LCD_HEIGHT / 2 + y);
		particles[i]->setVelocity(-x * SHRINK_SPEED / INIT_DIST, -y * SHRINK_SPEED / INIT_DIST);
	}
}

void spreadParticles() {
	for (int i = 0; i < particleCount; i++) {
		int deq = 360 / particleCount * i;
		int vx = getCosX(SHRINK_SPEED, deq);
		int vy = getSinY(SHRINK_SPEED, deq);
		particles[i]->setVelocity(vx, vy);
		particles[i]->setBounce(random(MIN_BOUNCE, MAX_BOUNCE));
	}
}

void drawGradientFrame() {
	const static int WIDTH = 8;
	const static int COLOR_STEP = 8;
	const static int COUNT = 8;
	for (int i = 0; i < COUNT; i++) {
		uint8_t color =  (uint8_t)(COLOR_STEP * (COUNT - i + 1));
		img.fillRect(0, i * WIDTH, LCD_WIDTH, WIDTH, color);
		img.fillRect(0, LCD_HEIGHT - (i + 1) * WIDTH, LCD_WIDTH, WIDTH, color);
	}
}

//
// setup()
//
void setup() {
	M5.begin();
	M5.Lcd.fillScreen(TFT_BLACK);

	Serial.println("create Sprite...");
	img.setColorDepth(8);
	img.createSprite(LCD_WIDTH, LCD_HEIGHT);

	Serial.println("init MPU9250...");
	Wire.begin();
	IMU.calibrateMPU9250(IMU.gyroBias, IMU.accelBias);
	IMU.initMPU9250();

	Serial.println("create Particles...");
	Particle::setWallSize(WALL_WIDTH, WALL_HEIGHT);
	Particle::setParticleRadius(RADIUS);
	Particle::minHeatingVelocity = MIN_HEATING_VELOCITY;
	for (uint8_t i = 0; i < MAX_PARTICLES; i++) {
		particles[i] = new Particle();
	}

	audio.setVolume(0);
	audio.setSampleRate(SAMPLE_RATE);

#ifdef NEO_PIXEL
	pixels.begin();
	pixels.setBrightness(NEOPIXEL_BRIGHTNESS);
#endif

	Serial.println("setup done.");
}

//
// loop()
//
void loop() {
	static unsigned long prev_ms = 0;
	static unsigned long state_ms = 0;

	unsigned long ms = millis();
	if (ms - prev_ms < INTERVAL_MS) return;
	prev_ms = ms;

	if (IMU.readByte(MPU9250_ADDRESS, INT_STATUS) & 0x01) {
		IMU.readAccelData(IMU.accelCount);
		IMU.getAres();

		IMU.ax = (float)IMU.accelCount[0] * IMU.aRes; // - accelBias[0];
		IMU.ay = (float)IMU.accelCount[1] * IMU.aRes; // - accelBias[1];
		IMU.az = (float)IMU.accelCount[2] * IMU.aRes; // - accelBias[2];
	}

	switch (state) {
	case RESTART:
		state = SHRINK;
		state_ms = ms;
		createParticles();
		createPalette(MIN_HUE);
		heatFrame = 0;
		heat = 0;
		freeze = 0;
		break;
	case SHRINK:
		if (ms - state_ms >= SHRINK_MS) {
			state = MOVING;
			state_ms = ms;
			spreadParticles();
		}
		break;
	case MOVING:
		if (++heatFrame >= HEAT_FRAMES) {
			heatFrame = 0;
			int n = Particle::heatAmount;
			Particle::heatAmount = 0;
			//Serial.printf("heat=%d, maxV=%d\r\n", n, Particle::maxVelocity);
			int prevHeat = heat;
			if (n > HEATUP_TH) {
				int heatUp = (n - HEATUP_TH) / HEATUP_RATE + 1;
				if (heatUp >= MAX_HEATUP_STEP) heatUp = MAX_HEATUP_STEP;
				if ((heat += heatUp)  >= MAX_HEAT) heat = MAX_HEAT;
			} else if (n < COOLDOWN_TH) {
				if (heat > 0) heat--;
			}
			if (heat != prevHeat) {
				createPalette(MIN_HUE + heat * HEAT_STEP);
			}
			if (heat == 0 && Particle::maxVelocity < FREEZE_VELOCITY) {
				if (++freeze >= FREEZE_WAIT) {
					state = RESTART;
				}
			} else {
				freeze = 0;
			}
		}
		break;
	}

	Particle::maxVelocity = 0;
	bool bounced = false;
	for (int i = 0; i < particleCount; i++) {
		if (state == MOVING) {
			particles[i]->setAccel(-IMU.ax * ACCEL_SCALE, IMU.ay * ACCEL_SCALE);
		}
		if (particles[i]->update()) {
			bounced = true;
		}
		if (state == SHRINK) {
			if (abs(particles[i]->getX() - LCD_WIDTH / 2) <= SHRINK_MARGIN
			 && abs(particles[i]->getY() - LCD_HEIGHT / 2) <= SHRINK_MARGIN) {
				 particles[i]->setVelocity(0, 0);
			}
		}
	}

	// img.fillSprite(TFT_BLACK);
	img.fill(0);
	drawGradientFrame();

	for (int i = 0; i < particleCount; i++) {
		// img.fillCircle(particles[i]->getX(), particles[i]->getY(),  RADIUS, TFT_GREEN);
		img.blendAlphaBitmap(particles[i]->getX() - RADIUS, particles[i]->getY() - RADIUS, RADIUS * 2, RADIUS * 2, bitmap_data);
	}

	// img.pushSprite(0, 0);
	img.flush(palette);

	if (M5.BtnA.wasPressed()) {
		isSound = !isSound;
		if (!isSound) {
			audio.stop();
		}
	}
	if (M5.BtnB.wasPressed()) {
		state = RESTART;
	}
#ifdef NEO_PIXEL
	if (M5.BtnC.wasPressed()) {
		showNeoPixel = !showNeoPixel;
		if (!showNeoPixel) {
			pixels.clear();
			pixels.show();
			pixels.show();
		}
	}
#endif

	if (bounced && isSound) {
		audio.setSampleRate(SAMPLE_RATE + (SAMPLE_RATE * heat / MAX_HEAT));
		audio.play(wave_data, sizeof(wave_data));
	}

	int n = fps.getFPS();
	if (n) {
		Serial.printf("FPS=%d ax=%.3f ay=%.3f az=%.3f\r\n", n, IMU.ax, IMU.ay, IMU.az);
	}

	M5.update();
}
