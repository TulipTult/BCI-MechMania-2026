# WiFi Swerve Motor Control - Setup Guide

## Overview
This guide covers setting up the **Mega2560 + WiFi R3** (ATmega2560 + ESP8266) to control the swerve motor via a local web interface.

## Prerequisites

### Hardware Required
- **Mega2560 + WiFi R3** board (ATmega2560 + ESP8266 32MB integrated)
- N20 Motor with encoder
- Motor driver (currently using M2 pins: 42, 43)
- Power supply (6-12V recommended)
- USB cable for programming (CH340G driver compatible)

### Software Required
- Arduino IDE 1.8.19 or later
- **Board support:** Arduino AVR Boards (should be pre-installed)
- **No additional WiFi libraries needed** (uses AT commands to ESP8266)

## Installation Steps

### 1. **Verify Board Selection**

1. Open Arduino IDE
2. Go to: `Tools > Board`
3. Select: **Arduino Mega 2560**
4. Go to: `Tools > Port` and select your COM port
5. Verify the port shows "(CH340)" - this confirms your board is detected

### 2. **Configure WiFi Credentials**

Edit these lines in `full_swerve_wifi.ino` (lines 18-19):

```cpp
const char* ssid = "Hatsune";        // Your WiFi network name
const char* password = "miku";       // Your WiFi password
```

**Note:** Password is case-sensitive!

Example:
```cpp
const char* ssid = "MechMania_Lab";
const char* password = "robot123456";
```

### 3. **Upload the Sketch**

1. Connect Mega2560 + WiFi R3 to computer via USB
2. Open `full_swerve_wifi.ino` in Arduino IDE
3. Verify board and port settings:
   - `Tools > Board > Arduino Mega 2560`
   - `Tools > Port > COM#` (CH340 device)
4. Click **Upload**
5. **Wait 10 seconds** for upload to complete (ESP8266 needs time to reboot after upload)
6. Open **Serial Monitor** (`Tools > Serial Monitor`)
   - Set baud rate to **115200**

### 4. **Monitor the Initialization**

In Serial Monitor, you should see:

```
=== Swerve Motor Control - WiFi Setup ===
Board: Mega2560 + WiFi R3 (ESP8266)

Initializing ESP8266...
Board: Mega2560 + WiFi R3 (ESP8266)

Firmware:
AT+GMR
AT+CWMODE=1
Connecting to: Hatsune
...
✓ WiFi Connected!
IP Address: 192.168.1.100
Access web interface at: http://192.168.1.100
HTTP Server started on port 80
```

**Write down this IP address** - you'll need it to access the web interface.

### 5. **Wait for Connection**

- ESP8266 may take **5-15 seconds** to connect to WiFi
- Watch Serial Monitor for the "✓ WiFi Connected!" message
- Once you see the IP address, you're ready to use the interface

## How to Access the Web Interface

### From the Same Network

1. Open a web browser (Chrome, Firefox, Safari, Edge)
2. Type the IP address in the address bar:
   ```
   http://192.168.1.100
   ```
   (Replace with your device's actual IP)

3. You should see the **Swerve Motor Control** dashboard

### From Mobile Devices

- Connect your phone/tablet to the same WiFi network
- Open browser and enter the IP address
- Full control from mobile is supported

### If Connection Fails

**Check:**
- ✓ Phone/computer on same WiFi network as Arduino
- ✓ Serial Monitor shows "WiFi Connected"
- ✓ IP address is correct
- ✓ Try refreshing the page
- ✓ Restart the Arduino (press reset button)

## Using the Web Interface

### Dashboard Elements

```
┌─────────────────────────────────┐
│     ⚙️ Swerve Motor Control     │
│   M2 Single Motor - Web Interface│
│                                  │
│         [POSITION DIAL]          │
│           Current: 45°           │
│                                  │
│  Set Target Angle: [====|======] │
│  Input: [  90  ] [Send Target]   │
│                                  │
│  [0°]  [90°]  [180°]  [270°]   │
│                                  │
│  Status: ✓ Ready                │
└─────────────────────────────────┘
```

### Controls Explained

| Element | Function |
|---------|----------|
| **Position Dial** | Shows current wheel position in real-time (0-359°) |
| **Slider** | Drag to select target angle (0-360°) |
| **Number Input** | Type exact angle value |
| **Preset Buttons** | Quick selection: 0°, 90°, 180°, 270° |
| **Send Target** | Send command and start rotation |
| **Status Line** | Shows connection status and current command |

### Step-by-Step Usage

1. **Open the page** → `http://YOUR_IP_ADDRESS`

2. **Set target angle** - Choose one method:
   - Drag the slider
   - Type in the number input (0-360)
   - Click a preset button (0°, 90°, 180°, 270°)

3. **Send Command** - Click **SEND TARGET** button

4. **Monitor Progress**:
   - Dial rotates to show current position
   - Status updates with feedback
   - Motor moves in real-time

5. **Position Guarantee** - Position always displays 0-359°, never exceeds 360°

## Technical Details

### API Endpoints

If you want to control via code/scripts, these endpoints are available:

#### Get Current Position
```
GET /api/position
Response: {"position": 45.25}
```

#### Set Target Angle
```
POST /api/setAngle
Content-Type: application/json
Body: {"angle": 90}
Response: {"success": true, "target": 90}
```

#### Get Status
```
GET /api/status
Response: {
  "connected": true,
  "currentTarget": 90,
  "position": 45.25,
  "reached": false
}
```

### Update Frequency
- Position updates: **100ms** (10 times per second)
- Web interface polls position continuously
- Motor control loop runs continuously

## Troubleshooting

### Problem: "No valid SSID response from ESP8266"
**Solution:**
- Wait 15-20 seconds after upload for ESP8266 to boot properly
- Press the reset button on the board
- Check Serial Monitor output is 115200 baud
- Verify SSID and password are correct (password is case-sensitive)

### Problem: "AT commands not recognized" or garbled output
**Solution:**
- ESP8266 baud rate may be wrong - Serial Monitor should show familiar output
- If garbled, try a different baud rate in Serial Monitor (74880 or 9600)
- Or press reset button and reopen Serial Monitor

### Problem: "Failed to connect to WiFi"
**Solution:**
- Verify WiFi network exists and you can connect from phone
- Check network name (SSID) is spelled correctly - case-sensitive!
- Ensure password is correct
- Some networks block IoT devices - try a different 2.4GHz network
- Restart both the board and WiFi router

### Problem: Page loads but position doesn't update
**Solution:**
- Check Serial Monitor baud rate is still 115200
- Verify encoder connection on pin 2
- Try refreshing browser with Ctrl+F5 (hard refresh)
- Check browser console for errors (press F12)

### Problem: Motor doesn't rotate when sending angle
**Solution:**
- Verify motor power connections (6-12V)
- Check M2_IN3 (pin 42) and M2_IN4 (pin 43) connections
- Test motor with original `full_swerve.ino` to verify hardware works
- Verify encoder is counting (check raw step counts in motor test sketch)

### Problem: Can't access webpage from phone/computer on network
**Solution:**
- Ensure phone/computer on **same WiFi network** as ESP8266
- Try the IP address from another device on the network
- Check if network requires login (captive portal)
- Try connecting to 5GHz network if available (may not work - try 2.4GHz)
- Disable VPN on client device
- Check firewall isn't blocking port 80

## Board Information

### Mega2560 + WiFi R3 Specifications

- **Main Processor:** ATmega2560 (standard Arduino Mega)
- **WiFi Chip:** ESP8266
- **Flash Memory:** 32MB on ESP8266
- **Communication:** Serial1 (pins 18/19) between Mega and ESP8266
- **USB Interface:** CH340G (Windows driver may need installation)
- **Operating Voltage:** 5V (USB) with 6-12V external power option

### How It Works

The Mega2560 and ESP8266 communicate via UART (Serial1):
- **Mega2560:** Controls motor, reads encoder, sends AT commands to ESP8266
- **ESP8266:** Handles WiFi connection, serves web pages, receives/sends HTTP traffic
- **Connection:** Mega → RX1 (pin 19) and TX1 (pin 18) ← ESP8266 at 115200 baud

## API Endpoints

If you want to control via code/scripts or mobile apps, these endpoints are available:

### Get Current Position
```
GET /api/position
Response: {"position": 45.25}
```

### Set Target Angle
```
POST /api/setAngle
Content-Type: application/json
Body: {"angle": 90}
Response: {"success": true, "target": 90}
```

### Get Status
```
GET /api/status
Response: {
  "connected": true,
  "currentTarget": 90,
  "position": 45.25,
  "reached": false
}
```

## Performance Notes

- **Update Rate:** 100ms position refresh (10 updates/second)
- **Response Time:** Motor commands execute within 200-500ms
- **WiFi Latency:** Typically 20-100ms (depends on signal)
- **Max Clients:** Up to 4 simultaneous connections

## Safety Considerations

⚠️ **Important:**
- Motor starts moving immediately when target is sent
- Keep hands and objects clear of rotating motor
- Always test in safe environment before deployment
- Power off before making hardware changes
- Don't exceed 360° - motor will rotate to target via shortest path

## Advanced Configuration

### Change Motor Speed (0-255)
```cpp
#define MOTOR_SPEED 150  // Lower = slower, higher = faster
```

### Adjust Position Update Rate
```cpp
const unsigned long PRINT_INTERVAL = 50;  // Faster (50ms)
```

### Change WiFi Settings at Runtime
Currently settings are fixed in code. To change WiFi without re-uploading:
1. Edit SSID and password at top of sketch
2. Re-upload code
3. Check Serial Monitor for new IP

## Installation Checklist

- [ ] Board selected: Arduino Mega 2560
- [ ] Port selected: COM# with CH340
- [ ] WiFi SSID entered correctly
- [ ] WiFi password entered correctly
- [ ] Sketch uploaded successfully
- [ ] Serial Monitor shows 115200 baud
- [ ] "WiFi Connected!" appears in Serial Monitor
- [ ] IP address displayed in Serial Monitor
- [ ] Can access webpage at that IP from browser
- [ ] Motor and encoder connections verified
- [ ] Motor test successful

## Questions or Issues?

1. Check **Serial Monitor output** for detailed debug info
2. Press **Reset button** on board if page won't load
3. Try **refreshing** browser page (Ctrl+F5)
4. Verify **WiFi network is 2.4GHz** (not 5GHz)
5. Make sure phone/computer on **same network** as board

---

**Last Updated:** 2026-04-25
**Board:** Mega2560 + WiFi R3 (ATmega2560 + ESP8266)
**Sketch:** full_swerve_wifi.ino

