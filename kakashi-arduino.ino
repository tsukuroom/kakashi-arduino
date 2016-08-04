#include <Servo.h>
#include <Adafruit_NeoPixel.h>
#include <NewPing.h>

#define PING_TRIGGER_PIN  12
#define PING_ECHO_PIN     12
#define PING_MAX_DISTANCE 200

#define LED_PIN   6
#define NUM_LEDS  8
#define LED_BRIGHTNESS  255

#define SERVO_PIN 10
#define SERVO_DELAY 300

//#define DEBUG 1
#ifdef DEBUG
#define DEBUG_PRINT(val) Serial.print(val)
#define DEBUG_PRINTLN(val) Serial.println(val)
#else
#define DEBUG_PRINT(val)
#define DEBUG_PRINTLN(val)
#endif

enum State {
  IDLE,
  MONITOR, // Monitoring distance
  RECOGNIZE  // Bird recognize
};

NewPing sonar(PING_TRIGGER_PIN, PING_ECHO_PIN, PING_MAX_DISTANCE);
Adafruit_NeoPixel stick(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);
Servo hand;

int pingThreshold = 50; //cm
State state = IDLE;

uint8_t gamma[] = {
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

  state = IDLE;
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
    
    if (command.startsWith("monitor")) {
      int param = command.substring(8).toInt();
      if (param == 0) {
        DEBUG_PRINTLN("Invalid threshold");
      } else {
        DEBUG_PRINTLN("enter monitor state");
        pingThreshold = param;
        state = MONITOR;
      }
    } else if (command.equals("stop")) {
      DEBUG_PRINTLN("enter idle state");
      state = IDLE;
    } else if (command.equals("recognize")) {
      DEBUG_PRINTLN("enter recognize state");
      pulseGreen(5, 2, 2);
      state = RECOGNIZE;
    } else if (command.equals("cracker")) {
      cracker();
      state = MONITOR;
    } else if (command.equals("green")) {
      pulseGreen(5, 2, 2);
    } else if (command.equals("blue")) {
      fadeBlue();
      state = MONITOR;
    } else {
      DEBUG_PRINTLN("Unknown command");
    }
  }

  if (state == MONITOR) {
    monitor();
  } else if (state == RECOGNIZE) {
    rainbow();
  } else {
    delay(10);
  }
}

void cracker() {
  DEBUG_PRINTLN("cracker");
  moveHand(SERVO_PIN, 180);
  pulseRed(1, 4, 10);
  fadeIn(1, 8, true, false, false);
  delay(2000);
  moveHand(SERVO_PIN, 0);
  fadeOut(5, 2, true, false, false);
}

void fadeBlue() {
  fadeIn(2, 2, false, false, true);
  delay(2000);
  fadeOut(2, 2, false, false, true);  
}

void monitor() {
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

void rainbow() {
  rainbowFade(5, 1);
}

uint8_t calcWait(unsigned long cm) {
  if (cm < 30) {
    return 1;
  } else {
    return (cm - 30) / 34 + 1;
  }
}

void pulseBlue(uint8_t wait, int speed, uint8_t times) {
  for (uint8_t i = 0; i < times; i++) {
    pulse(wait, speed, false, false, true);
  }
}

void pulseGreen(uint8_t wait, int speed, uint8_t times) {
  for (uint8_t i = 0; i < times; i++) {
    pulse(wait, speed, false, true, false);
  }  
}

void pulseRed(uint8_t wait, int speed, uint8_t times) {
  for (uint8_t i = 0; i < times; i++) {
    pulse(wait, speed, true, false, false);
  }    
}

void pulseWhite(uint8_t wait, int speed) {
  pulse(wait, speed, true, true, true);
}

void pulse(uint8_t wait, int speed, boolean rValid, boolean gValid, boolean bValid) {
  fadeIn(wait, speed, rValid, gValid, bValid);
  fadeOut(wait, speed, rValid, gValid, bValid);
}

void fadeIn(uint8_t wait, int speed, boolean rValid, boolean gValid, boolean bValid) {
  uint8_t rMask = rValid? 0xff : 0x0;
  uint8_t gMask = gValid? 0xff : 0x0;
  uint8_t bMask = bValid? 0xff : 0x0;
  
  for(int j = speed - 1; j < 256 ; j = j + speed){
    for(uint16_t i=0; i<stick.numPixels(); i++) {
      stick.setPixelColor(i, stick.Color(gamma[j] & rMask, gamma[j] & gMask, gamma[j] & bMask));
    }
    delay(wait);
    stick.show();
  }  
}

void fadeOut(uint8_t wait, int speed, boolean rValid, boolean gValid, boolean bValid) {
  uint8_t rMask = rValid? 0xff : 0x0;
  uint8_t gMask = gValid? 0xff : 0x0;
  uint8_t bMask = bValid? 0xff : 0x0;
  
  for(int j = 255; j >= 0 ; j = j - speed){
    for(uint16_t i=0; i<stick.numPixels(); i++) {
      stick.setPixelColor(i, stick.Color(gamma[j] & rMask, gamma[j] & gMask, gamma[j] & bMask));
    }
    delay(wait);
    stick.show();
  }  
}

void rainbowFade(uint8_t wait, int rainbowLoops) {
  float fadeMax = 100.0;
  int fadeVal = 100;
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

      stick.show();
      delay(wait);
    }
  
  }

  stick.clear();
  stick.show();
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

