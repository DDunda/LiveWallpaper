#pragma once

#include <chrono>
#define _USE_MATH_DEFINES
#include <math.h>
#include"Window.h"
#include"DrawingHelpers.h"

// Yes, I know you're yelling at me, but I can't hear you :)
using namespace std;
using namespace chrono;
typedef steady_clock SC;

class Cloud {
private:
	// The clouds are a repeating texture 960 pixels wide, so this is called 3 times per layer
	void DrawPart(bitmap* BG, int o) {
		dst.x = (int)round(x + o);
		BG->blit(dst, src);
	}

public:
	// Current layer offset (x)
	double x = 960;
	// Velocity of cloud movement
	double v;
	// Where it the cloud gets drawn (x gets modified)
	rect dst;
	// Where the cloud is gotten from
	point src;

	// y is the vertical offset on both the texture and the screen
	Cloud(double _v, int h, int y) {
		v = _v;
		dst = {
			0,
			y,
			960,
			h
		};
		src = {
			0,
			y,
		};
	}
	// Draws this cloud layer
	void Draw(bitmap* BG) {
		DrawPart(BG, -960);
		DrawPart(BG, 0);
		DrawPart(BG, 960);
	}
	// The clouds blow to the left
	void Move(double delta) {
		x -= v * delta;
		// Wraps to the right once the leftmost cloud part is completely off the screen
		while (x <= 0) x += 960;
	}
};

class Blinker {
public:
	double cycle;
	bool blinking;

	Blinker() {
		blinking = false;
		cycle = (rand() % 95 + 5) / 10.0; // Eyes open for 0.5 - 10.0 seconds
	}

	void Update(double delta) {
		cycle -= delta;
		if (cycle < 0) {
			if (blinking) {
				blinking = false;
				cycle = (rand() % 95 + 5) / 10.0;
			}
			else {
				blinking = true;
				cycle = (rand() % 25 + 5) / 100.0; // 0.05-0.30 second blink
			}
		}
	}
};

class Visitor {
public:
	virtual void Spawn() = 0;
	virtual void Despawn() = 0;
	// Return whether onscreen
	virtual bool Update(double delta) = 0;
	virtual void Render() = 0;
};

class Dragon : public Visitor {
public:
	// The dragon has two frames for its flapping animation - Why not use a bool?
	bool frame;

	// Whether the dragon is holding Sue
	bool holdingSue;

	// Cycles are from 0-1, counting upwards, and trigger some behaviour when they hit 1
	double xCyle;
	double yCyle;
	double flapCycle;

	// How fast the cycles are "filling"
	double xFreq = 3.0 / 7.0;
	double yFreq = 4.0 / 15.0;
	double flapFreq = 8.0;

	// Displacement across screen
	double x = 0;
	// Speed across screen
	double speed = 90;

	// The frames to draw
	bitmap* dragon;

	// The dragon floats randomly around the "true" position. It is offset by
	// two sine functions, of which progress at these two rates (can be thought of as frequencies)
	double xVariance = 6.5 * 3.0;
	double yVariance = 16 * 3.0;

	Dragon(HDC dc) {
		flapCycle = xCyle = yCyle = 0;
		frame = false; // Frame 1
		dragon = new bitmap(IDB_DRAGON, dc);
	}

	void Spawn() {
		x = -xVariance - 40 * 3; // Offscreen - to left
		holdingSue = (rand() % 2) == 1;
		if (holdingSue) x -= 3 * 3;
	}

	void Despawn() {

	}

	bool Update(double delta) {
		x += speed * delta;

		flapCycle += flapFreq * delta;
		xCyle += xFreq * delta;
		yCyle += yFreq * delta;
		while (flapCycle >= 1) {
			flapCycle--;
			frame = !frame;
		}
		while (xCyle >= 1) xCyle--;
		while (yCyle >= 1) yCyle--;

		return x - xVariance < 1920;
	}

	void DrawDragon(point p) {
		rect src{
			frame ? 40 : 0,
			0,
			40,
			36
		};

		rect dst{
			p.x,
			p.y,
			40 * 3,
			36 * 3
		};

		dragon->tBlit(dst, src, 0);
	}

	void DrawSue(point p) {
		rect src{
			80,
			0,
			16,
			16
		};

		rect dst{
			p.x + 27 * 3,
			p.y + 24 * 3,
			16 * 3,
			16 * 3
		};

		dragon->tBlit(dst, src, 0);
	}

	void Render() {
		point p = {
			x + xVariance * sin(xCyle * 2 * M_PI),
			660 + yVariance * sin(yCyle * 2 * M_PI)
		};
		DrawDragon(p);
		if (holdingSue) DrawSue(p);
	}
};

class Ballrog : public Visitor {
public:
	// Ballrog also has two frames, coincidentally enough
	bool frame;

	double xCyle;
	double yCyle;
	double flapCycle;

	double xFreq = 3.0 / 7.0;
	double yFreq = 4.0 / 15.0;
	double flapFreq = 12.0;

	// Displacement across screen
	double x = 0;
	// Speed across screen
	double speed = 90;

	// The frames to draw
	bitmap* ballrog;

	double xVariance = 6.5 * 3.0;
	double yVariance = 16 * 3.0;

	Ballrog(HDC dc) {
		flapCycle = xCyle = yCyle = 0;
		frame = false; // Frame 1
		ballrog = new bitmap(IDB_BALLROG, dc);
	}

	void Spawn() {
		x = -xVariance - ballrog->info.bmWidth * 3 / 2; // Offscreen - to left
	}

	void Despawn() {

	}

	bool Update(double delta) {
		x += speed * delta;

		flapCycle += flapFreq * delta;
		xCyle += xFreq * delta;
		yCyle += yFreq * delta;
		while (flapCycle >= 1) {
			flapCycle--;
			frame = !frame;
		}
		while (xCyle >= 1) xCyle--;
		while (yCyle >= 1) yCyle--;

		return x - xVariance < 1920;
	}

	void Render() {
		rect src{
			frame ? ballrog->info.bmWidth / 2 : 0,
			0,
			ballrog->info.bmWidth / 2,
			ballrog->info.bmHeight,
		};

		rect dst{
			x + xVariance * sin(xCyle * 2 * M_PI),
			660 + yVariance * sin(yCyle * 2 * M_PI),
			ballrog->info.bmWidth * 3 / 2,
			ballrog->info.bmHeight * 3
		};

		ballrog->tBlit(dst, src, 0);
	}
};

class Helicopter : public Visitor {
public:
	
	int propellerFrame;

	double xCyle;
	double yCyle;
	double propellerCycle;

	double xFreq = 3.0 / 7.0;
	double yFreq = 4.0 / 15.0;
	double propellerFreq = 60.0;

	// Displacement across screen
	double x = 0;
	// Speed across screen
	double speed = 90;

	// The frames to draw
	bitmap* helicopter;

	double xVariance = 6.5 * 3.0 / 4.0;
	double yVariance = 16 * 3.0 / 4.0;

	Blinker ChakoBlinker;
	Blinker SantaBlinker;
	Blinker MomorinBlinker;

	Helicopter(HDC dc) {
		propellerCycle = xCyle = yCyle = 0;
		propellerFrame = 0;
		helicopter = new bitmap(IDB_HELICOPTER, dc);
	}

	void Spawn() {
		x = -xVariance - 142 * 3; // Offscreen - to left
	}

	void Despawn() {

	}

	bool Update(double delta) {
		x += speed * delta;

		propellerCycle += propellerFreq * delta;
		xCyle += xFreq * delta;
		yCyle += yFreq * delta;
		while (propellerCycle >= 1) {
			propellerCycle--;
			propellerFrame++;
		}
		propellerFrame %= 4;
		while (xCyle >= 1) xCyle--;
		while (yCyle >= 1) yCyle--;

		ChakoBlinker.Update(delta);
		SantaBlinker.Update(delta);
		MomorinBlinker.Update(delta);

		return x - xVariance < 1920;
	}

	void DrawHelicopterBase(point p) {
		rect src{
			0,
			84,
			126,
			59
		};

		rect dst{
			p.x + 5 * 3,
			p.y + 13 * 3,
			126 * 3,
			59 * 3
		};

		helicopter->tBlit(dst, src, 0);
	}

	void DrawPropeller1(point p) {
		int i = propellerFrame == 3 ? 2 : propellerFrame;
		rect src{
			0,
			14 * i,
			70,
			14
		};

		rect dst{
			p.x,
			p.y + 5 * 3,
			70 * 3,
			14 * 3
		};

		helicopter->tBlit(dst, src, 0);
	}

	void DrawPropeller2(point p) {
		int i = propellerFrame == 3 ? 2 : propellerFrame;
		rect src{
			0,
			42 + 14 * i,
			112,
			14
		};

		rect dst{
			p.x + 30 * 3,
			p.y,
			112 * 3,
			14 * 3
		};

		helicopter->tBlit(dst, src, 0);
	}

	void DrawBlinks(point p) {
		if (ChakoBlinker.blinking) {
			rect src{
				118,
				2,
				8,
				3
			};

			rect dst{
				p.x + 28 * 3,
				p.y + 46 * 3,
				7 * 3,
				3 * 3
			};

			helicopter->tBlit(dst, src, 0);
		}
		if (SantaBlinker.blinking) {
			rect src{
				117,
				0,
				9,
				2
			};

			rect dst{
				p.x + 40 * 3,
				p.y + 47 * 3,
				9 * 3,
				2 * 3
			};

			helicopter->tBlit(dst, src, 0);
		}
		if (MomorinBlinker.blinking) {
			rect src{
				119,
				5,
				7,
				2
			};

			rect dst{
				p.x + 54 * 3,
				p.y + 40 * 3,
				7 * 3,
				2 * 3
			};

			helicopter->tBlit(dst, src, 0);
		}
	}

	void Render() {
		point p = {
			x + xVariance * sin(xCyle * 2 * M_PI),
			750 + yVariance * sin(yCyle * 2 * M_PI)
		};

		DrawHelicopterBase(p);
		DrawPropeller1(p);
		DrawPropeller2(p);
		DrawBlinks(p);
	}
};

class VisitorManager {
public:
	// Time before a visitor appears (appears when <= 0)
	double appearanceCountdown;
	
	Visitor** visitors;
	int visitorCount;
	Visitor* currentVisitor = NULL;

	VisitorManager(HDC dc) {
		visitors = new Visitor*[] {
			new Dragon(dc),
			new Ballrog(dc),
			new Helicopter(dc)
		};
		visitorCount = 3;

		appearanceCountdown = (rand() % 6) * 60.0; // Appears for the first time somewhere from 0-5 minutes
	}

	void Update(double delta) {

		if (currentVisitor == NULL && (appearanceCountdown <= 0 || Window::forceVisitor)) {
			currentVisitor = visitors[rand() % visitorCount];
			currentVisitor->Spawn();
			Window::forceVisitor = false;
		}
		else if (currentVisitor != NULL) {
			if (!currentVisitor->Update(delta)) {
				currentVisitor->Despawn();
				currentVisitor = NULL;
				appearanceCountdown = ((rand() % 6) + 5) * 60; // Appears consequent times every 5-10 minutes
			}
		}
		else {
			appearanceCountdown -= delta;
		}
	}

	void Draw() {
		if (currentVisitor != NULL) currentVisitor->Render();
	}
};

// This is the best part of this project. You can almost forgive Win32 for what happens here.
class Renderer {
public:
	// Current time in seconds (timestamp of end of previous frame / start of this frame)
	double current;
	// Current time in seconds (timestamp of end of previous previous frame / start of previous frame)
	double previous;
	// Length in time of previous frame, in seconds
	double delta;

	// Chrono timestamp, used for current time
	SC::time_point currentTP;
	// Chrono timestamp, used for previous time
	SC::time_point previousTP;
	// Chrono duration, used for delta time
	SC::duration deltaD;
	// Minimum delta time of a frame, to cap the framerate
	SC::duration minDeltaD;

	// Where to draw the stars
	rect starDst;
	// Where to get the stars from the bitmap
	point starSrc;

	// Cloud layers
	Cloud* clouds1;
	Cloud* clouds2;
	Cloud* clouds3;
	Cloud* clouds4;

	// Randomly visitors
	VisitorManager* visitors;

	// Background image
	bitmap* BG;

	// Magical DC
	HDC workerDC;

	HBITMAP renderBuffer;
	HDC renderDC;

	Renderer(HDC workerDC);

	~Renderer();

	// Manages the framerate and things to do with time
	void HandleTime();

	// Renders frames and does logic
	void RenderLoop();
};