#include <Arduino.h>
#include <freertos/semphr.h>

#define PUL 25
#define DIR 26
#define ENA 27
#define UART_RX 16
#define UART_TX 17

const int stepsPerRev = 12800;
const float TWO_PI_F = 6.28318530718f;

/* ===== PACKETS ===== */
struct AnglePacket {
    uint8_t  sync0;
    uint8_t  sync1;
    int32_t  angle_urad;
    uint32_t t_us;
    uint8_t  checksum;
} __attribute__((packed));

struct CmdPacket {
    uint8_t  sync0;
    uint8_t  sync1;
    int32_t  target_urad;
} __attribute__((packed));

/* ===== PROTECTED STATE ===== */
portMUX_TYPE stepMux = portMUX_INITIALIZER_UNLOCKED;
volatile long currentStep = 0;
volatile long targetStep  = 0;

/* ===== MATH ===== */
float stepsToRadians(long step) {
    return (step / (float)stepsPerRev) * TWO_PI_F;
}

long radiansToSteps(float rad) {
    while (rad <  0)         rad += TWO_PI_F;
    while (rad >= TWO_PI_F)  rad -= TWO_PI_F;
    long s = lroundf((rad / TWO_PI_F) * stepsPerRev);
    if (s >= stepsPerRev) s = 0;
    return s;
}

long shortestDelta(long cur, long tgt) {
    long d = tgt - cur;
    if (d >  stepsPerRev / 2) d -= stepsPerRev;
    if (d < -stepsPerRev / 2) d += stepsPerRev;
    return d;
}

/* ===== CORE 1 — STEPPER ===== */
void stepperTask(void *pvParameters) {
    unsigned long lastStepTime = 0;
    const unsigned int stepIntervalUs = 600;

    for (;;) {
        portENTER_CRITICAL(&stepMux);
        long cur = currentStep;
        long tgt = targetStep;
        portEXIT_CRITICAL(&stepMux);

        long delta = shortestDelta(cur, tgt);

        if (delta != 0) {
            unsigned long now = micros();
            if (now - lastStepTime >= stepIntervalUs) {
                lastStepTime = now;

                bool dir = (delta > 0);
                digitalWrite(DIR, dir);
                delayMicroseconds(5);
                digitalWrite(PUL, HIGH);
                delayMicroseconds(4);
                digitalWrite(PUL, LOW);

                portENTER_CRITICAL(&stepMux);
                currentStep = (currentStep + (dir ? 1 : -1) + stepsPerRev) % stepsPerRev;
                portEXIT_CRITICAL(&stepMux);
            }
        } else {
            taskYIELD();
        }
    }
}

/* ===== CORE 0 — UART ===== */
void serialTask(void *pvParameters) {
    unsigned long lastSend = 0;

    for (;;) {
        /* ===== RECEIVE COMMAND ===== */
        if (Serial2.available() >= (int)sizeof(CmdPacket)) {
            CmdPacket cmd;
            Serial2.readBytes((uint8_t*)&cmd, sizeof(cmd));

            if (cmd.sync0 != 0xAA || cmd.sync1 != 0x55) {
                // Discard until we see 0xAA
                while (Serial2.available()) {
                    if (Serial2.read() == 0xAA) break;
                }
                vTaskDelay(1);
                continue;
            }

            float rad = cmd.target_urad / 1000000.0f;
            long steps = radiansToSteps(rad);

            portENTER_CRITICAL(&stepMux);
            targetStep = steps;
            portEXIT_CRITICAL(&stepMux);
        }

        /* ===== SEND TELEMETRY ===== */
        if (millis() - lastSend > 20) {
            lastSend = millis();

            portENTER_CRITICAL(&stepMux);
            long cur = currentStep;
            portEXIT_CRITICAL(&stepMux);

            AnglePacket pkt;
            pkt.sync0      = 0xAA;
            pkt.sync1      = 0x55;
            pkt.angle_urad = (int32_t)(stepsToRadians(cur) * 1000000.0f);
            pkt.t_us       = micros();

            uint8_t *p = (uint8_t*)&pkt.angle_urad;
            pkt.checksum = 0;
            for (int i = 0; i < 8; i++) pkt.checksum ^= p[i];

            Serial2.write((uint8_t*)&pkt, sizeof(pkt));
        }

        vTaskDelay(1);
    }
}

/* ===== SETUP ===== */
void setup() {
    Serial.begin(115200);
    Serial2.begin(115200, SERIAL_8N1, UART_RX, UART_TX);

    pinMode(PUL, OUTPUT);
    pinMode(DIR, OUTPUT);
    pinMode(ENA, OUTPUT);
    digitalWrite(ENA, LOW);
    digitalWrite(PUL, LOW);
    digitalWrite(DIR, LOW);

    delay(200);

    Serial.println("Binary Stepper READY");
    xTaskCreatePinnedToCore(stepperTask, "StepperTask", 4096, NULL, 2, NULL, 1);
    xTaskCreatePinnedToCore(serialTask,  "SerialTask",  4096, NULL, 1, NULL, 0);
}

void loop() {}