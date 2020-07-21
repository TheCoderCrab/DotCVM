#ifndef WINDOW_H
#define WINDOW_H

#include <X11/Xlib.h>

struct WindowData
{
    Display* display;
    Window   window;
    int screen;
};

WindowData createWindow(const char* title, unsigned int width, unsigned int height);

extern WindowData mainWin;

#define XEventWaiting(d, t, e) XCheckTypedEvent(d, t, &e) != 0

void refreshScr(WindowData window, uint* screenBuffer, bool force = false);
void requestClose(int errCode, const char* msg);

#endif