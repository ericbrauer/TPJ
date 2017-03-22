#include "Arduino.h"
#include "WebHandler.h"
#include <ESP8266WebServer.h>
#include "LightStrip.h"

ESP8266WebServer _server;

WebHandler::WebHandler(ESP8266WebServer server)
{
    _server = server;
}

void WebHandler::handleRoot() {
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
    _server.send(200, "text/html", webPage);
}

void handleAck() {
    String message = "";
    message += "<p><b>Alarm has been set.</b></p>";
    message += "<p>Press OK to return to previous page.</p>&nbsp;";
    message += "<a href='/'><button>OK</button></a>";
    _server.send(200, "text/html", message);
}

void handleUserInputError() {
    String message = "";
    message += "<p><b>Error: Please enter a valid time.</b></p>";
    message += "<p>Press OK to return to previous page.</p>&nbsp;";
    message += "<a href='/'><button>OK</button></a>";
    _server.send(200, "text/html", message);
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

void handleSubmit() {
    char sunset_flag = 0;
    int x;
    int hour = 0;
    int minute = 0;
    if (_server.args() > 0 ) {
        for ( uint8_t i = 0; i < _server.args(); i++ ) {
            Serial.println(_server.argName(i));
            Serial.println(_server.arg(i));
            x = _server.arg(i).toInt();
            //if (!isNumber(x)) {
            //    handleUserInputError();
            //    return;
            //}
            if (_server.argName(i) == "rise_hour") {
                if ((x > 12) || (x < 1)) {
                    handleUserInputError();
                    return;
                }
                else {
                    hour = x;
                }
            }
            if (_server.argName(i) == "set_hour") {
                if ((x > 12) || (x < 1)) {
                    handleUserInputError();
                    return;
                }
                else {
                    hour = x;
                    sunset_flag = 1;
                }
            }
            if (_server.argName(i) == "rise_min") {
                if ((x > 59) || (x < 0)) {
                    handleUserInputError();
                    return;
                }
                else {
                    minute = x;
                }
            }
            if (_server.argName(i) == "set_min") {
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

void handleDemo() {
    String message = "";
    message += "<p><b>Running demo...</b></p>";
    message += "<p>Press OK to return to previous page.</p>&nbsp;";
    message += "<a href='/'><button>OK</button></a>";
    _server.send(200, "text/html", message);
    LightStrip::skyTransition1(20);
}

void handleNotFound(){
    String message = "File Not Found\n\n";
    message += "URI: ";
    message += _server.uri();
    message += "\nMethod: ";
    message += (_server.method() == HTTP_GET)?"GET":"POST";
    message += "\nArguments: ";
    message += _server.args();
    message += "\n";
    for (uint8_t i=0; i<_server.args(); i++){
        message += " " + _server.argName(i) + ": " + _server.arg(i) + "\n";
    }
    _server.send(404, "text/plain", message);
}
