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

#include <SoftwareSerial.h>

// Use Serial1 on Mega for ESP8266 (RX1=19, TX1=18)
#define ESP_RX 19
#define ESP_TX 18
#define ESP_BAUD 115200

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

long readEncoderSafe() {
  noInterrupts();
  long val = encoderCount;
  interrupts();
  return val;
}

void setup() {
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

  initializeESP8266();
}

void initializeESP8266() {
  Serial.println("\nInitializing ESP8266...");

  // Reset ESP8266
  sendATCommand("AT+RST", 3000);
  clearSerialBuffer();

  // Turn off echo
  sendATCommand("ATE0", 500);
  clearSerialBuffer();

  // Get firmware version
  Serial.println("Firmware: ");
  sendATCommand("AT+GMR", 500);

  // Set WiFi mode to Station
  sendATCommand("AT+CWMODE=1", 500);

  // Connect to WiFi
  String wifiCmd = "AT+CWJAP=\"" + String(ssid) + "\",\"" + String(password) + "\"";
  Serial.print("Connecting to: ");
  Serial.println(ssid);

  if (sendATCommand(wifiCmd, 10000)) {
    delay(1000);

    // Get IP address
    getESP8266IP();
    wifiConnected = true;

    // Set up as HTTP server
    setupHTTPServer();
  } else {
    Serial.println("Failed to connect to WiFi!");
  }
}

void getESP8266IP() {
  // Get local IP
  Serial1.println("AT+CIFSR");
  delay(500);

  String response = "";
  while (Serial1.available()) {
    response += (char)Serial1.read();
  }

  // Parse IP from response
  int ipStart = response.indexOf("+CIFSR:STAIP,\"");
  if (ipStart != -1) {
    ipStart += 14;
    int ipEnd = response.indexOf("\"", ipStart);
    deviceIP = response.substring(ipStart, ipEnd);

    Serial.println("\n✓ WiFi Connected!");
    Serial.print("IP Address: ");
    Serial.println(deviceIP);
    Serial.print("Access web interface at: http://");
    Serial.println(deviceIP);
  }
}

void setupHTTPServer() {
  // Enable multiple connections
  sendATCommand("AT+CIPMUX=1", 500);

  // Start server on port 80
  sendATCommand("AT+CIPSERVER=1,80", 500);

  Serial.println("HTTP Server started on port 80");
}

bool sendATCommand(String cmd, int timeout) {
  Serial1.println(cmd);
  unsigned long startTime = millis();
  String response = "";

  while (millis() - startTime < timeout) {
    if (Serial1.available()) {
      char c = Serial1.read();
      response += c;
      Serial.write(c);
    }
  }

  return response.indexOf("OK") != -1 || response.indexOf("ALREADY CONNECTED") != -1;
}

void clearSerialBuffer() {
  while (Serial1.available()) {
    Serial1.read();
  }
}

void loop() {
  // Handle serial data from ESP8266
  if (Serial1.available()) {
    String data = Serial1.readStringUntil('\n');
    handleESP8266Data(data);
  }

  // Motor control loop
  if (millis() - lastPrintTime >= PRINT_INTERVAL) {
    lastPrintTime = millis();
  }

  if (!targetReached && wifiConnected) {
    targetReached = rotateToAngle(currentTarget);
  }
}

void handleESP8266Data(String data) {
  // Check for incoming connection request (IPD = IP data)
  if (data.indexOf("+IPD") != -1) {
    // Extract connection ID
    int commaPos = data.indexOf(",");
    int colonPos = data.indexOf(":");

    if (commaPos != -1 && colonPos != -1) {
      String connID = data.substring(5, commaPos);
      String request = data.substring(colonPos + 1);

      handleHTTPRequest(connID, request);
    }
  }
}

void handleHTTPRequest(String connID, String request) {
  Serial.print("Request: ");
  Serial.println(request);

  String response = "";

  if (request.indexOf("GET /api/position") != -1) {
    float position = getCurrentPosition();
    response = "{\"position\":" + String(position, 2) + "}";
    sendHTTPResponse(connID, "application/json", response);
  }
  else if (request.indexOf("POST /api/setAngle") != -1) {
    // Parse angle from request body
    int angleStart = request.indexOf("angle");
    if (angleStart != -1) {
      int colonPos = request.indexOf(":", angleStart);
      int bracePos = request.indexOf("}", colonPos);
      String angleStr = request.substring(colonPos + 1, bracePos);
      angleStr.trim();

      float angle = angleStr.toFloat();
      if (angle >= 0 && angle <= 360) {
        currentTarget = angle;
        targetReached = false;
        response = "{\"success\":true,\"target\":" + String(angle) + "}";
      } else {
        response = "{\"success\":false,\"error\":\"Invalid angle\"}";
      }
    }
    sendHTTPResponse(connID, "application/json", response);
  }
  else if (request.indexOf("GET /api/status") != -1) {
    response = "{\"connected\":" + String(wifiConnected ? "true" : "false") +
               ",\"currentTarget\":" + String(currentTarget, 1) +
               ",\"position\":" + String(getCurrentPosition(), 2) +
               ",\"reached\":" + (targetReached ? "true" : "false") + "}";
    sendHTTPResponse(connID, "application/json", response);
  }
  else if (request.indexOf("GET /") == 0) {
    sendHTMLPage(connID);
  }
  else {
    sendHTTPResponse(connID, "text/plain", "404 Not Found");
  }
}

void sendHTMLPage(String connID) {
  String html = R"(<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Swerve Motor Control</title>
    <style>
        * { margin: 0; padding: 0; box-sizing: border-box; }
        body { font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif; background: linear-gradient(135deg, #667eea 0%, #764ba2 100%); min-height: 100vh; display: flex; align-items: center; justify-content: center; padding: 20px; }
        .container { background: white; border-radius: 20px; box-shadow: 0 20px 60px rgba(0, 0, 0, 0.3); padding: 40px; max-width: 500px; width: 100%; }
        h1 { color: #667eea; text-align: center; margin-bottom: 10px; font-size: 28px; }
        .subtitle { text-align: center; color: #666; margin-bottom: 30px; font-size: 14px; }
        .dial { position: relative; width: 200px; height: 200px; border-radius: 50%; background: linear-gradient(135deg, #667eea 0%, #764ba2 100%); margin: 30px auto; display: flex; align-items: center; justify-content: center; box-shadow: 0 10px 30px rgba(102, 126, 234, 0.4); }
        .dial-inner { position: absolute; width: 190px; height: 190px; border-radius: 50%; background: white; display: flex; flex-direction: column; align-items: center; justify-content: center; }
        .position-text { font-size: 48px; font-weight: bold; color: #667eea; }
        .position-unit { font-size: 14px; color: #999; margin-top: 5px; }
        .pointer { position: absolute; width: 3px; height: 80px; background: #667eea; bottom: 50%; transform-origin: bottom center; border-radius: 2px; transition: transform 0.3s ease-out; }
        .control-section { background: #f8f9fa; border-radius: 15px; padding: 25px; margin-top: 30px; }
        .control-label { color: #333; font-size: 14px; font-weight: 600; margin-bottom: 10px; text-transform: uppercase; }
        input[type="range"] { width: 100%; height: 8px; border-radius: 5px; background: linear-gradient(to right, #667eea 0%, #764ba2 100%); outline: none; -webkit-appearance: none; appearance: none; }
        input[type="range"]::-webkit-slider-thumb { -webkit-appearance: none; width: 20px; height: 20px; border-radius: 50%; background: white; cursor: pointer; box-shadow: 0 2px 10px rgba(0, 0, 0, 0.2); border: 3px solid #667eea; }
        input[type="range"]::-moz-range-thumb { width: 20px; height: 20px; border-radius: 50%; background: white; cursor: pointer; box-shadow: 0 2px 10px rgba(0, 0, 0, 0.2); border: 3px solid #667eea; }
        .input-group { display: grid; grid-template-columns: 1fr 100px; gap: 10px; margin-bottom: 15px; }
        input[type="number"] { padding: 12px; border: 2px solid #e0e0e0; border-radius: 8px; font-size: 14px; }
        input[type="number"]:focus { outline: none; border-color: #667eea; }
        .button-group { display: grid; grid-template-columns: 1fr 1fr; gap: 10px; margin-top: 20px; }
        button { padding: 12px 20px; border: none; border-radius: 8px; font-size: 14px; font-weight: 600; cursor: pointer; transition: all 0.3s; }
        .btn-send { background: linear-gradient(135deg, #667eea 0%, #764ba2 100%); color: white; grid-column: 1 / -1; }
        .btn-send:hover { transform: translateY(-2px); box-shadow: 0 5px 20px rgba(102, 126, 234, 0.4); }
        .btn-preset { background: #e0e0e0; color: #333; }
        .btn-preset:hover { background: #d0d0d0; }
        .status { background: white; border-left: 4px solid #667eea; padding: 12px; border-radius: 8px; margin-top: 20px; font-size: 12px; color: #666; }
        .instructions { background: #f0f4ff; border-radius: 10px; padding: 15px; margin-top: 20px; font-size: 12px; color: #555; line-height: 1.6; }
        .instructions h3 { color: #667eea; font-size: 13px; margin-bottom: 8px; }
    </style>
</head>
<body>
    <div class="container">
        <h1>⚙️ Swerve Motor</h1>
        <p class="subtitle">M2 Single Motor - Web Control</p>
        <div class="dial">
            <div class="pointer" id="pointer"></div>
            <div class="dial-inner">
                <div class="position-text" id="positionDisplay">0</div>
                <div class="position-unit">degrees</div>
            </div>
        </div>
        <div class="control-section">
            <div class="control-label">Target Angle</div>
            <div class="input-group">
                <input type="range" id="angleSlider" min="0" max="360" value="0" step="1">
                <input type="number" id="angleInput" min="0" max="360" value="0" step="1">
            </div>
            <div class="button-group">
                <button class="btn-preset" onclick="setAngle(0)">0°</button>
                <button class="btn-preset" onclick="setAngle(90)">90°</button>
                <button class="btn-preset" onclick="setAngle(180)">180°</button>
                <button class="btn-preset" onclick="setAngle(270)">270°</button>
            </div>
            <button class="btn-send" onclick="sendAngle()">SEND TARGET</button>
            <div class="status" id="status">Connecting...</div>
        </div>
        <div class="instructions">
            <h3>📱 How to Use:</h3>
            <p><strong>1. Set Angle:</strong> Use slider or input (0-360°)</p>
            <p><strong>2. Send:</strong> Click "SEND TARGET"</p>
            <p><strong>3. Monitor:</strong> Position updates in real-time</p>
        </div>
    </div>
    <script>
        const positionDisplay = document.getElementById('positionDisplay');
        const angleSlider = document.getElementById('angleSlider');
        const angleInput = document.getElementById('angleInput');
        const pointer = document.getElementById('pointer');
        const status = document.getElementById('status');

        angleSlider.addEventListener('input', () => { angleInput.value = angleSlider.value; });
        angleInput.addEventListener('input', () => { let val = parseInt(angleInput.value) || 0; val = Math.max(0, Math.min(360, val)); angleInput.value = val; angleSlider.value = val; });

        function setAngle(deg) { angleSlider.value = deg; angleInput.value = deg; }

        function updateDisplay(deg) {
            const norm = Math.abs(deg % 360);
            positionDisplay.textContent = Math.round(norm);
            pointer.style.transform = `rotate(${(norm/360)*360}deg)`;
        }

        function getPosition() {
            fetch('/api/position').then(r => r.json()).then(d => updateDisplay(d.position)).catch(e => { status.textContent = '❌ Error'; });
        }

        function sendAngle() {
            const angle = Math.max(0, Math.min(360, parseInt(angleInput.value) || 0));
            status.textContent = '⏳ Sending...';
            fetch('/api/setAngle', { method: 'POST', headers: {'Content-Type': 'application/json'}, body: JSON.stringify({angle: angle}) })
                .then(r => r.json()).then(d => { status.textContent = '✓ Target: ' + angle + '°'; }).catch(e => { status.textContent = '❌ Error'; });
        }

        setInterval(getPosition, 100);
        getPosition();
    </script>
</body>
</html>)";

  sendHTTPResponse(connID, "text/html", html);
}

void sendHTTPResponse(String connID, String contentType, String body) {
  String httpResponse = "HTTP/1.1 200 OK\r\n";
  httpResponse += "Content-Type: " + contentType + "\r\n";
  httpResponse += "Content-Length: " + String(body.length()) + "\r\n";
  httpResponse += "Connection: close\r\n\r\n";
  httpResponse += body;

  String cipSendCmd = "AT+CIPSEND=" + connID + "," + String(httpResponse.length());
  Serial1.println(cipSendCmd);
  delay(100);
  Serial1.print(httpResponse);
  delay(100);

  String cipCloseCmd = "AT+CIPCLOSE=" + connID;
  Serial1.println(cipCloseCmd);
}

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
}
