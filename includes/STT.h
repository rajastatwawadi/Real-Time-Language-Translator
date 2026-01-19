#ifndef __STT_H__
#define __STT_H__

#include "STT.h"

#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include "SPIFFS.h"

#include "driver/i2s.h" 

#include "OLED_Display.h"

extern const char* stt_wav_path;

void installMicDriver();
void uninstallI2SDriver_mic();
void createWavHeader(byte* header, int wavDataSize);
void recordAudio();
void sendToDeepgram(String &originalText);

#endif 
