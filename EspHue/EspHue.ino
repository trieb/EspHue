 #include <ESP8266WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include <Adafruit_NeoPixel.h>
#include "config.h"

// NeoPixel 
#define PIN            2
#define NUMPIXELS      5

/* Define pixel type flags, add together as needed:
 *  
 * NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
 * NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
 * NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
 * NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
 * NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)
*/
#define PIXEL_TYPE NEO_RGB + NEO_KHZ800

// Wifi - uncomment or define in config.h
//#define WLAN_SSID   "SSID"
//#define WLAN_PASS   "password"

// MQTT broker - uncomment or define in config.h
//#define MQTT_SERVER      "192.168.1.5"
//#define MQTT_PORT        1883   // use 8883 for SSL
//#define MQTT_USERNAME    "EspHue"
//#define MQTT_PASSWORD    ""

#define MAX_TOPIC_LENGTH 50
#define MQTT_BASE_TOPIC "esphue"

struct RGB {
  int r;
  int g;
  int b;
};

static int DEVICE_ID = ESP.getChipId();

// Function declarations
const char* buildTopic(char* topic, const String sub_topic);

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, PIXEL_TYPE);

WiFiClient client;
//WiFiClientSecure client;
Adafruit_MQTT_Client mqtt(&client, MQTT_SERVER, MQTT_PORT, MQTT_USERNAME, MQTT_PASSWORD);

// Setup feeds 
char topic_pub_status[MAX_TOPIC_LENGTH];
Adafruit_MQTT_Publish pub_status = Adafruit_MQTT_Publish(&mqtt, buildTopic(topic_pub_status, "/status"));

// Setup a feed called 'sub_color' for subscribing to changes.
Adafruit_MQTT_Subscribe sub_hello = Adafruit_MQTT_Subscribe(&mqtt, MQTT_BASE_TOPIC "/hello");
char topic_sub_color[MAX_TOPIC_LENGTH];
Adafruit_MQTT_Subscribe sub_color = Adafruit_MQTT_Subscribe(&mqtt, buildTopic(topic_sub_color, "/color"));
char topic_sub_color_rgb[MAX_TOPIC_LENGTH];
Adafruit_MQTT_Subscribe sub_color_rgb = Adafruit_MQTT_Subscribe(&mqtt, buildTopic(topic_sub_color_rgb, "/color/rgb"));

void MQTT_connect();

int delayval = 500; // delay for half a second

void setup() {
  pixels.begin(); // This initializes the NeoPixel library.
  
  Serial.begin(115200);
  delay(10);
  Serial.printf("\nESP8266 Chip id = %08X (%d)\n", DEVICE_ID, DEVICE_ID);

  // Connect to WiFi access point.
  Serial.printf("\n\nConnecting to %s\n", WLAN_SSID);

  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  Serial.println("WiFi connected");
  Serial.print("IP address: "); Serial.println(WiFi.localIP());

  // Setup MQTT subscriptions
  mqtt.subscribe(&sub_hello);
  mqtt.subscribe(&sub_color);
  mqtt.subscribe(&sub_color_rgb);
  pub_status.publish("hello");
}

void loop() {
  MQTT_connect();

  Adafruit_MQTT_Subscribe *subscription;
  while ((subscription = mqtt.readSubscription(5000))) {
    if (subscription == &sub_hello) {
      Serial.println(F("Got: Hello"));
      pub_status.publish("hello");
    }
    else if (subscription == &sub_color) {
      Serial.print(F("Got: "));
      Serial.println((char *)sub_color.lastread);
      char *value = (char *)sub_color.lastread;
      setColor(value);
    }
    else if (subscription == &sub_color_rgb) {
      Serial.print(F("Got: "));
      Serial.println((char *)sub_color_rgb.lastread);
      char *value = (char *)sub_color_rgb.lastread;
      setColorRgb(value);
    }
  }
}

const char* buildTopic(char* topic, const String sub_topic){
  char charBuf[MAX_TOPIC_LENGTH];
  String base_topic  = String(MQTT_BASE_TOPIC);
  String device_id   = String(DEVICE_ID);
  String temp_topic  = String(base_topic + "/" + device_id + sub_topic);

  memset(charBuf, 0, MAX_TOPIC_LENGTH);
  temp_topic.toCharArray(charBuf, MAX_TOPIC_LENGTH);
 
  strncpy(topic, charBuf, MAX_TOPIC_LENGTH);
  return topic;
}

// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i < pixels.numPixels(); i++) {
    pixels.setPixelColor(i, c);
    pixels.show();
    delay(wait);
  }
}

void allOff() {
  colorWipe(pixels.Color(0, 0, 0), 20);
}

void setColor(char *value)
{
  if (strcmp(value, "red") == 0) {
    Serial.println(F("Received command RED."));
    colorWipe(pixels.Color(255, 0, 0), 100); // Red
  }
  else if (strcmp(value, "green") == 0) {
    Serial.println(F("Received command GREEN."));
    colorWipe(pixels.Color(0, 255, 0), 100); // Green 
  }
  else if (strcmp(value, "blue") == 0) {
    Serial.println(F("Received command BLUE."));
    colorWipe(pixels.Color(0, 0, 255), 100); // Blue
  }
  else if (strcmp(value, "purple") == 0) {
    Serial.println(F("Received command BLUE."));
    colorWipe(pixels.Color(128, 0, 128), 100); // Blue
  }
  else if (strcmp(value, "magenta") == 0) {
    Serial.println(F("Received command BLUE."));
    colorWipe(pixels.Color(255, 0, 255), 100); // Blue
  }
  else if (strcmp(value, "yellow") == 0) {
    Serial.println(F("Received command BLUE."));
    colorWipe(pixels.Color(255, 255, 0), 100); // Blue
  }
  else if (strcmp(value, "cyan") == 0) {
    Serial.println(F("Received command BLUE."));
    colorWipe(pixels.Color(0, 255, 255), 100); // Blue
  }
  else if (strcmp(value, "off") == 0) {
    Serial.println(F("Received command OFF."));
    allOff();
  }
  else {
    Serial.println(F("Received unknown color."));
  }
}

void sendStatusWithRgbValue(RGB rgbData)
{
  String rVal = String(rgbData.r);
  String comma1 = ",";
  String gVal = String(rgbData.g);
  String comma2 = ",";
  String bVal = String(rgbData.b);
  String status_message = rVal + comma1 + gVal + comma2 + bVal;
  pub_status.publish(status_message.c_str());
}


void setColorRgb(char *rgb_string)
{
  int r,g,b = 0;
  RGB rgbData = {0};
  if(parseRgbString(rgb_string, &rgbData) == 3){
    colorWipe(pixels.Color(rgbData.r, rgbData.g, rgbData.b), 100);  
  }
  else{
    Serial.println(F("Rgb-color must be in the format of (r,g,b)"));
  }
  
  sendStatusWithRgbValue(rgbData);
}

/*
 * This function is used since sscanf is lacking
  int r,g,b = 0;
  int n = sscanf(rgb_buf, "%d,%d,%d", &r, &g, &b)
*/
int parseRgbString(char* rgb_string, RGB* out)
{
  String rgb = String(rgb_string);
  Serial.printf("Parsing rgb-string: %s\n", rgb_string);
  
  int firstCommaIndex = rgb.indexOf(',');
  int secondCommaIndex = rgb.indexOf(',', firstCommaIndex+1);
  
  if((firstCommaIndex != -1) && (secondCommaIndex != -1)){
    String firstValue = rgb.substring(0, firstCommaIndex);
    String secondValue = rgb.substring(firstCommaIndex+1, secondCommaIndex);
    String thirdValue = rgb.substring(secondCommaIndex+1);  
    
    out->r = firstValue.toInt();
    out->g = secondValue.toInt();
    out->b = thirdValue.toInt();
    Serial.printf("  (r,g,b) = (%d,%d,%d)\n", out->r, out->g, out->b);
    return 3;
  }
  else {
    Serial.println("ERROR: Could not parse data!");
  } 
  return -1;
}


void handleCommand(char *value)
{
  if (strcmp(value, "red") == 0) {
    Serial.println(F("Received command RED."));
    colorWipe(pixels.Color(255, 0, 0), 100); // Red
  }
  else if (strcmp(value, "green") == 0) {
    Serial.println(F("Received command GREEN."));
    colorWipe(pixels.Color(0, 255, 0), 100); // Green 
  }
  else if (strcmp(value, "blue") == 0) {
    Serial.println(F("Received command BLUE."));
    colorWipe(pixels.Color(0, 0, 255), 100); // Blue
  }
  else if (strcmp(value, "off") == 0) {
    Serial.println(F("Received command OFF."));
    allOff();
  }
  sendStatus();
}

void sendStatus(void)
{
  // Now we can publish stuff!
  Serial.print(F("\nSending status "));
  Serial.print("...");
  if (! pub_status.publish("ok")) {
    Serial.println(F("Failed"));
  } else {
    Serial.println(F("OK!"));
  }
}

/*
 * Function to connect and reconnect as necessary to the MQTT server.
 * Should be called in the loop function and it will take care if connecting.
 */
void MQTT_connect() {
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }

  Serial.print("Connecting to MQTT... ");

  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
       Serial.println(mqtt.connectErrorString(ret));
       Serial.println("Retrying MQTT connection in 5 seconds...");
       mqtt.disconnect();
       delay(5000);  // wait 5 seconds
       retries--;
       if (retries == 0) {
         // basically die and wait for WDT to reset me
         while (1);
       }
  }
  Serial.println("MQTT Connected!");
}
