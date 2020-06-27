#ifndef APP_MAIN_H
#define APP_MAIN_H

#include <stdint.h>

void setPixel(int32_t x, int32_t y, int32_t color);
int getPixel(int32_t x, int32_t y);
void refreshScr();
void requestClose();

#endif
