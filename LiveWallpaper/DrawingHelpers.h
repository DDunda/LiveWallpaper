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

// Wrapper for a WinGDI bitmap, because it's awful to directly use win32
class bitmap {
public:
	HBITMAP image;
	BITMAP info;
	HDC hdcMem;
	HGDIOBJ infoOld;
	HDC target;

	bitmap(int bitmapCode, HDC dc) {
		target = dc;
		image = LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(bitmapCode));
		if (image == NULL) {
			std::string errorMessage = "Bitmap \"" + std::to_string(bitmapCode) + "\"could not be opened";
			throw std::exception(errorMessage.c_str());
		}
		hdcMem = CreateCompatibleDC(target);
		infoOld = SelectObject(hdcMem, image);
		GetObject(image, sizeof(info), &info);
	}

	~bitmap() {
		SelectObject(hdcMem, infoOld);
		DeleteDC(hdcMem);
		DeleteObject(image);
	}

	void blit(rect dst, point src) {
		BitBlt(
			target,
			dst.x,
			dst.y,
			dst.w,
			dst.h,

			hdcMem,
			src.x,
			src.y,
			SRCCOPY
		);
	}
	void tBlit(rect dst, rect src, int key) {
		TransparentBlt(
			target,
			dst.x,
			dst.y,
			dst.w,
			dst.h,

			hdcMem,
			src.x,
			src.y,
			src.w,
			src.h,

			key
		);
	}
};