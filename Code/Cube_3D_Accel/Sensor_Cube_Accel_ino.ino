/*
 https://github.com/adafruit/Adafruit_SSD1306
 https://github.com/adafruit/Adafruit-GFX-Library
 https://github.com/adafruit/Adafruit_ADXL345
 https://github.com/adafruit/Adafruit_Sensor
*/

// The 3D Cube code was grabbed from a MicroView Example
// MicroViewCube.ino
// Rotating a 3-D Cube on the MicroView Display
// Jim Lindblom @ SparkFun Electronics
// Original Creation Date: June 9, 2014 

// Define includes
#include <SPI.h>  //needed
#include <Adafruit_GFX.h>  //NEW
#include <avr/pgmspace.h>
#include <Wire.h>        // Wire header file for I2C and 2 wire
#include <ADXL345.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_ADXL345_U.h>
#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET); 

//Accelerometer Stuff
Adafruit_ADXL345_Unified accel = Adafruit_ADXL345_Unified(12345);

#define SHAPE_SIZE 900 //600
#define ROTATION_SPEED 0 // ms delay between cube draws

float d = 3;
float px[] = { -d,  d,  d, -d, -d,  d,  d, -d };
float py[] = { -d, -d,  d,  d, -d, -d,  d,  d };
float pz[] = { -d, -d, -d, -d,  d,  d,  d,  d };

float p2x[] = {0,0,0,0,0,0,0,0};
float p2y[] = {0,0,0,0,0,0,0,0};

float r[] = {0,0,0};

// Setup the Arduino LCD (Columns, Rows)
void setup()
{ 
  Wire.begin();                  // Join the I2C bus as a master
  Serial.begin(9600);            // Set Serial Port speed

  display.begin(SSD1306_SWITCHCAPVCC, 0x3c);  // initialize with the I2C addr

  display.display(); // show splashscreen
  delay(1000);
  display.clearDisplay();   // clears the screen and buffer   
  delay(300); 
  accel.begin();

   display.clearDisplay();
   display.setTextSize(2);
   display.setTextColor(WHITE);

   display.setCursor(0,0); 
   display.println("3D Cube");
   display.setCursor(0,20); 
   display.println("and Accel");
   display.setCursor(0,40); 
   display.println("Test");
   display.display();
   delay(3000);
}

void loop()
{ 
   accel.setRange(ADXL345_RANGE_2_G);
   
   //Read Accelerometer
   sensors_event_t event; 
   accel.getEvent(&event);
   
   float r[] = {(event.acceleration.y/15.0*-1),event.acceleration.x/15.0,event.acceleration.z/15.0+100};
   
   if (r[0] >= 360.0*PI/180.0) r[0] = 0;
   if (r[1] >= 360.0*PI/180.0) r[1] = 0;
   if (r[2] >= 360.0*PI/180.0) r[2] = 0;

   for (int i=0;i<8;i++)
   {
    float px2 = px[i];
    float py2 = cos(r[0])*py[i] - sin(r[0])*pz[i];
    float pz2 = sin(r[0])*py[i] + cos(r[0])*pz[i];

    float px3 = cos(r[1])*px2 + sin(r[1])*pz2;
    float py3 = py2;
    float pz3 = -sin(r[1])*px2 + cos(r[1])*pz2;

    float ax = cos(r[2])*px3 - sin(r[2])*py3;
    float ay = sin(r[2])*px3 + cos(r[2])*py3;
    float az = pz3-150;
		
    p2x[i] = 128/2+ax*SHAPE_SIZE/az;
    p2y[i] = 64/2+ay*SHAPE_SIZE/az;
   }

    display.display();
    display.clearDisplay(); 

    for (int i=0;i<3;i++) 
   {
    display.drawLine(p2x[i],p2y[i],p2x[i+1],p2y[i+1], WHITE);
    display.drawLine(p2x[i+4],p2y[i+4],p2x[i+5],p2y[i+5], WHITE );
    display.drawLine(p2x[i],p2y[i],p2x[i+4],p2y[i+4], WHITE);
   }    
    display.drawLine(p2x[3],p2y[3],p2x[0],p2y[0], WHITE);
    display.drawLine(p2x[7],p2y[7],p2x[4],p2y[4], WHITE);
    display.drawLine(p2x[3],p2y[3],p2x[7],p2y[7], WHITE);
}
     
     
     
     
     
     









