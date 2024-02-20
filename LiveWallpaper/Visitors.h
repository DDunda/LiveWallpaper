#pragma once

#include "DrawingHelpers.h"

class Blinker {
public:
	double cycle;
	bool blinking;

	Blinker();

	void Update(double delta);
};

class Visitor {
public:
	virtual void Spawn() = 0;
	virtual void Despawn() = 0;
	// Return whether onscreen
	virtual bool Update(double delta) = 0;
	virtual void Render() = 0;

	virtual ~Visitor() {}
};

class Dragon : public Visitor {
public:
	static constexpr bool FLAP_UP = false;
	static constexpr bool FLAP_DOWN = true;

	// The speed of each cycle. The X/Y values are coprime, giving a long cycle
	static constexpr double X_RATE    = 45.0 / 105.0;
	static constexpr double Y_RATE    = 28.0 / 105.0;
	static constexpr double FLAP_RATE = 8.0;

	// The dragon floats randomly around the "true" position. It is offset by
	// two sine functions with different frequencies
	static constexpr double X_RADIUS =  6.5;
	static constexpr double Y_RADIUS = 16.0;

	bool frame = FLAP_UP;

	// Whether the dragon is holding Sue
	bool holdingSue = false;

	// Cycles are from 0-1, counting upwards, and trigger some behaviour when they hit 1
	double xCycle = 0.0;
	double yCycle = 0.0;
	double flapCycle = 0.0;

	// Displacement across screen
	double x = 0.0;
	// Speed across screen
	double speed = 0.0;

	// The frames to draw
	bitmap* dragon;

	Dragon(HDC dc);
	~Dragon();

	void Spawn();

	void Despawn();

	bool Update(double delta);

	void DrawDragon(point p);

	void DrawSue(point p);

	void Render();
};

class Balrog : public Visitor {
public:
	static constexpr bool FLAP_UP = false;
	static constexpr bool FLAP_DOWN = true;

	static constexpr double X_RATE = 45.0 / 105.0;
	static constexpr double Y_RATE = 28.0 / 105.0;
	static constexpr double FLAP_RATE = 12.0;

	static constexpr double X_RADIUS =  6.5;
	static constexpr double Y_RADIUS = 16.0;

	bool frame = FLAP_UP;

	double xCycle = 0.0;
	double yCycle = 0.0;
	double flapCycle = 0.0;

	// Displacement across screen
	double x = 0;
	// Speed across screen
	double speed = 90;

	// The frames to draw
	bitmap* balrog;

	Balrog(HDC dc);
	~Balrog();

	void Spawn();

	void Despawn();

	bool Update(double delta);

	void Render();
};

class Helicopter : public Visitor {
public:
	// The speed of each cycle. The X/Y values are coprime, giving a long cycle
	static constexpr double X_RATE = 45.0 / 105.0;
	static constexpr double Y_RATE = 28.0 / 105.0;
	static constexpr double PROPELLOR_RATE = 60.0;

	static constexpr double X_RADIUS = 6.5 * 3.0 / 4.0;
	static constexpr double Y_RADIUS = 16 * 3.0 / 4.0;

	int propellerFrame;

	double xCycle;
	double yCycle;
	double propellerCycle;

	// Displacement across screen
	double x = 0;
	// Speed across screen
	double speed;

	// The frames to draw
	bitmap* helicopter;

	Blinker ChakoBlinker;
	Blinker SantaBlinker;
	Blinker MomorinBlinker;

	Helicopter(HDC dc);
	~Helicopter();

	void Spawn();
	void Despawn();

	bool Update(double delta);

	void DrawHelicopterBase(point p);
	void DrawPropeller1(point p);
	void DrawPropeller2(point p);
	void DrawBlinks(point p);

	void Render();
};

class VisitorManager {
public:
	// Time before a visitor appears (appears when <= 0)
	double appearanceCountdown;

	Visitor** visitors;
	static constexpr size_t VISITOR_COUNT = 3;
	Visitor* currentVisitor = NULL;

	VisitorManager(HDC dc);

	void Update(double delta);
	void Render();
};