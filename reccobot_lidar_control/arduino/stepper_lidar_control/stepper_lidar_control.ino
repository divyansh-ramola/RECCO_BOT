#include <TimerOne.h>

#define PUL 2
#define DIR 3
#define ENA 4

const int stepsPerRev = 6400;
const float TWO_PI_F = 6.28318530718;

volatile long currentStep = 0;
volatile long targetStep  = 0;

volatile unsigned int stepPeriodUs = 2000;

void stepperISR() {
  if (currentStep == targetStep) return;

  digitalWrite(DIR, targetStep > currentStep);
  digitalWrite(PUL, HIGH);
  digitalWrite(PUL, LOW);
  currentStep += (targetStep > currentStep) ? 1 : -1;
}

float stepsToRadians(long step) {
  return (step / (float)stepsPerRev) * TWO_PI_F;
}

long radiansToSteps(float rad) {
  while (rad < 0) rad += TWO_PI_F;
  while (rad >= TWO_PI_F) rad -= TWO_PI_F;
  return (rad / TWO_PI_F) * stepsPerRev;
}

void setup() {
  Serial.begin(115200);

  pinMode(PUL, OUTPUT);
  pinMode(DIR, OUTPUT);
  pinMode(ENA, OUTPUT);
  digitalWrite(ENA, LOW);

  Timer1.initialize(stepPeriodUs);
  Timer1.attachInterrupt(stepperISR);

  Serial.println("Stepper + Serial ready");
}

void loop() {
  static unsigned long lastSend = 0;

  if (Serial.available()) {
    String input = Serial.readStringUntil('\n');
    input.trim();

    if (input.startsWith("cmd:")) {
      float rad = input.substring(4).toFloat();
      noInterrupts();
      targetStep = radiansToSteps(rad);
      interrupts();

      Serial.print("OK rad=");
      Serial.println(rad, 6);
    }
  }

  if (millis() - lastSend > 50) {
    lastSend = millis();
    noInterrupts();
    long s = currentStep;
    interrupts();

    Serial.print("angle:");
    Serial.println(stepsToRadians(s), 6);
  }
}
