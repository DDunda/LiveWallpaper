#include <windows.h>
#include <math.h>
#include "resource.h"
#include <chrono>
#include <exception>
#include <string>

// Yes, I know you're yelling at me, but I can't hear you :)
using namespace std;
using namespace chrono;
typedef chrono::steady_clock SC;

// Since the callbacks for EnumWindows is stupid and doesnt allow environment caputuring
// I MUST make environment variables GLOBAL in order to be used in the callback.
// Don't bother trying ANYTHING because C hates you and your family.
HWND tmp;
// This function gets workerW, some magical window that lets you get a device context that
// allows for drawing between the desktop icons and the background. Don't ask me how this
// black magic works, ask this guy: https://www.codeproject.com/Articles/856020/Draw-Behind-Desktop-Icons-in-Windows-plus
HWND GetWorkerW() {
	HWND progman = FindWindow(L"Progman", NULL);

#ifdef _WIN64 
	ULONG_PTR result = NULL;
#else _WIN32
	DWORD result = NULL;
#endif
	SendMessageTimeout(
		progman,
		0x052C,
		NULL,
		NULL,
		SMTO_NORMAL,
		1000,
		&result
	);

	tmp = NULL;
	EnumWindows(
		[](HWND tophandle, LPARAM topparamhandle)->BOOL
		{
			HWND p = FindWindowEx(tophandle,
				NULL,
				L"SHELLDLL_DefView",
				NULL);

			if (p != NULL)
			{
				tmp = FindWindowEx(NULL,
					tophandle,
					L"WorkerW",
					NULL);
			}

			return true;
		},
		NULL
			);

	return tmp;
}

// This is the !!!magic!! window that allows background rendering
HWND workerW;
// A device context is kind of like an API for draw calls, I think
HDC workerDC;

// Current time in seconds (timestamp of end of previous frame / start of this frame)
double current;
// Current time in seconds (timestamp of end of previous previous frame / start of previous frame)
double previous;
// Length in time of previous frame, in seconds
double delta;

// Chrono timestamp, used for current time
SC::time_point currentTP;
// Chrono timestamp, used for previous time
SC::time_point previousTP;
// Chrono duration, used for delta time
SC::duration deltaD;
// Minimum delta time of a frame, to cap the framerate
SC::duration minDeltaD;

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
			string errorMessage = "Bitmap \"" + to_string(bitmapCode) + "\"could not be opened";
			throw exception(errorMessage.c_str());
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

	void blit(int x, int y) {
		BitBlt(
			target,
			x,             // X
			y,             // Y
			info.bmWidth,  // W
			info.bmHeight, // H
			hdcMem,        // SRC
			0,             // SRC X
			0,             // SRC Y
			SRCCOPY
		);
	}
};

void HandleTime() {
	previousTP = currentTP;
	currentTP = SC::now();
	deltaD = currentTP - previousTP;

	previous = current;
	current = chrono::duration_cast<chrono::milliseconds>(currentTP.time_since_epoch()).count() / 1000.0;
	delta = current - previous;

	// Limits framerate
	if (deltaD < minDeltaD) {
		Sleep(chrono::duration_cast<chrono::milliseconds>(minDeltaD - deltaD).count());
		deltaD = minDeltaD; // Pretend that exactly the delta has passed
		currentTP = previousTP + minDeltaD;

		current = chrono::duration_cast<chrono::milliseconds>(currentTP.time_since_epoch()).count() / 1000.0;
		delta = current - previous;
	}
}

// For some reason Windows thinks this is a reasonable entry point
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) {

	// We all start somewhere
	delta = current = previous = 0;

	// 60 60ths of a second adds to a second - 60fps
	minDeltaD = chrono::milliseconds(1000 / 60);
	currentTP = SC::now();

	// Do magic
	workerW = GetWorkerW();
	workerDC = GetDCEx(workerW, NULL, 0x403);

	// For sanity
	bitmap* ball = new bitmap(IDB_BALL, workerDC);

	// Do rendering
	while (1) {
		ball->blit(
			(int)floor(cos(current * 2.0 * 3.1415) * 50 + 50), // X
			(int)floor(sin(current * 2.0 * 3.1415) * 50 + 50)  // Y
		);

		HandleTime();
	}

	delete ball;
	return 0;
}

// Did you know that comments free you of your programming sins?