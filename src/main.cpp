/*
-------------------------------------------------------------------------
    TITLE: Project Skylight
    AUTHOR: Eric Brauer
    DATE: 2017-04-04 (final version)

    DESCRIPTION: Project Skylight connects to a third-party API to collect
    dawn_time and dusk_time, and then simulates the current time of day using
    a Neopixel Array. Further detail will be explained in comments.
-------------------------------------------------------------------------
*/

/* The following are third-party libraries used by the project. I claim no
ownership / responsibility for their use. Anything in main.cpp was written by
me. NOTE: Links to the libraries can be found in the platformio.ini file. */
#include <NTPClient.h>          // used for setting time via NTP
#include "ESP8266WiFi.h"        // used for GET requests
#include <WiFiUdp.h>            // used for connection with NTPClient
#include <ArduinoJson.h>        // used to parse JSON response from API
#include <Adafruit_NeoPixel.h>  // used to control NeoPixels
#include <TimeLib.h>            // used for time-based functions
#include <ESP8266WebServer.h>   // used to serve web UI
#include <ESP8266mDNS.h>        // used to provide "skylight.local for web UI"
#include <WiFiManager.h>        // used for initial set up of WiFi credentials

#define PIN 14 // GPIO data pin used for NeoPixels
#define POT 0 // ADC pin used for brightness knob
#define TRANSITIONTIME 3600 //1 hour to get from night to day.  (in seconds)
enum {yellow, red, green}; // used for statusLight.
//states used for the STATE machine in loop()
enum {NOTIME, OLDTIMES, DAWNORDUSK, WAITFORDUSK, WAITFORDAWN, DUSK, DAWN};
unsigned char statusColor; //Initializing variables used with enums
unsigned char STATE;

/* The following variables are hard-coded, but in a commercial release would
need to be dynamic or otherwise set up properly. */

const int timezone_offset = -14400; // Eastern Daylight Timezone offset from UTC.
//const int timezone_offset = -18000; //Eastern Standard Timezone offset from UTC.

const char* tod_host = "api.openweathermap.org"; // url
const char* api_key = "ab33624b9ab307c2d056de2359eaedf5"; //api key
const char* my_city = "toronto"; // location
const char* my_country = "ca"; // country code

String webPage = "";

/* The following are the main global variables that define the times when
TOD transitions occur. They are set in either getTODRequest() or in parseSunset
or parseSunrise when a manual alarm is set. they are used in the STATE machine.
*/
volatile time_t dawn_time;
volatile time_t dusk_time;

// function delarations follow
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
void getTODRequest();

//Initializing objects
WiFiUDP ntpUDP;
WiFiClient client;

ESP8266WebServer server(80); // 80 = port for http
MDNSResponder mdns;

// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(16, PIN, NEO_GRB + NEO_KHZ800);

// You can specify the time server pool and the offset (in seconds, can be
// changed later with setTimeOffset() ). Additionaly you can specify the
// update interval (in milliseconds, can be changed using setUpdateInterval() ).
NTPClient timeClient(ntpUDP, "ca.pool.ntp.org", timezone_offset, 60000);


void handleRoot() { 
    String webPage = "";
    webPage += "<h1>Skylight</h1>";
    webPage += "<p>Time is now: ";
    webPage += timeClient.getFormattedTime();
    webPage += "</p><h3>Set Sunrise:</h3>";
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
    webPage += "<p><a href='reset'><button style='width:100%;'>Reset Manual Alarms</button></a>&nbsp;</p>";
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
    if (sunset_flag) {
        parseSunset(hour, minute);
    }
    else
        parseSunrise(hour, minute);
    handleAck();
}

void parseSunrise(int hour, int minute) {
    time_t time_temp;
    TimeElements tm;
    breakTime(now(), tm);
    //comment this out for presentation
    //tm.Day += 1;
    tm.Hour = hour;
    tm.Minute = minute;
    tm.Second = 0;
    time_temp = makeTime(tm);
    Serial.print("dawn time set to: ");
    Serial.println(time_temp);
    dawn_time = time_temp;
    STATE = DAWNORDUSK;
}

void parseSunset(int hour, int minute) {
    TimeElements tm;
    time_t time_temp;
    breakTime(now(), tm);
    tm.Hour = hour+12;
    tm.Minute = minute;
    tm.Second = 0;
    time_temp = makeTime(tm);
    Serial.print("dusk time set to: ");
    Serial.println(time_temp);
    dusk_time = time_temp;
    STATE = DAWNORDUSK;

}

void handleAck() {
    String message = "";
    message += "<p><b>Alarm has been set.</b></p>";
    message += "<p>Press OK to return to previous page.</p>&nbsp;";
    message += "<a href='/'><button>OK</button></a>";
    server.send(200, "text/html", message);
}


void handleRst() {
    String message = "";
    getTODRequest();
    STATE = DAWNORDUSK;
    message += "<p><b>Alarms have been forgotten. Using actual dawn/dusk.</b></p>";
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
    server.on("/reset", handleRst);
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
    time_t time_temp;
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

    //setTime(root["dt"]);

    time_temp = root["sys"]["sunrise"];
    if ((time_temp + timezone_offset) <= (now() + 43200)) //only save if in the next 12 hours
        dawn_time = time_temp + timezone_offset;
    time_temp = root["sys"]["sunset"];
    if ((time_temp + timezone_offset) <= (now() + 43200))
        dusk_time = time_temp + timezone_offset;
    Serial.println("dawn time: ");
    Serial.println(dawn_time);
    Serial.println("dusk time: ");
    Serial.println(dusk_time);

    Serial.println("closing connection");
}

void loop() {
    server.handleClient();
    changeBrightness();
    Serial.println(timeClient.getFormattedTime());
    switch (STATE) {
        case NOTIME:
            Serial.println(STATE);
            checkNTPServer();
            //getTODRequest();

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
            //dusk_time = 1490478890;
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
            if ((now() > dawn_time) && (now() > dusk_time))
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
    Serial.print("SkyState 1, state: ");
    Serial.println(i);
    skySim(strip.Color(0, 0, (i/2)), strip.Color((i/2), 0, i));
}

void skyState2(unsigned char i) {
    Serial.print("SkyState 2, state: ");
    Serial.println(i);
    skySim(strip.Color((i/2), 0, (127+(i/2))), strip.Color((127+(i/2)), i, (255-i)));
}

void skyState3(unsigned char i) {
    Serial.print("SkyState 3, state: ");
    Serial.println(i);
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
