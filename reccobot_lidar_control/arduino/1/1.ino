#include <UIPEthernet.h>

// ---------------- Ethernet Setup ----------------
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(192,168,1,150);

// ROS2-compatible TCP servers
EthernetServer stateServer(8000);     // Publishes joint state (radians)
EthernetServer commandServer(8001);   // Gets command (radians)

// ---------------- Stepper Setup ----------------
#define PUL 2
#define DIR 3
#define ENA 4

int stepsPerRev = 6400;
int currentStep = 0;
int targetStep = 0;

unsigned long lastStepMicros = 0;
int stepDelay = 1200;
bool pulseState = LOW;
bool moving = false;

// Constants
const float TWO_PI = 6.28318530718;

// ---------------- Convert Steps → Radians ----------------
float stepsToRadians(int step) {
  return (step / (float)stepsPerRev) * TWO_PI;
}

// ---------------- Convert Radians → Steps ----------------
int radiansToSteps(float rad) {
  // wrap angle into [0, 2π)
  while (rad < 0) rad += TWO_PI;
  while (rad >= TWO_PI) rad -= TWO_PI;

  return (rad / TWO_PI) * stepsPerRev;
}

// ---------------- Stepper Motion ----------------
void driveToTarget() {
  if (!moving) return;

  if (micros() - lastStepMicros >= stepDelay) {
    lastStepMicros = micros();

    if (currentStep < targetStep) {
      digitalWrite(DIR, HIGH);
      pulseState = !pulseState;
      digitalWrite(PUL, pulseState);
      if (pulseState == LOW) currentStep++;
    }
    else if (currentStep > targetStep) {
      digitalWrite(DIR, LOW);
      pulseState = !pulseState;
      digitalWrite(PUL, pulseState);
      if (pulseState == LOW) currentStep--;
    }9
    else {
      moving = false;  // reached target
    }
  }
}

// ---------------- Handle Command (Radians) ----------------
void handleCommand(EthernetClient &client) {
  if (!client.available()) return;

  String input = client.readStringUntil('\n');

  if (input.startsWith("cmd:")) {
    float targetRad = input.substring(4).toFloat();

    targetStep = radiansToSteps(targetRad);
    moving = true;

    client.print("OK rad=");
    client.println(targetRad, 6);
  }
}

// ---------------- Publish State (Radians) ----------------
void publishState(EthernetClient &client) {
  static unsigned long lastSend = 0;
  if (millis() - lastSend < 50) return;  // 20 Hz
  lastSend = millis();

  float rad = stepsToRadians(currentStep);

  client.print("angle:");
  client.println(rad, 6);
}

// ---------------- SETUP ----------------
void setup() {
  Serial.begin(115200);

  pinMode(PUL, OUTPUT);
  pinMode(DIR, OUTPUT);
  pinMode(ENA, OUTPUT);
  digitalWrite(ENA, LOW);

  Ethernet.begin(mac, ip);
  stateServer.begin();
  commandServer.begin();

  Serial.print("Arduino IP: ");
  Serial.println(Ethernet.localIP());
}

// ---------------- LOOP ----------------
void loop() {
  driveToTarget();

  // State publisher
  {
    EthernetClient stateClient = stateServer.available();
    if (stateClient) publishState(stateClient);
  }

  // Command receiver
  {
    EthernetClient cmdClient = commandServer.available();
    if (cmdClient) handleCommand(cmdClient);
  }
}
