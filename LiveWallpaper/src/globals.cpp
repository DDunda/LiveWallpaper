#include "globals.h"

HINSTANCE hInstance = NULL;
Window* mainWindow = NULL;
bool visitorsToggled = true;
bool forceVisitor = false;
bool isRunning = false;
UINT trayIconUID = ~0;
std::condition_variable_any isRunningChanged;
std::shared_mutex isRunning_mtx;