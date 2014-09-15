//Verified to be working on Rev 2.2
/*
 Logs, views and shows SD Card info.
 Thanks to Adafruit at http://www.adafruit.com for the great display and sensor libraries
 Hardware availible for purchase at https://www.tindie.com/products/miker/arduino-sensor-board/
 
 https://github.com/adafruit/Adafruit_SSD1306
 https://github.com/adafruit/Adafruit-GFX-Library
*/



#include <SD.h>   //SD Card Library
#include <Wire.h> //I2C Library
#include <avr/pgmspace.h>
#include <Wire.h>
//---------------FONT + GRAPHIC-----------------------------//
#include "data.h"
//==========================================================//

//Temp Stuff
int TMP75_Address = 0x49;
int numOfBytes = 2;

//OLED I2C bus address
#define OLED_address  0x3c

//RTC Stuff
#define DS1307_ADDRESS 0x68 
int second,minute,hour;

byte zero = 0x00; 
#define LEDpin 13

Sd2Card card;
SdVolume volume;
SdFile root;


//Button Stuff
const int buttonPin_0 = 11;   //direction switch
const int buttonPin_1 = 10;   //direction switch
const int buttonPin_2 = 9;    //direction switch
const int buttonPin_3= 8;     //direction switch
const int buttonPin_4= 7;     //direction switch
const int buttonPin_5= 5;     //pushbutton switch
//buttonState_0=In,buttonState_1=Left,buttonState_2=Down,buttonState_3=Up,buttonState_4=Right,buttonState_5=Switch

const int ledPin =  13;      // the number of the LED pin
int buttonState_0 = 0;
int buttonState_1 = 0;
int buttonState_2 = 0;
int buttonState_3 = 0;
int buttonState_4 = 0;
int buttonState_5 = 0;


unsigned long pre = 0;
int refresh_rate = 2000;  //Dataloger Refresh Rate
int temp_address = 72;   //Address of the I2C Temp Sensor
int id = 1;              //Use this to store the id # of our reading.



void setup()
{
  Wire.begin();
  //Serial.begin(9600);
  
  //while (!Serial) 
  //{
  // ; // wait for serial port to connect. Needed for Leonardo only
  //}

  //Serial.print("\nInitializing SD card...");

  // Initialize I2C and OLED Display
  init_OLED();
  reset_display();           // Clear logo and load saved mode 

  //Initialize the LED pin as an output:
  pinMode(LEDpin, OUTPUT); 

  //Initialize Card
  if (!SD.begin())
  {
    sendStrXY("Card Failure", 0, 0);
    return;
  }
}

int highlight_option=0; //from 0 to 3 for a 4 options menu

void loop()
{
  //Read buttons
  buttonState_0 = digitalRead(buttonPin_0); // IN
  buttonState_1 = digitalRead(buttonPin_1); // Left
  buttonState_2 = digitalRead(buttonPin_2); // Down
  buttonState_3 = digitalRead(buttonPin_3); // Up
  buttonState_4 = digitalRead(buttonPin_4);  // Right
  buttonState_5 = digitalRead(buttonPin_5);  // Switch

  //check witch option should be highlighted
  if(buttonState_2)             //if pushed down
  {
    highlight_option++;      //add 1 to the current value
    clear_display();
  }
  else if (buttonState_3)   //but if pushed up
  {
    highlight_option--;        // subtract 1
    clear_display(); 
  }
  //check current option overflow. 3+1=4 would be invalid, same as 1-1=0
  if (highlight_option < 0) highlight_option = 3; //this has the effect of the cursor dissapearing from up to the last option.
  else if (highlight_option > 3) highlight_option = 0;

  for (int i=0;i<4;i++) 
  { // loop where i goes from 0 to 3 each iteration
    if(highlight_option == i) 
    { //if current option is highlight option

    } 
    else 
    {

    }
    sendStrXY("SD Card Logger",1,1);
    sendStrXY("Start Logging",3,1);
    sendStrXY("View Log",4,1);   //Test new Cardinfo Code
    sendStrXY("Delete Log",5,1);
    sendStrXY("Card Info",6,1);
    sendStrXY("*",highlight_option+3,0);
  }

  // Check IN button
  if (buttonState_0);
  execute_menu(highlight_option); //execute current highlight option 
}


//should define some execute_menu function to execute any option based on input number range 0 to 3, being 0 first menu option and 3 last
void execute_menu (int option)
{
  if (highlight_option==0 && buttonState_0==1)       
  {
    clear_display();
    bool dont_exit=true;
    while(dont_exit)
    {   
      sendStrXY("Start Logging",0,1);
      Log_To_SD_Card();

      int value = digitalRead(buttonPin_5); 
      if(value==true)
      {
        clear_display(); 
        dont_exit=false; 
      }
    }
  }

  
  if (highlight_option==1 && buttonState_0==1)
  {
    clear_display();
    bool dont_exit=true;
    while(dont_exit)
    {   
      sendStrXY("View Log",0,1);
      View_Log();

      int value = digitalRead(buttonPin_5); 
      if(value==true)
      {
        clear_display(); 
        dont_exit=false; 
      }
    }
  }


  if (highlight_option==2 && buttonState_0==1)
  {
    clear_display();
    bool dont_exit=true;
    while(dont_exit)
    {   
      sendStrXY("Delete Log",0,1);
      Delete_Log_On_SD_Card();

      int value = digitalRead(buttonPin_5); 
      if(value==true)
      {
        clear_display(); 
        dont_exit=false; 
      }
    }
  }
  
  
  
  if (highlight_option==3 && buttonState_0==1)
  {
    clear_display();
    bool dont_exit=true;
    while(dont_exit)
    {   
      sendStrXY("Card Info",0,1);
      Card_Info();

      int value = digitalRead(buttonPin_5); 
      if(value==true)
      {
        clear_display(); 
        dont_exit=false; 
      }
    }
  } 
}


void Log_To_SD_Card()
{  
  //Create Data string for storing to SD card
  //We will use CSV Format
  int T, TD; 
  String data = ""; 
  printDate();
  float tempx = (readTemp());
  T = tempx;
  TD = (tempx - T)*10;
  data += String(hour);
    data += ":";
    data += String(minute);
    data += " ";
    data += String(T);
    data += ".";
    data += String(TD);
  unsigned long cur = millis();
  if(cur - pre > refresh_rate){
    pre = cur;
    digitalWrite(LEDpin, HIGH);
    delay(100);
    digitalWrite(LEDpin, LOW);
    delay(100);
    //Open a file to write to
    File logFile = SD.open("LOG.csv", FILE_WRITE);
    if (logFile)
    {
      logFile.print(data);
      logFile.close();
    }
    else
    {
      sendStrXY("Couldn't open log file", 0, 0);
    }
  }
 

  char tmp[5];
  sprintf(tmp,"Time");
  sendStrXY(tmp,3,1); //down, over
  
  sprintf(tmp,"%d",hour);
  sendStrXY(tmp,3,6);
  sendStrXY(":",3,8);
  if(minute < 10){
  sendStrXY("0",3,9);
  sprintf(tmp,"%d",minute);
  sendStrXY(tmp,3,10);
  }
  else{
  sprintf(tmp,"%d",minute);
  sendStrXY(tmp,3,9);}
  sendStrXY(":",3,11);
  if(second <10){
    sprintf(tmp,"0");
    sendStrXY(tmp,3,12);
  
    sprintf(tmp,"%d",second);
    sendStrXY(tmp,3,13);
    }
  else{
  sprintf(tmp,"%d",second);
  sendStrXY(tmp,3,12);}
  
  sprintf(tmp,"Temp");
  sendStrXY(tmp,5,1); //down, over

  sprintf(tmp,"%d", T);
  sendStrXY(tmp,5,6);
  sendStrXY(".",5,8);
  sprintf(tmp,"%d",TD);
  sendStrXY(tmp,5,9);
}


void Delete_Log_On_SD_Card()
{
  SD.remove("LOG.csv");
  sendStrXY("Card Deleted",3,0);
  delay(500);
} 


void View_Log()
{
  int x,y;
  x=1,y=0;
  int dat;

  File logFile = SD.open("LOG.csv");
  if (logFile)
  { 
    while (logFile.available()) 
    {
      dat = logFile.read();
      sendCharXY(dat,x,y); //down,over
      y=y+1;
      if (y>9) 
      {
        delay(300);
        x++;
        y=0; 
      }
      if(x>7)
      {
       x=0;
      }  
    }
  } 

  logFile.close();

} 


void Card_Info()
{
  char test[5];
  char print_volumesize[5];
 
  //if (!card.init(SPI_HALF_SPEED, chipSelect)) //TESTING
    if (!card.init(SPI_HALF_SPEED)) //TESTING
  
  {
    return;
  } 
 

  char card_type[5];
   
  sendStrXY("Card type:",2,0);
  switch(card.type()) {
    case SD_CARD_TYPE_SD1:
      //Serial.println("SD1");
      sendStrXY("SD1",2,11);
      break;
    case SD_CARD_TYPE_SD2:
      //Serial.println("SD2");
      sendStrXY("SD2",2,11);
      break;
    case SD_CARD_TYPE_SDHC:
      //Serial.println("SDHC");
      sendStrXY("SDHC",2,11);
      break;
    default:
      //Serial.println("Unknown");
      sendStrXY("Unknown",2,11);
    delay (100); 
  }

  // Now we will try to open the 'volume'/'partition' - it should be FAT16 or FAT32
  if (!volume.init(card)) 
  {
   //Serial.println("Could not find FAT16/FAT32 partition.\nMake sure you've formatted the card");
   return;
  }


  // print the type and size of the first FAT-type volume
  uint32_t volumesize;
  //Serial.print("\nVolume type is FAT");
  sendStrXY("Vol type FAT:",4,0);
  //Serial.println(volume.fatType(), DEC);
  //String volume_fat = volume.fatType();
    //char volume_fat = (volume.fatType(), DEC); //good
  int volume_fat = volume.fatType();
  
  sprintf(test,"%d",volume_fat);
  sendStrXY(test,4,13);
  //Serial.print("\nVol type FAT:");
  //Serial.println(volume.fatType(), DEC);
  //Serial.println(); 
  
  //delay (100);

  volumesize = volume.blocksPerCluster();    // clusters are collections of blocks
  volumesize *= volume.clusterCount();       // we'll have a lot of clusters
  volumesize *= 512;                            // SD card blocks are always 512 bytes
  
  //Serial.print("Volume size (bytes): ");
  //Serial.println(volumesize);
  //Serial.print("Volume size (Kbytes): ");
  volumesize /= 1024;
  //Serial.println(volumesize);
  //Serial.print("Volume size (Mbytes): ");
  
  volumesize /= 1024;
  //Serial.println(volumesize);
  //sprintf(test2,"%d",volumesize);
  //sendStrXY(test2,6,0);
  //sendStrXY(volumesize,6,0);
  sendStrXY("Size Mbyte:",6,0);              //My Test SD Card is only 120Mbytes in size
  sprintf(print_volumesize,"%d",volumesize);
  sendStrXY(print_volumesize,6,12);
} 


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
      for(i=0;i<128;i++)     
      {
        SendChar(0);         
      }
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
  //int weekDay = bcdToDec(Wire.read()); //0-6 -> sunday - Saturday
  //int monthDay = bcdToDec(Wire.read());
  //int month = bcdToDec(Wire.read());
  //int year = bcdToDec(Wire.read());
  //print the date EG   3/1/11 23:59:59
  //Serial.print(month);
  //Serial.print("/");
  //Serial.print(monthDay);
  //Serial.print("/");
  //Serial.print(year);
  //Serial.print(" ");
  //Serial.print(hour);
  //Serial.print(":");
  //Serial.print(minute);
  //Serial.print(":");
  //Serial.println(second);
  return second;
  return hour;
}


