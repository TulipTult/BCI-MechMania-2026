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

// Network Configuration
const char* ssid = "Hatsune";
const char* password = "miku";
String deviceIP = "";

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

bool targetReached = false;
float currentTarget = 360.0;
bool wifiConnected = false;

void encoderA() {
  encoderCount += motorDirection;
}


