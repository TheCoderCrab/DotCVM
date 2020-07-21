#include <window.h>
#include <log.h>
#include <err_code.h>
#include <app_main.h>

WindowData createWindow(const char* title, unsigned int width, unsigned int height)
{
   Display* display = XOpenDisplay(NULL);
   if (display == NULL)
      requestClose(WIN_DISPLAY_ERROR, "Cannot open display");
   int screen = DefaultScreen(display);
   Window win = XCreateSimpleWindow(display, RootWindow(display, screen), 0, 0, width, height, 1, BlackPixel(display, screen), WhitePixel(display, screen));
   XSelectInput(display, win, ExposureMask
                            | KeyPressMask 
                            | KeyReleaseMask
                            | ButtonPressMask
                            | ButtonReleaseMask
                            | ButtonPressMask
                            | PointerMotionMask);
   XStoreName(display, win, title);
   XMapWindow(display, win);
   return {display, win, screen};
}
