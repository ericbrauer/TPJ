#include <NTPClient.h>
#include "ESP8266WiFi.h"
#include <WiFiUdp.h>
#include <ArduinoJson.h>
#include <Adafruit_NeoPixel.h>
#include <TimeLib.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager WiFi Configuration Magic

#include <led.h>
#include <request.h>

#define TRANSITIONTIME 1800 //30 minutes x 60 seconds.
#define NUMSTEPS 255 //how many steps in led transition



ESP8266WebServer server(80);
MDNSResponder mdns;

const char *ssid     = "Goodsprings";
const char *password = "387iswhereweare";

const int timezone_offset = -14400; //dst
//const int timezone_offset = -18000;

const char* tod_host = "api.openweathermap.org";
const char* api_key = "ab33624b9ab307c2d056de2359eaedf5";
const char* my_city = "toronto";
const char* my_country = "ca";

String webPage = "";

//const char* my_lat = "43.7001";
//const char* my_long = "-79.4163";

time_t next_dawn;
time_t next_dusk;



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

    // THIS WORKS, but some questions about reconnecting...
    WiFiManager wifiManager;
    wifiManager.autoConnect("DIGITAL SKYLIGHT");


    //WiFi.begin(ssid, password);
    //while ( WiFi.status() != WL_CONNECTED ) {
    //    delay ( 500 );
    //    Serial.print ( "." );
    //}

    timeClient.begin();
    ledInit();

    server.on("/", handleRoot);

    server.on("/submit", handleSubmit);
    server.on("/demo", handleDemo);

    if (mdns.begin("skylight", WiFi.localIP())) {
    Serial.println("MDNS responder started");
    }

    server.onNotFound(handleNotFound);

    server.begin();
    Serial.println("HTTP server started");

    Serial.println("");
    Serial.print("Connected to ");
    Serial.println(ssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    attachInterrupt(POT, changeBrightness, CHANGE);
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

void getTODRequest() {
    String line;
    const int httpPort = 80;

    if (!client.connect(tod_host, httpPort)) {
        Serial.println("connection failed");
        return;
    }

    String url = "/data/2.5/weather?q=";
    url += my_city;
    url += ",";
    url += my_country;
    url += "&APPID=";
    url += api_key;

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



    Serial.println("request sent");
    while (client.connected()) {
        String line = client.readStringUntil('\n');
        if (line == "\r") {
            Serial.println("headers received");
            break;
        }
    }

    line = client.readStringUntil('\n');
    Serial.print(line);

    DynamicJsonBuffer jsonBuffer;
    JsonObject &root = jsonBuffer.parseObject(line);
    if (!root.success()) {
      Serial.println("parseObject() failed");
      return;
    }

    setTime(root["dt"]);
    next_dawn = root["sys"]["sunrise"];
    next_dusk = root["sys"]["sunset"];
    Serial.println("dawn time: ");
    Serial.println(next_dawn);
    Serial.println("dusk time: ");
    Serial.println(next_dusk);

    Serial.println("closing connection");
}

void loop() {
    //printScannedNetworks();

  // Some example procedures showing how to display to the pixels:
    //colorWipe(strip.Color(0, 0, 2), 5); // Red
    //delay(2000);


    getTODRequest();
    //checkNTPServer();



    for(;;) {
        // Wait a bit before scanning again
        server.handleClient();
        skyTransition1(20);
        skyTransition2(20);
        skyTransition3(20);
        Serial.println(now());
        //strip.setBrightness((analogRead(POT)>>4));
        //strip.show();
        //skySim(strip.Color(0, 0, 255), strip.Color(127, 127, 0));
        delay(500);
        changeBrightness();
    }
}

void transitionControl() {
    /*this is the main deal. check the present versus next transition. */
    static int counter;
    static time_t next_event; //? disregard
    // if
}
