#pragma once

#include <Windows.h>
#include <condition_variable>
#include <shared_mutex>
#include "window.h"

extern HINSTANCE hInstance;
extern Window* mainWindow;
extern bool visitorsToggled;
extern bool forceVisitor;
extern bool isRunning;
extern UINT trayIconUID;
extern std::condition_variable_any isRunningChanged;
extern std::shared_mutex isRunning_mtx;