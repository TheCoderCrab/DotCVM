#include <DotCVM/gui/window.h>
#include <DotCVM/utils/err_code.h>
#include <DotCVM/devices/devices.h>
#include <DotCVM/utils/log.h>


#ifdef __linux__

static Atom s_WMDeleteWindowAtom;

uint XScreenWidth(WindowData window)
{
   return WidthOfScreen(ScreenOfDisplay(window.display, window.screen));
}
uint XScreenHeigth(WindowData window)
{
   return HeightOfScreen(ScreenOfDisplay(window.display, window.screen));
}

Display* getXDisplay()
{
   static Display* display;
   if(display == nullptr)
   {
      display = XOpenDisplay(NULL);
      if (display == NULL)
         requestClose(WIN_DISPLAY_ERROR, "Cannot open display");
   }
   return display;
}

XWindowData createXWindow(const char* title, unsigned int width, unsigned int height)
{
   debug("Creating an X window");
   Display* display = getXDisplay();
   debug("XDisplay created");
   int screen = DefaultScreen(display);
   debug("Screen: " << screen);
   Window win = XCreateSimpleWindow(display, RootWindow(display, screen), 0, 0, width, height, 1, BlackPixel(display, screen), WhitePixel(display, screen));
   debug("Window created");
   XSelectInput(display, win, ExposureMask
                            | KeyPressMask 
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
   debug("Set Input Masks");
   XStoreName(display, win, title);
   debug("Set Window name");
   XMapWindow(display, win);
   debug("XWindow Mapped");
   XMoveWindow(display, win, (XScreenWidth({display, win, screen}) - width) / 2, (XScreenHeigth({display, win, screen}) - height) / 2);
   return {display, win, screen};
}

XWindowData setupXMainWindow(const char* title, unsigned int width, unsigned int height)
{
   debug("Using X");
   XWindowData window = createXWindow(title, width, height);
   debug("Created main window");
   s_WMDeleteWindowAtom = XInternAtom(window.display, "WM_DELETE_WINDOW", False);
   XSetWMProtocols(window.display, window.window, &s_WMDeleteWindowAtom, 1);
   debug("Added protocol WM_DELETE_WINDOW");
   return window;
}

void updateXMainWindow()
{
   static XEvent event;
   if(XEventWaiting(mainWin.display, Expose, event))
      refreshScr({mainWin}, devices::cpu->screen().buffer(), true);
   if(XEventWaiting(mainWin.display, KeyPress, event))
   {
      devices::cpu->keyboard().press(devices::cpu, event.xkey.keycode);
      return;
   }
   if(XEventWaiting(mainWin.display, KeyRelease, event))
   {
      devices::cpu->keyboard().release(devices::cpu, event.xkey.keycode);
      return;
   }
   if(XEventWaiting(mainWin.display, ButtonPress, event))
   {
      devices::cpu->mouse().press(devices::cpu, event.xbutton.button);
      return;
   }
   if(XEventWaiting(mainWin.display, ButtonRelease, event))
   {
      devices::cpu->mouse().release(devices::cpu, event.xbutton.button);
      return;
   }
   if(XEventWaiting(mainWin.display, MotionNotify, event))
   {
      devices::cpu->mouse().move(devices::cpu, event.xmotion.x, event.xmotion.y);
      return;
   }
   if(XEventWaiting(mainWin.display, ClientMessage, event) && event.xclient.data.l[0] == s_WMDeleteWindowAtom)
   {
      requestClose(0, "Window closed");
      return;
   }
}
void closeXWindow(WindowData window)
{
   XUnmapWindow(window.display, window.window);
   XDestroyWindow(window.display, window.window);
}
void closeXDisplay()
{
   XCloseDisplay(getXDisplay());
}
void XSetPixel(WindowData window, uint x, uint y, uint rgba)
{
   XSetForeground(window.display, DefaultGC(window.display, window.screen), rgba);
   XDrawPoint(window.display, window.window, DefaultGC(window.display, window.screen), x, y);
}
#endif

#ifdef __linux__
WindowData  (*createWindow)   (const char* title, unsigned int width, unsigned int height)    = createXWindow;
WindowData  (*setupMainWindow)   (const char* title, unsigned int width, unsigned int height) = setupXMainWindow;
void        (*updateMainWindow)  ()                                               = updateXMainWindow;
void        (*closeWindow)       (WindowData window)                              = closeXWindow;
void        (*closeDisplay)      ()                                               = closeXDisplay;
void        (*setPixel)          (WindowData window, uint x, uint y, uint rgba)   = XSetPixel;
#endif

