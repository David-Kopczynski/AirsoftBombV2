/* ---------- Display ---------- */
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2);


/* ---------- Functions ---------- */
void updateDisplay(String text, int line = 0);


/* ---------- Runtime ---------- */
String code;
unsigned long duration;

unsigned long startTime;
unsigned int charIndex = 1;


void setup() {

  // Initial
  Serial.begin(9600);

  // Turn on the blacklight and print a message.
  lcd.begin();
  updateDisplay("Defuse Code:");
  lcd.backlight();

  // Request communication
  Serial.write("Request code");
}

void loop() {

  // Get Data
  while (Serial.available() > 0) {

    String input = Serial.readString();

    // Get code
    if (code.length() == 0) {
      code = input;

      // Next request
      Serial.write("Request time");
    }
    else if (duration == 0) {
      duration = input.toInt();

      // Set start values
      startTime = millis();

      // Write placeholder
      for (int i = 0; i < code.length(); i++) {
        lcd.setCursor(i, 1);
        lcd.print("-");
      }
    }
  }

  // Display Code after time
  if (startTime && duration && charIndex <= code.length() && duration * charIndex / code.length() < millis() - startTime) {

    // Request write -- wait for response
    Serial.write("Request ping");
    while (!Serial.available());
    while (Serial.available()) {

      if (Serial.readString() == "Request ping") {

        // Display next
        lcd.setCursor(charIndex - 1, 1);
        lcd.print(code[charIndex - 1]);

        charIndex += 1;
      }
    }
  }
}

void updateDisplay(String text, int line = 0) {
  lcd.setCursor(0, line);

  while (text.length() < 16) text += " ";
  lcd.print(text);
}
