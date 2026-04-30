/*
  SWERVE DRIVE SYSTEM (Mega2560 - Non Blocking)
  Includes:
  - Mobility drive motors (FWD/RVS)
  - N20 encoder steering motors
  - SMR (rotation control)
  - SMP (power control)
  - SWRV (hybrid drive + steer)
  "Note: This is a simplified example for demonstration purposes. In a real implementation, you would likely want to add PID control for the steering, handle edge cases, and implement more complex drive patterns."
  we don't know PID so we kinda just wing it

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

  if (steps == 0) {
    analogWrite(N1AL, 0);
    analogWrite(N1BL, 0);
    steerActive_L = false;
    return;
  }

  steerActive_L = true;

  if (steps > 0) {
    // CCW example
    analogWrite(N1AL, 0);
    analogWrite(N1BL, 200);
  } else {
    // CW example
    analogWrite(N1AL, 200);
    analogWrite(N1BL, 0);
  }
}

void SMR_R(long steps) {
  encoderCount_R = 0;
  target_R = steps;

  if (steps == 0) {
    analogWrite(N2AL, 0);
    analogWrite(N2BL, 0);
    steerActive_R = false;
    return;
  }

  steerActive_R = true;

  if (steps > 0) {
    // CW example
    analogWrite(N2AL, 200);
    analogWrite(N2BL, 0);
  } else {
    // CCW example
    analogWrite(N2AL, 0);
    analogWrite(N2BL, 200);
  }
}

// =====================================================
// N20 ANGLE HELPERS (2048 steps per revolution)
// =====================================================
long degreesToSteps(float degrees) {
  const float stepsPerRev = 2048.0;
  return (long)(degrees * (stepsPerRev / 360.0));
}

float getSteerDegreesFromCount(long count) {
  const long stepsPerRev = 2048;
  long steps = count % stepsPerRev;
  if (steps < 0) steps += stepsPerRev;
  return steps * (360.0 / stepsPerRev);
}

float getSteerDegrees_L() {
  return getSteerDegreesFromCount(encoderCount_L);
}

float getSteerDegrees_R() {
  return getSteerDegreesFromCount(encoderCount_R);
}

float shortestDeltaDegrees(float targetDeg, float currentDeg) {
  float delta = targetDeg - currentDeg;
  while (delta > 180.0) delta -= 360.0;
  while (delta <= -180.0) delta += 360.0;
  return delta;
}

// Start steering to an absolute angle (0-360)
void SMR_L_deg(float targetDegrees) {
  float current = getSteerDegrees_L();
  float delta = shortestDeltaDegrees(targetDegrees, current);
  SMR_L(degreesToSteps(delta));
}

void SMR_R_deg(float targetDegrees) {
  float current = getSteerDegrees_R();
  float delta = shortestDeltaDegrees(targetDegrees, current);
  SMR_R(degreesToSteps(delta));
}

// =====================================================
// STEERING UPDATE LOOP (MUST RUN EVERY LOOP)
// =====================================================
void updateSteering() {

  if (steerActive_L &&
      ((target_L >= 0 && encoderCount_L >= target_L) ||
       (target_L < 0 && encoderCount_L <= target_L))) {
    analogWrite(N1AL, 0);
    analogWrite(N1BL, 0);
    steerActive_L = false;
  }

  if (steerActive_R &&
      ((target_R >= 0 && encoderCount_R >= target_R) ||
       (target_R < 0 && encoderCount_R <= target_R))) {
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
