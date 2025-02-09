#include <thread>

#include "window.h"
#include "renderer.h"

// ID definitions for embedded resources
#include "resource.h"

// This function gets a specific window with a device context that draws between
// the desktop icons and the background. Method used with thanks to:
// https://www.codeproject.com/Articles/856020/Draw-Behind-Desktop-Icons-in-Windows-plus
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

	HWND tmp = NULL;

	EnumWindows
	(
		[](HWND tophandle, LPARAM tmp)->BOOL
		{
			HWND p = FindWindowEx(tophandle,
				NULL,
				L"SHELLDLL_DefView",
				NULL);

			if (p != NULL)
			{
				*((HWND*)tmp) = FindWindowEx(NULL,
					tophandle,
					L"WorkerW",
					NULL);
			}

			return true;
		},
		(LPARAM)(&tmp)
	);

	return tmp;
}

// You can't give renderer.Renderloop directly to a thread
void SpawnRenderer()
{
	HWND workerW = GetWorkerW(); 
	HDC workerDC = GetDCEx(workerW, NULL, DCX_LOCKWINDOWUPDATE | DCX_CACHE | DCX_WINDOW); // flags == 0x403

	Renderer renderer(workerDC);

	renderer.RenderLoop();
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR, int)
{
	// Initialises stuff for a static window class, an object isn't actually created.
	Window::MakeWindow(hInstance);

	// Does drawing
	std::thread renderThread(SpawnRenderer);
	Window::MessageLoop(); // Do not put this in a thread, ever.

	// Close the renderer
	renderThread.join();

	return 0;
}