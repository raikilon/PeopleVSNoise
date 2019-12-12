#include <SPI.h>
#include <Wire.h>
#include "base64.hpp"
#include <memorysaver.h>
#include <ArduCAM.h>
#include <JPEGDecoder.h>
#include <WiFiEspAT.h>

#define CS 7

// SETTINGS
const char ssid[] = "...";    // your network SSID
const char pass[] = "...";    // your network password
const char* server = "192.168.43.134"; // server adress
const int buffsize  = 2000;  // Size of each http request
bool PRINT = false; // Debug

// Arducam variables
ArduCAM myCAM(OV2640, CS);
uint8_t temp;
uint8_t temp_last;
bool is_header = false;
uint32_t length;
uint32_t lengthb;
uint8_t buff[buffsize];
int mod;
int division;
int packet_size = 0;
int count = 0;
unsigned int base64_length;

//WiFi virable
WiFiClient client;
const IPAddress ip(192, 168, 43, 69);
const IPAddress gw(192, 168, 43, 1);
const IPAddress nm(255, 255, 255, 0);

void setup() {
  if (PRINT) {
    Serial.begin(115200);
    while (!Serial);
    myPrint("Serial connected");
  }

  Serial1.begin(115200);
  WiFi.init(Serial1);

  // ########### INIT WIFI ###########

  if (WiFi.status() == WL_NO_MODULE) {
    myPrint("Communication with WiFi module failed!");
    while (1) {} // do not continue
  }

  WiFi.endAP(true); // to disable default automatic start of persistent AP at startup

  WiFi.setPersistent(); // set the following WiFi connection as persistent

  myPrint("Attempting to connect to SSID: ");
  myPrint(ssid);

  //  Static IP otherwise esp8266 does not connect correctly

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

  // ########### INIT ARDUCAM ###########

  Wire.begin();
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
      myPrint(F("ACK CMD SPI interface Error! END"));
      delay(1000);
      continue;
    } else {
      myPrint(F("ACK CMD SPI interface OK. END"));
      break;
    }
  }

  // Use JPEG capture mode, since it allows us to specify
  // a resolution smaller than the full sensor frame
  myCAM.set_format(JPEG);
  myCAM.InitCAM();
  myPrint("Camera inizialized");
  myCAM.OV2640_set_JPEG_size(OV2640_352x288);//OV2640_160x120
  delay(100);

}

void loop() {
  myPrint("Starting capture");
  myCAM.flush_fifo(); // Make sure the buffer is emptied before each capture
  myCAM.clear_fifo_flag();
  myCAM.start_capture(); // Start capture
  // Wait for indication that it is done
  while (!myCAM.get_bit(ARDUCHIP_TRIG, CAP_DONE_MASK)) {}
  myPrint("Image captured");
  //delay(50);

  // ########### READ PICTURE ###########

  length = myCAM.read_fifo_length();

  lengthb = length;
  myPrint(String(lengthb));

  unsigned char binary[lengthb];
  unsigned char base64[4 * (lengthb / 3)];

  temp = 0;
  temp_last = 0;

  if (length >= MAX_FIFO_SIZE) //512 kb
  {
    myPrint(F("ACK CMD Over size. END"));
    return;
  }
  if (length == 0 ) //0 kb
  {
    myPrint(F("ACK CMD Size is 0. END"));
    return;
  }
  myCAM.CS_LOW();
  myCAM.set_fifo_burst();//Set fifo burst mode
  temp =  SPI.transfer(0x00);
  int i = 0;
  myPrint(String(length));
  length --;
  while ( length-- )
  {
    temp_last = temp;
    temp =  SPI.transfer(0x00);
    if (is_header == true)
    {
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

  // ########### START SENDING PICTURE ###########

  myPrint("start sending");


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
    reconnect();
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
      reconnect();
    }
    client.stop();
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
    //delay(100);

  }

  //client.stop();

  //Clear the capture done flag
  myCAM.clear_fifo_flag();

  //delay(500);
}


void myPrint(String str) {
  if (PRINT) {
    Serial.println(str);
  }
}

void reconnect() {
  delay(1000);
  myPrint("waiting");
  WiFi.endAP(true); // to disable default automatic start of persistent AP at startup

  WiFi.setPersistent(); // set the following WiFi connection as persistent

  myPrint("Attempting to connect to SSID: ");
  myPrint(ssid);

  WiFi.config(ip, gw, gw, nm);

  int status = WiFi.begin(ssid, pass);
}
