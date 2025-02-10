#include <iostream>
#include <thread>

#include "window.h"
#include "renderer.h"
#include "globals.h"

// ID definitions for embedded resources
#include "resource.h"

void ExitWithMessage(PCTSTR msg)
{
	MessageBox(NULL, msg, TEXT("Error"), MB_ICONERROR);

	ExitProcess(GetLastError());
}

const UINT WM_SPAWN_WORKERW = 0x052C;

// This function gets a specific window with a device context that draws between
// the desktop icons and the background. Method used with thanks to:
// https://www.codeproject.com/Articles/856020/Draw-Behind-Desktop-Icons-in-Windows-plus
HWND GetWorkerW()
{
	HWND progman_hwnd = FindWindow(TEXT("Progman"), NULL);

	if (progman_hwnd == 0) ExitWithMessage(TEXT("Critical error: Could not locate Progman"));

	HWND shellview_hwnd = FindWindowEx(progman_hwnd, NULL, TEXT("ShellDLL_DefView"), NULL);
	HWND tmp_workerW = NULL;

	while (shellview_hwnd == NULL && (tmp_workerW = FindWindowEx(NULL, tmp_workerW, TEXT("WorkerW"), NULL)) != NULL)
	{
		shellview_hwnd = FindWindowEx(tmp_workerW, NULL, TEXT("ShellDLL_DefView"), NULL); 
	}

	if (shellview_hwnd == NULL) ExitWithMessage(L"Critical error: Could not locate SHELLDLL_DEFView");

	HWND shellview_parent_hwnd = GetAncestor(shellview_hwnd, GA_PARENT);

	// This should never run
	if (shellview_parent_hwnd == GetDesktopWindow()) ExitWithMessage(L"Critical error: SHELLDLL_DEFView should not be in top level of hierarchy");

	if (SendMessageTimeout(
			progman_hwnd,
			WM_SPAWN_WORKERW,
			WPARAM(0xD),
			LPARAM(0x1),
			SMTO_NORMAL,
			1000,
			NULL
		) == 0) ExitWithMessage(L"Critical error: Could not communicate with Progman");

	HWND workerw_hwnd = FindWindowEx(
		NULL,
		shellview_parent_hwnd,
		TEXT("WorkerW"),
		NULL
	);

	if (workerw_hwnd != NULL) return workerw_hwnd;

	workerw_hwnd = FindWindowEx(
		shellview_parent_hwnd,
		shellview_hwnd,
		TEXT("WorkerW"),
		NULL
	);

	if (workerw_hwnd == NULL) ExitWithMessage(L"Critical error: Could not locate WorkerW");

	return workerw_hwnd;
}

void ShowContextMenu(HWND hwnd, point pt)
{
	HMENU hMenu = LoadMenu(hInstance, MAKEINTRESOURCE(IDC_CONTEXTMENU));
	if (hMenu == NULL) return;

	HMENU hSubMenu = GetSubMenu(hMenu, 0);
	if (hSubMenu == NULL)
	{
		DestroyMenu(hMenu);
		return;
	}

	TCHAR strMenuString[128];

	LoadString(
		hInstance,
		visitorsToggled ? IDS_TOGGLE_ENABLED : IDS_TOGGLE_DISABLED,
		strMenuString,
		ARRAYSIZE(strMenuString)
	);

	ModifyMenu(
		hSubMenu,
		ID__VISITORS,
		MF_BYCOMMAND | MF_STRING,
		ID__VISITORS,
		(LPCTSTR)strMenuString
	);

	// The window must be foreground before calling TrackPopupMenu or the menu will not disappear when the user clicks away
	SetForegroundWindow(hwnd);

	// Drop alignment
	const int uFlags = TPM_RIGHTBUTTON | (GetSystemMetrics(SM_MENUDROPALIGNMENT) ? TPM_RIGHTALIGN : TPM_LEFTALIGN);

	TrackPopupMenuEx(hSubMenu, uFlags, pt.x, pt.y, hwnd, NULL);

	DestroyMenu(hMenu);
}

// You can't give renderer.Renderloop directly to a thread
void SpawnRenderer(HDC windowDC)
{
	Renderer renderer(windowDC);

	renderer.RenderLoop();
}

LRESULT CALLBACK MainWinProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_CREATE:
		// Adds an icon to the tooltray
		if (!Window::AddTrayIcon(hwnd, IDS_TOOLTIP, trayIconUID))
		{
			PostQuitMessage(0);
			return -1;
		}
		break;

	case WM_MENUOPEN:
		// User opened the menu
		if (LOWORD(lParam) == WM_CONTEXTMENU)
			ShowContextMenu(hwnd, { LOWORD(wParam), HIWORD(wParam) });
		break;

	case WM_DESTROY:
		mainWindow->running = false;
		break;

	case WM_COMMAND:
		// User selected the quit button
		switch (LOWORD(wParam))
		{
		case ID__EXIT:
			mainWindow->running = false;
			break;
		case ID__VISITORS:
			visitorsToggled = !visitorsToggled;
			break;
		case ID__FORCEVISIT:
			visitorsToggled = true;
			forceVisitor = true;
			break;
		} // Continue otherwise...

	default:
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
	return 0;
};

static std::exception_ptr mainExceptionPtr = nullptr;

void SpawnMainWindow(HWND workerw_window)
{
	TCHAR class_name[128];
	TCHAR title_name[128];
	TCHAR menu_name[128];

	LoadString(hInstance, IDS_WALLPAPER_CLASS, class_name, ARRAYSIZE(class_name));
	LoadString(hInstance, IDS_TITLE, title_name, ARRAYSIZE(title_name));

	WindowClass wClass(menu_name, class_name, MainWinProc, CS_HREDRAW | CS_VREDRAW);

	RECT ww_shape;

	GetWindowRect(workerw_window, &ww_shape);

	try
	{
		mainWindow = new Window(
			wClass._class_atom,
			title_name,
			0,
			0,
			ww_shape.right - ww_shape.left,
			ww_shape.bottom - ww_shape.top,
			workerw_window,
			WS_POPUP | WS_DISABLED | WS_CHILD
		);

		SetParent(mainWindow->hwnd, workerw_window);
		ShowWindow(mainWindow->hwnd, SW_SHOW);
	}
	catch (...)
	{
		mainExceptionPtr = std::current_exception();
		return;
	}
	
	std::unique_lock<std::shared_mutex> lck(isRunning_mtx, std::defer_lock);
	
	lck.lock();
	isRunning = true;
	lck.unlock();
	isRunningChanged.notify_all();

	mainWindow->MessageLoop();

	lck.lock();
	isRunning = false;
	lck.unlock();
	isRunningChanged.notify_all();

	delete mainWindow;
	mainWindow = nullptr;
}

int WINAPI wWinMain(HINSTANCE _hInstance, HINSTANCE, PWSTR, int)
{
	hInstance = _hInstance;

	HWND workerW = GetWorkerW();

	std::thread mainWindowThread(SpawnMainWindow, workerW);

	{
		std::shared_lock<std::shared_mutex> lck(isRunning_mtx);
		isRunningChanged.wait(lck, [] { return isRunning; });
	}

	// WorkerW style: 0x58000000
	// 0x08000000: WS_DISABLED
	// 0x10000000: WS_VISIBLE
	// 0x40000000: WS_CHILD

	// Main window style: 0x9C000000
	// 0x04000000: WS_CLIPSIBLINGS
	// 0x08000000: WS_DISABLED
	// 0x10000000: WS_VISIBLE
	// 0x80000000: WS_POPUP

	HDC windowDC = GetDCEx(mainWindow->hwnd, NULL, DCX_LOCKWINDOWUPDATE | DCX_CACHE | DCX_WINDOW); // flags == 0x403

	std::thread renderThread(SpawnRenderer, windowDC);

	// Wait for renderer and window to stop running
	mainWindowThread.join();

	if (trayIconUID != ~0)
	{
		NOTIFYICONDATA nid;
		nid.uID = trayIconUID;
		Shell_NotifyIcon(NIM_DELETE, &nid);
		trayIconUID = ~0;
	}

	if (mainExceptionPtr)
	{
		try
		{
			std::rethrow_exception(mainExceptionPtr);
		}
		catch (std::exception& e)
		{
			LPSTR err_buffer;
			DWORD err_code = GetLastError();
			DWORD err_len = FormatMessageA(
				FORMAT_MESSAGE_ALLOCATE_BUFFER |
				FORMAT_MESSAGE_FROM_SYSTEM |
				FORMAT_MESSAGE_IGNORE_INSERTS,
				NULL,
				err_code,
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
				(LPSTR)&err_buffer,
				0, NULL);

			if (err_len == 0)
			{
				MessageBox(NULL, TEXT("FormatMessage failed"), TEXT("Error"), MB_OK);
				ExitProcess(err_code);
			}


			std::string err = e.what() + std::string("\r\nLast windows error: ") + std::string(err_buffer, err_len);
			LocalFree(err_buffer);

			std::cerr << "Menu creation class exited with exception: " << e.what() << std::endl;
			std::cerr << "Windows exception: " << err_buffer << std::endl;

			MessageBoxA(NULL, err.c_str(), "Error", MB_ICONERROR);
			ExitProcess(err_code);
		}
	}

	renderThread.join();

	return 0;
}