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

float currentTarget = 360.0;
bool targetReached = false;

// --- Encoder ---
void encoderA() {
  encoderCount += motorDirection;
}

long readEncoderSafe() {
  noInterrupts();
  long val = encoderCount;
  interrupts();
  return val;
}

// --- Serial Protocol ---
void handleSerial() {
  static String cmd = "";

  while (Serial1.available()) {
    char c = Serial1.read();

    if (c == '\n') {
      processCommand(cmd);
      cmd = "";
    } else {
      cmd += c;
    }
  }
}

void processCommand(String c) {
  if (c.startsWith("SET:")) {
    float angle = c.substring(4).toFloat();
    if (angle >= 0 && angle <= 360) {
      currentTarget = angle;
      targetReached = false;
    }
  }

  if (c == "GET") {
    Serial1.println(String(getCurrentPosition(), 2));
  }
}

// --- Setup ---
void setup() {
  Serial.begin(115200);
  Serial1.begin(115200);

  pinMode(M2_IN3, OUTPUT);
  pinMode(M2_IN4, OUTPUT);
  pinMode(ENC_A_PIN, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(ENC_A_PIN), encoderA, RISING);

  stopMotors();
}

// --- Loop ---
void loop() {
  handleSerial();

  if (!targetReached) {
    targetReached = rotateToAngle(currentTarget);
  }
}

// --- Motor Logic (unchanged from your code) ---
float getCurrentPosition() {
  long count = readEncoderSafe();
  float degrees = fmod((float)count * DEGREES_PER_STEP, 360.0f);
  if (degrees < 0) degrees += 360.0f;
  return degrees;
}

bool rotateToAngle(float targetDegrees) {
  long count = readEncoderSafe();

  float adjustedTarget = targetDegrees * GEAR_RATIO;
  long absoluteTarget;

  if (adjustedTarget >= 360.0f) {
    absoluteTarget = lround(adjustedTarget / DEGREES_PER_STEP);
  } else {
    long stepsPerRev = STEPS_PER_REV;
    long targetSteps = lround(adjustedTarget / DEGREES_PER_STEP);
    long countInRev = ((count % stepsPerRev) + stepsPerRev) % stepsPerRev;
    long delta = targetSteps - countInRev;

    if (delta >  stepsPerRev / 2) delta -= stepsPerRev;
    if (delta < -stepsPerRev / 2) delta += stepsPerRev;

    absoluteTarget = count + delta;
  }

  if (labs(absoluteTarget - count) <= DEADBAND_STEPS) {
    stopMotors();
    return true;
  }

  if (absoluteTarget > count) rotateForward(MOTOR_SPEED);
  else rotateBackward(MOTOR_SPEED);

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