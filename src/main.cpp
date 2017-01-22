#include <NTPClient.h>
#include "ESP8266WiFi.h"


#include <WiFiUdp.h>
const char *ssid     = "Goodsprings";
const char *password = "387iswhereweare";

WiFiUDP ntpUDP;

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

void loop() {
  printScannedNetworks();
  for(;;) {
      checkNTPServer();
      // Wait a bit before scanning again
      delay(1000);

  }
}
