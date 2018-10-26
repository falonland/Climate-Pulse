#include <OctoWS2811.h>

const int ledsPerStrip = 300;
const int numStrips = 1;

// Starting hue at the beginning of the first strip, 0 - 360
float hueMin = 180;

// Ending hue at the end of the last strip, 0 - 360
float hueMax = 280;

// Amount of color variation to apply, 0 - 360. Try 10, 30, 50
// 0 means no shift, so the start/end from above are used as-is
// bigger numbers shift the color further away/around the color wheel
float hueSpread = 20;

// How fast to apply the color shift, 0.1 - about 10?
// 0.1 is very very slow
// 5 is farily fast
float hueSpreadSpeed = 2;

// 
float brightnessScale = 0.25;
float brightnessSpeed = 2.;

// 0 is off, 100 is full brightness
float brightnessMin = 0;
float brightnessMax = 20;

// Don't touch anything below here :)

enum State {
  NORMAL,
  AWAITING_TEMPERATURE,
  AWAITING_HUMIDITY
};

State state = NORMAL;
int temperature = 0;
int humidity = 0;

const int numPixels = ledsPerStrip * numStrips;
DMAMEM int displayMemory[ledsPerStrip * 6];
int drawingMemory[ledsPerStrip * 6];
const int config = WS2811_GRB | WS2811_800kHz;
OctoWS2811 leds(ledsPerStrip, displayMemory, drawingMemory, config);

int hueShift = 0;

void setup() {
  leds.begin();
  leds.show();
}

void loop() {
  checkSerialInput();

  if (temperature >= 50) hueMin = 0;
  else if (temperature >= 40) hueMin = 30;
  else if (temperature >= 30) hueMin = 60;
  else if (temperature >= 20) hueMin = 90;
  else if (temperature >= 10) hueMin = 120;
  else if (temperature >= 0) hueMin = 150;
  hueMax = hueMin + 30;
  hueSpread = 0;
  
  unsigned long now = millis();
  hueShift = sin(now * hueSpreadSpeed * 0.001f) * hueSpread;

  for (int i = 0; i < numPixels; i++) {
    int color;
    
    int hue = map(i, 0, numPixels, hueMin, hueMax);
    float brightnessSine = sin((i * (1 / brightnessScale)) + (now * brightnessSpeed * 0.001f));
    int brightness = map(brightnessSine, -1, 1, brightnessMin, brightnessMax);
    color = makeColor(hue + hueShift, 100, brightness);

    leds.setPixel(i, color);
  }
  leds.show();

  //delay(1000.f / 60.f);
  delay(1);
}

void checkSerialInput() {
  if (Serial.available() > 0) {

    if (state == NORMAL) {
      int value = Serial.read();
      
      if (value == '?') {
        Serial.println("Menu:");
        Serial.println("1. Set temperature");
        Serial.println("2. Set humidity");
      }
      else if (value == '1') {
        Serial.println("Enter temperature, or '0' to cancel");
        state = AWAITING_TEMPERATURE;
      }
      else if (value == '2') {
        Serial.println("Enter humidity, or '0' to cancel");
        state = AWAITING_HUMIDITY;
      }
    }

    else if (state == AWAITING_TEMPERATURE) {
      int value = Serial.parseInt();
      
      if (value == 0) {
        Serial.println("Ok, aborting setting of temperature");
      }
      else {
        Serial.print("Old temperature: ");
        Serial.println(temperature);
        temperature = value;
        Serial.print("New temperature: ");
        Serial.println(temperature);
      }
      
      state = NORMAL;
    }

    else if (state == AWAITING_HUMIDITY) {
      int value = Serial.parseInt();
      
      if (value == 0) {
        Serial.println("Ok, aborting setting of humidity");
      }
      else {
        Serial.print("Old humidity: ");
        Serial.println(humidity);
        humidity = value;
        Serial.print("New humidity: ");
        Serial.println(humidity);
      }
      
      state = NORMAL;
    }
  }
}

int makeColor(unsigned int hue, unsigned int saturation, unsigned int lightness)
{
  unsigned int red, green, blue;
  unsigned int var1, var2;

  if (hue > 359) hue = hue % 360;
  if (saturation > 100) saturation = 100;
  if (lightness > 100) lightness = 100;

  // algorithm from: http://www.easyrgb.com/index.php?X=MATH&H=19#text19
  if (saturation == 0) {
    red = green = blue = lightness * 255 / 100;
  } else {
    if (lightness < 50) {
      var2 = lightness * (100 + saturation);
    } else {
      var2 = ((lightness + saturation) * 100) - (saturation * lightness);
    }
    var1 = lightness * 200 - var2;
    red = h2rgb(var1, var2, (hue < 240) ? hue + 120 : hue - 240) * 255 / 600000;
    green = h2rgb(var1, var2, hue) * 255 / 600000;
    blue = h2rgb(var1, var2, (hue >= 120) ? hue - 120 : hue + 240) * 255 / 600000;
  }
  return (red << 16) | (green << 8) | blue;
}

unsigned int h2rgb(unsigned int v1, unsigned int v2, unsigned int hue)
{
  if (hue < 60) return v1 * 60 + (v2 - v1) * hue;
  if (hue < 180) return v2 * 60;
  if (hue < 240) return v1 * 60 + (v2 - v1) * (240 - hue);
  return v1 * 60;
}
