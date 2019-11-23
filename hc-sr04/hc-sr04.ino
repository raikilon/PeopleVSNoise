// ---------------------------------------------------------------------------
// Install NewPing library before use
// ---------------------------------------------------------------------------

#include <NewPing.h>
const int trigPin1 = 3;
const int echoPin1 = 2;
const int trigPin2 = 5;
const int echoPin2 = 4;

NewPing sonar(trigPin1, echoPin1); 
NewPing sonar2(trigPin2, echoPin2); 
void setup() {
  Serial.begin(115200); 
}

void loop() {
  delay(100);                     
  Serial.print("Ping: ");
  Serial.print(sonar.ping_cm());
  Serial.print("-");
  Serial.print(sonar2.ping_cm());
  Serial.println("cm");
}
