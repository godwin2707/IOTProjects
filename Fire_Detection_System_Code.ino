#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Define ESP32 GPIOs
const int flameSensorPin = 13;
const int ledPin = 12;
const int buzzerPin = 14;
const int buttonPin = 27;

bool alarmEnabled = true;
bool lastButtonState = HIGH;

void setup() {
  pinMode(flameSensorPin, INPUT);
  pinMode(ledPin, OUTPUT);
  pinMode(buzzerPin, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP); 

  digitalWrite(ledPin, LOW);
  digitalWrite(buzzerPin, LOW);

  Serial.begin(115200);  

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("OLED failed"));
    while (true);
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Fire Detection");
  display.display();
  delay(2000);
}

void loop() {
  int flameDetected = digitalRead(flameSensorPin);
  int buttonState = digitalRead(buttonPin);

  Serial.print("Flame Sensor: ");
  Serial.println(flameDetected);
  Serial.print("Alarm Enabled: ");
  Serial.println(alarmEnabled);

  if (buttonState == LOW && lastButtonState == HIGH) {
    alarmEnabled = !alarmEnabled;
    delay(200); 
  }
  lastButtonState = buttonState;

  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Fire Detection");

  if (flameDetected == HIGH) {
    display.println("Status: FIRE!");
    if (alarmEnabled) {
      digitalWrite(ledPin, HIGH);
      digitalWrite(buzzerPin, HIGH);
    } else {
      digitalWrite(ledPin, LOW);
      digitalWrite(buzzerPin, LOW);
    }
  } else {
    display.println("Status: Safe");
    digitalWrite(ledPin, LOW);
    digitalWrite(buzzerPin, LOW);
  }

  display.display();
  delay(100);
}