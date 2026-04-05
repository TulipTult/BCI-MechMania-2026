// m1a, m2a control speed(0-255 -> speed)
//m1b, m2b control direction(high/low -> fwd/rev)

const int m1a = 2;  
const int m1b = 4; 
const int m2a = 3; 
const int m2b = 5;  

void setup() {
  pinMode(m1a, OUTPUT);
  pinMode(m1b, OUTPUT);
  pinMode(m2a, OUTPUT);
  pinMode(m2b, OUTPUT);
}

void loop() {
  //fwd half
  digitalWrite(m1a, HIGH);
  analogWrite(m1b, 128);
  digitalWrite(m2a, HIGH);
  analogWrite(m2b, 128);

  delay(2000);

  //rev full
  digitalWrite(m1a, LOW);
  analogWrite(m1b, 255);
  digitalWrite(m2a, LOW);
  analogWrite(m2b, 255);

  delay(2000);

  //full stop
  analogWrite(m1b, 0);
  analogWrite(m2b, 0);

  delay(1000);
}