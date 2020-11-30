#include "DrawingHelpers.h"

point GetScreenSize() {
	return {
		GetSystemMetrics(SM_CXSCREEN),
		GetSystemMetrics(SM_CYSCREEN)
	};
}

bitmap::bitmap(int bitmapCode, HDC dc) {
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

bitmap::~bitmap() {
	SelectObject(hdcMem, infoOld);
	DeleteDC(hdcMem);
	DeleteObject(image);
}

void bitmap::blit(rect dst, rect src) {
	StretchBlt(
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
void bitmap::rBblit(rect dst, rect src) {
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
void bitmap::tBlit(rect dst, rect src, int key) {
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