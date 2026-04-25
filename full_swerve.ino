/* Tracks wheel position in degrees using N20 encoder
 * Single-channel encoder per motor (direction inferred from motor command)
 *
 * Motor Control Pins:
 * M2_IN3: 42, M2_IN4: 43
 *
 * Encoder Pins:
 * Encoder A: 2 (interrupt 0) — single channel only
 *
 * N20 Motor: 2048 steps/rev = 360 degrees
 * Single-channel: counts every RISING edge, direction from motor state
 */

#define M2_IN3 42
#define M2_IN4 43

#define ENC_A_PIN 2

#define STEPS_PER_REV    2048
#define DEGREES_PER_STEP (360.0f / STEPS_PER_REV)
#define GEAR_RATIO       4.0f
#define DEADBAND_STEPS   2
#define MOTOR_SPEED      180

volatile long encoderCount = 0;
volatile int motorDirection = 0;

unsigned long lastPrintTime = 0;
const unsigned long PRINT_INTERVAL = 100;

void encoderA() {
  encoderCount += motorDirection;
}

long readEncoderSafe() {
  noInterrupts();
  long val = encoderCount;
  interrupts();
  return val;
}

void setup() {
  Serial.begin(9600);

  pinMode(M2_IN3, OUTPUT);
  pinMode(M2_IN4, OUTPUT);

  pinMode(ENC_A_PIN, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(ENC_A_PIN), encoderA, RISING);

  stopMotors();
  encoderCount = 0;

  Serial.println("Swerve Robot Control Started");
  Serial.println("Position: 0 degrees");
}

bool targetReached = false;
float currentTarget = 360.0;

void loop() {
  if (millis() - lastPrintTime >= PRINT_INTERVAL) {
    lastPrintTime = millis();
    printPosition();
  }

  if (!targetReached) {
    targetReached = rotateToAngle(currentTarget);
  }
}

float getCurrentPosition() {
  long count = readEncoderSafe();
  float degrees = fmod((float)count * DEGREES_PER_STEP, 360.0f);
  if (degrees < 0) degrees += 360.0f;
  return degrees;
}

void printPosition() {
  float position = getCurrentPosition();
  Serial.print("Wheel Position: ");
  Serial.print(position, 2);
  Serial.print(" deg | Steps: ");
  Serial.println(readEncoderSafe());
}

bool rotateToAngle(float targetDegrees) {
  long count = readEncoderSafe();

  // Apply gear ratio to target
  float adjustedTarget = targetDegrees * GEAR_RATIO;
  long absoluteTarget;

  // Special case: 360 = exactly one full revolution from zero
  if (adjustedTarget >= 360.0f) {
    absoluteTarget = lround(adjustedTarget / DEGREES_PER_STEP);  // = 2048
  } else {
    long stepsPerRev = STEPS_PER_REV;
    long targetSteps = lround(adjustedTarget / DEGREES_PER_STEP);
    long countInRev = ((count % stepsPerRev) + stepsPerRev) % stepsPerRev;
    long delta = targetSteps - countInRev;

    // Shortest path wrap
    if (delta >  stepsPerRev / 2) delta -= stepsPerRev;
    if (delta < -stepsPerRev / 2) delta += stepsPerRev;

    absoluteTarget = count + delta;
  }

  if (labs(absoluteTarget - count) <= DEADBAND_STEPS) {
    stopMotors();
    Serial.print("Target reached: ");
    Serial.print(targetDegrees);
    Serial.println(" degrees");
    return true;
  }

  if (absoluteTarget > count) {
    rotateForward(MOTOR_SPEED);
  } else {
    rotateBackward(MOTOR_SPEED);
  }

  return false;
}

void rotateForward(int speed) {
  motorDirection = 1;
  analogWrite(M2_IN3, speed);
  digitalWrite(M2_IN4, LOW);
}

void rotateBackward(int speed) {
  motorDirection = -1;
  digitalWrite(M2_IN3, LOW);
  analogWrite(M2_IN4, speed);
}

void stopMotors() {
  motorDirection = 0;
  digitalWrite(M2_IN3, LOW);
  digitalWrite(M2_IN4, LOW);
}

void resetPosition() {
  noInterrupts();
  encoderCount = 0;
  interrupts();
  targetReached = false;
  Serial.println("Position reset to 0 degrees");
}