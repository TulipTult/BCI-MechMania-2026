#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

const char *ssid = "Hatsune";
const char *password = "miku";

ESP8266WebServer server(80);

// --- Serial to Mega ---
void sendToMega(String cmd)
{
    Serial.println(cmd);
}

String readFromMega()
{
    String res = "";
    unsigned long t = millis();

    while (millis() - t < 200)
    {
        while (Serial.available())
        {
            char c = Serial.read();
            if (c == '\n')
                return res;
            res += c;
        }
    }
    return res;
}

// --- API ---
void handlePosition()
{
    sendToMega("GET\n");
    String pos = readFromMega();
    if (pos == "")
        pos = "0";

    server.send(200, "application/json", "{\"position\":" + pos + "}");
}

void handleSetAngle()
{
    String body = server.arg("plain");

    int i = body.indexOf("angle");
    int c = body.indexOf(":", i);
    int e = body.indexOf("}", c);

    float angle = body.substring(c + 1, e).toFloat();

    sendToMega("SET:" + String(angle) + "\n");

    server.send(200, "application/json", "{\"success\":true}");
}

// --- HTML ---
const char *html = R"rawliteral(<!DOCTYPE html>
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
</html>)rawliteral";


void handleRoot() {
  server.send(200, "text/html", html);
}

// --- Setup ---
void setup() {
  Serial.begin(115200);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) delay(500);

  Serial.println(WiFi.localIP());

  server.on("/", handleRoot);
  server.on("/api/position", handlePosition);
  server.on("/api/setAngle", HTTP_POST, handleSetAngle);

  server.begin();
}

// --- Loop ---
void loop() {
  server.handleClient();
}