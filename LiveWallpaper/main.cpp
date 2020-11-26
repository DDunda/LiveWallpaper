// Preface: Comments free you of your programming sins.

#include <thread>

// Pro tip: objects fix disgusting code
#include "Window.h"
#include "Renderer.h"

// All the pictures and stuff
#include "resource.h"

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

// You can't give renderer.Renderloop directly to a thread
void SpawnRenderer() {
	// This is the !!!magic!! window that allows background rendering
	HWND workerW = GetWorkerW();
	HDC workerDC = GetDCEx(workerW, NULL, 0x403);

	Renderer renderer(workerDC);

	renderer.RenderLoop();
}

// For some reason Windows thinks this is a reasonable entry point
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR, int) {

	// Initialises stuff for a static window class, an object isn't actually created.
	Window::MakeWindow(hInstance);

	// Does drawing
	std::thread renderThread(SpawnRenderer);
	Window::MessageLoop(); // Do not put this in a thread, ever.

	// Close the renderer
	renderThread.join();

	return 0;
}