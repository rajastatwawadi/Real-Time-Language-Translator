#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "OLED_Display.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);


void oledPrint(String text, int size = 2, int y = 10) {
  display.clearDisplay();
  display.setTextSize(size);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, y);
  display.println(text);
  display.display();
}


// void drawMainMenu() {
//   display.clearDisplay();
//   display.setTextSize(1);
//   display.setTextColor(SSD1306_WHITE);
//   display.setCursor(0, 0);

//   LanguagePair current = languages[currentLangPair];

//   if (direction_1_to_2) {
//     display.print(current.name1);
//     display.print(" -> ");
//     display.println(current.name2);
//   } else {
//     display.print(current.name2);
//     display.print(" -> ");
//     display.println(current.name1);
//   }

//   display.drawFastHLine(0, 10, display.width(), SSD1306_WHITE);

//   display.setTextSize(2);
//   display.setCursor(15, 25);
//   display.println("Hold to");
//   display.setCursor(20, 45);
//   display.println("Speak");
//   display.display();
// }


void oledInit(){
  
  Serial.println("Initializing OLED...");

  Wire.begin(22,23); 

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println(F("SSD1306 allocation failed"));
    while(1);
  }

  oledPrint("Booting...", 2, 30);
  delay(1000);
  
}

void oledPrintLanguage(String sourceLang){
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 44);
  if(sourceLang=="en"){
    display.println("English --> Hindi");
  }
  else{
    display.println("Hindi --> English");
  }
  display.display();
}
