#include <modules/user_device/window.hpp>
#include <dotcvm/utils/log.hpp>
#include <modules/user_device/user_device.hpp>

window_data main_win;

#if defined(__linux__) || defined(__FreeBSD__)

static Atom s_WMDeleteWindowAtom;

uint xscreen_width(window_data window)
{
   return WidthOfScreen(ScreenOfDisplay(window.display, window.screen));
}
uint xscreen_heigth(window_data window)
{
   return HeightOfScreen(ScreenOfDisplay(window.display, window.screen));
}

Display* get_xdisplay()
{
   static Display* display;
   if(display == nullptr)
   {
        display = XOpenDisplay(nullptr);
        if (display == nullptr)
            ERR_M("Cannot open display");
   }
   return display;
}

xwindow_data create_xwindow(std::string title, unsigned int width, unsigned int height)
{
   DEBUG_M("Creating an X window");
   Display* display = get_xdisplay();
   DEBUG_M("XDisplay created");
   int screen = DefaultScreen(display);
   DEBUG_M("Screen: " << screen);
   Window win = XCreateSimpleWindow(display, RootWindow(display, screen), 0, 0, width, height, 1, BlackPixel(display, screen), BlackPixel(display, screen));
   DEBUG_M("Window created");
   XSelectInput(display, win, KeyPressMask 
                            | KeyReleaseMask
                            | ButtonPressMask
                            | ButtonReleaseMask
                            | ButtonPressMask
                            | PointerMotionMask);
   XSizeHints wmSizeHints;
   wmSizeHints.flags = PMinSize | PMaxSize;
   wmSizeHints.max_width  = width;
   wmSizeHints.min_width  = width;
   wmSizeHints.max_height = height;
   wmSizeHints.min_height = height;
   XSetWMNormalHints(display, win, &wmSizeHints);
   DEBUG_M("Set Input Masks");
   XStoreName(display, win, title.c_str());
   DEBUG_M("Set Window name");
   XMapWindow(display, win);
   DEBUG_M("XWindow Mapped");
   XMoveWindow(display, win, (xscreen_width({display, win, screen}) - width) / 2,
                             (xscreen_heigth({display, win, screen}) - height) / 2);
   return {display, win, screen};
}

xwindow_data setup_xmain_window(std::string title, unsigned int width, unsigned int height)
{
   DEBUG_M("Using X");
   xwindow_data window = create_xwindow(title, width, height);
   DEBUG_M("Created main window");
   s_WMDeleteWindowAtom = XInternAtom(window.display, "WM_DELETE_WINDOW", False);
   XSetWMProtocols(window.display, window.window, &s_WMDeleteWindowAtom, 1);
   DEBUG_M("Added protocol WM_DELETE_WINDOW");
   return window;
}

void draw_xpixel(window_data w, uint x, uint y, uint rgba)
{
   XSetForeground(w.display, DefaultGC(w.display, w.screen), rgba);
   XDrawPoint(w.display, w.window, DefaultGC(w.display, w.screen), x, y);
}

void update_xmain_window()
{
   static XEvent event;
   if(xevent_waiting(main_win.display, KeyPress, event))
   {
      interrupt(interrupt_data{.interrupt_code = EVENT_KEYBOARD_PRESS,
                               .mode = interrupt_mode::REQUEST,
                               .data0 = event.xkey.keycode,
                               .data1 = 0 });
      return;
   }
   if(xevent_waiting(main_win.display, KeyRelease, event))
   {
      interrupt(interrupt_data{.interrupt_code = EVENT_KEYBOARD_RELEASE,
                               .mode = interrupt_mode::REQUEST,
                               .data0 = event.xkey.keycode,
                               .data1 = 0 });
      return;
   }
   if(xevent_waiting(main_win.display, ButtonPress, event))
   {
      interrupt(interrupt_data{.interrupt_code = EVENT_MOUSE_PRESS,
                               .mode = interrupt_mode::REQUEST,
                               .data0 = event.xbutton.button,
                               .data1 = 0 });
      return;
   }
   if(xevent_waiting(main_win.display, ButtonRelease, event))
   {
      interrupt(interrupt_data{.interrupt_code = EVENT_MOUSE_RELEASE,
                               .mode = interrupt_mode::REQUEST,
                               .data0 = event.xbutton.button,
                               .data1 = 0 });
      return;
   }
   if(xevent_waiting(main_win.display, MotionNotify, event))
   {
      interrupt(interrupt_data{.interrupt_code = EVENT_MOUSE_MOTION,
                               .mode = interrupt_mode::REQUEST,
                               .data0 = (uint) event.xmotion.x,
                               .data1 = (uint) event.xmotion.y });
      return;
   }
   if(xevent_waiting(main_win.display, ClientMessage, event) && event.xclient.data.l[0] == s_WMDeleteWindowAtom)
   {
      s_dc.fp_shutdown(0, "Window closed");
      return;
   }
}
void close_xwindow(window_data window)
{
   XUnmapWindow(window.display, window.window);
   XDestroyWindow(window.display, window.window);
}
void close_xdisplay()
{
   XCloseDisplay(get_xdisplay());
}
void set_xpixel(window_data window, uint x, uint y, uint rgba)
{
   XSetForeground(window.display, DefaultGC(window.display, window.screen), rgba);
   XDrawPoint(window.display, window.window, DefaultGC(window.display, window.screen), x, y);
}
#endif

#ifdef __linux__
window_data  (*create_window)          (std::string title, unsigned int width, unsigned int height)      = create_xwindow;
window_data  (*setup_main_window)      (std::string title, unsigned int width, unsigned int height)      = setup_xmain_window;
void         (*update_main_window)     ()                                                                = update_xmain_window;
void         (*draw_pixel)             (window_data w, uint x, uint y, uint rgba)                        = draw_xpixel;
void         (*close_window)           (window_data window)                                              = close_xwindow;
void         (*close_display)          ()                                                                = close_xdisplay;
#endif