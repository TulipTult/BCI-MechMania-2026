/*
 * Swerve Robot Wheel Control
 * Tracks wheel position in degrees using N20 encoder
 *
 * Motor Control Pins (from v1a.ino):
 * M1_IN1: 44, M1_IN2: 45
 * M2_IN3: 42, M2_IN4: 43
 *
 * Encoder Pins:
 * Encoder A: 2 (interrupt 0)
 * Encoder B: 3 (interrupt 1)
 *
 * N20 Motor: 2048 steps/rev = 360 degrees
 */

// Motor control pins (from v1a.ino)
#define M1_IN1 44
#define M1_IN2 45
#define M2_IN3 42
#define M2_IN4 43

// Encoder pins (from n20_encoder_test.ino)
#define ENC_A_PIN 2   // Encoder A (interrupt 0)
#define ENC_B_PIN 3   // Encoder B (interrupt 1)

// Encoder constants
#define STEPS_PER_REV 2048  // N20 encoder steps per revolution
#define DEGREES_PER_STEP (360.0 / STEPS_PER_REV)  // 0.176 degrees per step

// Global variables
volatile long encoderCount = 0;
unsigned long lastPrintTime = 0;
const unsigned long PRINT_INTERVAL = 100;  // Print every 100ms

// Encoder interrupt handler
void encoderA() {
  int b = digitalRead(ENC_B_PIN);
  if (b == HIGH) {
    encoderCount++;
  } else {
    encoderCount--;
  }
}

void setup() {
  Serial.begin(9600);

  // Configure motor pins as outputs
  pinMode(M1_IN1, OUTPUT);
  pinMode(M1_IN2, OUTPUT);
  pinMode(M2_IN3, OUTPUT);
  pinMode(M2_IN4, OUTPUT);

  // Configure encoder pins
  pinMode(ENC_A_PIN, INPUT_PULLUP);
  pinMode(ENC_B_PIN, INPUT_PULLUP);

  // Attach encoder interrupt
  attachInterrupt(digitalPinToInterrupt(ENC_A_PIN), encoderA, RISING);

  // Stop motors initially
  stopMotors();
  encoderCount = 0;

  Serial.println("Swerve Robot Control Started");
  Serial.println("Position: 0 degrees (facing forward)");
}

void loop() {
  // Print position at regular intervals
  if (millis() - lastPrintTime >= PRINT_INTERVAL) {
    lastPrintTime = millis();
    printPosition();
  }

  // Example: Rotate wheel slowly
  rotateToAngle(90);  // Uncomment to test
}

// Calculate current wheel position in degrees
float getCurrentPosition() {
  // Normalize to 0-360 range
  float degrees = (encoderCount * DEGREES_PER_STEP);
  degrees = fmod(degrees, 360.0);
  if (degrees < 0) degrees += 360.0;
  return degrees;
}

// Print current position
void printPosition() {
  float position = getCurrentPosition();
  Serial.print("Wheel Position: ");
  Serial.print(position);
  Serial.print(" degrees | Encoder Count: ");
  Serial.println(encoderCount);
}

// Rotate to specific angle (0-360 degrees)
void rotateToAngle(float targetDegrees) {
  long targetSteps = (long)(targetDegrees / DEGREES_PER_STEP);

  // Rotate forward if encoderCount < targetSteps
  if (encoderCount < targetSteps) {
    rotateForward();
  }
  // Rotate backward if encoderCount > targetSteps
  else if (encoderCount > targetSteps) {
    rotateBackward();
  }
  // At target position
  else {
    stopMotors();
  }
}

// Rotate motors forward
void rotateForward() {
  digitalWrite(M1_IN1, HIGH);
  digitalWrite(M1_IN2, LOW);
  digitalWrite(M2_IN3, HIGH);
  digitalWrite(M2_IN4, LOW);
}

// Rotate motors backward
void rotateBackward() {
  digitalWrite(M1_IN1, LOW);
  digitalWrite(M1_IN2, HIGH);
  digitalWrite(M2_IN3, LOW);
  digitalWrite(M2_IN4, HIGH);
}

// Stop motors
void stopMotors() {
  digitalWrite(M1_IN1, LOW);
  digitalWrite(M1_IN2, LOW);
  digitalWrite(M2_IN3, LOW);
  digitalWrite(M2_IN4, LOW);
}

// Reset position to 0 degrees
void resetPosition() {
  encoderCount = 0;
  Serial.println("Position reset to 0 degrees");
}
