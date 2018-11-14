#include <i2c_t3.h>
#include <Adafruit_AM2315.h>
#include <OctoWS2811.h>

// Connect RED of the AM2315 sensor to 5.0V
// Connect BLACK to Ground
// Connect WHITE to i2c clock - on '168/'328 Arduino Uno/Duemilanove/etc thats Analog 5
// Connect YELLOW to i2c data - on '168/'328 Arduino Uno/Duemilanove/etc thats Analog 4

Adafruit_AM2315 am2315;

const int ledsPerStrip = 300;
const int numStrips = 8;

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
float brightnessMin = 2;
float brightnessMax = 20;

// Amount to reduce brigthness globally
float brightnessDampen = 0.25;

int temperature = 0;
int humidity = 0;
unsigned long lastSensorReadTime = 0;

const int numPixels = ledsPerStrip * numStrips;
DMAMEM int displayMemory[ledsPerStrip * 6];
int drawingMemory[ledsPerStrip * 6];
const int config = WS2811_GRB | WS2811_800kHz;
OctoWS2811 leds(ledsPerStrip, displayMemory, drawingMemory, config);

int hueShift = 0;
float hueMinLerped = 0;
float hueMaxLerped = 0;
float sat = 0;
float satLerped = 0;
float brightnessMaxLerped = 0;
float lerpAmt = 0.1;

void setup() {
  // Start LEDs with black color
  leds.begin();
  leds.show();

  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB
  }
  Serial.println("AM2315 Test!");

  if (!am2315.begin()) {
    Serial.println("Sensor not found, check wiring & pullups!");
    while (1);
  }
}

void loop() {
  unsigned long now = millis();

  // Update sensor once per second
  if (now - lastSensorReadTime > 1000) {
    lastSensorReadTime = now;
    temperature = am2315.readTemperature();
    humidity = am2315.readHumidity();
  }

  bool shouldSparkle = (humidity > 60);
  updateColorFromSensor();

  hueMax = hueMin + 30;
  hueSpread = 0;

  hueMinLerped = lerp(hueMinLerped, hueMin, lerpAmt);
  hueMaxLerped = lerp(hueMaxLerped, hueMax, lerpAmt);

  satLerped = lerp(satLerped, sat, lerpAmt);
  brightnessMaxLerped = lerp(brightnessMaxLerped, brightnessMax, lerpAmt);

  hueShift = sin(now * hueSpreadSpeed * 0.001f) * hueSpread;

  for (int i = 0; i < numPixels; i++) {
    if (shouldSparkle && random(100) < 1) {
      int color = makeColor(0, 0, brightnessMax * brightnessDampen);
      leds.setPixel(i, color);
    } else {
      float hue = map(i, 0, numPixels, hueMinLerped, hueMaxLerped);
      float brightnessSine = sin((i * (1 / brightnessScale)) + (now * brightnessSpeed * 0.001f));
      float brightness = map(brightnessSine, -1, 1, brightnessMin, brightnessMax);

      // Getting weird color changes/flickers
      // Reducing the brightness helps...not sure what's up
      brightness *= brightnessDampen;

      int color = makeColor(hue + hueShift, 100, brightness);

      leds.setPixel(i, color);
    }
  }
  leds.show();

  //delay(1000.f / 60.f);
  delay(1);

}

void updateColorFromSensor() {
  if (temperature >= 25) {
    hueMin = 186;
    sat = 80;
    brightnessMax = 100;
  }
  else if (temperature >= 21) {
    hueMin = 216;
    sat = 80;
    brightnessMax = 100;
  }
  else if (temperature >= 19) {
    hueMin = 228;
    sat = 100;
    brightnessMax = 100;
  }
  else if (temperature >= 17) {
    hueMin = 240;
    sat = 100;
    brightnessMax = 100;
  }
  else if (temperature >= 15) {
    hueMin = 257;
    sat = 100;
    brightnessMax = 60;
  }
  else if (temperature >= 13) {
    hueMin = 275;
    sat = 96;
    brightnessMax = 100;
  }
  else if (temperature >= 11) {
    hueMin = 326;
    sat = 100;
    brightnessMax = 79;
  }
  else if (temperature >= 9) {
    hueMin = 320;
    sat = 100;
    brightnessMax = 100;
  }
  else if (temperature >= 7) {
    hueMin = 0;
    sat = 100;
    brightnessMax = 100;
  }
  else if (temperature >= 5) {
    hueMin = 12;
    sat = 93;
    brightnessMax = 100;
  }
  else if (temperature >= 3) {
    hueMin = 28;
    sat = 93;
    brightnessMax = 100;
  }
  else if (temperature >= 0) {
    hueMin = 60;
    sat = 100;
    brightnessMax = 100;
  }
  else {
    hueMin = 0;
    sat = 0;
    brightnessMax = 0;
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

float lerp(float val, float target, float amt) {
  return val + (target - val) * amt;
}

