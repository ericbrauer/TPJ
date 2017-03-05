#include <NTPClient.h>
#include "ESP8266WiFi.h"
#include <WiFiUdp.h>
#include <ArduinoJson.h>
#include <Adafruit_NeoPixel.h>
#include <TimeLib.h>
#define PIN 14
#define POT 0

// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(8, PIN, NEO_GRB + NEO_KHZ800);

const char *ssid     = "Goodsprings";
const char *password = "387iswhereweare";

const int timezone_offset = -18000;

const char* tod_host = "api.sunrise-sunset.org";

const char* my_lat = "43.7001";
const char* my_long = "-79.4163";

time_t time_next_event;

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
    //WiFi.mode(WIFI_STA);
    //WiFi.disconnect();
    //delay(2000);
    //Serial.println("Setup done");

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
    setTime(timeClient.getEpochTime());
}

void getTODRequest(const String &my_lat, const String &my_long, String &dawn_time, String &dusk_time) {
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
    url += "&formatted=0";

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
    while(client.available()) {
        line += client.readStringUntil('\r');
        if (line.endsWith("1e2"))
            line="";
        if (line.endsWith("OK\"}"))
            break;
    }
    // It's a reference to the JsonObject, the actual bytes are inside the
    // JsonBuffer with all the other nodes of the object tree.
    // Memory is freed when jsonBuffer goes out of scope.
    Serial.print(line);
    DynamicJsonBuffer jsonBuffer;
    JsonObject &root = jsonBuffer.parseObject(line);
    if (!root.success()) {
      Serial.println("parseObject() failed");
      return;
    }

    const char* ddawn_time = root["results"]["sunrise"];
    //char* x = strtok('T', ddawn_time);
    //Serial.println(x);
    String ddusk_time = root["results"]["sunset"];
    Serial.println("dawn time: ");
    Serial.println(ddawn_time);
    Serial.println("dusk time: ");
    Serial.println(ddusk_time);

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

    getTODRequest(my_lat, my_long, dawn_time, dusk_time);
    checkNTPServer();

    for(;;) {
        // Wait a bit before scanning again
        Serial.println(now());
        Serial.println(analogRead(POT));
        delay(1000);
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
