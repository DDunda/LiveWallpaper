#include"Renderer.h"

// Cloud layers
Cloud::Cloud(double _v, rect src, rect dst) : src(src), dst(dst) {
	section_width = Renderer::cloud_width * Renderer::scale;
	x = section_width;
	v = _v * Renderer::scale;
}

void Cloud::DrawPart(bitmap* txt, int o) {
	dst.x = (int)round(x + o);
	txt->blit(dst, src);
}

void Cloud::Update(double delta) {
	x -= v * delta;
	// Wraps to the right once the leftmost cloud part is completely off the screen
	while (x <= 0) x += section_width;
}
void Cloud::Render(bitmap* txt) {
	for (int x = -section_width; x < Renderer::resolution.x + section_width; x += section_width)
	{
		DrawPart(txt, x);
	}
}

Renderer::Renderer(HDC wDC)
{
	resolution = GetScreenSize();

	using namespace std::chrono;

	scale = ceil(resolution.y / reference_size.y);

	unscaled_resolution = {
		0,
		resolution.y - scale * reference_size.y,
		resolution.x,
		scale * reference_size.y
	};

	worker_dc = wDC;

	render_dc = CreateCompatibleDC(worker_dc);
	render_buffer = CreateCompatibleBitmap(
		worker_dc,
		unscaled_resolution.w,
		unscaled_resolution.h
	);
	SelectObject(render_dc, render_buffer);

	delta = current = previous = 0;

	// 60 60ths of a second adds to a second - 60fps
	minDeltaD = nanoseconds(1000000000 / 60);
	currentTP = clock::now();
	previousTP = currentTP;

	current = duration_cast<milliseconds>(currentTP.time_since_epoch()).count() / 1000.0;
	previous = current;

	srand(current);

	clouds = new bitmap(IDB_CLOUDS, render_dc);
	moon   = new bitmap(IDB_MOON,   render_dc);

	const int clouds_top = (reference_size.y - clouds_height) * scale; // If not perfectly scaled, the clouds will hang off the bottom of the screen

	clouds1 = new Cloud(3.75, cloud1_src, { 0, clouds_top,                      cloud1_src.w * scale, cloud1_src.h * scale });
	clouds2 = new Cloud( 7.5, cloud2_src, { 0, clouds1->dst.y + clouds1->dst.h, cloud2_src.w * scale, cloud2_src.h * scale });
	clouds3 = new Cloud(15.0, cloud3_src, { 0, clouds2->dst.y + clouds2->dst.h, cloud3_src.w * scale, cloud3_src.h * scale });
	clouds4 = new Cloud(30.0, cloud4_src, { 0, clouds3->dst.y + clouds3->dst.h, cloud4_src.w * scale, cloud4_src.h * scale });

	star_dst =
	{
		0,
		0,
		resolution.x,
		clouds_top // From 0, up to the top of the clouds
	};

	visitors = new VisitorManager(render_dc);

	rect moon_dst = { point(0,0), moon_size };

	star_dc = CreateCompatibleDC(worker_dc);
	star_buffer = CreateCompatibleBitmap(
		worker_dc,
		star_dst.w,
		star_dst.h
	);
	SelectObject(star_dc, star_buffer);

	stars = new bitmap(IDB_STARS, star_dc);
}

Renderer::~Renderer()
{
	if (clouds1 != nullptr) delete clouds1;
	if (clouds2 != nullptr) delete clouds2;
	if (clouds3 != nullptr) delete clouds3;
	if (clouds4 != nullptr) delete clouds4;

	if (clouds != nullptr) delete clouds;
	if (stars  != nullptr) delete stars;
	if (moon   != nullptr) delete moon;

	clouds1 = nullptr;
	clouds2 = nullptr;
	clouds3 = nullptr;
	clouds4 = nullptr;

	clouds  = nullptr;
	stars   = nullptr;
	moon    = nullptr;

	DeleteObject(star_buffer);
	DeleteObject(render_buffer);

	DeleteDC(star_dc);
	DeleteDC(render_dc);
	DeleteDC(worker_dc);
}

void Renderer::HandleTime()
{
	using namespace std::chrono;

	previousTP = currentTP;
	currentTP = clock::now();
	deltaD = currentTP - previousTP;

	previous = current;
	current = duration_cast<milliseconds>(currentTP.time_since_epoch()).count() / 1000.0;
	delta = current - previous;

	// Limits framerate
	if (deltaD < minDeltaD) {
		Sleep(duration_cast<milliseconds>(minDeltaD - deltaD).count());
		deltaD = minDeltaD; // Pretend that exactly the delta has passed
		currentTP = previousTP + minDeltaD;

		current = duration_cast<milliseconds>(currentTP.time_since_epoch()).count() / 1000.0;
		delta = current - previous;
	}
}

void Renderer::RenderLoop()
{
	while (Window::running)
	{
		// Moves clouds left
		clouds1->Update(delta);
		clouds2->Update(delta);
		clouds3->Update(delta);
		clouds4->Update(delta);

		// Draw stars
		BitBlt
		(
			render_dc,
			star_dst.x,
			star_dst.y,
			star_dst.w,
			star_dst.h,
			star_dc,
			0,
			0,
			SRCCOPY
		);

		// Draw clouds
		clouds1->Render(clouds);
		clouds2->Render(clouds);
		clouds3->Render(clouds);
		clouds4->Render(clouds);

		// Draw visitors
		if (Window::visitorsToggled)
		{
			visitors->Update(delta);
			visitors->Render();
		}

		BitBlt
		(
			worker_dc,
			unscaled_resolution.x,
			unscaled_resolution.y,
			unscaled_resolution.w,
			unscaled_resolution.h,
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
rect Renderer::unscaled_resolution = {};