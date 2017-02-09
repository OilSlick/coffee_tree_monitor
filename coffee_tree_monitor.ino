#include <Wire.h>                     //Needed for I2C 
#include <Adafruit_Sensor.h>          //Needed for both BMP180 and TSL2561
#include <SPI.h>                      //Needed for SD card adapter
#include <SD.h>                       //Needed for SD card adapter
#include "config.h"                   //Needed for Adafruit IO

/* I2C addresses:
  0x29  TSL2561 Lux sensor
  0x68  RTC
  0x77  BMP180 Baro sensor
*/

//ESP8266 mac address: 5C:CF:7F:C6:7B:BF

String Statuses[] =  { "WL_IDLE_STATUS=0", "WL_NO_SSID_AVAIL=1", "WL_SCAN_COMPLETED=2", "WL_CONNECTED=3", "WL_CONNECT_FAILED=4", "WL_CONNECTION_LOST=5", "WL_DISCONNECTED=6"};
String TimeStamp = "";                //String to contain the timestamp for log files
const int chipSelect = 15;            //CS pin for SD card adapter
const int SenorPowerPin = 2;          //Pin used to provide power to sensors
bool connERROR = 0;                   //Track connection failures
bool debug = 0;                       //Enable debugging with "1"
float PressureVal = 0;                //Value returned by sensor
float TSL2561Val = 0;                 //Value returned by sensor
float TemperatureVal = 0;             //Value returned by sensor
float AltitudeVal = 0;                //Value returned by sensor
bool BMP180Error = 0;                 //Track sensor errors
bool TSL2561Error = 0;                //Track sensor errors

//For Adafruit IO
AdafruitIO_Feed *luxFeed = io.feed("Lux");
AdafruitIO_Feed *tempFeed = io.feed("Temp");
AdafruitIO_Feed *presFeed = io.feed("Pressure");

//For SD card adapter
File DATALOG;
File ERRORLOG;

//For BMP180 pressure, temp, altitude sensor
#include <Adafruit_BMP085_U.h>
Adafruit_BMP085_Unified bmp = Adafruit_BMP085_Unified(10085);

//For RTC
#include "RTClib.h" 
RTC_PCF8523 rtc;

//For Lux sensor
#include <Adafruit_TSL2561_U.h>
Adafruit_TSL2561_Unified tsl = Adafruit_TSL2561_Unified(TSL2561_ADDR_LOW, 2561);

void setup() 
{
  //Power on sensors early in loop
  pinMode(SenorPowerPin,OUTPUT);
  digitalWrite(SenorPowerPin,HIGH);
  
  Serial.begin(115200);
  if ( debug == 1)
  { 
    Serial.setDebugOutput(true);
  }
  delay(2000);
  
  //For BMP180
  Serial.println("Initilizing Pressure Sensor"); Serial.println("");
  
  /* Initialise the sensor */
  if(!bmp.begin())
  {
    /* There was a problem detecting the BMP085 ... check your connections */
    Serial.println("No BMP180 detected ... Check your wiring or I2C address");
    BMP180Error = 1;
  }
  
  /* Display some basic information on this sensor */
  displayBMP180SensorDetails();

  //For TSL2561 sensor
  
  Serial.println("Initilizing Light Sensor"); Serial.println("");
  
  /* Initialise the sensor */
  if(!tsl.begin())
  {
    /* There was a problem detecting the TSL2561 ... check your connections */
    Serial.println("No TSL2561 detected ... Check your wiring or I2C address");
    TSL2561Error = 1;
  }
  
  /* Display some basic information on this sensor */
  displayTSL2561SensorDetails();
  
  /* Setup the sensor gain and integration time */
  configureSensor();
  
  //For SD card adapter
  Serial.print("Initializing SD card..."); 
  pinMode(SS, OUTPUT);
   
  if (!SD.begin(chipSelect)) {
    Serial.println("SD card initialization failed");
    return;
  }
  else
  {
    Serial.println("SD card initialization done");
  }

  DATALOG = SD.open("log.txt", FILE_WRITE);
  // if the file opened, write header to it:
  if (DATALOG)
  {
    DATALOG.println("Date Time,Lux,Temp.C,Pressure.hPa");
    DATALOG.close();
  }
  else 
  {
    // if the file didn't open, print an error:
    Serial.println("Error opening data log file");
  }

  ERRORLOG = SD.open("error.txt", FILE_WRITE);
  // if the file opened, write header to it:
  if (ERRORLOG)
  {
    ERRORLOG.println("Date Time,Error");
    ERRORLOG.close();
  }
  else 
  {
    // if the file didn't open, print an error:
    Serial.println("Error opening error log file");
  }

  //For RTC (must occur late in the loop)
  //Uncomment the line below to set the RTC
  if ( Serial )
  {
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  //For Adafruit IO
  Connect();  //Connect to WiFi first
  
  Serial.println("Connecting to Adafruit IO");
  // connect to io.adafruit.com
  // io.connect();
  // attach message handler for the each feed
  luxFeed->onMessage(handleMessage);
  tempFeed->onMessage(handleMessage);
  presFeed->onMessage(handleMessage);
  
  // wait for a connection
  int connAttempt = 0;
  while(io.status() < AIO_CONNECTED && connAttempt <= 60) 
    {
      Serial.print(".");
      delay(500);
      connAttempt++;
    }
  if ( connAttempt > 60 )
  {
    connERROR = 1;
    Serial.println("");
    Serial.println("Aborting connection attempt to Adafruit IO");
    Serial.print("IP address:   ");
    Serial.println(WiFi.localIP());
    ERRORLOG = SD.open("error.txt", FILE_WRITE);
    if (ERRORLOG)
    {
      SDTimeStamp(ERRORLOG);
      ERRORLOG.println("Adafruit IO connection failed");
      ERRORLOG.close();
    }
  }
    connAttempt = 0;

  // we are connected
  Serial.println();
  Serial.println(io.statusText());

  /* We're ready to go! */
  Serial.println("Beginning data collection");
}

void loop() 
{
  // io.run(); keeps the client connected to
  // io.adafruit.com, and processes any incoming data.
  if ( connERROR == 0 )
    {
      io.run();
    }
  
  DateTime now = rtc.now();
   
  if ( now.second() == 0 ) //Update SD log file every minute
  { 
    if ( BMP180Error == 0 )
    {
      ReadBMP180( BMP180Error );
    }
    if ( TSL2561Error == 0 )
    {
      ReadTSL2561();
    }
    if ( Serial )
    {
      Serial.print(now.hour(), DEC);
      Serial.print(":");
      Serial.print(now.minute(), DEC);
      Serial.print(":");
      Serial.println(now.second(), DEC);
      SerialWriteBMP180();
      SerialWriteTSL2561();
    }
      
    DATALOG = SD.open("log.txt", FILE_WRITE);
    // if the file opened okay, write to it:
    if (DATALOG) 
    {
      SDTimeStamp(DATALOG); 
      SDWriteTSL2561();
      SDWriteBMP180();
      IOWriteBMP180();
      IOWriteTSL2561();
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

void SDTimeStamp(File LogFile)  //Requires the name of the logfile to be passed, i.e. SDTimeStamp(ERRORLOG);
{
  DateTime now = rtc.now();
  
  String theyear = String(now.year(), DEC);
  String themonth = String(now.month(), DEC);
  String theday = String(now.day(), DEC);
  String thehour = String(now.hour(), DEC);
  String themin = String(now.minute(), DEC);
  //Put all the time and date strings into one String
  TimeStamp = String(theyear + "/" + themonth + "/" + theday + " " + thehour + ":" + themin + ",");

  LogFile.print(TimeStamp);

}

void Connect()
{
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);
  
  if (WiFi.status() != WL_CONNECTED)
  {
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    int Attempt = 0;
    while (WiFi.status() != WL_CONNECTED) 
    {
      delay(500);
      Attempt++;
      Serial.print(".");
      if (Attempt == 60)
      {
        Serial.println();
        Serial.println("Could not connect to WiFi");
        return;
       }
    }
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

