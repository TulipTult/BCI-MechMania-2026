/* WiFi-enabled Swerve Motor Control
 * M2 Motor + Web Interface
 *
 * Motor Control Pins:
 * M2_IN3: 42, M2_IN4: 43
 *
 * Encoder Pins:
 * Encoder A: 2 (interrupt 0)
 *
 * WiFi: Requires WiFi Shield or built-in WiFi
 */

#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>

// Network Configuration
const char* ssid = "Hatsune";
const char* password = "miku";
WebServer server(80);
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
  delay(1000);

  pinMode(M2_IN3, OUTPUT);
  pinMode(M2_IN4, OUTPUT);
  pinMode(ENC_A_PIN, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(ENC_A_PIN), encoderA, RISING);

  stopMotors();
  encoderCount = 0;

  Serial.println("\n\nInitializing WiFi...");
  connectToWiFi();

  setupWebServer();
}

void connectToWiFi() {
  Serial.print("Connecting to WiFi: ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    deviceIP = WiFi.localIP().toString();
    Serial.println("\n");
    Serial.println("WiFi Connected!");
    Serial.print("IP Address: ");
    Serial.println(deviceIP);
    Serial.print("Access the interface at: http://");
    Serial.println(deviceIP);
  } else {
    Serial.println("\nFailed to connect to WiFi");
    Serial.println("Check your SSID and password in the code");
  }
}

void setupWebServer() {
  server.on("/", HTTP_GET, handleRoot);
  server.on("/api/position", HTTP_GET, handleGetPosition);
  server.on("/api/setAngle", HTTP_POST, handleSetAngle);
  server.on("/api/status", HTTP_GET, handleStatus);

  server.begin();
  Serial.println("Web server started");
}

void handleRoot() {
  String html = R"(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Swerve Motor Control</title>
    <style>
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }

        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            min-height: 100vh;
            display: flex;
            align-items: center;
            justify-content: center;
            padding: 20px;
        }

        .container {
            background: white;
            border-radius: 20px;
            box-shadow: 0 20px 60px rgba(0, 0, 0, 0.3);
            padding: 40px;
            max-width: 500px;
            width: 100%;
        }

        h1 {
            color: #667eea;
            text-align: center;
            margin-bottom: 10px;
            font-size: 28px;
        }

        .subtitle {
            text-align: center;
            color: #666;
            margin-bottom: 30px;
            font-size: 14px;
        }

        .dial-container {
            display: flex;
            justify-content: center;
            align-items: center;
            margin: 30px 0;
        }

        .dial {
            position: relative;
            width: 200px;
            height: 200px;
            border-radius: 50%;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            display: flex;
            align-items: center;
            justify-content: center;
            box-shadow: 0 10px 30px rgba(102, 126, 234, 0.4);
        }

        .dial-inner {
            position: absolute;
            width: 190px;
            height: 190px;
            border-radius: 50%;
            background: white;
            display: flex;
            flex-direction: column;
            align-items: center;
            justify-content: center;
        }

        .position-text {
            font-size: 48px;
            font-weight: bold;
            color: #667eea;
        }

        .position-unit {
            font-size: 14px;
            color: #999;
            margin-top: 5px;
        }

        .pointer {
            position: absolute;
            width: 3px;
            height: 80px;
            background: #667eea;
            bottom: 50%;
            transform-origin: bottom center;
            border-radius: 2px;
            transition: transform 0.3s ease-out;
        }

        .control-section {
            background: #f8f9fa;
            border-radius: 15px;
            padding: 25px;
            margin-top: 30px;
        }

        .control-label {
            color: #333;
            font-size: 14px;
            font-weight: 600;
            margin-bottom: 10px;
            text-transform: uppercase;
            letter-spacing: 0.5px;
        }

        .slider-container {
            position: relative;
            margin-bottom: 20px;
        }

        input[type="range"] {
            width: 100%;
            height: 8px;
            border-radius: 5px;
            background: linear-gradient(to right, #667eea 0%, #764ba2 100%);
            outline: none;
            -webkit-appearance: none;
            appearance: none;
        }

        input[type="range"]::-webkit-slider-thumb {
            -webkit-appearance: none;
            appearance: none;
            width: 20px;
            height: 20px;
            border-radius: 50%;
            background: white;
            cursor: pointer;
            box-shadow: 0 2px 10px rgba(0, 0, 0, 0.2);
            border: 3px solid #667eea;
        }

        input[type="range"]::-moz-range-thumb {
            width: 20px;
            height: 20px;
            border-radius: 50%;
            background: white;
            cursor: pointer;
            box-shadow: 0 2px 10px rgba(0, 0, 0, 0.2);
            border: 3px solid #667eea;
        }

        .input-group {
            display: grid;
            grid-template-columns: 1fr 100px;
            gap: 10px;
            margin-bottom: 15px;
        }

        input[type="number"] {
            padding: 12px;
            border: 2px solid #e0e0e0;
            border-radius: 8px;
            font-size: 14px;
            transition: border-color 0.3s;
        }

        input[type="number"]:focus {
            outline: none;
            border-color: #667eea;
        }

        .button-group {
            display: grid;
            grid-template-columns: 1fr 1fr;
            gap: 10px;
            margin-top: 20px;
        }

        button {
            padding: 12px 20px;
            border: none;
            border-radius: 8px;
            font-size: 14px;
            font-weight: 600;
            cursor: pointer;
            transition: all 0.3s;
            text-transform: uppercase;
            letter-spacing: 0.5px;
        }

        .btn-send {
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            color: white;
            grid-column: 1 / -1;
        }

        .btn-send:hover {
            transform: translateY(-2px);
            box-shadow: 0 5px 20px rgba(102, 126, 234, 0.4);
        }

        .btn-send:active {
            transform: translateY(0);
        }

        .btn-preset {
            background: #e0e0e0;
            color: #333;
        }

        .btn-preset:hover {
            background: #d0d0d0;
        }

        .status {
            background: white;
            border-left: 4px solid #667eea;
            padding: 12px;
            border-radius: 8px;
            margin-top: 20px;
            font-size: 12px;
            color: #666;
        }

        .status.active {
            border-left-color: #4caf50;
            background: #f1f8e9;
        }

        .instructions {
            background: #f0f4ff;
            border-radius: 10px;
            padding: 15px;
            margin-top: 20px;
            font-size: 12px;
            color: #555;
            line-height: 1.6;
        }

        .instructions h3 {
            color: #667eea;
            font-size: 13px;
            margin-bottom: 8px;
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>⚙️ Swerve Motor Control</h1>
        <p class="subtitle">M2 Single Motor - Web Interface</p>

        <div class="dial-container">
            <div class="dial">
                <div class="pointer" id="pointer"></div>
                <div class="dial-inner">
                    <div class="position-text" id="positionDisplay">0</div>
                    <div class="position-unit">degrees</div>
                </div>
            </div>
        </div>

        <div class="control-section">
            <div class="control-label">Set Target Angle</div>

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

            <div class="status" id="status">
                Initializing...
            </div>
        </div>

        <div class="instructions">
            <h3>📱 How to Use:</h3>
            <p><strong>1. Set Angle:</strong> Use the slider or number input (0-360°)</p>
            <p><strong>2. Send:</strong> Click "SEND TARGET" to rotate the motor</p>
            <p><strong>3. Monitor:</strong> Current position updates in real-time</p>
            <p><strong>⚠️ Note:</strong> Position always displayed 0-359°</p>
        </div>
    </div>

    <script>
        const positionDisplay = document.getElementById('positionDisplay');
        const angleSlider = document.getElementById('angleSlider');
        const angleInput = document.getElementById('angleInput');
        const pointer = document.getElementById('pointer');
        const status = document.getElementById('status');

        // Synchronize slider and input
        angleSlider.addEventListener('input', () => {
            angleInput.value = angleSlider.value;
        });

        angleInput.addEventListener('input', () => {
            let val = parseInt(angleInput.value) || 0;
            val = Math.max(0, Math.min(360, val));
            angleInput.value = val;
            angleSlider.value = val;
        });

        angleInput.addEventListener('blur', () => {
            let val = parseInt(angleInput.value) || 0;
            val = Math.max(0, Math.min(360, val));
            angleInput.value = val;
            angleSlider.value = val;
        });

        function setAngle(degrees) {
            angleSlider.value = degrees;
            angleInput.value = degrees;
        }

        function updatePositionDisplay(degrees) {
            const normalized = Math.abs(degrees % 360);
            const displayDegrees = Math.round(normalized * 100) / 100;

            positionDisplay.textContent = Math.round(displayDegrees);

            const rotation = (displayDegrees / 360) * 360;
            pointer.style.transform = `rotate(${rotation}deg)`;
        }

        function getPosition() {
            fetch('/api/position')
                .then(response => response.json())
                .then(data => {
                    updatePositionDisplay(data.position);
                })
                .catch(error => {
                    console.error('Error fetching position:', error);
                    status.textContent = '❌ Connection error';
                    status.classList.remove('active');
                });
        }

        function sendAngle() {
            const angle = parseInt(angleInput.value) || 0;
            const normalized = Math.max(0, Math.min(360, angle));

            status.textContent = '⏳ Sending...';
            status.classList.add('active');

            fetch('/api/setAngle', {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json'
                },
                body: JSON.stringify({ angle: normalized })
            })
            .then(response => response.json())
            .then(data => {
                status.textContent = '✓ Target set to ' + normalized + '°';
                status.classList.add('active');
            })
            .catch(error => {
                console.error('Error:', error);
                status.textContent = '❌ Error sending angle';
                status.classList.remove('active');
            });
        }

        // Update position every 100ms
        setInterval(getPosition, 100);
        getPosition();
    </script>
</body>
</html>
  )";

  server.send(200, "text/html", html);
}

void handleGetPosition() {
  float position = getCurrentPosition();
  String response = "{\"position\":" + String(position, 2) + "}";
  server.send(200, "application/json", response);
}

void handleSetAngle() {
  if (server.hasArg("plain")) {
    String json = server.arg("plain");

    // Simple JSON parsing for {"angle": value}
    int anglePos = json.indexOf("angle");
    if (anglePos != -1) {
      int colonPos = json.indexOf(":", anglePos);
      int commaPos = json.indexOf(",", colonPos);
      if (commaPos == -1) commaPos = json.indexOf("}", colonPos);

      String angleStr = json.substring(colonPos + 1, commaPos);
      angleStr.trim();

      float angle = angleStr.toFloat();
      if (angle >= 0 && angle <= 360) {
        currentTarget = angle;
        targetReached = false;
        server.send(200, "application/json", "{\"success\":true,\"target\":" + String(angle) + "}");
        return;
      }
    }
  }

  server.send(400, "application/json", "{\"success\":false,\"error\":\"Invalid angle\"}");
}

void handleStatus() {
  String response = "{\"connected\":true,\"currentTarget\":" + String(currentTarget, 1) +
                    ",\"position\":" + String(getCurrentPosition(), 2) +
                    ",\"reached\":" + (targetReached ? "true" : "false") + "}";
  server.send(200, "application/json", response);
}

void loop() {
  server.handleClient();

  if (millis() - lastPrintTime >= PRINT_INTERVAL) {
    lastPrintTime = millis();
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
