#include <NTPClient.h>
#include "ESP8266WiFi.h"
#include <WiFiUdp.h>
#include <ArduinoJson.h>

#include <Adafruit_NeoPixel.h>
#define PIN 14 //D5? gpio 14

// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(8, PIN, NEO_GRB + NEO_KHZ800);

const char *ssid     = "Goodsprings";
const char *password = "387iswhereweare";

const int timezone_offset = -18000;

const char* tod_host = "api.sunrise-sunset.org";

const char* my_lat = "36.7201600";
const char* my_long = "-4.4203400";

void theaterChase(uint32_t c, uint8_t wait);
void colorWipe(uint32_t c, uint8_t wait);

WiFiUDP ntpUDP;
WiFiClient client;

// You can specify the time server pool and the offset (in seconds, can be
// changed later with setTimeOffset() ). Additionaly you can specify the
// update interval (in milliseconds, can be changed using setUpdateInterval() ).
NTPClient timeClient(ntpUDP, "ca.pool.ntp.org", timezone_offset, 60000);


void setup() {
  Serial.begin(9600);
  // Set WiFi to station mode and disconnect from an AP if it was previously connected
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(2000);
  Serial.println("Setup done");

  WiFi.begin(ssid, password);
  while ( WiFi.status() != WL_CONNECTED ) {
    delay ( 500 );
    Serial.print ( "." );
  }

  timeClient.begin();

  strip.begin();
  strip.show(); // Initialize all pixels to 'off'


}

void printScannedNetworks() {
    Serial.println("scan start");
    int n = WiFi.scanNetworks();// WiFi.scanNetworks will return the number of networks found
    Serial.println("scan done");
    if (n == 0)
      Serial.println("no networks found");
    else
    {
      Serial.print(n);
      Serial.println(" networks found");
      for (int i = 0; i < n; ++i)
      {
        // Print SSID and RSSI for each network found
        Serial.print(i + 1);
        Serial.print(": ");
        Serial.print(WiFi.SSID(i));
        Serial.print(" (");
        Serial.print(WiFi.RSSI(i));
        Serial.print(")");
        Serial.println((WiFi.encryptionType(i) == ENC_TYPE_NONE)?" ":"*");
        delay(10);
      }
    }
    Serial.println("");
}

void checkNTPServer() {
    timeClient.update();

  Serial.println(timeClient.getFormattedTime());
}

void getRequest(const String &my_lat, const String &my_long, String &dawn_time, String &dusk_time) {
    // Memory pool for JSON object tree.
    //
    // Inside the brackets, 200 is the size of the pool in bytes,
    // If the JSON object is more complex, you need to increase that value.
    //StaticJsonBuffer<2000> jsonBuffer;

    String line;

    const int httpPort = 80;
    if (!client.connect(tod_host, httpPort)) {
      Serial.println("connection failed");
      return;
    }
    // We now create a URI for the request
    String url = "/json?lat=";
    url += my_lat;
    url += "&lng=";
    url += my_long;
    url += "&date=today";

    Serial.print("Requesting URL: ");
    Serial.println(url);

    // This will send the request to the server
    client.print(String("GET ") + url + " HTTP/1.1\r\n" +
                 "Host: " + tod_host + "\r\n" +
                 "Connection: close\r\n\r\n");
    unsigned long timeout = millis();
    while (client.available() == 0) {
      if (millis() - timeout > 5000) {
        Serial.println(">>> Client Timeout !");
        client.stop();
        return;
      }
    }
    // Read all the lines of the reply from server and print them to Serial
    while(client.available()){
      line += client.readStringUntil('\r');
      //Serial.print(line);
  }
    DynamicJsonBuffer jsonBuffer;
    // Root of the object tree.
    //
    // It's a reference to the JsonObject, the actual bytes are inside the
    // JsonBuffer with all the other nodes of the object tree.
    // Memory is freed when jsonBuffer goes out of scope.
    Serial.print(line);
    String line2 = "{\"results\": {\"sunrise\": \"7:18:02 AM\",\"sunset\": \"5:44:52 PM\",\"solar_noon\": \"12:31:27 PM\",\"day_length\": \"10:26:50\",\"civil_twilight_begin\": \"6:51:00 AM\",\"civil_twilight_end\": \"6:11:54 PM\",\"nautical_twilight_begin\": \"6:20:09 AM\",\"nautical_twilight_end\": \"6:42:45 PM\",\"astronomical_twilight_begin\": \"5:49:46 AM\",\"astronomical_twilight_end\": \"7:13:08 PM\"},\"status\": \"OK\"}";
    JsonObject &root = jsonBuffer.parseObject(line); //TODO remove the header!

  // Test if parsing succeeds.
  if (!root.success()) {
    Serial.println("parseObject() failed");
    return;
  }

    String first = root["results"]["sunrise"];
    //dusk_time = root["results"]["sunset"];
    Serial.println(first);



    Serial.println("closing connection");
}

void loop() {
  printScannedNetworks();
  String dawn_time;
  String dusk_time;

  // Some example procedures showing how to display to the pixels:
  colorWipe(strip.Color(0, 0, 2), 5); // Red
  delay(2000);
  colorWipe(strip.Color(5, 0, 15), 5);
  delay(2000);
  colorWipe(strip.Color(127, 127, 192), 5);
  delay(2000);
  colorWipe(strip.Color(1, 1, 1), 5);
  delay(2000);
  //colorWipe(strip.Color(0, 255, 0), 50); // Green
  //colorWipe(strip.Color(0, 0, 255), 50); // Blue
//colorWipe(strip.Color(0, 0, 0, 255), 50); // White RGBW
  // Send a theater pixel chase in...
  //theaterChase(strip.Color(127, 127, 127), 50); // White

  //getRequest(my_lat, my_long, dawn_time, dusk_time);
  //for(;;) {
      //checkNTPServer();
      // Wait a bit before scanning again
      //delay(1000);


  //}
}

//Theatre-style crawling lights.
void theaterChase(uint32_t c, uint8_t wait) {
  for (int j=0; j<10; j++) {  //do 10 cycles of chasing
    for (int q=0; q < 3; q++) {
      for (uint16_t i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, c);    //turn every third pixel on
      }
      strip.show();

      delay(wait);

      for (uint16_t i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, 0);        //turn every third pixel off
      }
    }
  }
}

// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
    strip.show();
    delay(wait);
  }
}
