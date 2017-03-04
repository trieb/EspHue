# EspHue

## Usage

1. Edit config.h (add ssid & password)
2. Flash the sketch to the ESP8266 (or any Arduino)
3. Send a mqtt message with the topic *esphue/hello* 
  * The EspHue-device will answer with *esphue/device-id/status*
  
To set a color

**topic**: *esphue/<device-id>/color/rgb*
**message**: *red-value,green-value,blue-value* 