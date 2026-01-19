#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>

#include "SPIFFS.h"
#include "driver/i2s.h" 

#include "STT.h"
#include "TTT.h"
#include "TTS.h"
#include "OLED_Display.h"


#define START_BUTTON_PIN   15
//#define SELECT_BUTTON_PIN  13
#define DIRECTION_PIN      21


const char* ssid = "DESKTOP-SE00GAS 5587";
const char* password = "51215121";


String sourceLang="en";
String targetLang="hi";
String original_text;

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("\n\n========================================");
  Serial.println("ESP32 TRANSLATOR");
  Serial.println("========================================\n");

  //IO
  pinMode(START_BUTTON_PIN, INPUT_PULLUP);
  //pinMode(SELECT_BUTTON_PIN, INPUT_PULLUP);
  pinMode(DIRECTION_PIN, INPUT);

  //Oled
  oledInit();

  //SPIFFS
  Serial.println("Initializing SPIFFS...");
  if (!SPIFFS.begin(true)) {
    Serial.println("An Error occurred while mounting SPIFFS");
    Serial.println("SPIFFS FAILED! Halting.");
    while(1);
  }
  Serial.println("SPIFFS mounted successfully.");

  //WiFi
  Serial.print("Connecting to Wi-Fi: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi connected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  Serial.println("\n>>> I2S Drivers are idle. <<<");

  Serial.println("\n========= DEVICE READY =========");

}

void loop() {
  
  if(digitalRead(START_BUTTON_PIN)==LOW){

    Serial.println("\n[STEP 1] SPEECH TO TEXT");


    Serial.println("Record!");
    oledPrint("Recording started!",1,10);
    recordAudio();
    Serial.println("Recording Stopped!");
    oledPrint("Recording Stopped!",1,10);

    Serial.println("Sending to Deepgram!");
    oledPrint("Performing Speech to Text..",1,10);
    //oledPrint("Sending to Deepgram!",2,10);
    sendToDeepgram(original_text);
    Serial.println();
    Serial.println(original_text);

    oledPrint("Performing Text to Text..",1,10);
    String translated_text = translateText(original_text, sourceLang, targetLang);
    
    Serial.println();
    Serial.println();
    Serial.println();
    Serial.println(translated_text);

    Serial.println("\n[STEP 3] TEXT TO SPEECH");
    oledPrint("Performing Text To Speech..",1,10);
    playTranslatedAudio(translated_text, targetLang);

  }
  else{
    oledPrint("Push the Button to record!",1,10);
    if(digitalRead(DIRECTION_PIN)==1){
      sourceLang="hi";
      targetLang="en";
    }
    else{
      sourceLang="en";
      targetLang="hi";
    }
    oledPrintLanguage(sourceLang);
    delay(1000);
  }
  
  //delay(60000);

}
