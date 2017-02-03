#include <Wire.h>                     //Needed for I2C 
#include <Adafruit_Sensor.h>
#include <SPI.h>                      //Needed for SD card adapter
#include <SD.h>                       //Needed for SD card adapter
#include "LowPower.h"                 //Provides sleep/idle/powerdown functions

#include <SoftwareSerial.h>
#include "Adafruit_FONA.h"
#include "Adafruit_IO_FONA.h"

/* I2C addresses:
  0x29  TSL2561 Lux sensor
  0x57  RTC
  0x68  RTC
  0x77  BMP180 Baro sensor
*/

const int chipSelect = 10;            //CS pin for SD card adapter
const int SenorPowerPin = 11;         //Pin used to provide power to sensors
const int KeyPin = 12;                //Key pin to turn off cell module (Pulse low for a few seconds to change from on to off)
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

//For Lux sensor
#include <Adafruit_TSL2561_U.h>
Adafruit_TSL2561_Unified tsl = Adafruit_TSL2561_Unified(TSL2561_ADDR_LOW, 2561);

void setup(void) 
{
  //Power on sensors early in loop
  pinMode(SenorPowerPin,OUTPUT);
  digitalWrite(SenorPowerPin,HIGH);
  
  Serial.begin(115200);
  delay(2000);

  //For cell module 
  pinMode(KeyPin,OUTPUT);
  digitalWrite(KeyPin,HIGH);
  //Turn off cell module
  Serial.println("Turning off cell module");
  digitalWrite(KeyPin,LOW);       
  delay(3000);
  digitalWrite(KeyPin,HIGH);;
  Serial.println("Cell module turned off");
  

  //For BMP180
  Serial.println("Pressure Sensor Test"); Serial.println("");
  
  /* Initialise the sensor */
  if(!bmp.begin())
  {
    /* There was a problem detecting the BMP085 ... check your connections */
    Serial.println("No BMP085 detected ... Check your wiring or I2C ADDR!");
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
    Serial.print("No TSL2561 detected ... Check your wiring or I2C ADDR!");
    while(1);
  }
  
  /* Display some basic information on this sensor */
  displayTSL2561SensorDetails();
  
  /* Setup the sensor gain and integration time */
  configureSensor();
  
  //For SD card adapter
  Serial.print("Initializing SD card..."); 
  pinMode(SS, OUTPUT);
   
  if (!SD.begin(chipSelect)) {
    Serial.println("SD card initialization failed!");
    return;
  }
  else
  {
    Serial.println("SD card initialization done.");
  }

  DATALOG = SD.open("log.txt", FILE_WRITE);
  // if the file opened okay, write to it:
  if (DATALOG)
  {
    DATALOG.println("Date Time,Lux,Temp.C,Pressure.hPa");
    DATALOG.close();
  }
  else 
  {
    // if the file didn't open, print an error:
    Serial.println("error opening log file");
  }

  /* We're ready to go! */
  Serial.println("Beginning data collection");
}

void loop(void) 
{  
  DateTime now = rtc.now();
 
  if ( /*!Serial &&*/ now.second() <= 40 )  //Don't miss logging window at top of each minute
  {
    digitalWrite(SenorPowerPin,LOW);
    LowPower.idle(SLEEP_8S, ADC_OFF, TIMER4_OFF, TIMER3_OFF, TIMER1_OFF, 
        TIMER0_OFF, SPI_OFF, USART1_OFF, TWI_OFF, USB_OFF);
    digitalWrite(SenorPowerPin,HIGH);
    delay(500);             //Give TSL2561 sensor time to settle
    tsl.begin();
    configureSensor();
    delay(500);             //Give TSL2561 sensor time to settle
    
  }
  if ( now.second() == 0 ) //Update log file every minute
  { 
    DATALOG = SD.open("log.txt", FILE_WRITE);
    // if the file opened okay, write to it:
    if (DATALOG) 
    {
      Serial.print(now.hour(), DEC);
      Serial.print(":");
      Serial.print(now.minute(), DEC);
      Serial.print(":");
      Serial.println(now.second(), DEC);
      SDTimeStamp(); 
      ReadTSL2561();
      ReadBMP180();
      DATALOG.println();
      DATALOG.close();     
    } 
    else 
    {
      // if the file didn't open, print an error:
      Serial.println("error opening log file");
    }
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
  DATALOG.print(" ");
  DATALOG.print(now.hour(), DEC);
  DATALOG.print(":");
  DATALOG.print(now.minute(), DEC);
  DATALOG.print(",");
}
