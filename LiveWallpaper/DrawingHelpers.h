#pragma once

#include<Windows.h>
#include<wingdi.h>
#include<string>

struct point
{
	int x, y;

	constexpr point() noexcept : x(0), y(0) {}
	constexpr point(int x, int y) noexcept : x(x), y(y) {}
	constexpr point(const point& other) noexcept : x(other.x), y(other.y) {}
	constexpr point(point&& other) noexcept : x(other.x), y(other.y) {}

	constexpr point& operator=(const point& other) noexcept
	{
		x = other.x;
		y = other.y;
		return *this;
	}
	constexpr point& operator=(point&& other) noexcept
	{
		x = other.x;
		y = other.y;
		return *this;
	}

	constexpr point operator+(const point& other) const noexcept { return { x + other.x, y + other.y }; }
	constexpr point operator-(const point& other) const noexcept { return { x - other.x, y - other.y }; }
	constexpr point& operator+=(const point& other) noexcept
	{
		x += other.x;
		y += other.y;
		return *this;
	}
	constexpr point& operator-=(const point& other) noexcept
	{
		x -= other.x;
		y -= other.y;
		return *this; 
	}

	constexpr point operator*(int factor) const noexcept { return { x * factor, y * factor }; }
	constexpr point& operator*=(int factor) noexcept
	{
		x *= factor;
		y *= factor;
		return *this;
	}

	constexpr point operator/(int factor) const noexcept { return { x / factor, y / factor }; }
	constexpr point& operator/=(int factor) noexcept
	{
		x /= factor;
		y /= factor;
		return *this;
	}

	constexpr bool operator==(const point& other) const noexcept { return x == other.x && y == other.y; }
	constexpr bool operator!=(const point& other) const noexcept { return x != other.x || y != other.y; }

	// Treats x and y like a width and height
	constexpr int area() const noexcept { return x * y; }
	constexpr int perimeter() const noexcept { return 2 * (x + y); }
};

struct rect
{
	union
	{
		struct { int x, y, w, h; };
		struct { point pos, size; };
	};

	constexpr rect() noexcept : x(0), y(0), w(0), h(0) {}
	constexpr rect(const point& pos, const point& size) noexcept : x(pos.x), y(pos.y), w(size.x), h(size.y) {}
	constexpr rect(int x, int y, int w, int h) noexcept : x(x), y(y), w(w), h(h) {}
	constexpr rect(const rect& other) noexcept : x(other.x), y(other.y), w(other.w), h(other.h) {}
	constexpr rect(rect&& other) noexcept : x(other.x), y(other.y), w(other.w), h(other.h) {}

	constexpr rect& operator=(const rect& other) noexcept
	{
		x = other.x;
		y = other.y;
		w = other.w;
		h = other.h;
		return *this;
	}
	constexpr rect& operator=(rect&& other) noexcept
	{
		x = other.x;
		y = other.y;
		w = other.w;
		h = other.h;
		return *this;
	}

	constexpr rect operator+(const point& other) const { return { x + other.x, y + other.y, w, h }; }
	constexpr rect operator-(const point& other) const { return { x - other.x, y - other.y, w, h }; }
	constexpr rect& operator+=(const point& other)
	{
		x += other.x;
		y += other.y;
		return *this;
	}
	constexpr rect& operator-=(const point& other)
	{
		x -= other.x;
		y -= other.y;
		return *this;
	}

	constexpr rect operator+(const rect& other) const { return { x + other.x, y + other.y, w + other.w, h + other.h }; }
	constexpr rect operator-(const rect& other) const { return { x - other.x, y - other.y, w - other.w, h - other.h }; }
	constexpr rect& operator+=(const rect& other)
	{
		x += other.x;
		y += other.y;
		w += other.w;
		h += other.h;
		return *this;
	}
	constexpr rect& operator-=(const rect& other)
	{
		x -= other.x;
		y -= other.y;
		w += other.w;
		h += other.h;
		return *this;
	}

	constexpr rect operator*(int factor) const noexcept { return { x * factor, y * factor, w * factor, h * factor }; }
	constexpr rect& operator*=(int factor) noexcept
	{
		x *= factor;
		y *= factor;
		w *= factor;
		h *= factor;
		return *this;
	}

	constexpr rect operator/(int factor) const noexcept { return { x / factor, y / factor, w / factor, h / factor }; }
	constexpr rect& operator/=(int factor) noexcept
	{
		x /= factor;
		y /= factor;
		w /= factor;
		h /= factor;
		return *this;
	}

	constexpr bool operator==(const rect& other) const noexcept { return x == other.x && y == other.y && w == other.w && h == other.h; }
	constexpr bool operator!=(const rect& other) const noexcept { return x != other.x || y != other.y || w != other.w || h != other.h; }
};

point GetScreenSize();

rect GetScreenShape();

// Wrapper for a WinGDI bitmap, because it's awful to directly use win32
class bitmap {
public:
	HBITMAP image;
	BITMAP info;
	HDC hdcMem;
	HGDIOBJ infoOld;
	HDC target;

	bitmap(int bitmapCode, HDC dc);
	~bitmap();

	// Blit: blit with potential scaling (mapping rect to potentially different rect)
	void blit(rect dst, rect src);
	// Raw blit: a direct copy with no stretching (mapping rect to same rect)
	void rBblit(rect dst, rect src);
	// Transparent blit: blit with stretching, and transparency through colour keying
	void tBlit(rect dst, rect src, int key);
};