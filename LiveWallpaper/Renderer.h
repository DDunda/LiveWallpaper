#pragma once

#include <chrono>
#define _USE_MATH_DEFINES
#include <math.h>
#include"Window.h"
#include"DrawingHelpers.h"
#include"Visitors.h"

// Yes, I know you're yelling at me, but I can't hear you :)
using namespace std;
using namespace chrono;
typedef steady_clock SC;

class Cloud {
private:
	// The clouds are a repeating texture 320 raw pixels wide, so this is called 3 times per layer
	void DrawPart(bitmap* BG, int o);

public:
	int sectionWidth;
	// Current layer offset (x)
	double x;
	// Velocity of cloud movement
	double v;
	// Where it the cloud gets drawn (x gets modified)
	rect dst;
	// Where the cloud is gotten from
	rect src;

	// y is the vertical offset on both the texture and the screen
	Cloud(double _v, int h, int y);
	// The clouds blow to the left
	void Update(double delta);
	// Draws this cloud layer
	void Render(bitmap* BG);
};

// This is the best part of this project. You can almost forgive Win32 for what happens here.
class Renderer {
public:
	static int scale;
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
	rect starSrc;

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

	point resolution;
	rect fakeResolution;

	Renderer(HDC workerDC);

	~Renderer();

	// Manages the framerate and things to do with time
	void HandleTime();

	// Renders frames and does logic
	void RenderLoop();
};