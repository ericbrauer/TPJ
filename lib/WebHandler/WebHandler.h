/*
    WebHandler.h library includes functions for the web server, for handling
    configuration changes for project skylight.
*/

#ifndef WebHandler_h
#define WebHandler_h

#include "Arduino.h"

class WebHandler
{
public:
    WebHandler(void);
    void handleUserInputError();
    void handleSubmit();
    void handleDemo();
    void handleAck();
    void parseSunrise(int hour, int minute);
    void parseSunset(int hour, int minute);
};

#endif
