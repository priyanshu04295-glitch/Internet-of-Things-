#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

const int irPin1 = 2;
const int irPin2 = 3;

int in_count = 0;
int out_count = 0;
int current_count = 0;

const unsigned long timeout = 300; // increased timeout

void setup() {
  lcd.init();
  lcd.backlight();

  lcd.setCursor(0, 0);
  lcd.print("IN: 0   OUT: 0");
  lcd.setCursor(0, 1);
  lcd.print("Current: 0");

  pinMode(irPin1, INPUT_PULLUP);
  pinMode(irPin2, INPUT_PULLUP);

  Serial.begin(9600); // for debugging
}

void loop() {

  // ENTRY (IR1 → IR2)
  if (digitalRead(irPin1) == LOW) {
    Serial.println("IR1 Triggered");

    unsigned long startTime = millis();

    while (millis() - startTime < timeout) {
      if (digitalRead(irPin2) == LOW) {
        Serial.println("Entry Detected");

        in_count++;
        updateDisplay();
        delay(200); // debounce
        break;
      }
    }

    waitForSensors();
  }

  // EXIT (IR2 → IR1)
  else if (digitalRead(irPin2) == LOW) {
    Serial.println("IR2 Triggered");

    unsigned long startTime = millis();

    while (millis() - startTime < timeout) {
      if (digitalRead(irPin1) == LOW) {
        Serial.println("Exit Detected");

        if (out_count < in_count) {
          out_count++;
          updateDisplay();
        }
        delay(200); // debounce
        break;
      }
    }

    waitForSensors();
  }
}

// safer wait (prevents freeze)
void waitForSensors() {
  unsigned long waitStart = millis();
  while ((digitalRead(irPin1) == LOW || digitalRead(irPin2) == LOW) &&
         millis() - waitStart < 1000);
}

void updateDisplay() {
  lcd.setCursor(4, 0);
  lcd.print("   ");
  lcd.setCursor(4, 0);
  lcd.print(in_count);

  lcd.setCursor(13, 0);
  lcd.print("   ");
  lcd.setCursor(13, 0);
  lcd.print(out_count);

  current_count = in_count - out_count;

  lcd.setCursor(9, 1);
  lcd.print("   ");
  lcd.setCursor(9, 1);
  lcd.print(current_count);
}