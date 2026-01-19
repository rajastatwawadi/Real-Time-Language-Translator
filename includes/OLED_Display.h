#ifndef __OLED_H__
#define __OLED_H__

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

void oledPrint(String text, int size, int y);
void oledPrintLanguage(String sourceLang);
//void drawMainMenu();
void oledInit();

#endif 
