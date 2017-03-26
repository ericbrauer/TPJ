#include <NTPClient.h>
#include "ESP8266WiFi.h"
#include <WiFiUdp.h>
#include <ArduinoJson.h>
#include <Adafruit_NeoPixel.h>
#include <TimeLib.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager WiFi Configuration Magic

#define PIN 14
#define POT 0
#define TRANSITIONTIME 3600 //1 hour to get from night to day.
#define HALFTRANSITIION TRANSITIONTIME/2
enum {yellow, red, green};
enum {NOTIME, OLDTIMES, DAWNORDUSK, WAITFORDUSK, WAITFORDAWN, DUSK, DAWN};

// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(8, PIN, NEO_GRB + NEO_KHZ800);

ESP8266WebServer server(80);
MDNSResponder mdns;

unsigned char statusColor;
unsigned char STATE;
//const char *ssid     = "Goodsprings";
//const char *password = "387iswhereweare";

const int timezone_offset = -14400; //dst
//const int timezone_offset = -18000;

const char* tod_host = "api.openweathermap.org";
const char* api_key = "ab33624b9ab307c2d056de2359eaedf5";
const char* my_city = "toronto";
const char* my_country = "ca";

String webPage = "";

//const char* my_lat = "43.7001";
//const char* my_long = "-79.4163";

time_t dawn_time;
time_t dusk_time;

void colorWipe(uint32_t c, uint8_t wait);
void skySim(uint32_t outer, uint32_t inner);
void skyTransition(int wait);
void skyState1(unsigned char i);
void skyState2(unsigned char i);
void skyState3(unsigned char i);
void statusLight(unsigned char statusColor);
void handleUserInputError();
void handleSubmit();
void handleDemo();
void handleAck();
void parseSunrise(int hour, int minute);
void parseSunset(int hour, int minute);
void changeBrightness();
short int calculateStateOut(int x);

WiFiUDP ntpUDP;
WiFiClient client;

// You can specify the time server pool and the offset (in seconds, can be
// changed later with setTimeOffset() ). Additionaly you can specify the
// update interval (in milliseconds, can be changed using setUpdateInterval() ).
NTPClient timeClient(ntpUDP, "ca.pool.ntp.org", timezone_offset, 60000);

void handleRoot() {
    String webPage = "";
    webPage += "<h1>Skylight</h1>";
    webPage += "<p>Connected To</p>";
    webPage += "<p>Time is now</p>";
    webPage += "<h3>Set Sunrise:</h3>";
    webPage += "<form action='submit' method='POST'>";
    webPage += "<p>Hour: <input type='text' name='rise_hour' maxlength='2' style='width:50px;'>";
    webPage += "Minute:  <input type='text' name='rise_min' maxlength='2' style='width:50px;'>";
    webPage += "AM  <input type='submit' value='Save'></form> ";
    webPage += "<h3>Set Sunset:</h3> ";
    webPage += "<form action='submit' method='POST'>";
    webPage += "<p>Hour: <input type='text' name='set_hour' maxlength='2' style='width:50px;'>";
    webPage += "Minute:  <input type='text' name='set_min' maxlength='2' style='width:50px;'>";
    webPage += "PM  <input type='submit' value='Save'></form>";
    webPage += "<p><a href='demo'><button style='width:100%;'>Run a Demo</button></a>&nbsp;</p>";
    server.send(200, "text/html", webPage);
}

void handleSubmit() {
    char sunset_flag = 0;
    int x;
    int hour = 0;
    int minute = 0;
    if (server.args() > 0 ) {
        for ( uint8_t i = 0; i < server.args(); i++ ) {
            Serial.println(server.argName(i));
            Serial.println(server.arg(i));
            x = server.arg(i).toInt();
            //if (!isNumber(x)) {
            //    handleUserInputError();
            //    return;
            //}
            if (server.argName(i) == "rise_hour") {
                if ((x > 12) || (x < 1)) {
                    handleUserInputError();
                    return;
                }
                else {
                    hour = x;
                }
            }
            if (server.argName(i) == "set_hour") {
                if ((x > 12) || (x < 1)) {
                    handleUserInputError();
                    return;
                }
                else {
                    hour = x;
                    sunset_flag = 1;
                }
            }
            if (server.argName(i) == "rise_min") {
                if ((x > 59) || (x < 0)) {
                    handleUserInputError();
                    return;
                }
                else {
                    minute = x;
                }
            }
            if (server.argName(i) == "set_min") {
                if ((x > 59) || (x < 0)) {
                    handleUserInputError();
                    return;
                }
                else {
                    minute = x;
                }
            }
        }
    }
    if (sunset_flag)
        parseSunset(hour, minute);
    else
        parseSunrise(hour, minute);
    handleAck();
}

void parseSunrise(int hour, int minute) {
    TimeElements tm;
    breakTime(now(), tm);
    Serial.println(tm.Hour);
    Serial.println(hour);

}

void parseSunset(int hour, int minute) {
    TimeElements tm;
    breakTime(now(), tm);
    Serial.println(tm.Hour);
    Serial.println(hour);
}

void handleAck() {
    String message = "";
    message += "<p><b>Alarm has been set.</b></p>";
    message += "<p>Press OK to return to previous page.</p>&nbsp;";
    message += "<a href='/'><button>OK</button></a>";
    server.send(200, "text/html", message);
}


void handleUserInputError() {
    String message = "";
    message += "<p><b>Error: Please enter a valid time.</b></p>";
    message += "<p>Press OK to return to previous page.</p>&nbsp;";
    message += "<a href='/'><button>OK</button></a>";
    server.send(200, "text/html", message);
}

void handleDemo() {
    String message = "";
    message += "<p><b>Running demo...</b></p>";
    message += "<p>Press OK to return to previous page.</p>&nbsp;";
    message += "<a href='/'><button>OK</button></a>";
    server.send(200, "text/html", message);
    skyTransition(10);
}

void handleNotFound(){
    String message = "File Not Found\n\n";
    message += "URI: ";
    message += server.uri();
    message += "\nMethod: ";
    message += (server.method() == HTTP_GET)?"GET":"POST";
    message += "\nArguments: ";
    message += server.args();
    message += "\n";
    for (uint8_t i=0; i<server.args(); i++){
        message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
    }
    server.send(404, "text/plain", message);
}

void setup() {
    Serial.begin(9600);

    WiFiManager wifiManager;
    wifiManager.autoConnect("DIGITAL SKYLIGHT");
    if (wifiManager.autoConnect())
        statusLight(green);
    else
        statusLight(yellow);

    timeClient.begin();

    strip.begin();
    strip.show(); // Initialize all pixels to 'off'

    if (mdns.begin("skylight", WiFi.localIP())) {
    Serial.println("MDNS responder started");
    }

    server.on("/", handleRoot);
    server.on("/submit", handleSubmit);
    server.on("/demo", handleDemo);
    server.onNotFound(handleNotFound);
    server.begin();
    Serial.println("HTTP server started");

    Serial.println("");
    Serial.print("Connected to ");
    //Serial.println(ssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    attachInterrupt(POT, changeBrightness, CHANGE);
    STATE = NOTIME;
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
            statusLight(red);
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
    dawn_time = root["sys"]["sunrise"];
    dusk_time = root["sys"]["sunset"];
    Serial.println("dawn time: ");
    Serial.println(dawn_time);
    Serial.println("dusk time: ");
    Serial.println(dusk_time);

    Serial.println("closing connection");
}

void loop() {
    server.handleClient();
    changeBrightness();

    switch (STATE) {
        case NOTIME:
            Serial.println(STATE);
            getTODRequest();

            if (!timeNotSet)
                STATE = DAWNORDUSK;
        break;
        case OLDTIMES:
            Serial.println(STATE);
            getTODRequest();
            if ((now() > dawn_time) && (now() > dusk_time))
                STATE = OLDTIMES;
            else
                STATE = DAWNORDUSK;
                //dusk_time = now()+10;
        break;
        case DAWNORDUSK:
            Serial.println(STATE);
            dusk_time = 1490478890;
            Serial.print("dusk time: ");
            Serial.print(dusk_time);
            Serial.print("\n");
            Serial.print("now: ");
            Serial.print(now());
            Serial.print("\n");
            Serial.print("dawn time: ");
            Serial.print(dawn_time);
            Serial.print("\n");
            Serial.println(now() >= dawn_time);
            Serial.println(now() < dusk_time);
            if ((now() >= dawn_time) && (now() < dusk_time)) {
                Serial.println("It is daytime. Waiting for dusk.");
                STATE = WAITFORDUSK;
            }
            else if (now() < dawn_time) {
                Serial.println("It is nighttime. Waiting for dawn.");
                STATE = WAITFORDAWN;
            }
            else
                STATE = OLDTIMES;
        break;
        case WAITFORDUSK:
            Serial.println(STATE);
            Serial.print("Time until dusk: ");
            Serial.print(dusk_time - now());
            Serial.print("\n");
            if ((dusk_time - now()) <= (2400))
                STATE = DUSK;
            else {
                skyState3(255);
                delay(1000);
            }
        break;
        case WAITFORDAWN:
            Serial.println(STATE);
            Serial.print("Time until dawn: ");
            Serial.print(dawn_time - now());
            Serial.print("\n");
            if ((dawn_time - now()) <= (2400))
                STATE = DAWN;
            else {
                skyState1(0);
                delay(1000);
            }
        break;
        case DUSK:

            Serial.println(dusk_time - now());
            if ((dusk_time + (TRANSITIONTIME / 3)) < now())
                STATE = DAWNORDUSK;
            else {
                short int yy = (calculateStateOut(dusk_time - now()));
                Serial.println(yy);
                short int y = 765 - yy;
                Serial.println(y);
                if (y <= 255)
                    skyState1(y);
                else if (y > 510)
                    skyState3(y-510);
                else
                    skyState2(y-255);
            }
        break;
        case DAWN:
            if ((dawn_time + (TRANSITIONTIME / 3)) < now())
                STATE = DAWNORDUSK;
            else {
                short int y = calculateStateOut(dawn_time - now());
                if (y <= 255)
                    skyState1(y);
                else if (y > 510)
                    skyState3(y-510);
                else
                    skyState2(y-255);
            }
        break;

    }

}
/*
    getTODRequest();
    //checkNTPServer();



    for(;;) {
        // Wait a bit before scanning again

        skyTransition1(20);
        skyTransition2(20);
        skyTransition3(20);
        Serial.println(now());
        //strip.setBrightness((analogRead(POT)>>4));
        //strip.show();
        //skySim(strip.Color(0, 0, 255), strip.Color(127, 127, 0));
        //delay(500);
        changeBrightness();
    } */

short int calculateStateOut(int x) {
    return ((x - 2400) / (-TRANSITIONTIME / 765));
}

void statusLight(unsigned char statusColor) {
    uint32_t x;
    switch (statusColor) {
        case yellow:
            x = strip.Color(255, 255, 0);
            break;
        case red:
            x = strip.Color(255, 255, 0);
            break;
        case green:
            x = strip.Color(0, 255, 0);
            break;
    }
    for (int i = 0; i < 2; i++) {
        strip.setPixelColor(0, x);
        strip.show();
        delay(250);
        strip.setPixelColor(0, strip.Color(0,0,0));
        strip.show();
        delay(250);
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

void changeBrightness() {
    static char previous_state;
    if ((analogRead(POT)>>4) != previous_state) {
        strip.setBrightness((analogRead(POT)>>4));
        strip.show();
        previous_state = (analogRead(POT)>>4);
    }
}

void skyTransition(int wait) {
    for (int i =0; i < 255; i++) {
        skyState1(i);
        delay(wait);
    }
    for (int i=0; i < 255; i++) {
        skyState2(i);
        delay(wait);
    }
    for (int i=0; i < 255; i++) {
        skyState3(i);
        delay(wait);
    }
}

void skyState1(unsigned char i) {
    skySim(strip.Color(0, 0, (i/2)), strip.Color((i/2), 0, i));
}

void skyState2(unsigned char i) {
    skySim(strip.Color((i/2), 0, (127+(i/2))), strip.Color((127+(i/2)), i, (255-i)));
}

void skyState3(unsigned char i) {
    skySim(strip.Color((127-(i/2)), i, 255), strip.Color(255, 255, i));
}

void skySim(uint32_t outer, uint32_t inner) {
    uint8_t x = strip.numPixels() / 3;
    //Serial.println(strip.getBrightness());
    for (uint8_t i=0; i<strip.numPixels(); i++) {
        if ((i > x) && (i < (strip.numPixels() - x - 1)))
            strip.setPixelColor(i, inner);
        else
            strip.setPixelColor(i, outer);
    }
    changeBrightness();
    strip.show();
}
