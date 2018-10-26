#include <FastLED.h>
#include <i2c_t3.h>
#include <Adafruit_AM2315.h>

// set number of LEDs you want to control here
#define NUM_LEDS 50

// set the pin number on the teensy you're connected to here
#define DATA_PIN 19

CRGB leds[NUM_LEDS];
Adafruit_AM2315 am2315;

void setup() {
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB
  }

  Serial.println();
  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);

  Serial.println("Trying to connect to AM2315 sensor...");
  Serial.println("beginning sensor...");
  am2315.begin();

  Serial.println("Sleeping for 3 seconds...");
  delay(3000);

  Serial.println("reading again...");
  am2315.readTemperature();

}

void loop() {
  // read data from sensor
  float temp;
  float humidity;
  am2315.readTemperatureAndHumidity(temp, humidity);

  Serial.print("Temp: "); Serial.println(temp);
  Serial.print("Humidity: "); Serial.println(humidity);

  

  // wait for 1 sec before continuing
  delay(1000);
}

