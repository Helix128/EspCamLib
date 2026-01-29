# EspCamLib | An easy to use "esp_camera.h" wrapper 

## Features
TODO

## Supported boards
- AI-Thinker ESP32-CAM
- TODO: add more board pinouts, if your board model isn't here its easy to add more, and the library should be compatible with any camera that works with esp_camera.h

### CameraWebServer Example

```cpp
#include <Arduino.h>
#include <EspCamLib.h>
#include <WiFi.h>

const char *ssid = "YOUR_SSID";
const char *password = "YOUR_PASSWORD";

EspCam::Camera camera;
EspCam::WebServer server(camera);

void setup()
{
    Serial.begin(115200);
    while(!Serial);

    Serial.println("EspCamLib demo");

    WiFi.begin(ssid, password);
    Serial.print("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");

    Serial.println("WiFi connected");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    Serial.println("Initializing camera...");
    camera.setPinout(EspCam::PINOUT_AI_THINKER);
    camera.setBrownout(false);
    camera.setFrameSize(FRAMESIZE_HD);
    camera.setPixelFormat(PIXFORMAT_JPEG);
    camera.setJPEGQuality(24);
    camera.setFramebufferCount(3);

    if (!camera.begin())
    {
        Serial.println("Camera init failed");
        while (1);
    }

    Serial.println("Camera initialized");

    if (!server.begin()) {
        Serial.println("WebServer init failed");
        while (1);
    }

    Serial.println("WebServer started");
    Serial.println("Open browser to http://" + WiFi.localIP().toString());

}

void loop()
{   

}

```