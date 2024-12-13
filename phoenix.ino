#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif
#include <movingAvg.h>
#include <MobaTools.h>

#define photoPin1 A0
#define photoPin2 A1
#define pixelPin 9

Adafruit_NeoPixel strip = Adafruit_NeoPixel(59, pixelPin, NEO_GRB + NEO_KHZ800);

int lightVal1, lightVal2, currentReading, currentAvg, historicalAvg;
movingAvg longTermReading(20);
bool triggered = false;

MoToServo left_servo, middle_servo, right_servo;

const byte speed = 15;

const byte left_target_1 = 0;
const byte left_target_2 = 80;

const byte middle_target_1 = 180;
const byte middle_target_2 = 120;

const byte right_target_1 = 180;
const byte right_target_2 = 120;

const byte bottom_pause = 1000;

byte left_target_pos = left_target_1;
byte middle_target_pos = middle_target_1;
byte right_target_pos = right_target_1;

unsigned long start_time = millis();
// 30 seconds
unsigned long motor_runtime = 1000 * 30;

void setup() {
  Serial.begin(9600);
  left_servo.attach(3); //yellow
  middle_servo.attach(5); //blue
  right_servo.attach(6); //purple
  left_servo.write(0);
  //90 -> 0: moves clockwise up
  // 0 is relaxed, 180 is fully pulled down
  // I left it on 0.
  middle_servo.write(180);
  right_servo.write(180);

  left_servo.setSpeed(speed);
  middle_servo.setSpeed(speed);
  right_servo.setSpeed(speed);

  longTermReading.begin();
  strip.begin();
  strip.setBrightness(50);
  strip.show();
}

// IMPORTANT: To reduce NeoPixel burnout risk, add 1000 uF capacitor across
// pixel power leads, add 300 - 500 Ohm resistor on first pixel's data input
// and minimize distance between Arduino and first pixel.  Avoid connecting
// on a live circuit...if you must, connect GND first.
void loop() {
    lightVal1 = analogRead(photoPin1);
    lightVal2 = analogRead(photoPin2);
    currentReading = (lightVal1 + lightVal2)/2;
    historicalAvg = longTermReading.reading(currentReading);
   // Mimic a Serial.println
   delay(10);
   if (currentReading < (historicalAvg * 0.95) && !triggered) {
    trigger();
   }

 }

void trigger(){
  triggered = true;
  Serial.println("triggered");
  showLEDs();
  moveMotors();
  wipeLEDs();
  triggered = false;
}

void showLEDs(){
  colorWipe(strip.Color(255, 0, 0), 10); // Red
  // Come down from the phoenix tail, first pixel is the last one in the strip
  for (uint16_t i = 0; i <= 255; i=i+50) {
    // Yellow is 255, 255, 0, so this gradually wipes from red to orangeish to yellow
    colorWipe(strip.Color(255, i, 0), 1);
  }
  //hold at yellow
  strip.Color(255, 255, 0);
  strip.show();
}

void wipeLEDs(){
  strip.clear();
  strip.show();
}

void moveMotors(){
    // run for x seconds, and also be sure to end with motors pointing up
    start_time = millis();
    while (millis() - start_time < motor_runtime) {
      move();
    }
    left_servo.write(left_target_1);
    middle_servo.write(middle_target_1);
    right_servo.write(right_target_1);
}


void move() {
  if (!left_servo.moving()) {
    if (left_servo.read() == left_target_1) {
      left_target_pos = left_target_2;
    }
    if (left_servo.read() == left_target_2) {
      moveRightServo();
      left_target_pos = left_target_1;
    }
    left_servo.write(left_target_pos);  //will move slowly
  }
  moveMiddleServo();
  moveRightServo();
}

void moveMiddleServo() {
  if (!middle_servo.moving()) {
    if (middle_servo.read() == middle_target_1) {
      middle_target_pos = middle_target_2;
    }
    if (middle_servo.read() == middle_target_2) {
      delay(bottom_pause);
      middle_target_pos = middle_target_1;
    }
    middle_servo.write(middle_target_pos);
  }
}

void moveRightServo() {
  if (!right_servo.moving()) {
    if (right_servo.read() == right_target_1) {
      right_target_pos = right_target_2;
    }
    if (right_servo.read() == right_target_2) {
      right_target_pos = right_target_1;
    }
    right_servo.write(right_target_pos);
  }
}


// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait) {
  for(int i=strip.numPixels()-1; i >= 0; i--) {
    strip.setPixelColor(i, c);
    strip.show();
    delay(wait);
  }
}
