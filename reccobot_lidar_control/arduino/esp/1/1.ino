#include <Arduino.h>

#define PUL 25
#define DIR 26
#define ENA 27

const int stepsPerRev = 6400;
const float TWO_PI_F = 6.28318530718f;

/* ===== SHARED STATE ===== */
volatile long currentStep = 0;
volatile long targetStep  = 0;

unsigned long lastStepTime = 0;
unsigned int stepIntervalUs = 600;

/* ===== SERIAL BUFFER ===== */
String inputBuffer = "";

/* ===== MATH ===== */
float stepsToRadians(long step) {
  return (step / (float)stepsPerRev) * TWO_PI_F;
}

long radiansToSteps(float rad) {
  while (rad < 0) rad += TWO_PI_F;
  while (rad >= TWO_PI_F) rad -= TWO_PI_F;
  return (rad / TWO_PI_F) * stepsPerRev;
}

/* =====================================================
   CORE 1 TASK — STEPPER CONTROL
===================================================== */
void stepperTask(void *pvParameters) {

  for (;;) {

    if (currentStep != targetStep) {

      unsigned long now = micros();

      if (now - lastStepTime >= stepIntervalUs) {

        lastStepTime = now;

        digitalWrite(DIR, targetStep > currentStep);

        digitalWrite(PUL, HIGH);
        delayMicroseconds(4);   // proper pulse width
        digitalWrite(PUL, LOW);

        currentStep += (targetStep > currentStep) ? 1 : -1;
      }
    }

    // VERY IMPORTANT → allow other core to run
    taskYIELD();
  }
}

/* =====================================================
   CORE 0 TASK — SERIAL + STATE
===================================================== */
void serialTask(void *pvParameters) {

  unsigned long lastSend = 0;

  for (;;) {

    /* ===== SERIAL READ ===== */
    while (Serial.available()) {

      char c = Serial.read();

      if (c == '\n') {

        inputBuffer.trim();

        if (inputBuffer.startsWith("cmd:")) {

          float rad = inputBuffer.substring(4).toFloat();
          targetStep = radiansToSteps(rad);

          Serial.print("OK rad=");
          Serial.println(rad, 6);
        }

        inputBuffer = "";
      }
      else {
        inputBuffer += c;
      }
    }

    /* ===== STATE OUTPUT ===== */
    if (millis() - lastSend > 200) {

      lastSend = millis();

      Serial.print("angle:");
      Serial.println(stepsToRadians(currentStep), 6);
    }

    vTaskDelay(1);   // prevent watchdog
  }
}

/* =====================================================
   SETUP
===================================================== */
void setup() {

  Serial.begin(115200);
  delay(1000);

  pinMode(PUL, OUTPUT);
  pinMode(DIR, OUTPUT);
  pinMode(ENA, OUTPUT);

  digitalWrite(ENA, LOW);

  Serial.println("Dual Core Stepper READY");

  /* ===== CREATE TASKS ===== */

  // Core 1 → motion
  xTaskCreatePinnedToCore(
    stepperTask,
    "StepperTask",
    4096,
    NULL,
    2,
    NULL,
    1);

  // Core 0 → serial + state
  xTaskCreatePinnedToCore(
    serialTask,
    "SerialTask",
    4096,
    NULL,
    1,
    NULL,
    0);
}

/* =====================================================
   LOOP (unused)
===================================================== */
void loop() {
  // empty — RTOS handles everything
}
