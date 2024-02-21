#include"Renderer.h"

// Cloud layers
Cloud::Cloud(double _v, rect src, rect dst) : src(src), dst(dst)
{
	section_width = Renderer::cloud_width * Renderer::scale;
	x = section_width;
	v = _v * Renderer::scale;
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

Renderer::Renderer(HDC wDC)
{
	resolution = GetScreenSize();

	using namespace std::chrono;

	scale = ceil(resolution.y / (double)reference_size.y);

	unscaled_resolution =
	{
		resolution.x / scale,
		reference_size.y
	};

	worker_dc = wDC;

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

	const int clouds_top = (reference_size.y - clouds_height) * scale; // If not perfectly scaled, the clouds will hang off the bottom of the screen

	clouds[0] = new Cloud(3.75, cloud_srcs[0], { 0, clouds_top,                          cloud_srcs[0].w * scale, cloud_srcs[0].h * scale });
	clouds[1] = new Cloud(7.5,  cloud_srcs[1], { 0, clouds[0]->dst.y + clouds[0]->dst.h, cloud_srcs[1].w * scale, cloud_srcs[1].h * scale });
	clouds[2] = new Cloud(15.0, cloud_srcs[2], { 0, clouds[1]->dst.y + clouds[1]->dst.h, cloud_srcs[2].w * scale, cloud_srcs[2].h * scale });
	clouds[3] = new Cloud(30.0, cloud_srcs[3], { 0, clouds[2]->dst.y + clouds[2]->dst.h, cloud_srcs[3].w * scale, cloud_srcs[3].h * scale });

	visitors = new VisitorManager(render_dc);

	RenderSky();
}

void Renderer::RenderSky(bool force_render)
{
	if (render_dc == nullptr) throw std::exception("RenderSky > Expected render_dc to be created, was nullptr");

	rect new_sky_src =
	{
		0,
		0,
		unscaled_resolution.x,
		reference_size.y - clouds_height // From 0, up to the top of the clouds
	};

	if (!force_render && new_sky_src == sky_src) return; // Already rendered

	sky_src = new_sky_src;

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

	const point unscaled_star_area = { unscaled_resolution.x, reference_size.y - clouds_height };

	constexpr point moon_offset = { -22, -22 };
	constexpr point star_offset = { -4, -4 };
	constexpr point big_star_offset = { -2, -2 };
	constexpr unsigned min_big_star_distance = 120;

	constexpr point moon_position = { -108, -44 };

	constexpr unsigned moon_area = 872;
	const unsigned area = unscaled_star_area.area() - moon_area;

	const rect moon_dst =
	{
		point(
			(sky_src.w < 216
			? sky_src.w / 2 // Middle of screen (too thin)
			: sky_src.w + moon_position.x) + sky_src.x, // Static position to the right
			sky_src.h + moon_position.y + sky_src.y
		)
		+ moon_offset,
		moon_size
	};

	moon.blit
	(
		moon_dst,
		{ { 0, 0 }, moon_size }
	);

	sky_dst = sky_src * scale;
}

Renderer::~Renderer()
{
	for (Cloud*& cloud : clouds)
	{
		if (cloud == nullptr) continue;
		delete cloud;
		cloud = nullptr;
	}

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

	DeleteDC(worker_dc);
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

	if (delta_t >= min_delta_t) return;

	// Limits framerate
	Sleep(duration_cast<milliseconds>(min_delta_t - delta_t).count());
	delta_t = min_delta_t; // Pretend that exactly the delta has passed
	current_tp = previous_tp + min_delta_t;

	current = duration_cast<milliseconds>(current_tp.time_since_epoch()).count() / 1000.0;
	delta = current - previous;
}

void Renderer::RenderLoop()
{
	while (Window::running)
	{
		// Moves clouds left
		for (auto& cloud : clouds) cloud->Update(delta);

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
		for (auto& cloud : clouds) cloud->Render(*cloud_bmp);

		// Draw visitors
		if (Window::visitorsToggled)
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
	}
}

int Renderer::scale = 1;
double Renderer::current = 0.0;
double Renderer::previous = 0.0;

point Renderer::resolution = {};
point Renderer::unscaled_resolution = {};

std::array<Cloud*, 4> Renderer::clouds = { nullptr, nullptr, nullptr, nullptr };