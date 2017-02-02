#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <SPI.h>                      //Needed for SD card adapter
#include <SD.h>                       //Needed for SD card adapter

/* I2C addresses:
  0x29  TSL2561 Lux sensor
  0x57  RTC
  0x68  RTC
  0x77  BMP180 Baro sensor
*/

const int chipSelect = 10;            //CS pin for SD card adapter
const int SenorPowerPin = 11;         //Pin used to provide power to sensors
float PressureVal = 0;
float TSL2561Val = 0;                 
float TemperatureVal = 0;
float AltitudeVal = 0;

//For SD card adapter
File DATALOG;

//For BMP180 pressure, temp, altitude sensor
#include <Adafruit_BMP085_U.h>
Adafruit_BMP085_Unified bmp = Adafruit_BMP085_Unified(10085);

//For RTC
#include "RTClib.h" 
RTC_DS3231 rtc;

//For Sharp eInk display
//#include <Adafruit_GFX.h>
#include <Adafruit_SharpMem.h>

#define SCK 9
#define MOSI 6
#define SS 5

Adafruit_SharpMem display(SCK, MOSI, SS);

#define BLACK 0
#define WHITE 1

//For Lux sensor
#include <Adafruit_TSL2561_U.h>
   
Adafruit_TSL2561_Unified tsl = Adafruit_TSL2561_Unified(TSL2561_ADDR_LOW, 2561);

void setup(void) 
{
  //Power on sensors
  pinMode(SenorPowerPin,OUTPUT);
  digitalWrite(SenorPowerPin,HIGH);
  
  Serial.begin(9600);
  delay(1000);

  //For SD card adapter
  Serial.print("Initializing SD card...");
  pinMode(SS, OUTPUT);
   
  if (!SD.begin(chipSelect)) {
    Serial.println("SD card initialization failed!");
    return;
  }
  Serial.println("SD card initialization done.");

  //For BMP180
  Serial.println("Pressure Sensor Test"); Serial.println("");
  
  /* Initialise the sensor */
  if(!bmp.begin())
  {
    /* There was a problem detecting the BMP085 ... check your connections */
    Serial.print("Ooops, no BMP085 detected ... Check your wiring or I2C ADDR!");
    while(1);
  }
  
  /* Display some basic information on this sensor */
  displayBMP180SensorDetails();

  //For RTC
  if (rtc.lostPower()) {
    Serial.println("RTC lost power, lets set the time!");
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  }

  //For TSL2561 sensor
  
  Serial.println("Light Sensor Test"); Serial.println("");
  
  /* Initialise the sensor */
  if(!tsl.begin())
  {
    /* There was a problem detecting the TSL2561 ... check your connections */
    Serial.print("Ooops, no TSL2561 detected ... Check your wiring or I2C ADDR!");
    while(1);
  }
  
  /* Display some basic information on this sensor */
  displayTSL2561SensorDetails();
  
  /* Setup the sensor gain and integration time */
  configureSensor();

  //For eInk display
  // start & clear the display
  display.begin();
  display.clearDisplay();
  display.refresh();
  
  /* We're ready to go! */
  Serial.println("");
}

void loop(void) 
{  
  DateTime now = rtc.now();
 
  if ( now.minute() == 0 )  //Update SHARP display every hour
  {
    ReadTSL2561();
    ReadBMP180();
    einkDisplay();
  }
  if ( now.second() == 0 ) //Update log file every minute
  {
    DATALOG = SD.open("log.txt", FILE_WRITE);
    // if the file opened okay, write to it:
    if (DATALOG) 
    {
      SDTimeStamp(); 
      ReadTSL2561();
      DATALOG.close();     
    } 
    else 
    {
      // if the file didn't open, print an error:
      Serial.println("error opening test.txt");
    }
    
    ReadTSL2561();
  }
  delay(1000);
}

void SDTimeStamp()
{
  DateTime now = rtc.now();
  
  DATALOG.print(now.year(), DEC);
  DATALOG.print("/");
  DATALOG.print(now.month(), DEC);
  DATALOG.print("/");
  DATALOG.print(now.day(), DEC);
  DATALOG.print(",");
  DATALOG.print(now.hour(), DEC);
  DATALOG.print(":");
  DATALOG.print(now.minute(), DEC);
  DATALOG.print(":");
  DATALOG.print(now.second(), DEC);
  DATALOG.println(",");
}

void einkDisplay()
{
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(BLACK);
  display.setCursor(0,0);

  display.println("Latest reading:");
  DateTime now = rtc.now();
  
  display.print(now.year(), DEC);
  display.print('/');
  display.print(now.month(), DEC);
  display.print('/');
  display.print(now.day(), DEC);
  display.println(' ');
  display.print(now.hour(), DEC);
  display.print(':');
  display.print(now.minute(), DEC);
  display.print(':');
  display.print(now.second(), DEC);
  display.println();
  display.println();
  display.print("Lux:   ");
  display.println(TSL2561Val);
  display.print("Pres:  ");
  display.println(PressureVal);
  display.print("Temp:  ");
  display.println(TemperatureVal);
  display.refresh();
}
