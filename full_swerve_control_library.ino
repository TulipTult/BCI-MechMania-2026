/*
  SWERVE DRIVE SYSTEM (Mega2560 - Non Blocking)
  Includes:
  - Mobility drive motors (FWD/RVS)
  - N20 encoder steering motors
  - SMR (rotation control)
  - SMP (power control)
  - SWRV (hybrid drive + steer)
*/

// MOTOR PINS (MOBILITY)
const int M1AL = 22;
const int M1BL = 23;
const int M2AL = 24;
const int M2BL = 25;

const int M1AR = 26;
const int M1BR = 27;
const int M2AR = 28;
const int M2BR = 29;

// N20 STEERING MOTORS (The pins are connected into the BRV8833 board)
const int N1AL = 13;
const int N1BL = 12;
const int N2AL = 11;
const int N2BL = 10;

const int N1AR = 9;
const int N1BR = 8;
const int N2AR = 7;
const int N2BR = 6;

// ENCODERS (QUADRATURE)
const int ENC1A_L = 2;
const int ENC1B_L = 3;

const int ENC1A_R = 21;
const int ENC1B_R = 20;

// ENCODER STATE
volatile long encoderCount_L = 0;
volatile long encoderCount_R = 0;

long target_L = 0;
long target_R = 0;

bool steerActive_L = false;
bool steerActive_R = false;

// INTERRUPTS (ENCODERS)
void encoderA_L() {
  digitalRead(ENC1B_L) ? encoderCount_L++ : encoderCount_L--;
}

void encoderA_R() {
  digitalRead(ENC1B_R) ? encoderCount_R++ : encoderCount_R--;
}

// SETUP
void setup() {

  // Mobility motors
  pinMode(M1AL, OUTPUT); pinMode(M1BL, OUTPUT);
  pinMode(M2AL, OUTPUT); pinMode(M2BL, OUTPUT);

  pinMode(M1AR, OUTPUT); pinMode(M1BR, OUTPUT);
  pinMode(M2AR, OUTPUT); pinMode(M2BR, OUTPUT);

  // Steering motors
  pinMode(N1AL, OUTPUT); pinMode(N1BL, OUTPUT);
  pinMode(N2AL, OUTPUT); pinMode(N2BL, OUTPUT);

  // Encoders
  pinMode(ENC1A_L, INPUT_PULLUP);
  pinMode(ENC1B_L, INPUT_PULLUP);
  pinMode(ENC1A_R, INPUT_PULLUP);
  pinMode(ENC1B_R, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(ENC1A_L), encoderA_L, RISING);
  attachInterrupt(digitalPinToInterrupt(ENC1A_R), encoderA_R, RISING);
}

// =====================================================
// BASIC DRIVE FUNCTIONS
// =====================================================
void FWD() {
  digitalWrite(M1AL, HIGH); digitalWrite(M1BL, LOW);
  digitalWrite(M2AL, HIGH); digitalWrite(M2BL, LOW);

  digitalWrite(M1AR, HIGH); digitalWrite(M1BR, LOW);
  digitalWrite(M2AR, HIGH); digitalWrite(M2BR, LOW);
}

void STOP_DRIVE() {
  digitalWrite(M1AL, LOW); digitalWrite(M1BL, LOW);
  digitalWrite(M2AL, LOW); digitalWrite(M2BL, LOW);

  digitalWrite(M1AR, LOW); digitalWrite(M1BR, LOW);
  digitalWrite(M2AR, LOW); digitalWrite(M2BR, LOW);
}

// =====================================================
// SMR: START STEERING MOVE (NON-BLOCKING)
// =====================================================
void SMR_L(long steps) {
  encoderCount_L = 0;
  target_L = steps;
  steerActive_L = true;

  // CCW example
  analogWrite(N1AL, 0);
  analogWrite(N1BL, 200);
}

void SMR_R(long steps) {
  encoderCount_R = 0;
  target_R = steps;
  steerActive_R = true;

  // CW example
  analogWrite(N2AL, 200);
  analogWrite(N2BL, 0);
}

// =====================================================
// STEERING UPDATE LOOP (MUST RUN EVERY LOOP)
// =====================================================
void updateSteering() {

  if (steerActive_L && encoderCount_L >= target_L) {
    analogWrite(N1AL, 0);
    analogWrite(N1BL, 0);
    steerActive_L = false;
  }

  if (steerActive_R && encoderCount_R >= target_R) {
    analogWrite(N2AL, 0);
    analogWrite(N2BL, 0);
    steerActive_R = false;
  }
}

// =====================================================
// SWRV: HYBRID FUNCTION (CORE OF SWERVE SYSTEM)
// =====================================================
void SWRV(long steerStepsL, long steerStepsR, int durationMs) {

  // Start steering
  SMR_L(steerStepsL);
  SMR_R(steerStepsR);

  unsigned long start = millis();

  // Drive + steer simultaneously (non-blocking loop)
  while (millis() - start < durationMs) {

    FWD();              // keep driving
    updateSteering();   // keep checking encoders
  }

  STOP_DRIVE();
}

// =====================================================
// MAIN LOOP DEMO
// =====================================================
void loop() {

  // Example: move forward while steering modules
  SWRV(300, 300, 1500);

  delay(1000);

  // Straight drive
  FWD();
  delay(1000);

  STOP_DRIVE();
  delay(1000);
}
