/* WiFi-enabled Swerve Motor Control
 * Mega2560 + WiFi R3 (ATmega2560 + ESP8266)
 * M2 Motor + Web Interface
 *
 * Motor Control Pins:
 * M2_IN3: 42, M2_IN4: 43
 *
 * Encoder Pins:
 * Encoder A: 2 (interrupt 0)
 *
 * WiFi: ESP8266 via Serial1 (RX1: 19, TX1: 18)
 */

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <SoftwareSerial.h>

// Network Configuration
const char *ssid = "Hatsune";
const char *password = "miku";
String deviceIP = "";

ESP8266WebServer server(80);

#define M2_IN3 42
#define M2_IN4 43
#define ENC_A_PIN 2

#define STEPS_PER_REV 2048
#define DEGREES_PER_STEP (360.0f / STEPS_PER_REV)
#define GEAR_RATIO 4.0f
#define DEADBAND_STEPS 2
#define MOTOR_SPEED 180

volatile long encoderCount = 0;
volatile int motorDirection = 0;

unsigned long lastPrintTime = 0;
const unsigned long PRINT_INTERVAL = 100;

bool targetReached = false;
float currentTarget = 360.0;
bool wifiConnected = false;

void encoderA()
{
    encoderCount += motorDirection;
}

long readEncoderSafe()
{
    noInterrupts();
    long val = encoderCount;
    interrupts();
    return val;
}

void handleRoot()
{
    server.send(200, "text/html", html);
}

String readFromMega() {
//   String res = "";
//   unsigned long t = millis();

//   while (millis() - t < 200) {
//     while (Serial.available()) {
//       char c = Serial.read();
//       if (c == '\n') return res;
//       res += c;
//     }
//   }
//   return res;
}



void setup()
{
    Serial.begin(115200);
    Serial1.begin(ESP_BAUD);
    delay(1000);

    pinMode(M2_IN3, OUTPUT);
    pinMode(M2_IN4, OUTPUT);
    pinMode(ENC_A_PIN, INPUT_PULLUP);

    attachInterrupt(digitalPinToInterrupt(ENC_A_PIN), encoderA, RISING);

    stopMotors();
    encoderCount = 0;

    Serial.println("\n\n=== Swerve Motor Control - WiFi Setup ===");
    Serial.println("Board: Mega2560 + WiFi R3 (ESP8266)");


    server.on("/", handleRoot);

    initializeESP8266();
}

void initializeWiFi() {
  Serial.println("\nInitializing ESP8266 (native mode)...");

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  Serial.print("Connecting to WiFi");

  unsigned long start = millis();

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");

    // timeout (optional but smart)
    if (millis() - start > 15000) {
      Serial.println("\nWiFi connection failed!");
      return;
    }
  }

void rotateForward(int speed)
{
    motorDirection = 1;
    analogWrite(M2_IN3, speed);
    digitalWrite(M2_IN4, LOW);
}

void rotateBackward(int speed)
{
    motorDirection = -1;
    digitalWrite(M2_IN3, LOW);
    analogWrite(M2_IN4, speed);
}

void stopMotors()
{
    motorDirection = 0;
    digitalWrite(M2_IN3, LOW);
    digitalWrite(M2_IN4, LOW);
}

void resetPosition()
{
    noInterrupts();
    encoderCount = 0;
    interrupts();
    targetReached = false;
}