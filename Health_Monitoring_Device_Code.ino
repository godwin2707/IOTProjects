#include <Wire.h>
#include <U8g2lib.h>

// Hardware Pins
#define BUZZER_PIN 25
#define JOYSTICK_BUTTON 32

// OLED Display Settings
U8G2_SSD1306_128X64_NONAME_F_HW_I2C display(U8G2_R0, /* reset=*/ U8X8_PIN_NONE, 22, 21);

// Mode Settings
bool modeSelected = false;
bool pomodoroActive = false;
bool readingActive = false;
int selectedMode = 0;  // 0 = Pomodoro, 1 = Reading
int pomodoroCount = 0; // Counts completed Pomodoro sessions

// Timer Settings
unsigned long startTime = 0;
bool running = false;
bool workSession = true;  // true = Work, false = Break
const unsigned long WORK_DURATION = 25 * 60 * 1000;  // 25 minutes
const unsigned long BREAK_DURATION = 5 * 60 * 1000;  // 5 minutes
const unsigned long READING_DURATION = 30 * 60 * 1000; // 30 minutes
bool buttonPressed = false;

void setup() {
    Serial.begin(115200);
    Wire.begin(21, 22);
    display.begin();

    pinMode(BUZZER_PIN, OUTPUT);
    pinMode(JOYSTICK_BUTTON, INPUT_PULLUP);

    showModeSelection();
}

void loop() {
    handleJoystickInput();
    if (modeSelected) {
        checkTimer();
        updateDisplay();
    }
}

void handleJoystickInput() {
    static unsigned long lastPressTime = 0;
    static bool firstPressDetected = false;
    int buttonState = digitalRead(JOYSTICK_BUTTON);

    if (buttonState == LOW && !buttonPressed) {
        buttonPressed = true;
        unsigned long currentTime = millis();

        if (firstPressDetected && (currentTime - lastPressTime < 500)) {
            // Double press detected → Reset & go to mode selection
            running = false;
            modeSelected = false;
            pomodoroActive = false;
            readingActive = false;
            firstPressDetected = false;
            showModeSelection();
            return;
        }

        firstPressDetected = true;
        lastPressTime = currentTime;

        if (!modeSelected) {
            // Toggle Mode Selection
            selectedMode = !selectedMode;
            showModeSelection();
        } else if (!running) {
            // Start session if not running
            startTime = millis();
            running = true;
        } else {
            running = false;
        }
    }

    if (buttonState == HIGH) {
        buttonPressed = false;
    }
}

void checkTimer() {
    if (running) {
        unsigned long elapsed = millis() - startTime;
        unsigned long totalDuration;

        if (pomodoroActive) {
            totalDuration = workSession ? WORK_DURATION : BREAK_DURATION;
        } else {
            totalDuration = READING_DURATION;
        }

        if (elapsed >= totalDuration) {
            if (pomodoroActive) {
                workSession = !workSession;
                if (!workSession) pomodoroCount++; // Increment Pomodoro count after each work session
                startTime = millis();

                digitalWrite(BUZZER_PIN, HIGH);
                delay(3000);
                digitalWrite(BUZZER_PIN, LOW);

                showMessage(workSession ? "Work Start" : "Break Start", 1000);
            } else {
                // Reading mode finishes → Go back to mode selection
                digitalWrite(BUZZER_PIN, HIGH);
                delay(3000);
                digitalWrite(BUZZER_PIN, LOW);
                running = false;
                modeSelected = false;
                showModeSelection();
            }
        }
    }
}

void updateDisplay() {
    display.clearBuffer();
    display.setFont(u8g2_font_6x13_tf);

    if (running) {
        unsigned long elapsed = millis() - startTime;
        unsigned long totalDuration = pomodoroActive
                                      ? (workSession ? WORK_DURATION : BREAK_DURATION)
                                      : READING_DURATION;
        unsigned long remaining = totalDuration - elapsed;

        int minutes = (remaining / 1000) / 60;
        int seconds = (remaining / 1000) % 60;

        display.setCursor(30, 20);
        display.print(pomodoroActive ? (workSession ? "Work Time" : "Break Time") : "Reading Time");

        display.setCursor(45, 40);
        display.printf("%02d:%02d", minutes, seconds);

        // Progress Bar
        int progress = map(remaining, 0, totalDuration, 0, 100);
        display.drawFrame(15, 50, 100, 10);
        display.drawBox(15, 50, progress, 10);
    } else {
        display.setCursor(10, 20);
        display.print("Press to Start");
    }

    display.sendBuffer();
}

void showModeSelection() {
    display.clearBuffer();
    display.setFont(u8g2_font_6x13_tf);

    display.setCursor(20, 15);
    display.print("Select Mode:");

    display.setCursor(30, 35);
    display.print(selectedMode == 0 ? "> Pomodoro" : "  Pomodoro");

    display.setCursor(30, 50);
    display.print(selectedMode == 1 ? "> Reading" : "  Reading");

    display.setCursor(10, 60);
    display.print("Pomodoros: ");
    display.print(pomodoroCount);

    display.sendBuffer();
}

void showMessage(const char* msg, int delayTime) {
    display.clearBuffer();
    display.setFont(u8g2_font_6x13_tf);
    int textWidth = display.getStrWidth(msg);
    display.setCursor((128 - textWidth) / 2, 30);
    display.print(msg);
    display.sendBuffer();
    delay(delayTime);
}
