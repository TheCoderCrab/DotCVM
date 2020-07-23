#ifndef WINDOW_H
#define WINDOW_H

#ifdef __linux__

#include <X11/Xlib.h>
#include <X11/Xutil.h>



struct XWindowData
{
    Display* display;
    Window   window;
    int screen;
};
#define XEventWaiting(d, t, e) XCheckTypedEvent(d, t, &e) != 0

#else
#error "Plateforms other than linux are not supported yet"
#endif

#ifdef __linux__
typedef XWindowData WindowData;
#endif

extern WindowData (*createWindow)     (const char* title, unsigned int width, unsigned int height);
extern WindowData (*setupMainWindow)  (const char* title, unsigned int width, unsigned int height);
extern void       (*updateMainWindow) ();
extern void       (*closeWindow)      (WindowData window);
extern void       (*closeDisplay)     ();
extern void       (*setPixel)         (WindowData window, uint x, uint y, uint rgba);
extern WindowData mainWin;
void refreshScr(WindowData window, uint* screenBuffer, bool force = false);
void requestClose(int errCode, const char* msg);

#endif