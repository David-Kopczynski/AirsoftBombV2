/* ---------- Display ---------- */
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
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
  pinMode(serialConnected, INPUT);
  
  // Turn on the blacklight and print a message.
  lcd.begin();
  updateDisplay("Defuse Code:");
  lcd.backlight();
}

void loop() {

  // Check connection status
  checkConnection();

  // Send starting data -- Request communication
  if (code.length() == 0 && currentConnection) {
    
    while (!Serial.available()) {
      Serial.write(getCode);  
      delay(1);
    }
  }

  // Get Data
  if (Serial.available()) {

    // Get code
    if (code.length() == 0) {
      code = Serial.readString();

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
  }

  // Display Code after time   
  if (startTime > 0 && charIndex <= code.length() && (float)(duration * charIndex / code.length()) < millis() - startTime) {

    // Request write -- wait for response OR rebbot if disconnected
    Serial.write(ping);
    while (!Serial.available() && code.length() != 0) checkConnection();
    
    if (code.length() != 0 && Serial.read() == ping) {
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
  currentConnection = digitalRead(serialConnected);

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
