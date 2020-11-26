#include"Renderer.h"

Renderer::Renderer(HDC wDC) {
	workerDC = wDC;

	renderDC = CreateCompatibleDC(workerDC);
	renderBuffer = CreateCompatibleBitmap(
		workerDC,
		1920,
		1080
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
		BG->info.bmWidth,
		624
	};
	starSrc = {
		0,
		0
	};

	clouds1 = new Cloud(11.25, 105, 624);
	clouds2 = new Cloud(22.5, 69, 729);
	clouds3 = new Cloud(45, 93, 798);
	clouds4 = new Cloud(90, 189, 891);

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
		clouds1->Move(delta);
		clouds2->Move(delta);
		clouds3->Move(delta);
		clouds4->Move(delta);


		// Draw stars
		BG->blit(starDst, starSrc);

		// Draw clouds
		clouds1->Draw(BG);
		clouds2->Draw(BG);
		clouds3->Draw(BG);
		clouds4->Draw(BG);

		// Draw visitors
		if (Window::visitorsToggled) {
			visitors->Update(delta);
			visitors->Draw();
		}

		BitBlt(
			workerDC,
			0,
			0,
			1920,
			1080,
			renderDC,
			0,
			0,
			SRCCOPY
		);

		// Keep track of time to do animations
		HandleTime();
	}
}