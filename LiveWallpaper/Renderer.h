#pragma once

#include <chrono>
#define _USE_MATH_DEFINES
#include <math.h>
#include <array>
#include"Window.h"
#include"DrawingHelpers.h"
#include"Visitors.h"

class Cloud {
private:
	// The clouds are on a tileable, 320px wide texture
	void DrawPart(bitmap& bmp, int o);

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
	void Render(bitmap& bmp);
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
	clock::time_point current_tp;
	// Chrono timestamp, used for previous time
	clock::time_point previous_tp;
	// Chrono duration, used for delta time
	clock::duration delta_t;
	// Minimum delta time of a frame, to cap the framerate
	static constexpr clock::duration min_delta_t = std::chrono::nanoseconds(1000000000 / 60);

	// The size of the reference background
	static constexpr point reference_size =
	{
		640,
		360
	};

	// Where to get star bitmap (0,0 entire size of buffer)
	rect sky_src;
	// Where to draw the stars
	rect sky_dst;

	HBITMAP sky_buffer = nullptr;
	HDC sky_dc = nullptr;

	// Where to get the stars from the bitmap
	static constexpr std::array<rect, 5> small_star_srcs =
	{
		rect(  0, 0, 9, 9 ),
		rect(  5, 0, 9, 9 ),
		rect( 10, 0, 9, 9 ),
		rect( 15, 0, 9, 9 ),
		rect( 25, 0, 9, 9 ),
	};
	static constexpr rect big_star_src = { 27, 0, 9, 9 };

	static constexpr unsigned cloud_width = 320; // Width of cloud texture
	static constexpr unsigned clouds_height = 152; // Height of all clouds combined
	static constexpr std::array<rect, 4> cloud_srcs =
	{
		rect( 0,  0, cloud_width, 35 ),
		rect( 0, 35, cloud_width, 23 ),
		rect( 0, 58, cloud_width, 30 ),
		rect( 0, 88, cloud_width, 64 ),
	};

	static constexpr point moon_size = { 44, 44 };

	// Cloud layers
	static std::array<Cloud*, 4> clouds;

	bitmap* cloud_bmp = nullptr;

	// Random visitors
	VisitorManager* visitors;

	// Magical DC
	HDC worker_dc;

	HBITMAP render_buffer;
	HDC render_dc;

	static point resolution;
	static point unscaled_resolution;

	void RenderSky(bool force_render = false);

	Renderer(HDC worker_dc);

	~Renderer();

	// Manages the framerate and things to do with time
	void HandleTime();

	// Renders frames and does logic
	void RenderLoop();
};