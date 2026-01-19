#include "STT.h"

#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include "SPIFFS.h"

#include "driver/i2s.h" 

#include "OLED_Display.h"

#define I2S_PORT_NUM    I2S_NUM_0
#define I2S_MIC_BCLK    (12)
#define I2S_MIC_WS      (14)
#define I2S_MIC_SD      (5)

#define SAMPLE_RATE 16000
#define BITS_PER_SAMPLE 16
#define RECORD_TIME 3  // seconds
#define BUFFER_SIZE 512

#define START_BUTTON_PIN   15

const char* deepgramApiKey = "f46583d458c5f1687814fca8f49869f8955f4138";

const char* stt_wav_path = "/stt_recording.wav";
const int headerSize = 44;

void installMicDriver() {
  Serial.println("[Mic] Initializing I2S Port 0 for recording...");
  i2s_config_t i2s_config_mic = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate = 16000,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = I2S_COMM_FORMAT_STAND_I2S,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 8,
    .dma_buf_len = 64,
    .use_apll = false
  };
  i2s_driver_install(I2S_PORT_NUM, &i2s_config_mic, 0, NULL);
  i2s_pin_config_t pin_config_mic = {
    .bck_io_num = I2S_MIC_BCLK,
    .ws_io_num = I2S_MIC_WS,
    .data_out_num = I2S_PIN_NO_CHANGE,
    .data_in_num = I2S_MIC_SD
  };
  i2s_set_pin(I2S_PORT_NUM, &pin_config_mic);
  Serial.println("[Mic] I2S Port 0 initialized successfully.");
}

void uninstallI2SDriver_mic() {
  Serial.println("[I2S] De-initializing I2S Port 0...");
  i2s_driver_uninstall(I2S_PORT_NUM);
  delay(10); // Give time for the driver to release
}

void createWavHeader(byte* header, int wavDataSize) {
  header[0] = 'R'; header[1] = 'I'; header[2] = 'F'; header[3] = 'F';
  unsigned int fileSize = wavDataSize + headerSize - 8;
  header[4] = (byte)(fileSize & 0xFF);
  header[5] = (byte)((fileSize >> 8) & 0xFF);
  header[6] = (byte)((fileSize >> 16) & 0xFF);
  header[7] = (byte)((fileSize >> 24) & 0xFF);
  header[8] = 'W'; header[9] = 'A'; header[10] = 'V'; header[11] = 'E';
  header[12] = 'f'; header[13] = 'm'; header[14] = 't'; header[15] = ' ';
  header[16] = 16; header[17] = 0; header[18] = 0; header[19] = 0;
  header[20] = 1; header[21] = 0;
  header[22] = 1; header[23] = 0; // Mono
  header[24] = 0x80; header[25] = 0x3E; header[26] = 0x00; header[27] = 0x00; // 16000 Hz
  header[28] = 0x00; header[29] = 0x7D; header[30] = 0x00; header[31] = 0x00; // Byte rate (16000 * 2 * 1)
  header[32] = 2; header[33] = 0; // Block align
  header[34] = 16; header[35] = 0; // 16-bit
  header[36] = 'd'; header[37] = 'a'; header[38] = 't'; header[39] = 'a';
  header[40] = (byte)(wavDataSize & 0xFF);
  header[41] = (byte)((wavDataSize >> 8) & 0xFF);
  header[42] = (byte)((wavDataSize >> 16) & 0xFF);
  header[43] = (byte)((wavDataSize >> 24) & 0xFF);
}


void sendToDeepgram(String &originalText) {
  if (!SPIFFS.exists(stt_wav_path)) {
    Serial.println("No recording found. Record first!");
    return;
  }

  Serial.println("\n=== Sending to Deepgram ===");
  
  File file1 = SPIFFS.open(stt_wav_path, FILE_READ);
  if (!file1) {
    Serial.println("Failed to open file");
    return;
  }

  HTTPClient http;
  
  // Deepgram API endpoint
  String url = "https://api.deepgram.com/v1/listen?model=nova-2&language=hi&smart_format=true";
  
  http.begin(url);
  http.addHeader("Authorization", String("Token ") + deepgramApiKey);
  http.addHeader("Content-Type", "audio/wav");
  
  Serial.println("Uploading audio to Deepgram...");
  
  // Send the file
  int httpResponseCode = http.sendRequest("POST", &file1, file1.size());
  
  file1.close();
  Serial.println("Upload complete!");
  Serial.print("Waiting..");
  
  if (httpResponseCode > 0) {
    Serial.printf("Response code: %d\n", httpResponseCode);
    String response = http.getString();
    Serial.println("\n=== Deepgram Response ===");
    Serial.println(response);
    
    // Parse and display transcript
    int transcriptPos = response.indexOf("\"transcript\":");
    if (transcriptPos > 0) {
      int startQuote = response.indexOf("\"", transcriptPos + 13);
      int endQuote = response.indexOf("\"", startQuote + 1);
      String transcript = response.substring(startQuote + 1, endQuote);
      originalText=transcript;
      Serial.println("\n=== TRANSCRIPTION ===");
      Serial.println(transcript);
    }
  } 
  else {
    Serial.printf("Error: %s\n", http.errorToString(httpResponseCode).c_str());
  }
  
  http.end();
}

void recordAudio(){
  //--- 1. INSTALL MIC DRIVER ---
  pinMode(START_BUTTON_PIN, INPUT_PULLUP);
  installMicDriver();

  File wavFile = SPIFFS.open(stt_wav_path, FILE_WRITE);
  if (!wavFile) {
    Serial.println("Failed to open stt.wav on SPIFFS");
    uninstallI2SDriver_mic();// Clean up
    return;
  }
  byte header[headerSize];
  createWavHeader(header, 0);
  wavFile.write(header, headerSize);
  const int buffer_size = 512;
  int16_t sample_buffer[buffer_size];
  unsigned long totalDataSize = 0;
  Serial.println("\n>>> Listening... (Release button to stop) <<<");
  
  while (digitalRead(START_BUTTON_PIN) == LOW) { 
    size_t bytes_read = 0;
    i2s_read(I2S_PORT_NUM, &sample_buffer, buffer_size, &bytes_read, portMAX_DELAY);
    if (bytes_read > 0) {
      wavFile.write((byte*)sample_buffer, bytes_read);
      totalDataSize += bytes_read;
    }
  }
  Serial.println("...Recording finished.");
  Serial.println(">>> Processing... <<<");

  Serial.println("Fixing WAV header...");
  createWavHeader(header, totalDataSize);
  wavFile.seek(0);
  wavFile.write(header, headerSize);
  wavFile.close();
  Serial.println("WAV file saved to SPIFFS.");
  
  // --- 2. UNINSTALL MIC DRIVER ---
  uninstallI2SDriver_mic();

}