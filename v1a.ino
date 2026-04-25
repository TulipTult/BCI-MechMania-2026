#define M1_IN1 44
#define M1_IN2 45

#define M2_IN3 42
#define M2_IN4 43

void setup() {
  pinMode(M1_IN1, OUTPUT);
  pinMode(M1_IN2, OUTPUT);

  pinMode(M2_IN3, OUTPUT);
  pinMode(M2_IN4, OUTPUT);
}

void loop() {

  // Forward
  digitalWrite(M1_IN1, HIGH);
  digitalWrite(M1_IN2, LOW);
  digitalWrite(M2_IN3, HIGH);
  digitalWrite(M2_IN4, HIGH);

  delay(1000);
  //Reverse
  digitalWrite(M1_IN1, HIGH);
  digitalWrite(M1_IN2, HIGH);
  digitalWrite(M2_IN3, HIGH);
  digitalWrite(M2_IN4, LOW);
  delay(1000);
  //stop
  digitalWrite(M1_IN1, LOW);
  digitalWrite(M1_IN2, LOW);
  digitalWrite(M2_IN3, LOW);
  digitalWrite(M2_IN4, LOW);
  delay(500);

}