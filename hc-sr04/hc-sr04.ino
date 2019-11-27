// ---------------------------------------------------------------------------
// Install NewPing library before use
// ---------------------------------------------------------------------------

#include <NewPing.h>
// first sensor
const int trigPin1 = 3;
const int echoPin1 = 2; // never connect directly to Arduino Nano 33 because it is 5v
// second sensor
const int trigPin2 = 5;
const int echoPin2 = 4;
bool check1 = true;
bool check2 = true;
const int distance = 100;
int people = 0;
unsigned long t1 = 0;
unsigned long t2 = 0;
NewPing sonar(trigPin1, echoPin1);
NewPing sonar2(trigPin2, echoPin2);
void setup() {
  Serial.begin(115200);
}

void loop() {
  delay(100);

  int v1 = int(sonar.ping_cm());
  int v2 = int(sonar2.ping_cm());


  if (v1 < distance and check1 == true) {
    t1 = millis();
    check1 = false;
  }

  if (v2 < distance and check2 == true) {
    t2 = millis();
    check2 = false;
  }
  Serial.println(t1);
  Serial.println(t2);

  if (t1 > 0 and t2 > 0) {
    Serial.println(t1 - t2);
    if (t1 - t2 < 0 ) {
      people ++;
      reset(v1, v2);
    } else if (t1 - t2 > 0) {
      people--;
      reset(v1, v2);
    }
  }





  Serial.print(String(v1));
  Serial.print("-");
  Serial.println(String(v2));


  Serial.println("People: " + String(people));
}

void reset(int v1, int v2) {
  if (v1 > distance and v2 > distance) {
    t1 = 0;
    t2 = 0;
    check1 = true;
    check2 = true;
  }
}
