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
#define THIRDOFTTIME TRANSITIONTIME / 3
#define TWOTHIRDSOFTIME THIRDOFTTIME * 2
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

//String webPage = "";

/* The following are the main global variables that define the times when
TOD transitions occur. They are set in either getTODRequest() or in parseSunset
or parseSunrise when a manual alarm is set. they are used in the STATE machine.
time_t means that these are seconds since Epoch (Jan 1st, 1970), and in local time.
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


void handleRoot() { //this creates HTML code for the main webPage.
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

void handleSubmit() { // when changes are submitted, this parses the user input.
    char sunset_flag = 0;
    int x;
    int hour = 0;
    int minute = 0;
    if (server.args() > 0 ) { // if user input exists,
        for ( uint8_t i = 0; i < server.args(); i++ ) { // for each argument,
            Serial.println(server.argName(i));
            Serial.println(server.arg(i));
            x = server.arg(i).toInt(); // convert to an integer
            if (server.argName(i) == "rise_hour") { // if it's a sunrise time,
                if ((x > 12) || (x < 1)) { // check if it's out of bounds
                    handleUserInputError();
                    return;
                }
                else {
                    hour = x; //set hour to the user input.
                }
            }
            if (server.argName(i) == "set_hour") { // if it's a sunset time,
                if ((x > 12) || (x < 1)) { // check if out of bounds.
                    handleUserInputError();
                    return;
                }
                else {
                    hour = x; //set hour to user input, and raise sunset flag.
                    sunset_flag = 1;
                }
            }
            if ((server.argName(i) == "rise_min") || (server.argName(i) == "set_min")) { //if it's minutes,
                if ((x > 59) || (x < 0)) { //check if out of bounds
                    handleUserInputError();
                    return;
                }
                else {
                    minute = x;
                }
            }
        }
    }
    if (sunset_flag) { //if sunset flag raised, handle the input as sunset.
        parseSunset(hour, minute);
    }
    else
        parseSunrise(hour, minute); //else, sunrise.
    handleAck(); //send a response to user.
}

void parseSunrise(int hour, int minute) { //sets dawn_time from use input.
    time_t time_temp;
    TimeElements tm;
    breakTime(now(), tm); // get year, month, day for today.
    // comment this out for presentation
    // tm.Day += 1;
    tm.Hour = hour; // manually set hour to user input
    tm.Minute = minute; // manually set minute to user input
    tm.Second = 0; // set seconds to zero
    time_temp = makeTime(tm); // set a new time with same date.
    Serial.print("dawn time set to: ");
    Serial.println(time_temp);
    dawn_time = time_temp; // set dawn_time
    STATE = DAWNORDUSK; // change STATE to handle new info
}

void parseSunset(int hour, int minute) { //set dusk_time from user input
    TimeElements tm;
    time_t time_temp;
    breakTime(now(), tm);
    tm.Hour = hour+12; // add 12 to make it a PM time.
    tm.Minute = minute;
    tm.Second = 0;
    time_temp = makeTime(tm);
    Serial.print("dusk time set to: ");
    Serial.println(time_temp);
    dusk_time = time_temp;
    STATE = DAWNORDUSK;

}

void handleAck() { //sends a acknowledgement message.
    String message = "";
    message += "<p><b>Alarm has been set.</b></p>";
    message += "<p>Press OK to return to previous page.</p>&nbsp;";
    message += "<a href='/'><button>OK</button></a>";
    server.send(200, "text/html", message);
}


void handleRst() { //sends a reset message
    String message = "";
    getTODRequest(); // this overwrites user dusk and dawn_time with values from web.
    STATE = DAWNORDUSK;
    message += "<p><b>Alarms have been forgotten. Using actual dawn/dusk.</b></p>";
    message += "<p>Press OK to return to previous page.</p>&nbsp;";
    message += "<a href='/'><button>OK</button></a>";
    server.send(200, "text/html", message);
}

void handleUserInputError() { //sends an error message if user input is invalid.
    String message = "";
    message += "<p><b>Error: Please enter a valid time.</b></p>";
    message += "<p>Press OK to return to previous page.</p>&nbsp;";
    message += "<a href='/'><button>OK</button></a>";
    server.send(200, "text/html", message);
}

void handleDemo() { //runs a demo.
    String message = "";
    message += "<p><b>Running demo...</b></p>";
    message += "<p>Press OK to return to previous page.</p>&nbsp;";
    message += "<a href='/'><button>OK</button></a>";
    server.send(200, "text/html", message);
    skyTransition(10); // runs a neopixel transition with 10 ms between states.
}

void handleNotFound(){ // recommended from libary. Handles a bad page request.
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

    /* wifiManager tries to connect to a remembered wifi access point. If it
    fails, then will set the ESP-12 to station mode, where it provides its own
    access point with SSID "DIGITAL SKYLIGHT." The use can then scan available
    access points and enter credentials. statusLight() blinks one of the
    neopixels with the colour to indicate status. */
    WiFiManager wifiManager;
    wifiManager.autoConnect("DIGITAL SKYLIGHT");
    if (wifiManager.autoConnect())
        statusLight(green);
    else
        statusLight(yellow);

    timeClient.begin();

    strip.begin();
    strip.show(); // Initialize all pixels to 'off'

    //starts up micro-DNS server. The Skylight will be reachable at skylight.local.
    if (mdns.begin("skylight", WiFi.localIP())) {
    Serial.println("MDNS responder started");
    }

    /* sets the behaviour of the web server. each of these indicates a suffix after
    skylight.local. So skylight.local/ is where we find the main UI, and so on. */
    server.on("/", handleRoot);
    server.on("/submit", handleSubmit);
    server.on("/demo", handleDemo);
    server.on("/reset", handleRst);
    server.onNotFound(handleNotFound);
    server.begin();
    Serial.println("HTTP server started");

    //IP to use if DNS server fails. Used for debugging.
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    //attachInterrupt(POT, changeBrightness, CHANGE);
    //set initial state for loop().
    STATE = NOTIME;
}

void checkNTPServer() { // requests sync with NTP.
    timeClient.update();
    Serial.println(timeClient.getFormattedTime()); //debugging
    setTime(timeClient.getEpochTime()); //debugging
}

void getTODRequest() { // sends a GET request to get TOD, parses response, sets times
    time_t time_temp;
    String line;
    const int httpPort = 80;

    // stop if connection fails
    if (!client.connect(tod_host, httpPort)) {
        Serial.println("connection failed");
        return;
    }

    // generate the get request
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
            statusLight(red); // indicate failure with red status light.
            client.stop();
            return;
        }
    }

    // Read Response, ignore HTTP headers
    Serial.println("request sent");
    while (client.connected()) {
        String line = client.readStringUntil('\n');
        if (line == "\r") {
            Serial.println("headers received");
            break;
        }
    }

    line = client.readStringUntil('\n');
    // variable line stores the actual JSON response that we wish to parse.
    Serial.print(line);

    // Initialize object to parse JSON. stop if failure.
    DynamicJsonBuffer jsonBuffer;
    JsonObject &root = jsonBuffer.parseObject(line);
    if (!root.success()) {
      Serial.println("parseObject() failed");
      return;
    }

    /* the times sent from openweathermap are in epoch times at UTC.
    So, to deal with them, we add the timezone_offset and check that they aren't
    too far in the future. Otherwise we might overwrite an upcoming event with
    an event happening tomorrow. */
    time_temp = root["sys"]["sunrise"];
    if ((time_temp + timezone_offset) <= (now() + 43200)) //only save if in the next 12 hours
        dawn_time = time_temp + timezone_offset; //dawn_time is an epoch in local timezone.
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
    server.handleClient(); // if user requests a webPage, handle the request.
    changeBrightness(); // if ADC value changes, set a different brightness
    Serial.println(timeClient.getFormattedTime());
    /* Here is the main state machine. Used to set up transitions between events.*/
    switch (STATE) {
        case NOTIME: //initial state. No time set on power up, sync with NTP.
            checkNTPServer();

            if (!timeNotSet) // if time is set successfully, move to next state.
                STATE = DAWNORDUSK;
        break;

        case OLDTIMES: // if times are too old, run a TOD request to update them.
            getTODRequest();
            STATE = DAWNORDUSK;
        break;

        case DAWNORDUSK: // try to determine what is the next transition event.
            Serial.println(STATE);
            Serial.print("dusk time: "); //debugging messages.
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
            // if dawn_time and dusk_time are both older than now, we need an update.
            if ((now() > dawn_time) && (now() > dusk_time))
                STATE = OLDTIMES;
        break;

        case WAITFORDUSK: // set Neopixels to daylight. Wait.
            Serial.println(STATE);
            Serial.print("Time until dusk: ");
            Serial.print(dusk_time - now());
            Serial.print("\n");
            // we will begin a transition about 40 minutes before a scheduled event.
            if ((dusk_time - now()) <= TWOTHIRDSOFTIME)
                STATE = DUSK;
            else {
                skyState3(255);
                delay(100);
            }
        break;

        case WAITFORDAWN: // set Neopixels to off. Wait.
            Serial.println(STATE);
            Serial.print("Time until dawn: ");
            Serial.print(dawn_time - now());
            Serial.print("\n");
            // we will begin a transition about 40 minutes before a scheduled event.
            if ((dawn_time - now()) <= TWOTHIRDSOFTIME)
                STATE = DAWN;
            else {
                skyState1(0);
                delay(1000);
            }
        break;

        case DUSK: //run the transition
            Serial.println(dusk_time - now());
            // if we're twenty minutes past dusk, the event is over. go back to waiting.
            if ((dusk_time + THIRDOFTTIME) < now())
                STATE = DAWNORDUSK;
            /* if not, the show is still on. we need to translate a time duration
            into 3 sets chars. Each transition has 255 possible states.
            calculateStateOut will translate time into a state. Since this is
            dusk, everything is happening in reverse, and so we subtract this result
            from 765 (255 * 3). */
            else {
                /* we use the time remaining before dusk to calculate the state
                that we want our neopixels to be in. */
                short int yy = (calculateStateOut(dusk_time - now()));
                Serial.println(yy);
                short int y = 765 - yy;
                Serial.println(y);
                // skyStates are the Neopixel functions.
                if (y <= 255)
                    skyState1(y);
                else if (y > 510)
                    skyState3(y-510);
                else
                    skyState2(y-255);
            }
        break;
        case DAWN: // dawn occurs in the same way as dusk, but not reversed.
            if ((dawn_time + THIRDOFTTIME) < now())
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

short int calculateStateOut(int x) { // returns an integer from a time remaining
    return ((x - 2400) / (-TRANSITIONTIME / 765));
}

void statusLight(unsigned char statusColor) { // blinks a status light
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

void changeBrightness() { // changes brightness level if there's a change on ADC
    static char previous_state;
    if ((analogRead(POT)>>4) != previous_state) {
        strip.setBrightness((analogRead(POT)>>4));
        strip.show();
        previous_state = (analogRead(POT)>>4);
    }
}

void skyTransition(int wait) { // used for demo. Cycles through all the skyStates.
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

/* The following are functions written to control the neopixels. Some explanation
is probably necessary: The NeoPixel libary sets Pixel colour using a strip.Color()
data type. There are three 8-bit numbers here, and they correspond to a level of
colour between 0 and 255 for Red Green Blue. So Strip.Color(255, 0, 0) sets Red
to maximum, and other colours off. Strip.Color(255, 255, 255) is a pure white */

//first state. No lights to a pre-dawn state.
void skyState1(unsigned char i) { // outer lights set to 50% blue, inner lights set to 50% red, 100% blue.
    Serial.print("SkyState 1, state: ");
    Serial.println(i);
    skySim(strip.Color(0, 0, (i/2)), strip.Color((i/2), 0, i));
}

//second state. Pre-dawn to dawn.
void skyState2(unsigned char i) { //outer lights from 50% blue to 50% red and 100% blue.
    //inner lights from 50% red, 100% blue to Yellow.
    Serial.print("SkyState 2, state: ");
    Serial.println(i);
    skySim(strip.Color((i/2), 0, (127+(i/2))), strip.Color((127+(i/2)), i, (255-i)));
}

//third state. Dawn to Day.
void skyState3(unsigned char i) { //outer from magenta to cyan. Inner from yellow to white.
    Serial.print("SkyState 3, state: ");
    Serial.println(i);
    skySim(strip.Color((127-(i/2)), i, 255), strip.Color(255, 255, i));
}

void skySim(uint32_t outer, uint32_t inner) { //divides the array into threes, sets the lights.
    uint8_t x = strip.numPixels() / 3;
    for (uint8_t i=0; i<strip.numPixels(); i++) {
        if ((i > x) && (i < (strip.numPixels() - x - 1))) //if the pixel address is in the middle, set it to inner colour.
            strip.setPixelColor(i, inner);
        else
            strip.setPixelColor(i, outer); // else, set it to outer colour.
    }
    changeBrightness(); //change the brightness based on ADC reading.
    strip.show(); // Output the new state to the Neopixels.
}
