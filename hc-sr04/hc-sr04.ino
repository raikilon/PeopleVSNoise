// ---------------------------------------------------------------------------
// Install NewPing library before use
// ---------------------------------------------------------------------------

#include <NewPing.h>
// first sensor
const int trigPin1 = 5;
const int echoPin1 = 4; // never connect directly to Arduino Nano 33 because it is 5v
// second sensor
const int trigPin2 = 12;
const int echoPin2 = 11;

// value to check if it is in or out direction
bool checkIn = false;
bool checkOut = false;

// count how many people enter a room (one by one)
int count = 0;
// timer to reset
unsigned long t = 0;

// old ultrasonic sensor value (previous iteration)
int ov1 = 0;
int ov2 = 0;
// max variation of the sensor 
const int MAX_VARIATION = 5;

// dalay before reset
const int DELAY = 3000;


// ultrasonic sensor object
NewPing sonar(trigPin1, echoPin1);
NewPing sonar2(trigPin2, echoPin2);


void setup() {
  Serial.begin(115200);

  // inizialise values
  ov1 = int(sonar.ping_cm());
  // delay between readings otherwise it does not work correctly
  delay(50);
  ov2 = int(sonar2.ping_cm());
}

void loop() {
  delay(100);

  int v1 = int(sonar.ping_cm());
  delay(50);
  int v2 = int(sonar2.ping_cm());

  // if there are not action for DELAY time reset values
  if (millis() - t > DELAY) {
    reset();
  }

  // value can vary a little bit
  // if it change more (it decrease drastically), someone is on v1
  if (v1 < ov1 - MAX_VARIATION) {
    // if somebody was detected before in out sensor then somebody is leaving
    if (checkOut ==  true) {
      count--;
      Serial.println("OUT");
      reset();
    } else {
      checkIn = true;
    }

    t =  millis();
  } 
  if (v2 < ov2 - MAX_VARIATION) {
    if (checkIn ==  true) {
      count++;
      Serial.println("IN");
      reset();
    } else {
      checkOut = true;
    }

    t =  millis();
  }

  // replace old values
  ov1 = v1;
  ov2 = v2;

  //Serial.print(String(v1));
  //Serial.print("-");
  //Serial.println(String(v2));


  //Serial.println("People: " + String(count));
}

void reset() {
  checkIn  = false;
  checkOut = false;
}
