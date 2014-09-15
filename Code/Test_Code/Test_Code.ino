
/*
 Test for all buttons, sensors and display of the Sensor Board.
 Thanks to Adafruit at http://www.adafruit.com for the great display and sensor libraries
 Purchase the hardware at https://www.tindie.com/products/miker/arduino-sensor-board/
 
 https://github.com/adafruit/Adafruit_SSD1306
 https://github.com/adafruit/Adafruit-GFX-Library
 https://github.com/adafruit/Adafruit_MPL115A2
 https://github.com/adafruit/Adafruit_ADXL345
 https://github.com/adafruit/Adafruit_Sensor
*/



// Changes:
// changed the ADXL345 sensitivity range from 2G to 4G 


// Define includes
#include <SPI.h>  //needed
#include <Adafruit_GFX.h>  //NEW
#include <avr/pgmspace.h>
#include <Wire.h>        // Wire header file for I2C and 2 wire
#include <ADXL345.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_MPL115A2.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_ADXL345_U.h>
#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);


//Accelerometer Stuff
//#include <Adafruit_Sensor.h>
//#include <Adafruit_ADXL345.h>
/* Assign a unique ID to this sensor at the same time */
//Adafruit_ADXL345 accel = Adafruit_ADXL345(12345);
Adafruit_ADXL345_Unified accel = Adafruit_ADXL345_Unified(12345);
Adafruit_MPL115A2 mpl115a2;

//Temp Stuff
int TMP75_Address = 0x49;
int numOfBytes = 2;

//RTC Stuff
#define DS1307_ADDRESS 0x68 
int second,minute,hour,weekDay,monthDay,month,year;
byte zero = 0x00; 

#define XPOS 0
#define YPOS 1
#define DELTAY 2
#define LOGO16_GLCD_HEIGHT 16 
#define LOGO16_GLCD_WIDTH  16

//static uint8_t menu_item_location = 1; //??

//Button Stuff
const int buttonPin_0 = 11;   //direction switch
const int buttonPin_1 = 10;   //direction switch
const int buttonPin_2 = 9;    //direction switch
const int buttonPin_3= 8;     //direction switch
const int buttonPin_4= 7;     //direction switch
const int buttonPin_5= 5;     //pushbutton switch
int Set = 0;
float angle;
int x;
int y;
//buttonState_0=In,buttonState_1=Left,buttonState_2=Down,buttonState_3=Up,buttonState_4=Right,buttonState_5=Switch

const int ledPin =  13;      // the number of the LED pin
int buttonState_0 = 0;
int buttonState_1 = 0;
int buttonState_2 = 0;
int buttonState_3 = 0;
int buttonState_4 = 0;
int buttonState_5 = 0;

//Position Etch-a-sketch start point
int a=64;
int b=32;


// Setup the Arduino LCD (Columns, Rows)
void setup()
{ 
  Wire.begin();                      // Join the I2C bus as a master
  Serial.begin(9600);            // Set Serial Port speed

  display.begin(SSD1306_SWITCHCAPVCC, 0x3c);  // initialize with the I2C addr 0x3D (for the 128x64)

  display.display(); // show splashscreen
  delay(1000);
  display.clearDisplay();   // clears the screen and buffer  
  // initialize the LED pin as an output:
  pinMode(ledPin, OUTPUT);      
  // initialize the pushbutton pin as an input:
  pinMode(buttonPin_0, INPUT);     
  delay(300); 
  accel.begin();
  mpl115a2.begin();
  
  // Initialize TMP75
  Wire.beginTransmission(TMP75_Address);       // Address the TMP75 sensor
  Wire.write(0x01);                       // Address the Configuration register 
  Wire.write(B01100000);                         // Set the temperature resolution 
  Wire.endTransmission();                      // Stop transmitting
  Wire.beginTransmission(TMP75_Address);       // Address the TMP75 sensor
  Wire.write(0x00);                          // Address the Temperature register 
  Wire.endTransmission();                      // Stop transmitting 
  
}



char *options[] = {
  "- Draw Sketch","- Time Screen","- Temp Screen","- Accelerometer","- Barometer+Temp"};
int highlight_option=0; //from 0 to 4 for a 5 options menu
// Main Program Infinite loop 
void loop()
{ 
  //Read Acceler0meter
  //sensors_event_t event; 
  //accel.getEvent(&event);
  //setDateTime();
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
    display.clearDisplay();
    highlight_option++;      //add 1 to the current value

  }
  else if (buttonState_3)   //but if pushed up
  {
    display.clearDisplay();
    highlight_option--;        // subtract 1

  }
  //check current option overflow. 3+1=4 would be invalid, same as 1-1=0
  if (highlight_option < 0) highlight_option = 4; //this has the effect of the cursor dissapearing from up to the last option.
  else if (highlight_option > 4) highlight_option = 1;

  //print menu header
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(30,0);
  display.println("Menu");

  //print menu options
  display.setTextSize(1); //this is same for all options

  for (int i=0;i<5;i++) 
  { // loop where i goes from 0 to 3 each iteration
    if(highlight_option == i) 
    { //if current option is highlight option
      display.setTextColor(BLACK, WHITE); // 'inverted' text 
    } 
    else 
    {
      display.setTextColor(WHITE); // normal text
      display.display(); 
    }
    display.setCursor(0,10+i*10); //calculate position for each menu entry
    display.println(options[i]); //print current option from the array
  }

  // Check IN button
  if (buttonState_0)
    execute_menu(highlight_option); //execute current highlight option
}




//should define some execute_menu function to execute any option based on input number range 0 to 3, being 0 first menu option and 3 last
void execute_menu (int option)
{
  if (highlight_option==0)
  {
    display.clearDisplay();
    while(1)
    {
      // read the state of the pushbutton value: 
      buttonState_0 = digitalRead(buttonPin_0);
      buttonState_1 = digitalRead(buttonPin_1);
      buttonState_2 = digitalRead(buttonPin_2);
      buttonState_3 = digitalRead(buttonPin_3);
      buttonState_4 = digitalRead(buttonPin_4); 
      buttonState_5 = digitalRead(buttonPin_5);

      //text display tests
      display.setTextSize(1);
      display.setTextColor(WHITE);
      display.setCursor(20,0);
      display.println("Etch-a sketch");
      display.display();

      if (buttonState_4 == HIGH)  //Right button
      {      
        a=a+1;
        display.drawPixel(a, b, WHITE);
        display.display();
        delay(50); 
      }


      if (buttonState_1 == HIGH)  //Left button
      {      
        a=a-1;
        display.drawPixel(a, b, WHITE);
        display.display();
        delay(50); 
      }

      if (buttonState_2 == HIGH)  //Down button
      {      
        b=b+1;
        display.drawPixel(a, b, WHITE); 
        display.display();
        delay(50);
      }

      if (buttonState_3 == HIGH)  //Up button
      {      
        b=b-1;
        display.drawPixel(a, b, WHITE); 
        display.display();
        delay(50);
      }

      if (buttonState_0 == HIGH)  //In button
      {  
        //Position Etch-a-sketch start point 
        a=64;
        b=32; 
        display.display();  
        delay(50);  
        display.clearDisplay();
        delay(50);
        display.display();

        display.setTextSize(1);
        display.setTextColor(WHITE);
        display.setCursor(20,0);
        display.println("Display Cleared"); 
        display.display();
        delay(1000);

        display.display();  
        delay(50);  
        display.clearDisplay();
        delay(50);
        display.display();

        display.setTextSize(1);
        display.setTextColor(WHITE);
        display.setCursor(20,0);
        display.println("Etch-a sketch"); 
        display.display();
      }

      buttonState_5 = digitalRead(buttonPin_5); 
      if(buttonState_5 == HIGH)
      {
        display.clearDisplay();
        break;
      }

    }
  } 




  if (highlight_option==1)
  {
    while(1)
    {
      printDate();
      if(hour > 12) //12 hour format
      {
        hour=hour-12; 
        display.setTextSize(2);
        display.setTextColor(WHITE);
        display.setCursor(10,0);  //20,20
        //display.print(":0"); display.println(hour); 
        display.print( ( int ) hour, 1 );
        display.setCursor(105,0);
        display.print("P");
      }  

      else //chops off leading zero
      {
        display.setTextSize(2);
        display.setTextColor(WHITE);
        display.setCursor(10,0);  //10,20
        //display.print(":"); display.println(hour);
        display.print( ( int ) hour, 1 );
      } 


      //display.setTextSize(1);
      //display.setTextColor(WHITE);
      //display.setCursor(50,55);
      //display.print( ( int ) minute, 1 );
      if(minute < 10) //chops off leading zero
      {
        display.setTextSize(2);
        display.setTextColor(WHITE);
        display.setCursor(30,0);  //50,20
        display.print(":0"); 
        display.println(minute); 
      }  

      else //chops off leading zero
      {
        display.setTextSize(2);
        display.setTextColor(WHITE);
        display.setCursor(30,0);  //40,20
        display.print(":"); 
        display.println(minute);
      } 
      if(second < 10) //chops off leading zero
      {
        display.setTextSize(2);
        display.setTextColor(WHITE);
        display.setCursor(65,0);  //80,20
        display.print(":0"); 
        display.println(second); 
      }  

      else //chops off leading zero
      {
        display.setTextSize(2);
        display.setTextColor(WHITE);
        display.setCursor(65,0);  //80,20
        display.print(":"); 
        display.println(second);
      }  
      /*display.setCursor(0,20);
      if(weekDay == 1){
        display.print("Sunday");
      }
      if(weekDay == 2){
        display.print("Monday");
      }
      if(weekDay == 3){
        display.print("Tuesday");
      }
      if(weekDay == 4){
        display.print("Wednesday");
      }
      if(weekDay == 5){
        display.print("Thursday");
      }
      if(weekDay == 6){
        display.print("Friday");
      }
      if(weekDay == 7){
        display.print("Saturday");
      }
      display.setCursor(0,40);
      display.print(monthDay);
      display.print("/");
      display.print(month);
      display.print("/");
      display.print(year+2000); */
      display.drawCircle(64,40,23,WHITE);
      angle = second*6;
      angle = (angle/57.29577951);
      x = (64+(sin(angle)*(22)));
      y = (40-(cos(angle)*(22)));
      display.drawLine(64,40,x,y,WHITE);
      angle = minute*6;
      angle = (angle/57.29577951);
      x = (64+(sin(angle)*(20)));
      y = (40-(cos(angle)*(20)));
      display.drawLine(64,40,x,y,WHITE);
      angle = hour*30+int((minute/12)*6);
      angle = (angle/57.29577951);
      x = (64+(sin(angle)*(17)));
      y = (40-(cos(angle)*(17)));
      display.drawLine(64,40,x,y,WHITE);


      //display.setTextSize(2);
      //display.setTextColor(WHITE);
      //display.clearDisplay();
      display.display();
      delay(20);
      display.clearDisplay();  //this order only
      buttonState_1 = digitalRead(buttonPin_1);
      if (buttonState_1 == HIGH){
        SetTime(); 
        delay(50);
      }
      buttonState_5 = digitalRead(buttonPin_5); //Exit Button
      if(buttonState_5 == HIGH)
      {
        break;   
      }   
    } 

  }

  if (highlight_option==2)
  {
    while(1)
    {
      float tempx = readTemp();
      display.setTextSize(2);
      display.setTextColor(WHITE);
      display.setCursor( 0, 20 ); //0,5
      display.print("Temp:"); 
      //Serial.print(temp,1);        //Read temp over serial at 9600 baud
      display.setCursor(65,20);
      //display.print((float)tempx, 1); //Sometimes only display 0.5 degrees difference ?????
      display.print(tempx, 1);
      //Bar graph 
      display.drawRect(0, 40, 100, 8, WHITE);   //0, 30, 50, 8, WHITE
      display.fillRect(0, 40, tempx, 8, WHITE); //0, 30, temp, 8, WHITE
      display.display();
      display.clearDisplay();

      buttonState_5 = digitalRead(buttonPin_5); //Exit Button
      if(buttonState_5 == HIGH)
      {
        display.clearDisplay(); 
        break; 
      }
    }
  }

  if (highlight_option==3)  
  {
    while(1)
    {
      //accel.setRange(ADXL345_RANGE_2_G);
      accel.setRange(ADXL345_RANGE_4_G);
      //accel.setRange(ADXL345_RANGE_8_G);
      //accel.setRange(ADXL345_RANGE_16_G);
      
      
      
      //Read Accelerometer
      sensors_event_t event; 
      accel.getEvent(&event);
      display.clearDisplay();
      display.setTextSize(2);
      display.setTextColor(WHITE);

      display.setCursor(0,5); 
      display.println("X:");
      display.setCursor(30,5); 
      display.println(event.acceleration.x);

      display.setCursor(0,25); 
      display.println("Y:");
      display.setCursor(30,25); 
      display.println(event.acceleration.y);

      display.setCursor(0,45); 
      display.println("Z:");
      display.setCursor(30,45); 
      display.println(event.acceleration.z);

      display.display();

      buttonState_5 = digitalRead(buttonPin_5); //Quit
      if(buttonState_5)
      {
        display.clearDisplay();
        break;
      }


    }
  }
  if(highlight_option==4){
    while(1)
    {
      display.clearDisplay();
      display.setTextSize(1);
      display.setTextColor(WHITE);
      float pressureKPA, temperatureC;
      //mpl115a2.getPT(&pressureKPA,&temperatureC);
      display.setCursor(0,0);
      display.println("Temp & Pressure");
      display.setTextSize(2);
      display.setTextColor(WHITE);
      display.setCursor(0,15);
      temperatureC = mpl115a2.getTemperature();
      display.print(temperatureC,1);
      display.print(" C");
      display.setCursor(0,35);
      pressureKPA = mpl115a2.getPressure();
      display.print(pressureKPA,2);
      display.print(" Kpa");
      display.display();
      delay(500);

      buttonState_5 = digitalRead(buttonPin_5); //Quit
      if(buttonState_5)
      {
        display.clearDisplay();
        break;
      }
    }
  }
}
/*
  // OLD
 // Begin the reading the TMP75 Sensor 
 float readTemp()
 {
 // Now take a Temerature Reading
 Wire.requestFrom(TMP75_Address,numOfBytes);  // Address the TMP75 and set number of bytes to receive
 char MostSigByte = Wire.read();              // Read the first byte this is the MSB  //was byte
 char LeastSigByte = Wire.read();             // Now Read the second byte this is the LSB
 // Being a 12 bit integer use 2's compliment for negative temperature values
 //int TempSum = (((MostSigByte << 8) | LeastSigByte) >> 4); 
 // From Datasheet the TMP75 has a quantisation value of 0.0625 degreesC per bit
 //float temp = (TempSum*0.0625);
 int TmpSum = word( MostSigByte, LeastSigByte) / 16 ;
 float temp = TmpSum / 16.0; // convert the value to a float 
 Serial.println(temp);
 return temp;   // Return the temperature value
 }
 */


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

void setDateTime(){

  byte second =      00; //0-59
  //byte minute; //=      Min; //0-59
  //byte hour; //=        Hou; //0-23
  //byte weekDay; =     6; //1-7
  //byte monthDay; =    21; //1-31
  //byte month; =       6; //1-12
  //byte year;  =       13; //0-99

  Wire.beginTransmission(DS1307_ADDRESS);
  Wire.write(zero); //stop Oscillator

  Wire.write(decToBcd(second));
  Wire.write(decToBcd(minute));
  Wire.write(decToBcd(hour));
  //Wire.write(decToBcd(weekDay));
  //Wire.write(decToBcd(monthDay));
  //Wire.write(decToBcd(month));
  //Wire.write(decToBcd(year));

  Wire.write(zero); //start 

  Wire.endTransmission();
  display.clearDisplay();
  display.print("Time Save!");
  display.display();
  delay(1000);
  display.clearDisplay();
}

void SetTime(){
  if(buttonState_5 == LOW){
    while(1){
      display.clearDisplay();
      display.setTextSize(2);
      display.setTextColor(WHITE);
      display.setCursor(0,0);
      display.println("Time Set");
      display.setCursor(0,20);
      buttonState_0 = digitalRead(buttonPin_0); // IN Enter
      buttonState_1 = digitalRead(buttonPin_1); // Left
      buttonState_2 = digitalRead(buttonPin_2); // Down
      buttonState_3 = digitalRead(buttonPin_3); // Up
      buttonState_4 = digitalRead(buttonPin_4); // Right
      buttonState_5 = digitalRead(buttonPin_5); // Switch "Back"
      if(buttonState_1 == HIGH){
        Set--; 
        delay(500);
      }
      if(buttonState_4 == HIGH){
        Set++; 
        delay(500);
      }
      if(Set > 1){
        Set = 0;
      }
      if(Set < 0){
        Set = 1;
      }
      if(Set == 0){
        display.print("Min-> ");
        display.print(minute);
        if(buttonState_2 == HIGH){
          minute--; 
          delay(100);
        }
        if(buttonState_3 == HIGH){
          minute++; 
          delay(100);
        }
        if(minute > 59){
          minute = 0;
        }
        if(minute < 0){
          minute = 59;
        }        
      }

      if(Set == 1){
        display.print("Hou-> ");
        display.print(hour);
        if(buttonState_2 == HIGH){
          hour--; 
          delay(100);
        }
        if(buttonState_3 == HIGH){
          hour++; 
          delay(100);
        }
        if(hour > 23){
          hour = 0;
        }
        if(hour < 0){
          hour = 23;
        }
      }
     /* if(Set == 2){
        display.print("Week Day: ");
        if(buttonState_2 == HIGH){
          weekDay--; 
          delay(100);
        }
        if(buttonState_3 == HIGH){
          weekDay++; 
          delay(100);
        }
        if(weekDay > 7){
          weekDay = 1;
        }
        if(weekDay < 1){
          weekDay = 7;
        }
        if(weekDay == 1){
          display.print("Su");
        }
        if(weekDay == 2){
          display.print("Mo");
        }
        if(weekDay == 3){
          display.print("Tu");
        }
        if(weekDay == 4){
          display.print("We");
        }
        if(weekDay == 5){
          display.print("Th");
        }
        if(weekDay == 6){
          display.print("Fr");
        }
        if(weekDay == 7){
          display.print("Sa");
        }
      }
      if(Set == 3){
        display.print("Month Day: ");
        display.print(monthDay);

        if(buttonState_2 == HIGH){
          monthDay--; 
          delay(100);
        }
        if(buttonState_3 == HIGH){
          monthDay++; 
          delay(100);
        }
        if(monthDay > 31){
          monthDay = 1;
        }
        if(monthDay < 1){
          monthDay = 31;
        }
      }
      if(Set == 4){
        display.print("Month: ");
        display.print(month);
        if(buttonState_2 == HIGH){
          month--; 
          delay(100);
        }
        if(buttonState_3 == HIGH){
          month++; 
          delay(100);
        }
        if(month > 12){
          month = 1;
        }
        if(month < 1){
          month = 12;
        }
      }
      if(Set == 5){
        display.print("Year: ");
        display.print(year);
        if(buttonState_2 == HIGH){
          year--; 
          delay(100);
        }
        if(buttonState_3 == HIGH){
          year++; 
          delay(100);
        }
        if(year > 99){
          year = 0;
        }
        if(year < 0){
          year = 99;
        }        
      } */
      display.display();
      buttonState_5 = digitalRead(buttonPin_5); //Quit
      if(buttonState_5 == HIGH)
      {      
        display.clearDisplay();
        display.print("Saving time!");
        display.display();
        delay(500);
        setDateTime();
        break;
      }
    }

  }
}












