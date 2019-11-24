// ---------------------------------------------------------------------------
// Test AT commands on the ESP8266
// ---------------------------------------------------------------------------

#include <Arduino.h>


void setup()
{
  Serial.begin(9600);
  while (!Serial);
  Serial.println("Started Serial");


  Serial1.begin(115200);
  while (!Serial1);
  Serial.println("Started Serial1");
}

void loop()
{
  if (Serial.available()) {
    Serial1.write(Serial.read());
  }
  if (Serial1.available()) {
    Serial.write(Serial1.read());
  }
}
