#include "renderer.h"
#include "resource.h"
#include "globals.h"
#include <vector>

Cloud::Cloud() {}

// Cloud layers
Cloud::Cloud(double _v, rect src, rect dst) : src(src), dst(dst)
{
	section_width = Renderer::CLOUDS_SIZE.x * Renderer::pixel_scale;
	x = section_width;
	v = _v * Renderer::pixel_scale;
}

void Cloud::DrawPart(bitmap& txt, int o)
{
	dst.x = (int)round(x + o);
	txt.blit(dst, src);
}

void Cloud::Update(double delta)
{
	x -= v * delta;
	// Wraps to the right once the leftmost cloud part is completely off the screen
	while (x <= 0) x += section_width;
}

void Cloud::Render(bitmap& txt)
{
	for (int x = -section_width; x < Renderer::resolution.x + section_width; x += section_width)
	{
		DrawPart(txt, x);
	}
}

uint32_t rand32()
{
	if constexpr (RAND_MAX >= 0xFFFFFFFF)
	{
		return rand();
	}
	else if constexpr (RAND_MAX == 0xFFFF)
	{
		return
			static_cast<uint32_t>(static_cast<unsigned int>(rand())) |
			(static_cast<uint32_t>(static_cast<unsigned int>(rand())) << 16);
	}
	else if constexpr (RAND_MAX == 0xFF)
	{
		return
			static_cast<uint32_t>(static_cast<unsigned int>(rand())) |
			(static_cast<uint32_t>(static_cast<unsigned int>(rand())) << 8) |
			(static_cast<uint32_t>(static_cast<unsigned int>(rand())) << 16) |
			(static_cast<uint32_t>(static_cast<unsigned int>(rand())) << 24);
	}
	else if constexpr (RAND_MAX <= 0) throw std::exception("RAND_MAX is <= 0?");

	uint32_t num_chunks = 0;
	uint32_t tmp = 0xFFFFFFFF;

	do {
		num_chunks++;
		tmp /= RAND_MAX;
	} while (tmp > 0);

	uint32_t out_val = static_cast<uint32_t>(static_cast<unsigned int>(rand()));
	num_chunks--;

	do {
		out_val *= RAND_MAX + 1;
		out_val += static_cast<uint32_t>(static_cast<unsigned int>(rand()));
		num_chunks--;
	} while (num_chunks > 0);

	return out_val;
}

void Renderer::RenderSky(bool force_render)
{
	if (render_dc == nullptr) throw std::exception("RenderSky > Expected render_dc to be created, was nullptr");

	rect new_sky_src =
	{
		0,
		0,
		unscaled_resolution.x,
		REFERENCE_SIZE.y - CLOUDS_SIZE.y // From 0, up to the top of the clouds
	};

	if (!force_render && new_sky_src == sky_src) return; // Already rendered

	sky_src = new_sky_src;
	sky_dst = sky_src * pixel_scale;

	if (sky_buffer != nullptr)
	{
		DeleteObject(sky_buffer);
		DeleteDC(sky_dc);
	}

	sky_dc = CreateCompatibleDC(worker_dc);
	sky_buffer = CreateCompatibleBitmap
	(
		worker_dc,
		sky_src.w,
		sky_src.h
	);
	SelectObject(sky_dc, sky_buffer);

	// These are cleaned up by going out of scope
	bitmap stars(IDB_STARS, sky_dc);
	bitmap moon(IDB_MOON, sky_dc);

	const point moon_pos
	{
		(sky_src.w < 216
		? sky_src.w / 2 // Middle of screen (too thin)
		: sky_src.w - 108) + sky_src.x, // Static position to the right
		sky_src.h - 44 + sky_src.y
	};

	const rect moon_dst =
	{
		moon_pos + MOON_OFFSET,
		MOON_SIZE
	};

	for (unsigned int i = 0; i < SMALL_STAR_NUM; i++)
	{
		const unsigned min_stars = (SMALL_STAR_DENSITIES_MIN[i] * sky_src.area()) / 1000000;
		const unsigned max_stars = (SMALL_STAR_DENSITIES_MAX[i] * sky_src.area()) / 1000000;
		unsigned num_stars = min_stars + (min_stars == max_stars ? 0 : rand() % (max_stars - min_stars));

		const rect& src = SMALL_STAR_SRCS[i];

		while (num_stars != 0)
		{
			const uint32_t pos_rng = rand32() % (sky_src.area());

			stars.blit
			(
				{
					static_cast<int>(pos_rng % sky_src.w) + STAR_OFFSET.x,
					static_cast<int>(pos_rng / sky_src.w) + STAR_OFFSET.y,
					src.w,
					src.h
				},
				src
			);
			num_stars--;
		}
	}

	moon.blit
	(
		moon_dst,
		{ 0, 0, MOON_SIZE.x, MOON_SIZE.y }
	);

	const rect big_star_sky =
	{
		4,
		4,
		sky_src.w - 8,
		sky_src.h - 8,
	};

	const int area = big_star_sky.area() - M_PI * BIG_STAR_MIN_MOON_DISTANCE * BIG_STAR_MIN_MOON_DISTANCE;

	if (area <= 0) return;

	const unsigned min_big_stars = (BIG_STAR_DENSITY_MIN * area) / 1000000;
	const unsigned max_big_stars = (BIG_STAR_DENSITY_MAX * area) / 1000000;
	unsigned num_big_stars = min_big_stars + (min_big_stars == max_big_stars ? 0 : rand() % (max_big_stars - min_big_stars));

	constexpr size_t MAX_FAILS = 20;
	size_t fails = 0;

	std::vector<point> big_stars = {};

	while (num_big_stars != 0)
	{
		const uint32_t pos_rng = rand32() % (big_star_sky.area());
		const int x = static_cast<int>(pos_rng % big_star_sky.w) + big_star_sky.x;
		const int y = static_cast<int>(pos_rng / big_star_sky.w) + big_star_sky.y;

		unsigned long min_dist2 = ~0ul;

		if (!big_stars.empty())
		{
			min_dist2 = (x - big_stars[0].x) * (x - big_stars[0].x) + (y - big_stars[0].y) * (y - big_stars[0].y);

			for (size_t i = 1; i < big_stars.size(); i++)
			{
				unsigned long dist2 = (x - big_stars[i].x) * (x - big_stars[i].x) + (y - big_stars[i].y) * (y - big_stars[i].y);
				min_dist2 = min(min_dist2, dist2);
			}
		}

		if (min_dist2 >= BIG_STAR_MIN_DISTANCE * BIG_STAR_MIN_DISTANCE &&
			(x - moon_pos.x) * (x - moon_pos.x) +
			(y - moon_pos.y) * (y - moon_pos.y) >=
			BIG_STAR_MIN_MOON_DISTANCE * BIG_STAR_MIN_MOON_DISTANCE)
		{
			stars.blit
			(
				{
					x + STAR_OFFSET.x,
					y + STAR_OFFSET.y,
					BIG_STAR_SRC.w,
					BIG_STAR_SRC.h
				},
				BIG_STAR_SRC
			);
			big_stars.push_back({ x,y });
		}
		else if constexpr (MAX_FAILS > 0)
		{
			fails++;
			if (fails < MAX_FAILS) continue;
		}

		fails = 0;
		num_big_stars--;
	}

}

Renderer::Renderer(HWND window)
{
	resolution = GetScreenSize();

	using namespace std::chrono;

	pixel_scale = ceil(resolution.y / (double)REFERENCE_SIZE.y);

	unscaled_resolution =
	{
		resolution.x / pixel_scale,
		REFERENCE_SIZE.y
	};

	worker_hwnd = window;
	worker_dc = GetDCEx(window, NULL, DCX_LOCKWINDOWUPDATE | DCX_CACHE | DCX_WINDOW); // flags == 0x403

	render_dc = CreateCompatibleDC(worker_dc);
	render_buffer = CreateCompatibleBitmap
	(
		worker_dc,
		resolution.x,
		resolution.y
	);

	SelectObject(render_dc, render_buffer);

	delta = current = previous = 0;

	// 60 60ths of a second adds to a second - 60fps
	previous_tp = current_tp = clock::now();

	current = duration_cast<milliseconds>(current_tp.time_since_epoch()).count() / 1000.0;
	previous = current;

	srand(current);

	cloud_bmp = new bitmap(IDB_CLOUDS, render_dc);

	const int clouds_top = (REFERENCE_SIZE.y - CLOUDS_SIZE.y) * pixel_scale; // If not perfectly scaled, the clouds will hang off the bottom of the screen

	clouds =
	{
		Cloud( 3.75, CLOUD_SRCS[0], { 0, 0, CLOUD_SRCS[0].w * pixel_scale, CLOUD_SRCS[0].h * pixel_scale }),
		Cloud( 7.50, CLOUD_SRCS[1], { 0, 0, CLOUD_SRCS[1].w * pixel_scale, CLOUD_SRCS[1].h * pixel_scale }),
		Cloud(15.00, CLOUD_SRCS[2], { 0, 0, CLOUD_SRCS[2].w * pixel_scale, CLOUD_SRCS[2].h * pixel_scale }),
		Cloud(30.00, CLOUD_SRCS[3], { 0, 0, CLOUD_SRCS[3].w * pixel_scale, CLOUD_SRCS[3].h * pixel_scale }),
	};

	clouds[0].dst.y = clouds_top;
	clouds[1].dst.y = clouds[0].dst.y + clouds[0].dst.h;
	clouds[2].dst.y = clouds[1].dst.y + clouds[1].dst.h;
	clouds[3].dst.y = clouds[2].dst.y + clouds[2].dst.h;

	visitors = new VisitorManager(render_dc);

	RenderSky();
}

Renderer::~Renderer()
{
	if (cloud_bmp != nullptr)
	{
		delete cloud_bmp;
		cloud_bmp = nullptr;
	}

	if (sky_buffer != nullptr)
	{
		DeleteObject(sky_buffer);
		DeleteDC(sky_dc);
		sky_buffer = nullptr;
		sky_dc = nullptr;
	}

	if (render_dc != nullptr)
	{
		DeleteObject(render_buffer);
		DeleteDC(render_dc);
		render_buffer = nullptr;
		render_dc = nullptr;
	}

	DeleteDC(render_dc);
	ReleaseDC(worker_hwnd, worker_dc);
}

void Renderer::HandleTime()
{
	using namespace std::chrono;

	previous_tp = current_tp;
	current_tp = clock::now();
	delta_t = current_tp - previous_tp;

	previous = current;
	current = duration_cast<milliseconds>(current_tp.time_since_epoch()).count() / 1000.0;
	delta = current - previous;

	if (delta_t >= MIN_DELTA_T) return;

	// Limits framerate
	Sleep(duration_cast<milliseconds>(MIN_DELTA_T - delta_t).count());
	delta_t = MIN_DELTA_T; // Pretend that exactly the delta has passed
	current_tp = previous_tp + MIN_DELTA_T;

	current = duration_cast<milliseconds>(current_tp.time_since_epoch()).count() / 1000.0;
	delta = current - previous;
}

void Renderer::RenderLoop()
{
	std::shared_lock<std::shared_mutex> lck(isRunning_mtx, std::defer_lock);

	while (true)
	{
		lck.lock();
		if (!isRunning) return;

		// Moves clouds left
		for (auto& cloud : clouds) cloud.Update(delta);

		// Draw stars
		StretchBlt
		(
			render_dc,
			sky_dst.x,
			sky_dst.y,
			sky_dst.w,
			sky_dst.h,

			sky_dc,
			sky_src.x,
			sky_src.y,
			sky_src.w,
			sky_src.h,
			SRCCOPY
		);

		// Draw clouds
		for (auto& cloud : clouds) cloud.Render(*cloud_bmp);

		// Draw visitors
		if (visitorsToggled)
		{
			visitors->Update(delta);
			visitors->Render();
		}

		BitBlt
		(
			worker_dc,
			0,
			0,
			resolution.x,
			resolution.y,
			render_dc,
			0,
			0,
			SRCCOPY
		);

		// Keep track of time to do animations
		HandleTime();

		lck.unlock(); // Unlock between loops to allow the running state to change
	}
}

int Renderer::pixel_scale = 1;
double Renderer::current = 0.0;
double Renderer::previous = 0.0;

point Renderer::resolution = {};
point Renderer::unscaled_resolution = {};