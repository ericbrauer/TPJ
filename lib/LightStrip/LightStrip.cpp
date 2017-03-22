#include <Adafruit_NeoPixel.h>


#define PIN 14
#define POT 0

// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(8, PIN, NEO_GRB + NEO_KHZ800);

LightStrip::LightStrip(void) {

}

void changeBrightness() {
    strip.setBrightness((analogRead(POT)>>4));
    strip.show();
    //Serial.println("brightness interrupt");
    //Serial.println((analogRead(POT)>>4));
}

void skySim(uint32_t outer, uint32_t inner) {
    uint8_t x = strip.numPixels() / 3;
    Serial.println(strip.getBrightness());
    for (uint8_t i=0; i<strip.numPixels(); i++) {
        if ((i > x) && (i < (strip.numPixels() - x - 1)))
            strip.setPixelColor(i, inner);
        else
            strip.setPixelColor(i, outer);
    }
    changeBrightness();
    strip.show();
}

void ledInit() {
    strip.begin();
    strip.show(); // Initialize all pixels to 'off'
}

void skyTransition1(uint8_t i, int wait) {
    //for (int i = 0; i < 255; i++) {
        skySim(strip.Color(0, 0, (i/2)), strip.Color((i/2), 0, i));
        delay(wait);
    //}
}

void skyTransition2(uint8_t i, int wait) {
    //for (int i = 0; i < 255; i++) {
        skySim(strip.Color((i/2), 0, (127+(i/2))), strip.Color((127+(i/2)), i, (255-i)));
        delay(wait);
    //}
}

void skyTransition3(uint8_t i, int wait) {
    //for (int i = 0; i < 255; i++) {
        skySim(strip.Color((127-(i/2)), i, 255), strip.Color(255, 255, i));
        delay(wait);
    //}
}

// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait) {
    for(uint16_t i=0; i<strip.numPixels(); i++) {
        strip.setPixelColor(i, c);
        strip.show();
        delay(wait);
    }
}
