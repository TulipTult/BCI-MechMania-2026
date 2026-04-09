/*
 * Arduino Uno Pinout for Encoded N20 Motor
 *
 * Motor/Encoder Pin   | Arduino Pin
 * -------------------|-------------
 * M1 Motor Power  -  |  9  (PWM)
 * M2 Motor Power  +  | 10  (PWM)
 * Encoder GND        |  GND
 * Encoder VCC        |  5V
 * C1 Encoder A phase |  2  (Interrupt 0)
 * C2 Encoder B phase |  3  (Interrupt 1)
 *
 * N20 Encoder: 600 steps/rev
 * Spins motor 360 degrees (1 rev)
 */

#define M1_PIN 9      // Motor Power -
#define M2_PIN 10     // Motor Power +
#define ENC_A_PIN 2   // Encoder A (interrupt 0)
#define ENC_B_PIN 3   // Encoder B (interrupt 1)

volatile long encoderCount = 0;
int targetSteps = 2048; // 1 revolution

void encoderA() {
  int b = digitalRead(ENC_B_PIN);
  if (b == HIGH) {
    encoderCount++;
  } else {
    encoderCount--;
  }
}

void setup() {
  pinMode(M1_PIN, OUTPUT);
  pinMode(M2_PIN, OUTPUT);
  pinMode(ENC_A_PIN, INPUT_PULLUP);
  pinMode(ENC_B_PIN, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(ENC_A_PIN), encoderA, RISING);

  // Stop motor initially
  analogWrite(M1_PIN, 0);
  analogWrite(M2_PIN, 0);
  encoderCount = 0;
}

void loop() {
  // Spin motor forward until 600 steps (1 rev)
  if (encoderCount < targetSteps) {
    analogWrite(M1_PIN, 0);    // M1 low
    analogWrite(M2_PIN, 200);  // M2 high (speed 200/255)
  } else {
    // Stop motor
    analogWrite(M1_PIN, 0);
    analogWrite(M2_PIN, 0);
  }
}
