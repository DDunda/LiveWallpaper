#pragma once

#include <chrono>
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

	// Background image
	bitmap* BG;

	// Magical DC
	HDC workerDC;

	Renderer(HDC workerDC);

	~Renderer();

	// Manages the framerate and things to do with time
	void HandleTime();

	// Renders frames and does logic
	void RenderLoop();
};