/*
 Draws a 3d rotating cube on the freetronics OLED screen.
 Original code was found at http://forum.freetronics.com/viewtopic.php?f=37&t=5495
 Thanks to Adafruit at http://www.adafruit.com for the great display and sensor libraries
 Hardware availible for purchase at https://www.tindie.com/products/miker/arduino-sensor-board/
 
 https://github.com/adafruit/Adafruit_SSD1306
 https://github.com/adafruit/Adafruit-GFX-Library
 https://github.com/adafruit/Adafruit_MPL115A2
 https://github.com/adafruit/Adafruit_Sensor
 */

#include <Adafruit_GFX.h>  //NEW
#include <avr/pgmspace.h>
#include "data.h"
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);

//Barometer Stuff
#include <Adafruit_MPL115A2.h>
#include <Adafruit_Sensor.h>
Adafruit_MPL115A2 mpl115a2;

//RTC Stuff
#define DS1307_ADDRESS 0x68 
int second,minute,hour,weekDay,monthDay,month,year;
byte zero = 0x00; 

//Temp Stuff
int TMP75_Address = 0x49;
int numOfBytes = 2;

// OLED I2C bus address
#define OLED_address  0x3c 

int16_t h = 64;
int16_t w = 128;

float xx,xy,xz;
float yx,yy,yz;
float zx,zy,zz;

float fact;

int Xan,Yan;

int Xoff;
int Yoff;
int Zoff;

struct Point3d
{
  int x;
  int y;
  int z;
};

struct Point2d
{
  int x;
  int y;
};



int LinestoRender; // lines to render.
int OldLinestoRender; // lines to render just in case it changes. this makes sure the old lines all get erased.



struct Line3d
{
  Point3d p0;
  Point3d p1;
};



struct Line2d
{
  Point2d p0;
  Point2d p1;
};


Line3d Lines[20];
Line2d Render[20];
Line2d ORender[20];






/***********************************************************************************************************************************/
// Sets the global vars for the 3d transform. Any points sent through "process" will be transformed using these figures.
// only needs to be called if Xan or Yan are changed.
void SetVars(void)
{
  float Xan2,Yan2,Zan2;
  float s1,s2,s3,c1,c2,c3;
  
  Xan2 = Xan / fact; // convert degrees to radians.
  Yan2 = Yan / fact;
  
  // Zan is assumed to be zero
    
  s1 = sin(Yan2);
  s2 = sin(Xan2);
  
  c1 = cos(Yan2);
  c2 = cos(Xan2);

  xx = c1;
  xy = 0; 
  xz = -s1;

  yx = (s1 * s2);
  yy = c2;
  yz = (c1 * s2);

  zx = (s1 * c2);
  zy = -s2;
  zz = (c1 * c2);
}


/***********************************************************************************************************************************/
// processes x1,y1,z1 and returns rx1,ry1 transformed by the variables set in SetVars()
// fairly heavy on floating point here.
// uses a bunch of global vars. Could be rewritten with a struct but not worth the effort.
void ProcessLine(struct Line2d *ret,struct Line3d vec)
{
  float zvt1;
  int xv1,yv1,zv1;

  float zvt2;
  int xv2,yv2,zv2;
  
  int rx1,ry1;
  int rx2,ry2;
 
  int x1;
  int y1;
  int z1;

  int x2;
  int y2;
  int z2;
  
  int Ok;
  
  x1=vec.p0.x;
  y1=vec.p0.y;
  z1=vec.p0.z;

  x2=vec.p1.x;
  y2=vec.p1.y;
  z2=vec.p1.z;

  Ok=0; // defaults to not OK

  xv1 = (x1 * xx) + (y1 * xy) + (z1 * xz);
  yv1 = (x1 * yx) + (y1 * yy) + (z1 * yz);
  zv1 = (x1 * zx) + (y1 * zy) + (z1 * zz);

  zvt1 = zv1 - Zoff;


  if( zvt1 < -5){
    rx1 = 256 * (xv1 / zvt1) + Xoff;
    ry1 = 256 * (yv1 / zvt1) + Yoff;
    Ok=1; // ok we are alright for point 1.
  }
  
  
  xv2 = (x2 * xx) + (y2 * xy) + (z2 * xz);
  yv2 = (x2 * yx) + (y2 * yy) + (z2 * yz);
  zv2 = (x2 * zx) + (y2 * zy) + (z2 * zz);

  zvt2 = zv2 - Zoff;


  if( zvt2 < -5){
    rx2 = 256 * (xv2 / zvt2) + Xoff;
    ry2 = 256 * (yv2 / zvt2) + Yoff;
  } else
  {
    Ok=0;
  }
  
  
  
  
  if(Ok==1){
    
  ret->p0.x=rx1;
  ret->p0.y=ry1;

  ret->p1.x=rx2;
  ret->p1.y=ry2;
  }
 // The ifs here are checks for out of bounds. needs a bit more code here to "safe" lines that will be way out of whack, so they dont get drawn and cause screen garbage.
 
}



/***********************************************************************************************************************************/
void setup() {
 
  Wire.begin();
  display.begin(SSD1306_SWITCHCAPVCC, 0x3c);  // initialize with the I2C addr 0x3D (for the 128x64)
  display.clearDisplay();   // clears the screen and buffer 
  Wire.begin();
  init_OLED();
  reset_display();           // Clear logo and load saved mode 

  fact = 180 / 3.14159259; // conversion from degrees to radians.
  
  Xoff = 90; // 100 positions the center of the 3d conversion space into the center of the OLED screen. This is usally screen_x_size / 2.
  Yoff = 35; // 30 screen_y_size /2
  Zoff = 650;   //700


// line segments to draw a cube. basically p0 to p1. p1 to p2. p2 to p3 so on.

// Front Face.

  Lines[0].p0.x=-50;
  Lines[0].p0.y=-50;
  Lines[0].p0.z=50;
  Lines[0].p1.x=50;
  Lines[0].p1.y=-50;
  Lines[0].p1.z=50;



  Lines[1].p0.x=50;
  Lines[1].p0.y=-50;
  Lines[1].p0.z=50;
  Lines[1].p1.x=50;
  Lines[1].p1.y=50;
  Lines[1].p1.z=50;




  Lines[2].p0.x=50;
  Lines[2].p0.y=50;
  Lines[2].p0.z=50;
  Lines[2].p1.x=-50;
  Lines[2].p1.y=50;
  Lines[2].p1.z=50;



  Lines[3].p0.x=-50;
  Lines[3].p0.y=50;
  Lines[3].p0.z=50;
  Lines[3].p1.x=-50;
  Lines[3].p1.y=-50;
  Lines[3].p1.z=50;






//back face.

  Lines[4].p0.x=-50;
  Lines[4].p0.y=-50;
  Lines[4].p0.z=-50;
  Lines[4].p1.x=50;
  Lines[4].p1.y=-50;
  Lines[4].p1.z=-50;



  Lines[5].p0.x=50;
  Lines[5].p0.y=-50;
  Lines[5].p0.z=-50;
  Lines[5].p1.x=50;
  Lines[5].p1.y=50;
  Lines[5].p1.z=-50;




  Lines[6].p0.x=50;
  Lines[6].p0.y=50;
  Lines[6].p0.z=-50;
  Lines[6].p1.x=-50;
  Lines[6].p1.y=50;
  Lines[6].p1.z=-50;



  Lines[7].p0.x=-50;
  Lines[7].p0.y=50;
  Lines[7].p0.z=-50;
  Lines[7].p1.x=-50;
  Lines[7].p1.y=-50;
  Lines[7].p1.z=-50;




// now the 4 edge lines.

  Lines[8].p0.x=-50;
  Lines[8].p0.y=-50;
  Lines[8].p0.z=50;
  Lines[8].p1.x=-50;
  Lines[8].p1.y=-50;
  Lines[8].p1.z=-50;


  Lines[9].p0.x=50;
  Lines[9].p0.y=-50;
  Lines[9].p0.z=50;
  Lines[9].p1.x=50;
  Lines[9].p1.y=-50;
  Lines[9].p1.z=-50;




  Lines[10].p0.x=-50;
  Lines[10].p0.y=50;
  Lines[10].p0.z=50;
  Lines[10].p1.x=-50;
  Lines[10].p1.y=50;
  Lines[10].p1.z=-50;


  Lines[11].p0.x=50;
  Lines[11].p0.y=50;
  Lines[11].p0.z=50;
  Lines[11].p1.x=50;
  Lines[11].p1.y=50;
  Lines[11].p1.z=-50;



  LinestoRender=12;
  OldLinestoRender=LinestoRender;


  mpl115a2.begin();

   // Initialize TMP75
  Wire.beginTransmission(TMP75_Address);       // Address the TMP75 sensor
  Wire.write(0x01);                       // Address the Configuration register 
  Wire.write(B01100000);                         // Set the temperature resolution 
  Wire.endTransmission();                      // Stop transmitting
  Wire.beginTransmission(TMP75_Address);       // Address the TMP75 sensor
  Wire.write(0x00);                          // Address the Temperature register 
  Wire.endTransmission();   
  
}
/***********************************************************************************************************************************/
void RenderImage( void)
{
  // renders all the lines after erasing the old ones.
  // in here is the only code actually interfacing with the OLED. so if you use a different lib, this is where to change it.

 for (int i=0; i<OldLinestoRender; i++ )
  {
   display.drawLine(ORender[i].p0.x,ORender[i].p0.y,ORender[i].p1.x,ORender[i].p1.y, BLACK); // erase the old lines.
  }

    
  for (int i=0; i<LinestoRender; i++ )
  {
   display.drawLine(Render[i].p0.x,Render[i].p0.y,Render[i].p1.x,Render[i].p1.y, WHITE);
  }
  OldLinestoRender=LinestoRender;
  
  
    printDate();
      if(hour > 12) //12 hour format
      {
        hour=hour-12; 
        display.setTextSize(1);
        display.setTextColor(WHITE);
        display.setCursor(1,0);  //10,0
        display.print( ( int ) hour, 1 );
        display.setCursor(50,0); //105,0
        display.print("P");
      }  

      else //chops off leading zero
      {
        display.setTextSize(1);
        display.setTextColor(WHITE);
        display.setCursor(1,0);  //10,0
        display.print( ( int ) hour, 1 );
      } 


      if(minute < 10) //chops off leading zero
      {
        display.setTextSize(1);
        display.setTextColor(WHITE);
        display.setCursor(12,0);  //30,0
        display.print(":0"); 
        display.println(minute); 
      }  

      else //chops off leading zero
      {
        display.setTextSize(1);
        display.setTextColor(WHITE);
        display.setCursor(12,0);  //30,20
        display.print(":"); 
        display.println(minute);
      } 
      if(second < 10) //chops off leading zero
      {
        display.setTextSize(1);
        display.setTextColor(WHITE);
        display.setCursor(29,0);  //65,0
        display.print(":0"); 
        display.println(second); 
      }  

      else //chops off leading zero
      {
        display.setTextSize(1);
        display.setTextColor(WHITE);
        display.setCursor(29,0);  //65,0
        display.print(":"); 
        display.println(second);
      }  
  
      float tempx = readTemp();
      display.setTextSize(1);
      display.setTextColor(WHITE);
      display.setCursor(1,10);
      display.print(tempx, 1);
      display.print(" C");
      display.setCursor(10,10);
 

      display.setTextSize(1);
      display.setTextColor(WHITE);
      float pressureKPA;
      display.setTextColor(WHITE);
      display.setCursor(1,20);
      pressureKPA = mpl115a2.getPressure();
      display.print(pressureKPA,2);
      display.print(" Kpa");
  
}


/***********************************************************************************************************************************/

void loop() {
  
  display.display();
  display.clearDisplay();   // clears the screen and buffer  
 
 // PIX=GREEN; // colours of all lines drawn will be green until changed.
  
  Xan++;
  Yan++;


  Yan=Yan % 360;
  Xan=Xan % 360; // prevents overflow.
  


  SetVars(); //sets up the global vars to do the conversion.

  for(int i=0; i<LinestoRender ; i++)
  {
    ORender[i]=Render[i]; // stores the old line segment so we can delete it later.
    ProcessLine(&Render[i],Lines[i]); // converts the 3d line segments to 2d.
  }  
  
  RenderImage(); // go draw it!
 
}
/***********************************************************************************************************************************/



//==========================================================//
// Resets display depending on the actual mode.
static void reset_display(void)
{
  displayOff();
  clear_display();

  
  displayOn();
}




//==========================================================//
// Turns display on.
void displayOn(void)
{
    sendcommand(0xaf);        //display on
}

//==========================================================//
// Turns display off.
void displayOff(void)
{
  sendcommand(0xae);		//display off
}

//==========================================================//
// Clears the display by sendind 0 to all the screen map.
static void clear_display(void)
{
  unsigned char i,k;
  for(k=0;k<8;k++)
  {	
    setXY(k,0);    
    {
      for(i=0;i<128;i++)     //clear all COL
      {
        SendChar(0);         //clear all COL
        //delay(10);
      }
    }
  }
}






//==========================================================//
static void printBigTime(char *string)
{

  int Y;
  int lon = strlen(string);
  if(lon == 3) {
    Y = 0;
  } else if (lon == 2) {
    Y = 3;
  } else if (lon == 1) {
    Y = 6;
  }
  
  int X = 2;
  while(*string)
  {
    printBigNumber(*string, X, Y);
    
    Y+=3;
    X=2;
    setXY(X,Y);
    *string++;
  }
}


//==========================================================//
// Prints a display big number (96 bytes) in coordinates X Y,
// being multiples of 8. This means we have 16 COLS (0-15) 
// and 8 ROWS (0-7).
static void printBigNumber(char string, int X, int Y)
{    
  setXY(X,Y);
  int salto=0;
  for(int i=0;i<96;i++)
  {
    if(string == ' ') {
      SendChar(0);
    } else 
      //SendChar(pgm_read_byte(bigNumbers[string-0x30]+i));
   
    if(salto == 23) {
      salto = 0;
      X++;
      setXY(X,Y);
    } else {
      salto++;
    }
  }
}

//==========================================================//
// Actually this sends a byte, not a char to draw in the display. 
// Display's chars uses 8 byte font the small ones and 96 bytes
// for the big number font.
static void SendChar(unsigned char data) 
{ 
  Wire.beginTransmission(OLED_address); // begin transmitting
  Wire.write(0x40);//data mode
  Wire.write(data);
  Wire.endTransmission();    // stop transmitting
}

//==========================================================//
// Prints a display char (not just a byte) in coordinates X Y,
// being multiples of 8. This means we have 16 COLS (0-15) 
// and 8 ROWS (0-7).
static void sendCharXY(unsigned char data, int X, int Y)
{
  setXY(X, Y);
  Wire.beginTransmission(OLED_address); // begin transmitting
  Wire.write(0x40);//data mode
  
  for(int i=0;i<8;i++)
    Wire.write(pgm_read_byte(myFont[data-0x20]+i));
    
  Wire.endTransmission();    // stop transmitting
}

//==========================================================//
// Used to send commands to the display.
static void sendcommand(unsigned char com)
{
  Wire.beginTransmission(OLED_address);     //begin transmitting
  Wire.write(0x80);                          //command mode
  Wire.write(com);
  Wire.endTransmission();                    // stop transmitting
 
}

//==========================================================//
// Set the cursor position in a 16 COL * 8 ROW map.
static void setXY(unsigned char row,unsigned char col)
{
  sendcommand(0xb0+row);                //set page address
  sendcommand(0x00+(8*col&0x0f));       //set low col address
  sendcommand(0x10+((8*col>>4)&0x0f));  //set high col address
}


//==========================================================//
// Prints a string regardless the cursor position.
static void sendStr(unsigned char *string)
{
  unsigned char i=0;
  while(*string)
  {
    for(i=0;i<8;i++)
    {
      SendChar(pgm_read_byte(myFont[*string-0x20]+i));
    }
    *string++;
  }
}

//==========================================================//
// Prints a string in coordinates X Y, being multiples of 8.
// This means we have 16 COLS (0-15) and 8 ROWS (0-7).
static void sendStrXY( char *string, int X, int Y)
{
  setXY(X,Y);
  unsigned char i=0;
  while(*string)
  {
    for(i=0;i<8;i++)
    {
      SendChar(pgm_read_byte(myFont[*string-0x20]+i));
    }
    *string++;
  }
}


//==========================================================//
// Inits oled and draws logo at startup
static void init_OLED(void)
{
  sendcommand(0xae);		//display off
  sendcommand(0xa6);            //Set Normal Display (default)
    // Adafruit Init sequence for 128x64 OLED module
    sendcommand(0xAE);             //DISPLAYOFF
    sendcommand(0xD5);            //SETDISPLAYCLOCKDIV
    sendcommand(0x80);            // the suggested ratio 0x80
    sendcommand(0xA8);            //SSD1306_SETMULTIPLEX
    sendcommand(0x3F);
    sendcommand(0xD3);            //SETDISPLAYOFFSET
    sendcommand(0x0);             //no offset
    sendcommand(0x40 | 0x0);      //SETSTARTLINE
    sendcommand(0x8D);            //CHARGEPUMP
    sendcommand(0x14);
    sendcommand(0x20);             //MEMORYMODE
    sendcommand(0x00);             //0x0 act like ks0108
    
    sendcommand(0xA0 | 0x1);      //SEGREMAP   //Rotate screen 180 deg
    //sendcommand(0xA0);
    
    sendcommand(0xC8);            //COMSCANDEC  Rotate screen 180 Deg
    //sendcommand(0xC0);
    
    sendcommand(0xDA);            //0xDA
    sendcommand(0x12);           //COMSCANDEC
    sendcommand(0x81);           //SETCONTRAS
    sendcommand(0xCF);           //
    sendcommand(0xd9);          //SETPRECHARGE 
    sendcommand(0xF1); 
    sendcommand(0xDB);        //SETVCOMDETECT                
    sendcommand(0x40);
    sendcommand(0xA4);        //DISPLAYALLON_RESUME        
    sendcommand(0xA6);        //NORMALDISPLAY             

  clear_display();
  sendcommand(0x2e);            // stop scroll
  //----------------------------REVERSE comments----------------------------//
  //  sendcommand(0xa0);		//seg re-map 0->127(default)
  //  sendcommand(0xa1);		//seg re-map 127->0
  //  sendcommand(0xc8);
  //  delay(1000);
  //----------------------------REVERSE comments----------------------------//
  // sendcommand(0xa7);  //Set Inverse Display  
  // sendcommand(0xae);		//display off
  sendcommand(0x20);            //Set Memory Addressing Mode
  sendcommand(0x00);            //Set Memory Addressing Mode ab Horizontal addressing mode
  //  sendcommand(0x02);         // Set Memory Addressing Mode ab Page addressing mode(RESET)  
  
   setXY(0,0);
  /*
  for(int i=0;i<128*8;i++)     // show 128* 64 Logo
  {
    SendChar(pgm_read_byte(logo+i));
  }
  */
  sendcommand(0xaf);		//display on
  
 
}





byte bcdToDec(byte val) 
{
  // Convert binary coded decimal to normal decimal numbers
  return ( (val/16*10) + (val%16) );
}


byte decToBcd(byte val)
{
  // Convert normal decimal numbers to binary coded decimal
  return ( (val/10*16) + (val%10) );
}



int printDate()
{  
  // Reset the register pointer
  Wire.beginTransmission(DS1307_ADDRESS);
  Wire.write(zero);
  Wire.endTransmission();

  Wire.requestFrom(DS1307_ADDRESS, 7);

  //int second = bcdToDec(Wire.read());
  second = bcdToDec(Wire.read());
  minute = bcdToDec(Wire.read());
  //hour = bcdToDec(Wire.read() & 0b111111); //24 hour time
  hour = bcdToDec(Wire.read()&0x3f); //12 hour time?
  return second;
  return hour;
}


//==========================================================//
// Read temperature
float readTemp()
{
  // Now take a Temerature Reading
  Wire.requestFrom(TMP75_Address,2);  // Address the TMP75 and set number of bytes to receive
  byte MostSigByte = Wire.read();              // Read the first byte this is the MSB
  byte LeastSigByte = Wire.read();             // Now Read the second byte this is the LSB

  // Being a 12 bit integer use 2's compliment for negative temperature values
  int TempSum = (((MostSigByte << 8) | LeastSigByte) >> 4); 
  // From Datasheet the TMP75 has a quantisation value of 0.0625 degreesC per bit
  float temp = (TempSum*0.0625);
  return temp;                           // Return the temperature value
}



