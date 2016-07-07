#include <NewPing.h>

#define PING_TRIGGER_PIN  12
#define PING_ECHO_PIN     12
#define PING_MAX_DISTANCE 200
#define PING_INTERVAL 1000 //msec

NewPing sonar(PING_TRIGGER_PIN, PING_ECHO_PIN, PING_MAX_DISTANCE);

int pingThreshold = 0; //cm

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  if (Serial.available() > 0) {
    String command = Serial.readString();
    command.trim();
    
    if (command.startsWith("start")) {
      Serial.println("monitoring start");
      pingThreshold = command.substring(6).toInt();
      if (pingThreshold == 0) {
        Serial.println("Invalid threshold");
      } else {
        sonar.timer_ms(PING_INTERVAL, ping);
      }
      Serial.println(pingThreshold);
    } else if (command.equals("stop")) {
      Serial.println("monitoring stop");
      sonar.timer_stop();
    } else if (command.equals("cracker")) {
      Serial.println("pull cracker");
    } else {
      Serial.println("Unknown command");
    }
  }
}

void ping() {
  //int cm = sonar.ping_cm();
  unsigned int us = sonar.ping_median(10);
  unsigned int cm = us / US_ROUNDTRIP_CM;
  
  Serial.print("Ping: ");
  Serial.print(cm);
  Serial.println("cm");

  if (cm != 0 && cm < pingThreshold) {
    Serial.println("chikai");
  }
}

