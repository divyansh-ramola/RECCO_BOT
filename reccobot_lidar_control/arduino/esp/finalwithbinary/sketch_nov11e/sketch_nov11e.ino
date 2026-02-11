#include <Arduino.h>

#define PUL 25
#define DIR 26
#define ENA 27

#define UART_RX 16
#define UART_TX 17

const int stepsPerRev = 12800;
const float TWO_PI_F = 6.28318530718f;

/* ===== PACKETS ===== */
struct AnglePacket {
  int32_t angle_urad;
  uint32_t t_us;
};

struct CmdPacket {
  int32_t target_urad;
};

/* ===== STATE ===== */
volatile long currentStep = 0;
volatile long targetStep  = 0;

unsigned long lastStepTime = 0;
unsigned int stepIntervalUs = 600;

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
   CORE 1 — STEPPER
===================================================== */
void stepperTask(void *pvParameters) {

  for (;;) {

    if (currentStep != targetStep) {

      unsigned long now = micros();

      if (now - lastStepTime >= stepIntervalUs) {

        lastStepTime = now;

        digitalWrite(DIR, targetStep > currentStep);

        digitalWrite(PUL, HIGH);
        delayMicroseconds(4);
        digitalWrite(PUL, LOW);

        currentStep += (targetStep > currentStep) ? 1 : -1;
      }
    }

    vTaskDelay(1);
  }
}

/* =====================================================
   CORE 0 — UART BINARY
===================================================== */
void serialTask(void *pvParameters) {

  unsigned long lastSend = 0;
  CmdPacket cmd;

  for (;;) {

    /* ===== RECEIVE COMMAND ===== */
    if (Serial2.available() >= sizeof(CmdPacket)) {

      Serial2.readBytes((uint8_t*)&cmd, sizeof(cmd));

      float rad = cmd.target_urad / 1000000.0f;
      targetStep = radiansToSteps(rad);
    }

    /* ===== SEND TELEMETRY ===== */
    if (millis() - lastSend > 20) {

      lastSend = millis();

      AnglePacket pkt;

      float a = stepsToRadians(currentStep);

      pkt.angle_urad = (int32_t)(a * 1000000.0f);
      pkt.t_us = micros();

      Serial2.write((uint8_t*)&pkt, sizeof(pkt));
    }

    vTaskDelay(1);
  }
}

/* =====================================================
   SETUP
===================================================== */
void setup() {

  Serial.begin(115200);
  Serial2.begin(115200, SERIAL_8N1, UART_RX, UART_TX);

  delay(1500);

  pinMode(PUL, OUTPUT);
  pinMode(DIR, OUTPUT);
  pinMode(ENA, OUTPUT);

  digitalWrite(ENA, LOW);

  Serial.println("Binary Stepper READY");

  xTaskCreatePinnedToCore(stepperTask,"StepperTask",4096,NULL,2,NULL,1);
  xTaskCreatePinnedToCore(serialTask,"SerialTask",4096,NULL,1,NULL,0);
}

void loop(){}
