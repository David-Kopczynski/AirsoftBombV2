/* ---------- Display ---------- */
#include <Wire.h>
#include "src/Arduino-LiquidCrystal-I2C-library-master/LiquidCrystal_I2C.h"
LiquidCrystal_I2C lcd(0x27, 16, 2);


/* ---------- Functions ---------- */
void updateDisplay(String text, uint8_t line = 0);
uint32_t readSerialLong();
void checkConnection();
void reboot();


/* ---------- Runtime ---------- */
String code;
uint32_t duration;

uint32_t startTime;
uint8_t charIndex = 1;

// Serial communication
uint8_t getCode = 1;
uint8_t getTime = 2;
uint8_t ping = 3;

uint8_t serialConnected = 2;
bool restartOnConnected = false;
bool currentConnection = false;


void setup() {

  // Initial
  Serial.begin(2000000);

  // Serial communication active 
  pinMode(serialConnected, INPUT_PULLUP);
  
  // Turn on the blacklight and print a message.
  lcd.begin();
  updateDisplay("Defuse Code:");
  lcd.backlight();

  // Restart code if connection status changes
  attachInterrupt(digitalPinToInterrupt(serialConnected), checkConnection, CHANGE);
  checkConnection();
}

void loop() {

  // Send starting data -- Request communication
  if (code.length() == 0 && currentConnection) {
    
    while (!Serial.available()) {
      Serial.write(getCode);  
      delay(1);
    }
  }

  // Get Data
  while (Serial.available()) {

    // Get code
    if (code.length() == 0) {
      String input = Serial.readString();

      // Get code length by checkup value at the beginning
      code = input.substring(1, input[0] + 1);

      // Next request
      Serial.write(getTime);
    }
    else if (duration == 0) {
      duration = readSerialLong();

      // Set start values
      startTime = millis();

      // Write placeholder
      for (uint8_t i = 0; i < code.length(); i++) {
        lcd.setCursor(i, 1);
        lcd.print("-");
      }
    }
    // Flush data
    else Serial.read();
  }

  // Display Code after time   
  if (startTime > 0 && charIndex <= code.length() && (float)(duration * charIndex / code.length()) < millis() - startTime) {

    // Request write -- wait for response OR rebbot if disconnected
    while (Serial.read() != ping && code.length() != 0) {
      Serial.write(ping);
      delay(1);
    }
    
    if (code.length() != 0) {
      // Display next
      lcd.setCursor(charIndex - 1, 1);
      lcd.print(code[charIndex - 1]);

      charIndex += 1;   
    }
  }

  // Stop arduino
  else if (startTime > 0 && charIndex > code.length()) exit(0);
}

void updateDisplay(String text, uint8_t line = 0) {
  lcd.setCursor(0, line);

  while (text.length() < 16) text += " ";
  lcd.print(text);
}

uint32_t readSerialLong() {
  
  byte input[4];
  uint8_t index = 0;
  
  while (Serial.available() && index < 4) {
    input[index] = Serial.read();
    index++;
  }

  return input[0] + (input[1] << 8) + (input[2] << 16) + (input[3] << 24);
}

void checkConnection() {
  // Reverse signal as 1 is disconnected and 0 connected
  currentConnection = !digitalRead(serialConnected);

  if (restartOnConnected && currentConnection) reboot();
  else if (!currentConnection) restartOnConnected = true;
}

void reboot() {

  // Resets data and starts again
  code = "";
  duration = 0;
  startTime = 0;
  charIndex = 1;
  restartOnConnected = false;
  currentConnection = false;
}
