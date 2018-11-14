#include <OctoWS2811.h>
#include <Adafruit_AM2315.h>

Adafruit_AM2315 am2315;
void setup() {
  // Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB
  }

  Serial.println();
  Serial.println("Trying to connect to AM2315 sensor...");
  Serial.println("beginning sensor...");
  am2315.begin();

  Serial.println("Sleeping for 3 seconds...");
  delay(3000);

  Serial.println("reading again...");
  am2315.readTemperature();

}

void loop() {
  // put your main code here, to run repeatedly:
  float temp;
  float humidity;
  am2315.readTemperatureAndHumidity(temp, humidity);

  Serial.print("Temp: "); Serial.println(temp);
  Serial.print("Humidity: "); Serial.println(humidity);
// wait for 1 sec before continuing
  delay(1000);
}
