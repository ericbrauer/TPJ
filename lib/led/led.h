#include <Adafruit_NeoPixel.h>


#define PIN 14
#define POT 0

void colorWipe(uint32_t c, uint8_t wait);
void skySim(uint32_t outer, uint32_t inner);
void skyTransition1(int wait);
void skyTransition2(int wait);
void skyTransition3(int wait);
void changeBrightness();
void ledInit();
