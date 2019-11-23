/*
  Web client

  This sketch connects to a website (http://arduino.cc)
  using the WiFi module.

  created 13 July 2010
  by dlf (Metodo2 srl)
  modified 31 May 2012
  by Tom Igoe
  modified in Jul 2019 for WiFiEspAT library
  by Juraj Andrassy https://github.com/jandrassy
*/

#include <WiFiEspAT.h>

const char ssid[] = "noli";    // your network SSID (name)
const char pass[] = "Nolinoli";    // your network password (use for WPA, or use as key for WEP)

const char* server = "192.168.43.75";

WiFiClient client;

void setup() {
  Serial.begin(9600);
  while (!Serial);

  Serial1.begin(115200);
  WiFi.init(Serial1);

  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println();
    Serial.println("Communication with WiFi module failed!");
    // don't continue
    while (true);
  }

  WiFi.endAP(true); // to disable default automatic start of persistent AP at startup

  WiFi.setPersistent(); // set the following WiFi connection as persistent

  //  uncomment this lines for persistent static IP. set addresses valid for your network


  Serial.println();
  Serial.print("Attempting to connect to SSID: ");
  Serial.println(ssid);

  //  use following lines if you want to connect with bssid
  //  const byte bssid[] = {0x8A, 0x2F, 0xC3, 0xE9, 0x25, 0xC0};
  //  int status = WiFi.begin(ssid, pass, bssid);
  IPAddress ip(192, 168, 43, 10);
  IPAddress gw(192, 168, 43, 1);
  IPAddress nm(255, 255, 255, 0);
  WiFi.config(ip, gw, gw, nm);
  int status = WiFi.begin(ssid, pass);

  if (status == WL_CONNECTED) {
    WiFi.setAutoConnect(true);
    Serial.println();
    Serial.println("Connected to WiFi network.");
    printWifiStatus();
  } else {
    WiFi.disconnect(); // remove the WiFi connection
    Serial.println();
    Serial.println("Connection to WiFi network failed.");
  }

  Serial.println("Starting connection to server...");
  if (client.connect(server, 5000)) {
    Serial.println("connected to server");

//    String msg = "{\"username\":\"test\"}";
//    String post = "POST /data HTTP/1.1 \r\n Host: "+ String(server) + ":5000 \r\n Content-Length: " + String(msg.length()) + "\r\n Content-Type: application/json \r\n\r\n" + msg + "\r\n";
//    Serial.println(post);
//    char copy[post.length()];
//    post.toCharArray(copy,post.length());
//    client.write(copy);

    String PostData = "{\"username\":\"test\"}";
    client.println("POST /data HTTP/1.1");
    client.println("User-Agent: Arduino/1.0");
    client.println("Connection: close");
    client.print("Content-Length: ");
    client.println(PostData.length());
    client.println("Content-Type: application/json");
    client.println();
    client.println(PostData);
    
    //client.flush();
  }
}

void loop() {

  // if there are incoming bytes available
  // from the server, read them and print them
  while (client.available()) {
    char c = client.read();
    Serial.write(c);
  }

  // if the server's disconnected, stop the client
  if (!client.connected()) {
    Serial.println();
    Serial.println("disconnecting from server.");
    client.stop();

    // do nothing forevermore
    while (true);
  }
}

void printWifiStatus() {

  // print the SSID of the network you're attached to:
  char ssid[33];
  WiFi.SSID(ssid);
  Serial.print("SSID: ");
  Serial.println(ssid);

  // print the BSSID of the network you're attached to:
  uint8_t bssid[6];
  WiFi.BSSID(bssid);
  Serial.print("BSSID: ");
  printMacAddress(bssid);

  uint8_t mac[6];
  WiFi.macAddress(mac);
  Serial.print("MAC: ");
  printMacAddress(mac);

  // print your board's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}

void printMacAddress(byte mac[]) {
  for (int i = 5; i >= 0; i--) {
    if (mac[i] < 16) {
      Serial.print("0");
    }
    Serial.print(mac[i], HEX);
    if (i > 0) {
      Serial.print(":");
    }
  }
  Serial.println();
}
