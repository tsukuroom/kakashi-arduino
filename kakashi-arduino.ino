#include <Servo.h>
#include <Adafruit_NeoPixel.h>
#include <NewPing.h>

#define PING_TRIGGER_PIN  12
#define PING_ECHO_PIN     12
#define PING_MAX_DISTANCE 200

#define LED_PIN   6
#define NUM_LEDS  8
#define LED_BRIGHTNESS  10

#define SERVO_PIN 10
#define SERVO_DELAY 300

#define DEBUG 1
#ifdef DEBUG
#define DEBUG_PRINT(val) Serial.print(val)
#define DEBUG_PRINTLN(val) Serial.println(val)
#else
#define DEBUG_PRINT(val)
#define DEBUG_PRINTLN(val)
#endif

NewPing sonar(PING_TRIGGER_PIN, PING_ECHO_PIN, PING_MAX_DISTANCE);
Adafruit_NeoPixel stick(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);
Servo hand;

int pingThreshold = 0; //cm
boolean monitoring = false;

int gamma[] = {
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1,
    1,  1,  1,  1,  1,  1,  1,  1,  1,  2,  2,  2,  2,  2,  2,  2,
    2,  3,  3,  3,  3,  3,  3,  3,  4,  4,  4,  4,  4,  5,  5,  5,
    5,  6,  6,  6,  6,  7,  7,  7,  7,  8,  8,  8,  9,  9,  9, 10,
   10, 10, 11, 11, 11, 12, 12, 13, 13, 13, 14, 14, 15, 15, 16, 16,
   17, 17, 18, 18, 19, 19, 20, 20, 21, 21, 22, 22, 23, 24, 24, 25,
   25, 26, 27, 27, 28, 29, 29, 30, 31, 32, 32, 33, 34, 35, 35, 36,
   37, 38, 39, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 50,
   51, 52, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 66, 67, 68,
   69, 70, 72, 73, 74, 75, 77, 78, 79, 81, 82, 83, 85, 86, 87, 89,
   90, 92, 93, 95, 96, 98, 99,101,102,104,105,107,109,110,112,114,
  115,117,119,120,122,124,126,127,129,131,133,135,137,138,140,142,
  144,146,148,150,152,154,156,158,160,162,164,167,169,171,173,175,
  177,180,182,184,186,189,191,193,196,198,200,203,205,208,210,213,
  215,218,220,223,225,228,231,233,236,239,241,244,247,249,252,255 };

void setup() {
  Serial.begin(9600);

  stick.setBrightness(LED_BRIGHTNESS);
  stick.begin();
  stick.show();

  moveHand(SERVO_PIN, 0);
}

void loop() {
  if (Serial.available() > 0) {
    String command;
    while (1) {
      if(Serial.available()) {
        char temp = char(Serial.read());
        command += temp;
        if (temp == '\n') {
          break;
        }
      }
    }
    command.trim();
    
    if (command.startsWith("start")) {
      DEBUG_PRINTLN("monitoring start");
      pingThreshold = command.substring(6).toInt();
      if (pingThreshold == 0) {
        DEBUG_PRINTLN("Invalid threshold");
      } else {
        monitoring = true;
      }
    } else if (command.equals("stop")) {
      DEBUG_PRINTLN("monitoring stop");
      monitoring = false;
    } else if (command.equals("cracker")) {
      DEBUG_PRINTLN("pull cracker");
      moveHand(SERVO_PIN, 180);
      rainbowFade(1, 5);
      moveHand(SERVO_PIN, 0);
    } else {
      DEBUG_PRINTLN("Unknown command");
    }
  }

  if (monitoring) {
    unsigned long cm = sonar.ping_cm();
    DEBUG_PRINT("Ping: ");
    DEBUG_PRINT(cm);
    DEBUG_PRINTLN("cm");

    if (cm <= 0) {
      cm = PING_MAX_DISTANCE;
    }
    if (cm > 0 && cm < pingThreshold) {
      Serial.println("chikai");
    }

    uint8_t wait = calcWait(cm);
    pulseWhite(wait, 2);
  }
}

uint8_t calcWait(unsigned long cm) {
  if (cm < 30) {
    return 1;
  } else {
    return (cm - 30) / 34 + 1;
  }
}

void pulseWhite(uint8_t wait, int speed) {
  Serial.println(wait);
  for(int j = speed - 1; j < 256 ; j = j + speed){
    for(uint16_t i=0; i<stick.numPixels(); i++) {
      stick.setPixelColor(i, stick.Color(gamma[j],gamma[j],gamma[j] ) );
    }
    delay(wait);
    stick.show();
  }

  for(int j = 255; j >= 0 ; j = j - speed){
    for(uint16_t i=0; i<stick.numPixels(); i++) {
      stick.setPixelColor(i, stick.Color(gamma[j],gamma[j],gamma[j] ) );
    }
    delay(wait);
    stick.show();
  }
}

void rainbowFade(uint8_t wait, int rainbowLoops) {
  float fadeMax = 100.0;
  int fadeVal = 0;
  uint32_t wheelVal;
  int redVal, greenVal, blueVal;

  for(int k = 0 ; k < rainbowLoops ; k ++){
    
    for(int j=0; j<256; j++) { // 5 cycles of all colors on wheel

      for(int i=0; i< stick.numPixels(); i++) {

        wheelVal = Wheel(((i * 256 / stick.numPixels()) + j) & 255);

        redVal = red(wheelVal) * float(fadeVal/fadeMax);
        greenVal = green(wheelVal) * float(fadeVal/fadeMax);
        blueVal = blue(wheelVal) * float(fadeVal/fadeMax);

        stick.setPixelColor( i, stick.Color( redVal, greenVal, blueVal ) );

      }

      //First loop, fade in!
      if(k == 0 && fadeVal < fadeMax-1) {
          fadeVal++;
      }

      //Last loop, fade out!
      else if(k == rainbowLoops - 1 && j > 255 - fadeMax ){
          fadeVal--;
      }

        stick.show();
        delay(wait);
    }
  
  }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return stick.Color(255 - WheelPos * 3, 0, WheelPos * 3,0);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return stick.Color(0, WheelPos * 3, 255 - WheelPos * 3,0);
  }
  WheelPos -= 170;
  return stick.Color(WheelPos * 3, 255 - WheelPos * 3, 0,0);
}

uint8_t red(uint32_t c) {
  return (c >> 8);
}
uint8_t green(uint32_t c) {
  return (c >> 16);
}
uint8_t blue(uint32_t c) {
  return (c);
}

void moveHand(int pin, int value) {
  hand.attach(pin);
  hand.write(value);
  delay(SERVO_DELAY);
  hand.detach();
}
