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
	int section_width = 0;
	// Current layer offset (x)
	double x = 0;
	// Velocity of cloud movement
	double v = 0;
	// Where the cloud gets drawn (x gets modified)
	rect dst = {};
	// Where the cloud is gotten from
	rect src = {};

	Cloud();

	// y is the vertical offset on both the texture and the screen
	Cloud(double _v, rect src, rect dst);
	// The clouds blow to the left
	void Update(double delta);
	// Draws this cloud layer
	void Render(bitmap& bmp);
};

class Renderer {
public:
	typedef std::chrono::steady_clock clock;

	// The size of the reference background (background.bmp)
	static constexpr point REFERENCE_SIZE = { 640, 360 };

	static constexpr size_t SMALL_STAR_NUM = 5; // Number of small stars on bitmap
	// Where each small star is on the texture
	static constexpr std::array<rect, SMALL_STAR_NUM> SMALL_STAR_SRCS =
	{
		rect(0, 0, 9, 9),
		rect(5, 0, 9, 9),
		rect(10, 0, 9, 9),
		rect(15, 0, 9, 9),
		rect(20, 0, 9, 9),
	};
	static constexpr rect BIG_STAR_SRC = { 27, 0, 9, 9 }; // Where the big star is on the texture

	static constexpr point STAR_OFFSET = { -4, -4 }; // Offset from star to upper left corner of sprite

	static constexpr std::array<unsigned, SMALL_STAR_NUM> SMALL_STAR_DENSITIES_MIN = { 40,  90, 140, 140, 40 }; // Minimum small stars per million pixels
	static constexpr std::array<unsigned, SMALL_STAR_NUM> SMALL_STAR_DENSITIES_MAX = { 60, 110, 160, 160, 60 }; // Maximum small stars per million pixels

	static constexpr unsigned BIG_STAR_DENSITY_MIN = 40; // Minimum big stars per million pixels
	static constexpr unsigned BIG_STAR_DENSITY_MAX = 60; // Maximum big stars per million pixels

	static constexpr unsigned BIG_STAR_MIN_DISTANCE = 120; // Big stars may not be within 120px of each other
	static constexpr unsigned BIG_STAR_MIN_MOON_DISTANCE = 27; // Big stars may not be within ~10px of the moon (moon has radius of 17px)

	static constexpr point MOON_SIZE = { 44, 44 }; // Size of moon texture
	static constexpr point MOON_OFFSET = { -22, -22 }; // Offset from center of moon to upper left corner of texture

	static constexpr point CLOUDS_SIZE = { 320, 152 }; // Size of cloud texture
	// Where each cloud layer is on the texture
	static constexpr std::array<rect, 4> CLOUD_SRCS =
	{
		rect(0,  0, CLOUDS_SIZE.x, 35),
		rect(0, 35, CLOUDS_SIZE.x, 23),
		rect(0, 58, CLOUDS_SIZE.x, 30),
		rect(0, 88, CLOUDS_SIZE.x, 64),
	};

	// Minimum delta time of a frame, to cap the framerate
	static constexpr clock::duration MIN_DELTA_T = std::chrono::nanoseconds(1000000000 / 60);
	
	static double current; // Current time in seconds (timestamp of end of previous frame / start of this frame)
	static double previous; // Previous time in seconds (timestamp of end of previous previous frame / start of previous frame)
	
	double delta; // Length of previous frame, in seconds
	
	clock::time_point current_tp; // Chrono timestamp, used for current time
	clock::time_point previous_tp; // Chrono timestamp, used for previous time
	clock::duration delta_t; // Chrono duration, used for delta time

	static int pixel_scale;

	// Where to get star bitmap (0,0 entire size of buffer)
	rect sky_src;
	// Where to draw the stars
	rect sky_dst;

	HBITMAP sky_buffer = nullptr;
	HDC sky_dc = nullptr;

	bitmap* cloud_bmp = nullptr;

	// Cloud layers
	std::array<Cloud, 4> clouds;

	// Random visitors
	VisitorManager* visitors;

	// Magical DC
	HDC worker_dc;
	HWND worker_hwnd;

	HBITMAP render_buffer;
	HDC render_dc;

	static point resolution;
	static point unscaled_resolution;

	void RenderSky(bool force_render = false);

	Renderer(HWND window);

	~Renderer();

	// Manages the framerate and things to do with time
	void HandleTime();

	// Renders frames and does logic
	void RenderLoop();
};