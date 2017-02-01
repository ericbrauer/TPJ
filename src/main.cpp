#include <NTPClient.h>
#include "ESP8266WiFi.h"


#include <WiFiUdp.h>
const char *ssid     = "Goodsprings";
const char *password = "387iswhereweare";

const char* host = "data.sparkfun.com";
const char* host2 = "api.sunrise-sunset.org";

const char* streamId   = "....................";
const char* privateKey = "....................";

WiFiUDP ntpUDP;
WiFiClient client;

// You can specify the time server pool and the offset (in seconds, can be
// changed later with setTimeOffset() ). Additionaly you can specify the
// update interval (in milliseconds, can be changed using setUpdateInterval() ).
NTPClient timeClient(ntpUDP, "ca.pool.ntp.org", -18000, 60000);


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
  //printScannedNetworks();
  //Serial.println("Connecting to %c... ", ssid)

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

void getRequest(void) {
    int value = 0;

    const int httpPort = 80;
    if (!client.connect(host2, httpPort)) {
      Serial.println("connection failed");
      return;
    }
    // We now create a URI for the request
    String url2 = "/json?lat=36.7201600&lng=-4.4203400&date=today";
    String url = "/input/";
    url += streamId;
    url += "?private_key=";
    url += privateKey;
    url += "&value=";
    url += value;

    Serial.print("Requesting URL: ");
    Serial.println(url2);

    // This will send the request to the server
    client.print(String("GET ") + url2 + " HTTP/1.1\r\n" +
                 "Host: " + host2 + "\r\n" +
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
      String line = client.readStringUntil('\r');
      Serial.print(line);
    }

    Serial.println();
Serial.println("closing connection");
}

void loop() {
  printScannedNetworks();
  getRequest();
  for(;;) {
      checkNTPServer();
      // Wait a bit before scanning again
      delay(1000);
  }
}
