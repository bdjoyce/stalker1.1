//Stalker 1.1
//by Brian Joyce
//based on 'The Stalker' by Heuschele et al. 2019
//https://www.hardware-x.com/article/S2468-0672(18)30118-4/fulltext
//https://github.com/UMNBBETech/Stalker

//includes
#include <Wire.h>
#include <SPI.h>
#include <SD.h>

//sensors and library versions below

#include <HX711_ADC.h>
//https://github.com/olkal/HX711_ADC 
//version 1.2.12

#include <SparkFun_VL53L1X.h>
//https://github.com/sparkfun/SparkFun_VL53L1X_Arduino_Library
//version 1.2.12

#include <SparkFun_MMA8452Q.h>
//https://github.com/adafruit/Adafruit_MMA8451_Library
//version 1.2.21




//objects and definitions


// HX711 circuit
const int LOADCELL_DOUT_PIN = 4;
const int LOADCELL_SCK_PIN = 5;
HX711_ADC LoadCell(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);

// distance sensor
SFEVL53L1X distanceSensor;

//acceleramotoer
MMA8452Q accel;
#define UPDOWN_AXIS -ay     //updown directon based on orientation of the accel sensor
#define FLAT_AXIS az       //flat to the ground based on oreintation of the accel sensor


// LED defs
const int BLUE_LED_PIN = 7;
const int RED_LED_PIN = 6;





//setup functions
void setup() {

  //Serial init for OpenTx logger
  Serial.begin(9600);
  Serial.print("init start");

  //indicator LED init
  Serial.print(" led");
  pinMode(BLUE_LED_PIN, OUTPUT);
  digitalWrite(BLUE_LED_PIN, HIGH);  //digital pin state high means LED off
  pinMode(RED_LED_PIN, OUTPUT);
  digitalWrite(RED_LED_PIN, LOW);

  //init for I2C (used for measurements)
  Serial.print(" wire");
  Wire.begin();

  //loadcell initialization
  Serial.print(" loadcell");
  LoadCell.begin();
  LoadCell.start(0);
  LoadCell.setCalFactor(696.0);

  //distance sensor initialization
  Serial.print(" dist");
  distanceSensor.begin();
  distanceSensor.startRanging();

  //acceleraomoter initialization
  Serial.print(" accel");
  accel.init();

  //init done
  Serial.print(" done");
  Serial.print("\n");

  //print a dataline to start the process
  dataline();

  //pause for 3sec to let the sensors settle
  //flashing the LED to let the user know
  for (int i=0; i<25; i++) {
    digitalWrite(BLUE_LED_PIN, (i%2));
    digitalWrite(RED_LED_PIN, ((i+1)%2));
    delay(3000/25);
  }

  //setup done, put LED blue to start the measurement
  digitalWrite(RED_LED_PIN, HIGH);  //LED off
  digitalWrite(BLUE_LED_PIN, LOW);   //LED on

  //put a nicer header to make parsing easier
  Serial.print("========\n");
  Serial.print("millis,loadcell,distance,accel x, accel y, accel z,angle\n");
  
}

void loop() {

  //now with initialization complete collect data in a loop
  dataline();

}

void dataline() {

  //start with timestame (in milliseconds)
  Serial.print(millis());
  Serial.print(",");

  //update weight data
  LoadCell.update();
  Serial.print(LoadCell.getData());
  Serial.print(",");

  //update distance data
  int distance = distanceSensor.getDistance();
  Serial.print(distance);
  Serial.print(",");

  //update angle data
  float ax = accel.getCalculatedX();
  float ay = accel.getCalculatedY();
  float az = accel.getCalculatedZ();
  Serial.print(ax, 3);
  Serial.print(",");
  Serial.print(ay, 3);
  Serial.print(",");
  Serial.print(az, 3);
  Serial.print(",");
  float angle = ((atan2(FLAT_AXIS,UPDOWN_AXIS))*180)/PI;
  Serial.print(angle,3);
  Serial.print(",");

  //update LED based on angle
  if (angle>45) {
    digitalWrite(BLUE_LED_PIN, HIGH);  //LED off
    digitalWrite(RED_LED_PIN, LOW);   //LED on
  }
  
  //newline to setup for next entry
  Serial.print("\n");
  
}
