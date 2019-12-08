// Required by Arducam library
#include <SPI.h>
#include <Wire.h>
#include "base64.hpp"
#include <memorysaver.h>
// Arducam library
#include <ArduCAM.h>
// JPEGDecoder library
#include <JPEGDecoder.h>

#include <WiFiEspAT.h>

// The pin connected to the Arducam Chip Select
#define CS 7
uint8_t temp;
bool is_header = false;
// Camera library instance
ArduCAM myCAM(OV2640, CS);

bool check = true;

const char ssid[] = "DrawsHoutlet 2.4";    // your network SSID
const char pass[] = "piggy4-Nerve7-Alpha3";    // your network password (use for WPA, or use as key for WEP)

const char* server = "192.168.1.34"; // server adress
unsigned int base64_length;
const int buffsize  = 2000;
uint8_t  buff[buffsize];
int mod;
int division;
int packet_size = 0;
int count = 0;

bool PRINT = false;

WiFiClient client;


void setup() {
  // put your setup code here, to run once:

  if (PRINT) {
    Serial.begin(115200);
    while (!Serial);
    myPrint("Serial connected");
  }

  Serial1.begin(115200);
  WiFi.init(Serial1);

  if (WiFi.status() == WL_NO_MODULE) {
    myPrint("Communication with WiFi module failed!");
    while (1) {}
  }

  WiFi.endAP(true); // to disable default automatic start of persistent AP at startup

  WiFi.setPersistent(); // set the following WiFi connection as persistent


  myPrint("Attempting to connect to SSID: ");
  myPrint(ssid);

  //  Static IP otherwise esp8266 does not connect correctly
  IPAddress ip(192, 168, 1, 35);
  IPAddress gw(192, 168, 1, 1);
  IPAddress nm(255, 255, 255, 0);
  WiFi.config(ip, gw, gw, nm);

  int status = WiFi.begin(ssid, pass);

  if (status == WL_CONNECTED) {
    WiFi.setAutoConnect(true);
    myPrint("Connected to WiFi network.");
  } else {
    WiFi.disconnect(); // remove the WiFi connection
    myPrint("Connection to WiFi network failed.");
    while (1);
  }


  myPrint("Attempting to start Arducam");
  Wire.begin();
  myPrint("Wire inizialized");
  // Configure the CS pin
  pinMode(CS, OUTPUT);
  digitalWrite(CS, HIGH);
  // initialize SPI
  SPI.begin();
  myPrint("SPI inizialized");
  // Reset the CPLD
  myCAM.write_reg(0x07, 0x80);
  delay(100);
  myCAM.write_reg(0x07, 0x00);
  delay(100);

  while (1) {
    //Check if the ArduCAM SPI bus is OK
    myCAM.write_reg(ARDUCHIP_TEST1, 0x55);
    temp = myCAM.read_reg(ARDUCHIP_TEST1);
    if (temp != 0x55) {
      //myPrint(F("ACK CMD SPI interface Error! END"));
      delay(1000);
      continue;
    } else {
      //myPrint(F("ACK CMD SPI interface OK. END"));
      break;
    }
  }

  // Use JPEG capture mode, since it allows us to specify
  // a resolution smaller than the full sensor frame
  myCAM.set_format(JPEG);
  myCAM.InitCAM();
  myPrint("Camera inizialized");
  myCAM.OV2640_set_JPEG_size(OV2640_640x480);//OV2640_160x120
  delay(100);

}

void loop() {
  myPrint("Starting capture");
  // Make sure the buffer is emptied before each capture
  myCAM.flush_fifo();
  myCAM.clear_fifo_flag();
  // Start capture
  myCAM.start_capture();
  // Wait for indication that it is done
  while (!myCAM.get_bit(ARDUCHIP_TRIG, CAP_DONE_MASK)) {}
  myPrint("Image captured");
  delay(50);
  myPrint("calling burst");
  read_fifo_burst(myCAM);
  //Clear the capture done flag
  myCAM.clear_fifo_flag();

  //delay(50);
}


void myPrint(String str) {
  if (PRINT) {
    Serial.println(str);
  }

}

uint8_t read_fifo_burst(ArduCAM myCAM)
{
  uint32_t length = 0;
  length = myCAM.read_fifo_length();

  const uint32_t lengthb = length;

  unsigned char binary[lengthb];
  uint8_t temp = 0, temp_last = 0;

  if (length >= MAX_FIFO_SIZE) //512 kb
  {
    myPrint(F("ACK CMD Over size. END"));
    return 0;
  }
  if (length == 0 ) //0 kb
  {
    myPrint(F("ACK CMD Size is 0. END"));
    return 0;
  }
  myCAM.CS_LOW();
  myCAM.set_fifo_burst();//Set fifo burst mode
  temp =  SPI.transfer(0x00);
  int i = 0;
  length --;
  while ( length-- )
  {
    temp_last = temp;
    temp =  SPI.transfer(0x00);
    if (is_header == true)
    {
      //Serial.write(temp);
      binary[i++] = temp;
    }
    else if ((temp == 0xD8) & (temp_last == 0xFF))
    {
      is_header = true;
      binary[i++] = temp_last;
      binary[i++] = temp;
    }
    if ( (temp == 0xD9) && (temp_last == 0xFF) ) //If find the end ,break while,
      break;
    delayMicroseconds(15);
  }
  myCAM.CS_HIGH();
  is_header = false;

  myPrint("start sending");

  unsigned char base64[4 * (lengthb / 3)];
  base64_length = encode_base64(binary, lengthb, base64);

  mod = base64_length % buffsize;
  division = (base64_length / buffsize);

  if (mod == 0) {
    count = division ;
  } else {
    count = division + 1;
  }

  myPrint(String(count));


  // wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    myPrint("waiting");
    WiFi.endAP(true); // to disable default automatic start of persistent AP at startup

    WiFi.setPersistent(); // set the following WiFi connection as persistent

    myPrint("Attempting to connect to SSID: ");
    myPrint(ssid);

    //  Static IP otherwise esp8266 does not connect correctly
    IPAddress ip(192, 168, 43, 10);
    IPAddress gw(192, 168, 43, 1);
    IPAddress nm(255, 255, 255, 0);
    WiFi.config(ip, gw, gw, nm);

    int status = WiFi.begin(ssid, pass);

  }

  String data = "{\"count\":" + String(count) + "}";
  client.stop();
  if (client.connect(server, 5000)) {
    myPrint("connected to server");
    client.println("POST /img HTTP/1.1");
    client.println("User-Agent: Arduino/1.0");
    client.println("Connection: keep-alive");
    client.print("Content-Length: ");
    client.println(data.length());
    client.println("Content-Type: application/json");
    client.println();
    client.println(data);

    client.flush();
  }




  for (int i = 0; i < count; i++) {
    if (mod != 0 and i + 1 == count )
      packet_size  = mod;
    else {
      packet_size  = buffsize;
    }

    memset(buff, 0, buffsize);
    memcpy(buff, base64 + (i * buffsize), packet_size);

    // wait for connection
    while (WiFi.status() != WL_CONNECTED) {
      delay(1000);
      myPrint("waiting");
      WiFi.endAP(true); // to disable default automatic start of persistent AP at startup

      WiFi.setPersistent(); // set the following WiFi connection as persistent

      myPrint("Attempting to connect to SSID: ");
      myPrint(ssid);

      //  Static IP otherwise esp8266 does not connect correctly
      IPAddress ip(192, 168, 43, 10);
      IPAddress gw(192, 168, 43, 1);
      IPAddress nm(255, 255, 255, 0);
      WiFi.config(ip, gw, gw, nm);

      int status = WiFi.begin(ssid, pass);
    }
    if (client.connect(server, 5000)) {
      client.println("POST /img HTTP/1.1");
      client.println("User-Agent: Arduino/1.0");
      client.println("Connection: keep-alive");
      client.print("Content-Length: ");
      client.println(packet_size);
      client.println("Content-Type: application/octet-stream");
      client.println();
      client.write(buff, packet_size);
      //Serial.write(buff, packet_size);
      client.println();
      client.flush();
    }


    while (client.available()) {
      char c = client.read();
      if (PRINT) {
        Serial.write(c);
      }
    }
    //delay(50);

  }

  client.stop();

  return 1;
}
