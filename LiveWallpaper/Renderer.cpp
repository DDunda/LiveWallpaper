#include"Renderer.h"

// Cloud layers
Cloud::Cloud(double _v, int h, int y) {
	sectionWidth = 320 * Renderer::scale;
	x = sectionWidth;
	v = _v * Renderer::scale;
	dst = {
		0,
		y * Renderer::scale,
		320 * Renderer::scale,
		h * Renderer::scale
	};
	src = {
		0,
		y,
		320,
		h
	};
}

void Cloud::DrawPart(bitmap* BG, int o) {
	dst.x = (int)round(x + o);
	BG->blit(dst, src);
}

void Cloud::Update(double delta) {
	x -= v * delta;
	// Wraps to the right once the leftmost cloud part is completely off the screen
	while (x <= 0) x += sectionWidth;
}
void Cloud::Render(bitmap* BG) {
	DrawPart(BG, -sectionWidth);
	DrawPart(BG, 0);
	DrawPart(BG, sectionWidth);
}

Renderer::Renderer(HDC wDC) {
	resolution = GetScreenSize();

	int xScale = ceil(resolution.x / 640.0);
	int yScale = ceil(resolution.y / 360.0);

	scale = xScale > yScale ? xScale : yScale;

	fakeResolution = { 0, 0, scale * 640, scale * 360 };

	fakeResolution.x = resolution.x - fakeResolution.w;
	fakeResolution.y = resolution.y - fakeResolution.h;

	workerDC = wDC;

	renderDC = CreateCompatibleDC(workerDC);
	renderBuffer = CreateCompatibleBitmap(
		workerDC,
		fakeResolution.w,
		fakeResolution.h
	);
	SelectObject(renderDC, renderBuffer);

	delta = current = previous = 0;

	// 60 60ths of a second adds to a second - 60fps
	minDeltaD = nanoseconds(1000000000 / 60);
	currentTP = SC::now();
	previousTP = currentTP;

	current = duration_cast<milliseconds>(currentTP.time_since_epoch()).count() / 1000.0;
	previous = current;

	srand(current);

	BG = new bitmap(IDB_BG, renderDC);

	starDst = {
		0,
		0,
		640 * scale,
		208 * scale
	};
	starSrc = {
		0,
		0,
		640,
		208
	};

	clouds1 = new Cloud(3.75, 35, 208);
	clouds2 = new Cloud(7.5, 23, 243);
	clouds3 = new Cloud(15, 31, 266);
	clouds4 = new Cloud(30, 63, 297);

	visitors = new VisitorManager(renderDC);
}
Renderer::~Renderer() {
	delete BG;
	DeleteDC(workerDC);
}

void Renderer::HandleTime() {
	previousTP = currentTP;
	currentTP = SC::now();
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

void Renderer::RenderLoop() {

	while (Window::running) {
		// Moves clouds left
		clouds1->Update(delta);
		clouds2->Update(delta);
		clouds3->Update(delta);
		clouds4->Update(delta);


		// Draw stars
		BG->blit(starDst, starSrc);

		// Draw clouds
		clouds1->Render(BG);
		clouds2->Render(BG);
		clouds3->Render(BG);
		clouds4->Render(BG);

		// Draw visitors
		if (Window::visitorsToggled) {
			visitors->Update(delta);
			visitors->Render();
		}

		BitBlt(
			workerDC,
			fakeResolution.x,
			fakeResolution.y,
			fakeResolution.w,
			fakeResolution.h,
			renderDC,
			0,
			0,
			SRCCOPY
		);

		// Keep track of time to do animations
		HandleTime();
	}
}

int Renderer::scale = 0;