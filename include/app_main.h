#ifndef APP_MAIN_H
#define APP_MAIN_H

#include <stdint.h>
#include <window.h>

void refreshScr(WindowData window, uint* screenBuffer, bool force = false);
void requestClose(int errCode, const char* msg);

#endif
