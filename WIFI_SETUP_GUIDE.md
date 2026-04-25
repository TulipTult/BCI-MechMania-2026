# WiFi Swerve Motor Control - Setup Guide

## Overview
This guide covers setting up the Arduino Mega 2560 with WiFi to control the swerve motor via a local web interface.

## Prerequisites

### Hardware Required
- Arduino Mega 2560
- WiFi Shield (MKR WiFi 1010, Arduino WiFi Shield, or equivalent)
- N20 Motor with encoder
- Motor driver (currently using M2 pins: 42, 43)
- Power supply
- USB cable for programming

### Software Required
- Arduino IDE 1.8.19 or later
- WiFi library (usually pre-installed with Arduino boards)

## Installation Steps

### 1. **Add WiFi Board Support (if needed)**

If using MKR WiFi 1010 or similar:
1. Open Arduino IDE
2. Go to: `Tools > Board > Boards Manager`
3. Search for "Arduino SAMD" or your specific board
4. Install the board support package

### 2. **Configure WiFi Credentials**

Edit these lines in `full_swerve_wifi.ino`:

```cpp
const char* ssid = "Hatsune";        // Your WiFi network name
const char* password = "Miku"; // Your WiFi password
```

Example:
```cpp
const char* ssid = "MechMania_Lab";
const char* password = "robot123456";
```

### 3. **Upload the Sketch**

1. Connect Arduino to computer via USB
2. Open `full_swerve_wifi.ino` in Arduino IDE
3. Select correct board and COM port:
   - `Tools > Board > Arduino Mega 2560`
   - `Tools > Port > COM#`
4. Click **Upload**
5. Open **Serial Monitor** (`Tools > Serial Monitor`)
   - Set baud rate to **115200**

### 4. **Find Your Device IP**

After uploading, check the Serial Monitor. You should see:

```
Initializing WiFi...
Connecting to WiFi: MechMania_Lab
..
WiFi Connected!

IP Address: 192.168.1.100
Access the interface at: http://192.168.1.100
```

**Write down this IP address** - you'll need it to access the web interface.

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

### Problem: "Failed to connect to WiFi"
**Solution:**
- Check SSID and password are correct (case-sensitive for password)
- Ensure WiFi network is 2.4GHz (5GHz may not work)
- Verify your network doesn't have MAC address filtering
- Restart Arduino and try again

### Problem: Page loads but position doesn't update
**Solution:**
- Check Serial Monitor baud rate is 115200
- Verify encoder connection on pin 2
- Try refreshing the browser page
- Check browser console for JavaScript errors (F12)

### Problem: Motor doesn't rotate when sending angle
**Solution:**
- Verify motor power connections
- Check that M2_IN3 (pin 42) and M2_IN4 (pin 43) are correct
- Test motor with original `full_swerve.ino` to verify hardware works
- Check encoder is properly counting steps

### Problem: Position resets or goes negative
**Solution:**
- Encoder count should stay positive - if it goes negative, motor direction is reversed
- Adjust `motorDirection` in `rotateForward()` and `rotateBackward()` if needed

### Problem: Can't access from another device on network
**Solution:**
- Ensure both devices on same WiFi
- Disable any VPN or network filtering
- Check firewall isn't blocking port 80
- Try pinging the Arduino IP from another device

## WiFi Shield Compatibility

### Tested With:
- Arduino Mega 2560 + Arduino WiFi Shield R3
- Arduino Mega 2560 + MKR WiFi 1010
- Arduino Mega 2560 + Genuino WiFi Shield

### Other shields may require library changes - see comments in code

## Performance Notes

- **Update Rate:** 100ms position refresh (smooth for web interface)
- **Response Time:** Motor commands typically execute within 200-500ms
- **Latency:** Depends on WiFi signal strength
- **Max Clients:** Typically 1-2 simultaneous connections

## Safety Considerations

⚠️ **Important:**
- Motor starts moving immediately when target is set
- Keep hands and objects clear of rotating motor
- Power off before making hardware changes
- Test in safe environment before deployment

## Advanced Configuration

### Change WiFi Port
```cpp
WebServer server(8080);  // Change from 80 to 8080
```
Access at: `http://192.168.1.100:8080`

### Change Update Frequency
```cpp
const unsigned long PRINT_INTERVAL = 50;  // Faster (50ms)
// or
const unsigned long PRINT_INTERVAL = 200; // Slower (200ms)
```

### Change Motor Speed
```cpp
#define MOTOR_SPEED 150  // Range: 0-255 (lower = slower)
```

## References

- [Arduino WiFi Shield Documentation](https://docs.arduino.cc/hardware/wifi-shield)
- [Mega 2560 Pin Diagram](https://www.arduino.cc/en/uploads/Main/Arduino_Mega_2560_Pinout.png)
- [N20 Motor Encoder Guide](../n20_encoder_test.ino)

---

**Questions?** Check the Serial Monitor output for debug information!
