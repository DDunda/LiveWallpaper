#include "Visitors.h"
#include "Renderer.h"

// Controls blinking animations
Blinker::Blinker() {
	blinking = false;
	cycle = (rand() % 95 + 5) / 10.0; // Eyes open for 0.5 - 10.0 seconds
}

void Blinker::Update(double delta) {
	cycle -= delta;
	if (cycle < 0) {
		if (blinking) {
			blinking = false;
			cycle = (rand() % 95 + 5) / 10.0;
		}
		else {
			blinking = true;
			cycle = (rand() % 25 + 5) / 100.0; // 0.05-0.30 second blink
		}
	}
}

// Dragon visitor
Dragon::Dragon(HDC dc) {
	speed = 30 * Renderer::scale;
	xVariance = 6.5 * Renderer::scale;
	yVariance = 16 * Renderer::scale;
	flapCycle = xCyle = yCyle = 0;
	frame = false; // Frame 1
	dragon = new bitmap(IDB_DRAGON, dc);
}

void Dragon::Spawn() {
	x = -xVariance - 40 * Renderer::scale; // Offscreen - to left
	holdingSue = (rand() % 2) == 1;
	if (holdingSue) x -= 3 * Renderer::scale;
}
void Dragon::Despawn() {

}

void Dragon::DrawDragon(point p) {
	rect src{
		frame ? 40 : 0,
		0,
		40,
		36
	};

	rect dst{
		p.x,
		p.y,
		40 * Renderer::scale,
		36 * Renderer::scale
	};

	dragon->tBlit(dst, src, 0);
}
void Dragon::DrawSue(point p) {
	rect src{
		80,
		0,
		16,
		16
	};

	rect dst{
		p.x + 27 * Renderer::scale,
		p.y + 24 * Renderer::scale,
		16 * Renderer::scale,
		16 * Renderer::scale
	};

	dragon->tBlit(dst, src, 0);
}

bool Dragon::Update(double delta) {
	x += speed * delta;

	flapCycle += flapFreq * delta;
	xCyle += xFreq * delta;
	yCyle += yFreq * delta;
	while (flapCycle >= 1) {
		flapCycle--;
		frame = !frame;
	}
	while (xCyle >= 1) xCyle--;
	while (yCyle >= 1) yCyle--;

	return x - xVariance < 640 * Renderer::scale;
}
void Dragon::Render() {
	point p = {
		x + xVariance * sin(xCyle * 2 * M_PI),
		220 * Renderer::scale + yVariance * sin(yCyle * 2 * M_PI)
	};
	DrawDragon(p);
	if (holdingSue) DrawSue(p);
}

// Ballrog visitor
Ballrog::Ballrog(HDC dc) {
	speed = 30 * Renderer::scale;
	xVariance = 6.5 * Renderer::scale;
	yVariance = 16 * Renderer::scale;
	flapCycle = xCyle = yCyle = 0;
	frame = false; // Frame 1
	ballrog = new bitmap(IDB_BALLROG, dc);
}

void Ballrog::Spawn() {
	x = -xVariance - 39 * Renderer::scale; // Offscreen - to left
}
void Ballrog::Despawn() {

}

bool Ballrog::Update(double delta) {
	x += speed * delta;

	flapCycle += flapFreq * delta;
	xCyle += xFreq * delta;
	yCyle += yFreq * delta;
	while (flapCycle >= 1) {
		flapCycle--;
		frame = !frame;
	}
	while (xCyle >= 1) xCyle--;
	while (yCyle >= 1) yCyle--;

	return x - xVariance < 640 * Renderer::scale;
}
void Ballrog::Render() {
	rect src{
		frame ? 39 : 0,
		0,
		39,
		36,
	};

	rect dst{
		x + xVariance * sin(xCyle * 2 * M_PI),
		220 * Renderer::scale + yVariance * sin(yCyle * 2 * M_PI),
		39 * Renderer::scale,
		36 * Renderer::scale
	};

	ballrog->tBlit(dst, src, 0);
}

// Helicopter visitor
Helicopter::Helicopter(HDC dc) {
	speed = 30 * Renderer::scale;
	xVariance = 6.5 / 4.0 * Renderer::scale;
	yVariance = 16 / 4.0 * Renderer::scale;
	propellerCycle = xCyle = yCyle = 0;
	propellerFrame = 0;
	helicopter = new bitmap(IDB_HELICOPTER, dc);
}

void Helicopter::Spawn() {
	x = -xVariance - 142 * Renderer::scale; // Offscreen - to left
}
void Helicopter::Despawn() {

}

void Helicopter::DrawHelicopterBase(point p) {
	rect src{
		0,
		84,
		126,
		59
	};

	rect dst{
		p.x + 5 * Renderer::scale,
		p.y + 13 * Renderer::scale,
		126 * Renderer::scale,
		59 * Renderer::scale
	};

	helicopter->tBlit(dst, src, 0);
}
void Helicopter::DrawPropeller1(point p) {
	int i = propellerFrame == 3 ? 2 : propellerFrame;
	rect src{
		0,
		14 * i,
		70,
		14
	};

	rect dst{
		p.x,
		p.y + 5 * Renderer::scale,
		70 * Renderer::scale,
		14 * Renderer::scale
	};

	helicopter->tBlit(dst, src, 0);
}
void Helicopter::DrawPropeller2(point p) {
	int i = propellerFrame == 3 ? 2 : propellerFrame;
	rect src{
		0,
		42 + 14 * i,
		112,
		14
	};

	rect dst{
		p.x + 30 * Renderer::scale,
		p.y,
		112 * Renderer::scale,
		14 * Renderer::scale
	};

	helicopter->tBlit(dst, src, 0);
}
void Helicopter::DrawBlinks(point p) {
	if (ChakoBlinker.blinking) {
		rect src{
			118,
			2,
			8,
			3
		};

		rect dst{
			p.x + 28 * Renderer::scale,
			p.y + 46 * Renderer::scale,
			7 * Renderer::scale,
			3 * Renderer::scale
		};

		helicopter->tBlit(dst, src, 0);
	}
	if (SantaBlinker.blinking) {
		rect src{
			117,
			0,
			9,
			2
		};

		rect dst{
			p.x + 40 * Renderer::scale,
			p.y + 47 * Renderer::scale,
			9 * Renderer::scale,
			2 * Renderer::scale
		};

		helicopter->tBlit(dst, src, 0);
	}
	if (MomorinBlinker.blinking) {
		rect src{
			119,
			5,
			7,
			2
		};

		rect dst{
			p.x + 54 * Renderer::scale,
			p.y + 40 * Renderer::scale,
			7 * Renderer::scale,
			2 * Renderer::scale
		};

		helicopter->tBlit(dst, src, 0);
	}
}

bool Helicopter::Update(double delta) {
	x += speed * delta;

	propellerCycle += propellerFreq * delta;
	xCyle += xFreq * delta;
	yCyle += yFreq * delta;
	while (propellerCycle >= 1) {
		propellerCycle--;
		propellerFrame++;
	}
	propellerFrame %= 4;
	while (xCyle >= 1) xCyle--;
	while (yCyle >= 1) yCyle--;

	ChakoBlinker.Update(delta);
	SantaBlinker.Update(delta);
	MomorinBlinker.Update(delta);

	return x - xVariance < 640 * Renderer::scale;
}
void Helicopter::Render() {
	point p = {
		x + xVariance * sin(xCyle * 2 * M_PI),
		250 * Renderer::scale + yVariance * sin(yCyle * 2 * M_PI)
	};

	DrawHelicopterBase(p);
	DrawPropeller1(p);
	DrawPropeller2(p);
	DrawBlinks(p);
}

// Visitor manager
VisitorManager::VisitorManager(HDC dc) {
	visitors = new Visitor * [] {
		new Dragon(dc),
			new Ballrog(dc),
			new Helicopter(dc)
	};
	visitorCount = 3;

	appearanceCountdown = (rand() % 6) * 60.0; // Appears for the first time somewhere from 0-5 minutes
}

void VisitorManager::Update(double delta) {

	if (currentVisitor == NULL && (appearanceCountdown <= 0 || Window::forceVisitor)) {
		currentVisitor = visitors[rand() % visitorCount];
		currentVisitor->Spawn();
		Window::forceVisitor = false;
	}
	else if (currentVisitor != NULL) {
		if (!currentVisitor->Update(delta)) {
			currentVisitor->Despawn();
			currentVisitor = NULL;
			appearanceCountdown = ((rand() % 6) + 5) * 60; // Appears consequent times every 5-10 minutes
		}
	}
	else {
		appearanceCountdown -= delta;
	}
}
void VisitorManager::Render() {
	if (currentVisitor != NULL) currentVisitor->Render();
}