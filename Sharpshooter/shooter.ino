#include <Servo.h>

//
// ====== PIN DEFINITIONS ======
//

// ---- Intake Servos ----
Servo intakeServoLeft;
Servo intakeServoRight;
int intakeServoLeftPin  = 51;
int intakeServoRightPin = 50;

// ---- Intake Large DC Motors (L298N #1) ----
int in1 = 22;     
int in2 = 23;     
int ena = 5;      

int in3 = 24;     
int in4 = 25;     
int enb = 6;      

// ---- Index / Gate Servos (TWO SERVOS) ----
Servo gateServoLeft;
Servo gateServoRight;
int gateServoLeftPin  = 53;
int gateServoRightPin = 52;

// ---- Shooter Flywheels ----
// Small DC motors powered directly from Mega
int smallFlyLeft  = 7;
int smallFlyRight = 8;

// Large DC motors (L298N #2)
int s_in1 = 26;
int s_in2 = 27;
int s_ena = 9;

int s_in3 = 28;
int s_in4 = 29;
int s_enb = 10;

//
// ====== SETUP ======
//
void setup() {
  // Intake Servos
  intakeServoLeft.attach(intakeServoLeftPin);
  intakeServoRight.attach(intakeServoRightPin);

  // Gate Servos
  gateServoLeft.attach(gateServoLeftPin);
  gateServoRight.attach(gateServoRightPin);

  // Intake L298N Motor Pins
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  pinMode(ena, OUTPUT);
  pinMode(in3, OUTPUT);
  pinMode(in4, OUTPUT);
  pinMode(enb, OUTPUT);

  // Shooter small DC motors
  pinMode(smallFlyLeft, OUTPUT);
  pinMode(smallFlyRight, OUTPUT);

  // Shooter L298N Motor Pins
  pinMode(s_in1, OUTPUT);
  pinMode(s_in2, OUTPUT);
  pinMode(s_ena, OUTPUT);
  pinMode(s_in3, OUTPUT);
  pinMode(s_in4, OUTPUT);
  pinMode(s_enb, OUTPUT);

  // Initial state
  stopIntakeMotors();
  stopShooterMotors();
  gateClose();

  Serial.begin(115200);
}

//
// ====== INTAKE FUNCTIONS ======
//
void intakePuck() {
  Serial.println("Intaking puck...");

  // Move intake servos inward
  intakeServoLeft.write(60);
  intakeServoRight.write(120);

  // Spin intake DC motors
  analogWrite(ena, 200);
  digitalWrite(in1, HIGH);
  digitalWrite(in2, LOW);

  analogWrite(enb, 200);
  digitalWrite(in3, HIGH);
  digitalWrite(in4, LOW);
}

void stopIntakeMotors() {
  analogWrite(ena, 0);
  analogWrite(enb, 0);
}

//
// ====== GATE FUNCTIONS (TWO SERVOS) ======
//
void gateOpen() {
  // Adjust angles as needed
  gateServoLeft.write(40);
  gateServoRight.write(140);
}

void gateClose() {
  // Adjust angles as needed
  gateServoLeft.write(120);
  gateServoRight.write(60);
}

void indexPuck() {
  Serial.println("Indexing puck...");

  gateOpen();
  delay(500); // allow puck to enter
  gateClose();
}

//
// ====== SHOOTER FUNCTIONS ======
//
void startShooterMotors() {
  Serial.println("Starting shooter...");

  // Small flywheels → ON
  digitalWrite(smallFlyLeft, HIGH);
  digitalWrite(smallFlyRight, HIGH);

  // Large flywheels → ON
  analogWrite(s_ena, 255);
  digitalWrite(s_in1, HIGH);
  digitalWrite(s_in2, LOW);

  analogWrite(s_enb, 255);
  digitalWrite(s_in3, HIGH);
  digitalWrite(s_in4, LOW);
}

void stopShooterMotors() {
  digitalWrite(smallFlyLeft, LOW);
  digitalWrite(smallFlyRight, LOW);

  analogWrite(s_ena, 0);
  analogWrite(s_enb, 0);
}

void firePuck() {
  Serial.println("Firing puck...");

  startShooterMotors();
  delay(1500);  // spin-up time
}

//
// ====== MAIN LOOP (EXAMPLE) ======
//
void loop() {

  intakePuck();         // bring puck inside
  delay(2000);

  stopIntakeMotors();   
  indexPuck();          // open gate → puck enters → close gate

  firePuck();           // spin up flywheels + shoot
  delay(2000);

  stopShooterMotors();

  delay(3000);          // pause
}
