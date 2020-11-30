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
};

class Dragon : public Visitor {
public:
	// The dragon has two frames for its flapping animation - Why not use a bool?
	bool frame;

	// Whether the dragon is holding Sue
	bool holdingSue;

	// Cycles are from 0-1, counting upwards, and trigger some behaviour when they hit 1
	double xCyle;
	double yCyle;
	double flapCycle;

	// How fast the cycles are "filling"
	double xFreq = 3.0 / 7.0;
	double yFreq = 4.0 / 15.0;
	double flapFreq = 8.0;

	// Displacement across screen
	double x = 0;
	// Speed across screen
	double speed;

	// The frames to draw
	bitmap* dragon;

	// The dragon floats randomly around the "true" position. It is offset by
	// two sine functions, of which progress at these two rates (can be thought of as frequencies)
	double xVariance;
	double yVariance;

	Dragon(HDC dc);

	void Spawn();

	void Despawn();

	bool Update(double delta);

	void DrawDragon(point p);

	void DrawSue(point p);

	void Render();
};

class Ballrog : public Visitor {
public:
	// Ballrog also has two frames, coincidentally enough
	bool frame;

	double xCyle;
	double yCyle;
	double flapCycle;

	double xFreq = 3.0 / 7.0;
	double yFreq = 4.0 / 15.0;
	double flapFreq = 12.0;

	// Displacement across screen
	double x = 0;
	// Speed across screen
	double speed = 90;

	// The frames to draw
	bitmap* ballrog;

	double xVariance = 6.5 * 3.0;
	double yVariance = 16 * 3.0;

	Ballrog(HDC dc);

	void Spawn();

	void Despawn();

	bool Update(double delta);

	void Render();
};

class Helicopter : public Visitor {
public:

	int propellerFrame;

	double xCyle;
	double yCyle;
	double propellerCycle;

	double xFreq = 3.0 / 7.0;
	double yFreq = 4.0 / 15.0;
	double propellerFreq = 60.0;

	// Displacement across screen
	double x = 0;
	// Speed across screen
	double speed;

	// The frames to draw
	bitmap* helicopter;

	double xVariance = 6.5 * 3.0 / 4.0;
	double yVariance = 16 * 3.0 / 4.0;

	Blinker ChakoBlinker;
	Blinker SantaBlinker;
	Blinker MomorinBlinker;

	Helicopter(HDC dc);

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
	int visitorCount;
	Visitor* currentVisitor = NULL;

	VisitorManager(HDC dc);

	void Update(double delta);
	void Render();
};