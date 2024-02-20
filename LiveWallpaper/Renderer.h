#pragma once

#include <chrono>
#define _USE_MATH_DEFINES
#include <math.h>
#include"Window.h"
#include"DrawingHelpers.h"
#include"Visitors.h"

class Cloud {
private:
	// The clouds are on a tileahpe, 320px wide texture
	void DrawPart(bitmap* BG, int o);

public:
	int section_width;
	// Current layer offset (x)
	double x;
	// Velocity of cloud movement
	double v;
	// Where the cloud gets drawn (x gets modified)
	rect dst;
	// Where the cloud is gotten from
	rect src;

	// y is the vertical offset on both the texture and the screen
	Cloud(double _v, rect src, rect dst);
	// The clouds blow to the left
	void Update(double delta);
	// Draws this cloud layer
	void Render(bitmap* BG);
};

class Renderer {
public:
	static int scale;
	// Current time in seconds (timestamp of end of previous frame / start of this frame)
	static double current;
	// Current time in seconds (timestamp of end of previous previous frame / start of previous frame)
	static double previous;
	// Length in time of previous frame, in seconds
	double delta;

	typedef std::chrono::steady_clock clock;

	// Chrono timestamp, used for current time
	clock::time_point currentTP;
	// Chrono timestamp, used for previous time
	clock::time_point previousTP;
	// Chrono duration, used for delta time
	clock::duration deltaD;
	// Minimum delta time of a frame, to cap the framerate
	clock::duration minDeltaD;

	// The size of the reference background
	static constexpr point reference_size =
	{
		640,
		360
	};

	// Where to draw the stars
	rect star_dst;
	// Where to get the stars from the bitmap
	static constexpr rect star1_src    = {  0, 0, 9, 9 };
	static constexpr rect star2_src    = {  5, 0, 9, 9 };
	static constexpr rect star3_src    = { 10, 0, 9, 9 };
	static constexpr rect star4_src    = { 15, 0, 9, 9 };
	static constexpr rect star5_src    = { 25, 0, 9, 9 };
	static constexpr rect big_star_src = { 27, 0, 9, 9 };

	static constexpr unsigned cloud_width = 320; // Width of cloud texture
	static constexpr unsigned clouds_height = 152; // Height of all clouds combined
	static constexpr rect cloud1_src = { 0,  0, cloud_width, 35 };
	static constexpr rect cloud2_src = { 0, 35, cloud_width, 23 };
	static constexpr rect cloud3_src = { 0, 58, cloud_width, 30 };
	static constexpr rect cloud4_src = { 0, 88, cloud_width, 64 };

	static constexpr point moon_size = { 44, 44 };

	// Cloud layers
	Cloud* clouds1 = nullptr;
	Cloud* clouds2 = nullptr;
	Cloud* clouds3 = nullptr;
	Cloud* clouds4 = nullptr;

	// Background images
	bitmap* clouds = nullptr;
	bitmap* stars = nullptr;
	bitmap* moon = nullptr;

	// Random visitors
	VisitorManager* visitors;

	// Magical DC
	HDC worker_dc;

	HBITMAP star_buffer;
	HBITMAP render_buffer;
	HDC star_dc;
	HDC render_dc;

	static point resolution;
	static rect unscaled_resolution;

	Renderer(HDC workerDC);

	~Renderer();

	// Manages the framerate and things to do with time
	void HandleTime();

	// Renders frames and does logic
	void RenderLoop();
};