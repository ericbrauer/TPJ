# TPJ
Final Project for ECT @ Seneca. "Project Skylight"

# PROJECT SKYLIGHT
Eric Brauer 

## Executive Summary
Project Skylight is an Internet-connected light that simulates daylight cycles. It’s meant for
use in places where there are no external windows, and it can be used to get a quick visual
reference for the time of day. Project skylight is composed of a small microcontroller with wifi
capabilities, and 16 RGB LEDs. The device is meant to run with very little set-up, but a
web-based user interface can be used for further configuration. In the report, the operation
and specifications of Project Skylight are covered in more detail.

## Functional Features
The product requirements, as envisioned, are as follows:
- Project Skylight (PS) asks for WiFi credentials, and connects to a WiFi router.
- PS serves a webpage which is used to configure the device and to provide the user’s
city of residence.
- PS uses the OpenWeatherMap API to get the time of dawn and dusk. The device
also uses an NTP server to synchronise the current time.
- PS changes the colour and brightness of LEDs based on current time in reference to
sunrise and sunset times.
- PS monitors a potentiometer, which will be used to dim or turn off the LED light.
- Further configuration is possible from the web interface. This includes setting manual
daylight times.

## Product Specifications
- Powered from micro USB port
- Operates at nominal indoor temperatures, not for outdoor use
- Luminosity of around 200 - 400 Lumens
- WiFi range 20m
- Current consumption no higher than 1.5A

## Theory of Operation
### Power Concerns
The ESP-12f requires a typical voltage of 3.3 Volts. The Neopixel array require a voltage of 4
to 7 Volts. Thus a voltage regulation stage is required. USB provides 5 Volts, and this power
is used for the Neopixels. Power for the ESP-12f is supplied by an LD1117.

Power consumption is based heavily on several factors: transmission and reception by the
WiFi module on the ESP-12f, signal strength of the same, and brightness levels of the
Neopixels. If a single Neopixel is set to white with maximum brightness, then each of its
three RGB LEDs requires 20 mA. There is never a situation when the Skylight is generating
this state, however. Full brightness of the daytime SkyState would represent the maximum
current consumption.

4 LEDs displaying white = 60mA * 4 240mA

12 LEDs displaying cyan = 40mA * 12 480mA

Total Current for Neopixels: **720mA**

In addition, the ESP-12f requires a maximum of 100mA. Meaning that a USB adapter rated
at 1A should be sufficient for proper function and also provide a buffer of almost 20%.

### Initial State / Connectivity
The ESP-12f is a popular IC among the maker community, and much work has been done to
implement a thorough Arduino library which exposes much of the connectivity capability of
the chip. The Skylight in particular makes use of several libraries to implement the initial
Access Point state (used to enter WiFi credentials), to communicate to OpenWeatherMap,
and to serve a web-based user interface.

**ESP8266WiFi** exposes functions for connecting to a WiFi access point and communicating
over TCP/IP.

**ESP8266WebServer** allows to ESP to also act as a web server, and serve HTTP code to
clients.

**ESP8266mDNS** provides a ‘micro DNS’ server which will associate a URL with whatever IP
address is assigned to the ESP’s web server.

**WifiManager** checks flash for a WiFi SSID and password that was previously used to
connect. If these don’t exist, WifiManager will set the ESP-12 to station mode with an SSID
set in software. The user can connect to this access point, and then navigate to the url
specified by the micro-DNS server. From this webpage, the user has the option to scan for
available access points, and then enter a password to connect to this access point. From
this point, the ESP exits station mode and attempts to connect to the access point as a
client. If it is successful, these credentials will be saved in flash memory to be used on
startup.

### The Web Interface
The ESP handles web requests directed at the url created by the micro DNS server. On the
Skylight, the url is skylight.local. The main interface is located at the root. Other webpages
are used to acknowledge user requests, and exist at /submit, /demo and /reset. All other
requests will result in a HTTP code of 404.
A typical user request is to manually change the time at which a sunset event occurs. The
time desired is typed into text boxes, and the ‘save’ button is pressed. Information inside the
text boxes is saved as arguments. The arguments are validated first. The arguments are
checked to ensure that they are numerical and in a proper 12-hour time format. A bad
request will result in a ‘bad request’ message and a link back to the root page. A good
request will be accepted. Finally, the browser will navigate to skylight.local/submit, where the
user is notified that the alarm has been set.

### Adafruit NeoPixel Array
Neopixels are addressable RGB LEDs. Control circuitry, LEDs, and internal oscillators are all
included in one package. Data can be received from a microcontroller at speeds of 800kbps,
and outputted to the next neopixel. This allows an array of neopixels to be controlled from
one data pin.
Each neopixel requires 24bits of information. The first neopixel uses 24 bits to set the values
of discrete red, green and blue LEDs. Each of these LEDs takes a PWM 8-bit signal to set its
intensity, resulting in 255 possible states. Once the state of neopixel has been set,
subsequent data is amplified and sent through a ‘Data Out’ line to program the next
neopixel. The advantage of neopixels is their flexibility, but larger arrays will require far more
memory and processing than some microcontrollers can provide. Since the Skylight uses
only 16 neopixels, performance requirements are relatively light.9
Adafruit provides an Arduino Library for communicating with the Neopixel array. The
Neopixels are powered from a 5V VCC line and the data line is connected to the ESP’s
GPIO pin 14.
### Brightness Control
A potentiometer is connected to the ESP-12’s single Analog-to-Digital converter. A standard
Arduino library function is used to convert this number to a 10-bit value, which is then to an
8-bit value using bit shift. The 8-bit value is used as an argument for the setBrightness
function which is part of the Adafruit Neopixel library. Using the ADC with interrupts proved
to be problematic, and so the ADC is polled in loop() and if its value changes, the
setBrightness function is called. The potentiometer is connected to the ADC pin identified as
0.
### Setting dusk_time and dawn_time From OpenWeatherMap.org
OpenWeatherMap provides an API that can be used for free with registration for a smaller
number of requests. For the Skylight prototype, this is sufficient. Documentation for the API
is located at ​ [OpenWeatherMap](http://openweathermap.org/current).​ The API can be used with several different
formats of location data; for simplicity’s sake a GET request is used encoded with the city
and country code are used along with an API key.
The HTTP response is in a JSON format. First, the HTTP header of the response is
discarded. Then ArduinoJson library is used to parse the response. Most information is
related to weather conditions and is not used. The sunrise and sunset times are sent in
epoch time based on UTC. In order to set this to local time, a timezone offset is added to
these times and the result is stored in memory.
### Using NTP to Sync Local Time
NTPClient is another library used to communicate with NTP servers. This is used with
TimeLib to set the ESP’s time. A timezone offset is required by the library.
### State Machine
Assuming that NTP and GET Requests have succeeded, the Skylight will compare current
time with the next scheduled event. Once current time is within forty minutes of a sunrise or
sunset, the Skylight enters a DUSK or DAWN event. Both events are identical, but DUSK
occurs in reverse order.
There are three parts to a transition, and each takes 20 minutes. Two of the parts of the
transition occur before the event time, and one occurs after. Each part of the transition is
handled by a function, and each function takes as an argument an 8-bit number.

`SkyState1(0-255) SkyState2(0-255) SkyState3(0-255)`

Duration: 1200 seconds Duration: 1200 seconds Duration: 1200 seconds
In the case of DAWN, the actual sunrise_time is the time between SkyState2 and SkyState3.
In order to set the argument for each SkyState function, a simple equation derived from the
Radiometric Equation is used.

`y = (x - 2400) / (-3600 / 765)`

Where x is equal to seconds until event and y is equal to the skyState argument.
What this means is that approximately every 7 seconds, SkyState functions will increment
the levels of the Neopixel LEDs which results in a simulated dawn or dusk event. Once the
current time is over twenty minutes past the last event, the state machine will return to a
state where it decides if it is waiting for dawn or waiting for dusk. If both dawn and dusk
events are earlier than the current time, the state machine will attempt to request new times
from OpenWeatherMap.
