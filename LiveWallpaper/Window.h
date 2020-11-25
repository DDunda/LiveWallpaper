#pragma once

#include<Windows.h>
#include"resource.h"
#include "DrawingHelpers.h"

#define WM_MENUOPEN (WM_APP + 1)

// Code relevant to rendering in the system tray and rendering the exit popup is taken from here:
// https://docs.microsoft.com/en-us/windows/win32/shell/samples-notificationicon#downloading-the-sample
// I really have no clue what it does but it works, so nice!
class Window {
public:
	static HINSTANCE hInst;
	static HWND hwnd;
	// What's the difference??
	static const WCHAR* className;
	static WCHAR* titleName;
	static bool running;

	// Creates the window
	static void MakeWindow(HINSTANCE instance);

	// I dont know what a window class is or why it's important but this makes one
	static void RegisterWindowClass(PCWSTR className, PCWSTR menuName, WNDPROC windowProcedure);

	// A windows procedure callback, this processes messages sent to windows
	static LRESULT CALLBACK WindowProc(HWND hwnd, UINT messageInt, WPARAM wParam, LPARAM lParam);

	// Recieves messages and handballs them to WindowProc
	static void MessageLoop();

	// Creates an icon in the system tray
	static BOOL AddTrayIcon(HWND hwnd);

	// Creates a menu
	static void ShowContextMenu(HWND hwnd, point pt);
};