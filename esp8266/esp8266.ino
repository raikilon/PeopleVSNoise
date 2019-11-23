#include <Arduino.h>
#define DEBUG true  // turn debug message on or off in serial

void setup()
{

  Serial.begin(9600);
  while (!Serial);
  Serial.println("Started Serial");

  
  Serial1.begin(115200);
  while (!Serial1);
  Serial.println("Started Serial1");
  
//  sendData("AT\r\n",2000,DEBUG);
//  sendData("AT+RST\r\n", 2000, DEBUG); // reset module
//  sendData("AT+CWMODE=1\r\n", 1000, DEBUG); // configure as sta
//  sendData("AT+CWLAP\r\n", 1000, DEBUG); // show AP
}
void loop()
{
  if (Serial.available()){
    Serial1.write(Serial.read());
  }
  if (Serial1.available()){
    Serial.write(Serial1.read());
  }
}

String sendData(String command, const int timeout, boolean debug)
{
  String response = "";

  Serial1.print(command); // send the read character to the esp8266

  long int time = millis();

  while ( (time + timeout) > millis())
  {
    while (Serial1.available())
    {

      // The esp has data so display its output to the serial window
      char c = Serial1.read(); // read the next character.
      response += c;
    }
  }

  if (debug)
  {
    Serial.print(response);
  }

  return response;
}
