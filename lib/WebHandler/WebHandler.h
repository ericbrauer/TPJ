/*
    WebHandler.h library includes functions for the web server, for handling
    configuration changes for project skylight.
*/

#ifndef WebHandler_h
#define WebHandler_h

#include "Arduino.h"
#include "ESP8266WebServer.h"

class WebHandler
{
public:
    WebHandler(ESP8266WebServer server);
    void handleRoot();
    void handleNotFound();
    void handleUserInputError();
    void handleSubmit();
    void handleDemo();
    void handleAck();
    void parseSunrise(int hour, int minute);
    void parseSunset(int hour, int minute);
private:
    ESP8266WebServer _server;
};

#endif
