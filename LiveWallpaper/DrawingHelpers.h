#pragma once

#include<Windows.h>
#include<wingdi.h>
#include<string>

struct point {
	int x, y;
};

struct rect {
	int x, y, w, h;
};

point GetScreenSize();

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