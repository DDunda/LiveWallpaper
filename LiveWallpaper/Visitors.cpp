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
	speed = 30 * Renderer::pixel_scale;
	flapCycle = xCycle = yCycle = 0.0;
	frame = FLAP_UP; // Frame 1
	dragon = new bitmap(IDB_DRAGON, dc);
}

Dragon::~Dragon()
{
	if (dragon != nullptr) delete dragon;
	dragon = nullptr;
}

void Dragon::Spawn()
{
	x = -(X_RADIUS + 40.0) * Renderer::pixel_scale; // Offscreen - to left
	holdingSue = (rand() % 2) == 1;
	if (holdingSue) x -= 3 * Renderer::pixel_scale;
}

void Dragon::Despawn() { }

void Dragon::DrawDragon(point p)
{
	rect src
	{
		frame == FLAP_DOWN ? 40 : 0,
		0,
		40,
		36
	};

	rect dst
	{
		p.x,
		p.y,
		40 * Renderer::pixel_scale,
		36 * Renderer::pixel_scale
	};

	dragon->tBlit(dst, src, 0);
}
void Dragon::DrawSue(point p)
{
	rect src
	{
		80,
		0,
		16,
		16
	};

	rect dst
	{
		p.x + 27 * Renderer::pixel_scale,
		p.y + 24 * Renderer::pixel_scale,
		16 * Renderer::pixel_scale,
		16 * Renderer::pixel_scale
	};

	dragon->tBlit(dst, src, 0);
}

bool Dragon::Update(double delta)
{
	x += speed * delta;

	flapCycle += FLAP_RATE * delta;
	xCycle += X_RATE * delta;
	yCycle += Y_RATE * delta;

	while (flapCycle >= 1.0)
	{
		flapCycle -= 1.0;
		frame = !frame;
	}

	// Restrict to [0.0, 1.0) range
	while (xCycle >= 1.0) xCycle -= 1.0;
	while (yCycle >= 1.0) yCycle -= 1.0;

	return x - X_RADIUS * Renderer::pixel_scale < Renderer::resolution.x;
}
void Dragon::Render()
{
	point p =
	{
		(int)round( x     + X_RADIUS * sin(xCycle * 2.0 * M_PI)  * Renderer::pixel_scale),
		(int)round((220.0 + Y_RADIUS * sin(yCycle * 2.0 * M_PI)) * Renderer::pixel_scale)
	};
	DrawDragon(p);
	if (holdingSue) DrawSue(p);
}

// Balrog visitor
Balrog::Balrog(HDC dc)
{
	speed = 30 * Renderer::pixel_scale;
	flapCycle = xCycle = yCycle = 0;
	frame = false; // Frame 1
	balrog = new bitmap(IDB_BALROG, dc);
}

Balrog::~Balrog()
{
	if (balrog != nullptr) delete balrog;
	balrog = nullptr;
}

void Balrog::Spawn()
{
	x = -(X_RADIUS + 39.0) * Renderer::pixel_scale; // Offscreen - to left
}
void Balrog::Despawn() { }

bool Balrog::Update(double delta)
{
	x += speed * delta;

	flapCycle += FLAP_RATE * delta;
	xCycle += X_RATE * delta;
	yCycle += Y_RATE * delta;

	while (flapCycle >= 1.0)
	{
		flapCycle -= 1.0;
		frame = !frame;
	}

	while (xCycle >= 1.0) xCycle -= 1.0;
	while (yCycle >= 1.0) yCycle -= 1.0;

	return x - X_RADIUS * Renderer::pixel_scale < Renderer::resolution.x;
}
void Balrog::Render()
{
	rect src
	{
		frame == FLAP_DOWN ? 39 : 0,
		0,
		39,
		36,
	};

	rect dst
	{
		(int)round( x     + X_RADIUS * sin(xCycle * 2.0 * M_PI)  * Renderer::pixel_scale),
		(int)round((220.0 + Y_RADIUS * sin(yCycle * 2.0 * M_PI)) * Renderer::pixel_scale),
		39 * Renderer::pixel_scale,
		36 * Renderer::pixel_scale
	};

	balrog->tBlit(dst, src, 0);
}

// Helicopter visitor
Helicopter::Helicopter(HDC dc)
{
	speed = 30 * Renderer::pixel_scale;
	propellerCycle = xCycle = yCycle = 0;
	propellerFrame = 0;
	helicopter = new bitmap(IDB_HELICOPTER, dc);
}

Helicopter::~Helicopter()
{
	if (helicopter != nullptr) delete helicopter;
	helicopter = nullptr;
}

void Helicopter::Spawn()
{
	x = -(X_RADIUS + 142.0) * Renderer::pixel_scale; // Offscreen - to left
}
void Helicopter::Despawn() { }

void Helicopter::DrawHelicopterBase(point p)
{
	rect src
	{
		0,
		84,
		126,
		59
	};

	rect dst
	{
		p.x +  5 * Renderer::pixel_scale,
		p.y + 13 * Renderer::pixel_scale,
		126 * Renderer::pixel_scale,
		 59 * Renderer::pixel_scale
	};

	helicopter->tBlit(dst, src, 0);
}

void Helicopter::DrawPropeller1(point p)
{
	int i = propellerFrame == 3 ? 2 : propellerFrame;

	rect src
	{
		0,
		14 * i,
		70,
		14
	};

	rect dst
	{
		p.x,
		p.y + 5 * Renderer::pixel_scale,
		70 * Renderer::pixel_scale,
		14 * Renderer::pixel_scale
	};

	helicopter->tBlit(dst, src, 0);
}

void Helicopter::DrawPropeller2(point p)
{
	int i = propellerFrame == 3 ? 2 : propellerFrame;

	rect src
	{
		0,
		42 + 14 * i,
		112,
		14
	};

	rect dst
	{
		p.x + 30 * Renderer::pixel_scale,
		p.y,
		112 * Renderer::pixel_scale,
		14 * Renderer::pixel_scale
	};

	helicopter->tBlit(dst, src, 0);
}

void Helicopter::DrawBlinks(point p)
{
	if (ChakoBlinker.blinking)
	{
		rect src
		{
			118,
			2,
			8,
			3
		};

		rect dst
		{
			p.x + 28 * Renderer::pixel_scale,
			p.y + 46 * Renderer::pixel_scale,
			7 * Renderer::pixel_scale,
			3 * Renderer::pixel_scale
		};

		helicopter->tBlit(dst, src, 0);
	}

	if (SantaBlinker.blinking)
	{
		rect src
		{
			117,
			0,
			9,
			2
		};

		rect dst
		{
			p.x + 40 * Renderer::pixel_scale,
			p.y + 47 * Renderer::pixel_scale,
			9 * Renderer::pixel_scale,
			2 * Renderer::pixel_scale
		};

		helicopter->tBlit(dst, src, 0);
	}

	if (MomorinBlinker.blinking)
	{
		rect src
		{
			119,
			5,
			7,
			2
		};

		rect dst
		{
			p.x + 54 * Renderer::pixel_scale,
			p.y + 40 * Renderer::pixel_scale,
			7 * Renderer::pixel_scale,
			2 * Renderer::pixel_scale
		};

		helicopter->tBlit(dst, src, 0);
	}
}

bool Helicopter::Update(double delta)
{
	x += speed * delta;

	propellerCycle += PROPELLOR_RATE * delta;
	xCycle += X_RATE * delta;
	yCycle += Y_RATE * delta;

	while (propellerCycle >= 1.0) {
		propellerCycle -= 1.0;
		propellerFrame++;
		propellerFrame %= 4;
	}

	while (xCycle >= 1.0) xCycle -= 1.0;
	while (yCycle >= 1.0) yCycle -= 1.0;

	ChakoBlinker.Update(delta);
	SantaBlinker.Update(delta);
	MomorinBlinker.Update(delta);

	return x - X_RADIUS * Renderer::pixel_scale < Renderer::resolution.x;
}
void Helicopter::Render()
{
	point p =
	{
		(int)round( x     + X_RADIUS * sin(xCycle * 2.0 * M_PI) * Renderer::pixel_scale),
		(int)round((250.0 + Y_RADIUS * sin(yCycle * 2.0 * M_PI)) * Renderer::pixel_scale)
	};

	DrawHelicopterBase(p);
	DrawPropeller1(p);
	DrawPropeller2(p);
	DrawBlinks(p);
}

// Visitor manager
VisitorManager::VisitorManager(HDC dc)
{
	visitors = new Visitor*[VISITOR_COUNT] {
		new Dragon(dc),
		new Balrog(dc),
		new Helicopter(dc)
	};

	appearanceCountdown = (rand() % 6) * 60.0; // Appears for the first time somewhere from 0-5 minutes
}

void VisitorManager::Update(double delta)
{
	if (currentVisitor == NULL)
	{
		if (appearanceCountdown > 0 && !Window::forceVisitor)
		{
			appearanceCountdown -= delta;
			return;
		}

		currentVisitor = visitors[rand() % VISITOR_COUNT];
		currentVisitor->Spawn();
		Window::forceVisitor = false;
	}
	else
	{
		if (currentVisitor->Update(delta)) return;

		currentVisitor->Despawn();
		currentVisitor = NULL;
		appearanceCountdown = ((rand() % 6) + 5) * 60; // Appears consequent times every 5-10 minutes
	}
}

void VisitorManager::Render()
{
	if (currentVisitor != NULL) currentVisitor->Render();
}