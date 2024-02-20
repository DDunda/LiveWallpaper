#include "DrawingHelpers.h"
#include <stdexcept>

point GetScreenSize()
{
	return
	{
		GetSystemMetricsForDpi(SM_CXVIRTUALSCREEN, 96),
		GetSystemMetricsForDpi(SM_CYVIRTUALSCREEN, 96)
	};
}

bitmap::bitmap(int bitmapCode, HDC dc)
{
	target = dc;
	image = LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(bitmapCode));

	if (image == NULL) throw std::runtime_error("Bitmap \"" + std::to_string(bitmapCode) + "\"could not be opened");

	hdcMem = CreateCompatibleDC(target);
	infoOld = SelectObject(hdcMem, image);
	GetObject(image, sizeof(info), &info);
}

bitmap::~bitmap()
{
	SelectObject(hdcMem, infoOld);
	DeleteDC(hdcMem);
	DeleteObject(image);
}

void bitmap::blit(rect dst, rect src)
{
	StretchBlt
	(
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
		SRCCOPY
	);
}

void bitmap::rBblit(rect dst, rect src)
{
	BitBlt
	(
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

void bitmap::tBlit(rect dst, rect src, int key)
{
	TransparentBlt
	(
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