#include <NTPClient.h>
#include "ESP8266WiFi.h"
#include <WiFiUdp.h>
#include <ArduinoJson.h>

const char *ssid     = "Goodsprings";
const char *password = "387iswhereweare";

const int timezone_offset = -18000;

const char* tod_host = "api.sunrise-sunset.org";

const char* my_lat = "36.7201600";
const char* my_long = "-4.4203400";


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

  getRequest(my_lat, my_long, dawn_time, dusk_time);
  for(;;) {
      checkNTPServer();
      // Wait a bit before scanning again
      delay(1000);
  }
}
