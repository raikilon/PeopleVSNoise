/*
  This example reads audio data from the on-board PDM microphones, and prints
  out the samples to the Serial console. The Serial Plotter built into the
  Arduino IDE can be used to plot the audio data (Tools -> Serial Plotter)

  Circuit:
  - Arduino Nano 33 BLE board

  This example code is in the public domain.
*/

#include <PDM.h>

#include <WiFiEspAT.h>

const char ssid[] = "noli";    // your network SSID
const char pass[] = "Nolinoli";    // your network password (use for WPA, or use as key for WEP)

const char* server = "192.168.43.75"; // server adress

WiFiClient client;
const IPAddress ip(192, 168, 43, 42);
const IPAddress gw(192, 168, 43, 1);
const IPAddress nm(255, 255, 255, 0);

// Samples to read to compute RMS
const int samples = 16384;

// buffer to read samples into, each sample is 16-bits
short sampleBuffer[256];

volatile int samplesRead = 0;
volatile int totalSamplesRead = 0;
volatile int totalSquare = 0;

float rms = 0.0;
float db = 0.0;

bool PRINT = true; // Debug

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

void setup() {
  Serial.begin(115200);
  while (!Serial);

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

  // ### INIT PDM ###

  // configure the data receive callback
  PDM.onReceive(onPDMdata);

  // optionally set the gain, defaults to 20
  // PDM.setGain(30);

  // initialize PDM with:
  // - one channel (mono mode)
  // - a 16 kHz sample rate
  if (!PDM.begin(1, 16000)) {
    myPrint("Failed to start PDM!");
    while (1);
  }
}

// Compute square component of the RMS
int compute_samples_square(short sampleBuffer[256], int samplesRead)
{
  int square = 0;
  for (int i = 0; i < samplesRead; i++) {
    square += pow(abs(sampleBuffer[i]), 2);
  }
  return square;
}

void loop() {
    // wait for samples to be read
    if (samplesRead) {
      // Accumulating the square component
      totalSquare += compute_samples_square(sampleBuffer, samplesRead);
      totalSamplesRead += samplesRead;
      // clear the read count
      samplesRead = 0;
    }

    if (totalSamplesRead >= samples)
    {
      // Computing the mean and the square root
      rms = sqrt(totalSquare/totalSamplesRead);
      // clear the accumulated values
      totalSamplesRead = 0;
      totalSquare = 0;
      // computing the decibels from the logarithmic regression formula
      db = 15.871*log(rms)-7.0586;
      myPrint(String(db));

       // wait for connection
      while (WiFi.status() != WL_CONNECTED) {
        reconnect();
      }
  
      String data = "{\"db\":" + String(db) + "}";
//      client.stop();
      myPrint("Starting connection to server...");

      if (client.connect(server, 5000)) {
        myPrint("connected to server");
        client.println("POST /decibels HTTP/1.1");
        client.println("User-Agent: Arduino/1.0");
        client.println("Connection: keep-alive");
        client.print("Content-Length: ");
        client.println(data.length());
        client.println("Content-Type: application/json");
        client.println();
        client.println(data);
        client.flush();
      }
    }
}

void onPDMdata() {
  // query the number of bytes available
  int bytesAvailable = PDM.available();

  // read into the sample buffer
  PDM.read(sampleBuffer, bytesAvailable);

  // 16-bit, 2 bytes per sample
  samplesRead = bytesAvailable / 2;
}
