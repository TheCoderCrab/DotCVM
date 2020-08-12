#ifndef WINDOW_H
#define WINDOW_H

#include <dotcvm/dotcvm.hpp>
#include <modules/interrupt_bus.hpp>
#include <string>

#if defined(__linux__) || defined(__FreeBSD__)

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#define EVENT_MOUSE_MOTION      MotionNotify
#define EVENT_MOUSE_PRESS       ButtonPress
#define EVENT_MOUSE_RELEASE     ButtonRelease
#define EVENT_KEYBOARD_PRESS    KeyPress
#define EVENT_KEYBOARD_RELEASE  KeyRelease


struct xwindow_data
{
    Display* display;
    Window   window;
    int screen;
};

typedef xwindow_data window_data;

#define xevent_waiting(d, t, e) (XCheckTypedEvent(d, t, &e) != 0)

#else
#error "Plateforms other than unix are not supported yet"
#endif

extern window_data (*create_window)     (std::string title, unsigned int width, unsigned int height);
extern window_data (*setup_main_window) (std::string title, unsigned int width, unsigned int height);
extern void       (*update_main_window) ();
extern void       (*close_window)       (window_data window);
extern void       (*close_display)      ();
extern void       (*draw_pixel)         (window_data w, uint x, uint y, uint rgba);
extern window_data main_win;

#endif