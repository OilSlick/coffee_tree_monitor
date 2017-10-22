#include <Wire.h>                     //Needed for I2C 
#include <Adafruit_Sensor.h>          //Needed for both BMP180 and TSL2561
#include <SPI.h>                      //Needed for SD card adapter
#include <SD.h>                       //Needed for SD card adapter
#include "config.h"                   //Needed for Adafruit IO

//For OLED display
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#define OLED_RESET 0
Adafruit_SSD1306 display(OLED_RESET);

/* I2C addresses:
*  0x29  (41) TSL2561 Lux sensor
*  0x3C       OLED Display
*  0x68 (104) RTC
*  0x77 (119) BMP180 Baro sensor
*/

String Statuses[] =  { "WL_IDLE_STATUS=0", "WL_NO_SSID_AVAIL=1", "WL_SCAN_COMPLETED=2", "WL_CONNECTED=3", "WL_CONNECT_FAILED=4", "WL_CONNECTION_LOST=5", "WL_DISCONNECTED=6"};
String TimeStamp = "";                //String to contain the timestamp for log files

const int SenorPowerPin = 2;          //Pin used to provide power to sensors
const int chipSelect = 15;            //CS pin for SD card adapter
const int lclvPin = 16;               //Controls the LCLV glass

const int TSL2561I2CAdd = 41;         //I2C address of TSL2561 (found using I2C sketch)
const int BMP180I2CAdd = 119;         //I2C address of BMP180 (found using I2C sketch)
bool IOconnERROR = 0;                 //Track connection failures
bool debug = 0;                       //Enable debugging with "1"
bool JustPrintRuntimeOnce = 0;        //Just give us one runtime reading
bool LCLVactive = 0;                  //Track if LCLV is on or off
int LCLVValue = 0;                    //Track LCLV value
float PressureVal = 0;                //Value returned by sensor
float TSL2561Val = 0;                 //Value returned by sensor
float TemperatureVal = 0;             //Value returned by sensor
bool BMP180Error = 0;                 //Track sensor errors
bool TSL2561Error = 0;                //Track sensor errors
bool SDError = 0;                     //Track SD adapter errors
bool WiFiError = 0;                   //Track WiFi connection error
byte I2Cerror;                        //Track I2C errors (when pinging individual addresses for up/down status)
int StartLoopRuntime;                 //Track loop run time (enabled with "debug = 1;" above)
int LoopRuntime;                      //Track loop run time (enabled with "debug = 1;" above)

int ThisSecond = 0;                   //Used to display on OLED for short duration
int DisplayDuration = ThisSecond + 10;  //Used to display on OLED for short duration

//For Adafruit IO
AdafruitIO_Feed *luxFeed = io.feed("Lux");
AdafruitIO_Feed *tempFeed = io.feed("Temp");
AdafruitIO_Feed *presFeed = io.feed("Pressure");
AdafruitIO_Feed *errorFeed = io.feed("Error");

//For SD card adapter
File DATALOG;
File ERRORLOG;
File LUXLOG;

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
  wdt_enable(WDTO_4S);  //Configure watchdog timer with 4-second timeout
  
  //For OLED display
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3C (for the 128x32)
  display.clearDisplay();
  display.display(); // NOTE: You _must_ call display after making any drawing commands
  
  rtc.begin();

  //Power on sensors early in loop
  pinMode(SenorPowerPin,OUTPUT);
  digitalWrite(SenorPowerPin,HIGH);

  pinMode(lclvPin,OUTPUT);
  analogWrite(lclvPin,0);
  
  Serial.begin(115200);
  if ( debug == 1)
  { 
    Serial.setDebugOutput(true);
    if ( Serial )
    {
      Serial.print("Debug enabled");
    }
  }

  Wire.begin();

  //For SD card adapter
  Serial.print("Initializing SD card..."); 
  pinMode(SS, OUTPUT);
   
  if (!SD.begin(chipSelect)) {
    Serial.println("SD card initialization failed");
    SDError = 1;
    //return;  //Need to better handle this error
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
    // if the file didn't open, print an error to serial:
    if (Serial) 
    {
      Serial.println("Error opening data log file");
    }
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
    if (Serial)
    {
      Serial.println("Error opening error log file");
    }
  }
  
  //For BMP180
  if (Serial)
  {
    Serial.println("Initilizing Pressure Sensor"); Serial.println("");
  }
  
  /* Initialise the sensor */
  
  if(!bmp.begin())
  {
    /* There was a problem detecting the BMP085 ... check your connections */
    if (Serial)
    {
      Serial.println("No BMP180 detected ... Check your wiring or I2C address");
    }
    ERRORLOG = SD.open("error.txt", FILE_WRITE);
    if (ERRORLOG)
    {
      TimeStampSD(ERRORLOG);
      ERRORLOG.print("BMP180 is not responding");
      ERRORLOG.close();
    }
    BMP180Error = 1;
  }
  
  /* Display some basic information on this sensor */
  displayBMP180SensorDetails();

  //For TSL2561 sensor
  
  if (Serial)
  {
    Serial.println("Initilizing Light Sensor"); Serial.println("");
  }
  
  /* Initialise the sensor */

  Wire.beginTransmission(TSL2561I2CAdd);
  I2Cerror = Wire.endTransmission();
  if (Serial && debug == 1)
  {
    Serial.print("TSL2561 I2C error: ");
    Serial.println(I2Cerror);
  }
  if ( I2Cerror != 0 )  //Error "0" indicates a successful contact
  {
    if (Serial)
    {
      Serial.print("TSL2561 is not responding at address: ");
      Serial.print(TSL2561I2CAdd);
      Serial.print(" ("); Serial.print(TSL2561I2CAdd, HEX); Serial.println(")");
    }

    ERRORLOG = SD.open("error.txt", FILE_WRITE);
    if (ERRORLOG)
    {
      TimeStampSD(ERRORLOG);
      ERRORLOG.print("TSL2561 is not responding at address: ");
      ERRORLOG.print(TSL2561I2CAdd);
      ERRORLOG.print("(0x"); ERRORLOG.print(TSL2561I2CAdd, HEX); ERRORLOG.println(")");
      ERRORLOG.close();
      TSL2561Error = 1;
    }
  }
  
  if(!tsl.begin())
  {
    /* There was a problem detecting the TSL2561 ... check your connections */
    if (Serial)
    {
      Serial.println("No TSL2561 detected ... Check your wiring or I2C address");
    }
    TSL2561Error = 1;
  }
  
  /* Display some basic information on this sensor */
  displayTSL2561SensorDetails();
  
  /* Setup the sensor gain and integration time */
  configureSensor();
  
  //For RTC (must occur late in the loop)
  //Uncomment the line below to set the RTC
  if ( Serial )
  {
   //rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  //Connect to WiFi
  wdt_reset();
  Connect();  

  //Connect to Adafruit IO
  if ( WiFiError == 0)
  {
    Serial.println("Connecting to Adafruit IO");
    // connect to io.adafruit.com
    // io.connect();
    // attach message handler for the each feed
    wdt_reset();
    luxFeed->onMessage(handleMessage);
    tempFeed->onMessage(handleMessage);
    presFeed->onMessage(handleMessage);
    errorFeed->onMessage(handleMessage);
  
    // wait for a connection
    int IOconnAttempt = 0;
    while(io.status() < AIO_CONNECTED && IOconnAttempt <= 60) 
    {
      Serial.print(".");
      delay(500);
      IOconnAttempt++;
    }
    if ( IOconnAttempt > 60 )
    {
      IOconnERROR = 1;
      if ( Serial )
      {
        Serial.println("");
        Serial.println("Aborting connection attempt to Adafruit IO");
        Serial.print("IP address:   ");
        Serial.println(WiFi.localIP());
      }
      ERRORLOG = SD.open("error.txt", FILE_WRITE);
      if (ERRORLOG)
      {
        TimeStampSD(ERRORLOG);
        ERRORLOG.println("Adafruit IO connection failed");
        ERRORLOG.close();
      }
    }
    IOconnAttempt = 0;

      // we are connected
      if ( Serial );
      {
        Serial.println();
        Serial.println(io.statusText());
      }
  }
  if ( IOconnERROR == 0 && WiFiError == 0 && TSL2561Error == 0 && BMP180Error == 0 && SDError == 0 )
  {
    ERRORLOG = SD.open("error.txt", FILE_WRITE);
    TimeStampSD(ERRORLOG);
    ERRORLOG.println("REBOOT: OK");
    ERRORLOG.close();
    errorFeed->save("REBOOT: OK");
  }

  if ( IOconnERROR == 0 )
    {
      if ( BMP180Error == 1 )
      {
        errorFeed->save("REBOOT: BMP180 error");
      }
      if ( TSL2561Error == 1 )
      {
        errorFeed->save("REBOOT: TSL2561 error");
      }
      if ( SDError == 1 )
      {
        errorFeed->save("REBOOT: SD card error");
      }
    }

  LUXLOG = SD.open("luxlog.txt", FILE_WRITE);
    if (LUXLOG )
    {
      LUXLOG.println("Date Time,LCLV Raw,LCLV Modified,LCLV Value");
      LUXLOG.close();
    }
    
  if ( Serial )
  {
    /* We're ready to go! */
    Serial.println("Beginning data collection");
    //For OLED display
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(10,0);
    display.print("Ready");
    display.display();    // NOTE: You _must_ call display after making any drawing commands
    delay(2500);
    display.clearDisplay();
    display.display();
    }
}

void loop() 
{
  DateTime now = rtc.now();
    
  if ( debug == 1 )
  {
    StartLoopRuntime = millis();
    analogWrite(lclvPin,1023);
    ReadTSL2561();
    //For OLED display
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(10,0);
    display.print(TSL2561Val);
    display.display();    // NOTE: You _must_ call display after making any drawing commands
    /*LUXLOG = SD.open("luxlog.txt", FILE_WRITE);
    if (LUXLOG )
    {
      TimeStampSD(LUXLOG);
      LUXLOG.print(TSL2561Val);
      LUXLOG.print(",");
      LUXLOG.close();
    } */
    delay(2000);

    analogWrite(lclvPin,511);
    ReadTSL2561();
    //For OLED display
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(10,0);
    display.print(TSL2561Val);
    display.display();    // NOTE: You _must_ call display after making any drawing commands
    /*LUXLOG = SD.open("luxlog.txt", FILE_WRITE);
    if (LUXLOG )
    {
      TimeStampSD(LUXLOG);
      LUXLOG.print(TSL2561Val);
      LUXLOG.print(",");
      LUXLOG.close();
    }*/
    delay(2000);

    analogWrite(lclvPin,0);
    ReadTSL2561();
    //For OLED display 
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0,0);
    display.print(TSL2561Val);
    display.display();    // NOTE: You _must_ call display after making any drawing commands
    /*LUXLOG = SD.open("luxlog.txt", FILE_WRITE);
    if (LUXLOG )
    {
      LUXLOG.println(TSL2561Val);
      LUXLOG.close();
    }*/
    //delay(2000);
  }

  //Clear display after 15s
    if ( now.second() >= DisplayDuration )
    {
      display.clearDisplay();
      display.display();
    }
  // io.run(); keeps the client connected to
  // io.adafruit.com, and processes any incoming data.
  if ( WiFiError == 0 && IOconnERROR == 0 )
    {
      io.run();
    }
  
  if ( now.second() == 0 ) //Update SD log file every minute
  { 
    if ( LCLVactive == 1 )
    {
      //shut off LCLV to get good read on LUX
      analogWrite(lclvPin,0);
      LCLVactive = 0;
    }
    if ( BMP180Error == 0 )
    {
      ReadBMP180();
    }
    if ( TSL2561Error == 0 )
    {
      ReadTSL2561();
    }
    LUXLOG = SD.open("luxlog.txt", FILE_WRITE);
    if (LUXLOG )
    {  
      TimeStampSD(LUXLOG);
      LUXLOG.print(TSL2561Val);
      LUXLOG.print(",");
      LUXLOG.close();
    }
    // If lux is nearing saturation, utlize LCLV and take a new reading
    if ( TSL2561Val >= 10000 ) 
    {
      LCLVValue = 859;  //LCLV at 859; 84% of 3v which is 50% at 5v, the voltage of LCLV
      analogWrite(lclvPin,LCLVValue); 
      LCLVactive = 1;
      ReadTSL2561();
      analogWrite(lclvPin,0); //Turn off LCLV to save juice
      LCLVactive = 0;
      LUXLOG = SD.open("luxlog.txt", FILE_WRITE);
      if (LUXLOG )
      {
        LUXLOG.print(TSL2561Val);
        LUXLOG.print(",");
        LUXLOG.println(LCLVValue);
        LUXLOG.close();
      }
    }
    else 
    {
      LUXLOG = SD.open("luxlog.txt", FILE_WRITE);
      if (LUXLOG )
      {
        LUXLOG.print("0");
        LUXLOG.print(",");
        LUXLOG.println("0");
        LUXLOG.close();
      }
    }
    if ( Serial )
    {
      Serial.print(now.hour(), DEC);
      Serial.print(":");
      Serial.print(now.minute(), DEC);
      Serial.print(":");
      Serial.println(now.second(), DEC);
      WriteBMP180Serial();
      WriteTSL2561Serial();
      ThisSecond = now.second();
      DisplayDuration = ThisSecond + 15;
      WriteBMP180OLED();
      WriteTSL2561OLED();
      display.setTextSize(1); //textsize 1 = 7 px high
      display.setTextColor(WHITE);
      display.setCursor(50,24);
      display.print(now.hour(), DEC);
      display.print(":");
      display.print(now.minute(), DEC);
      display.print(":");
      display.println(now.second(), DEC);
      display.display();
    }
    DATALOG = SD.open("log.txt", FILE_WRITE);
    // if the file opened okay, write to it:
    if (DATALOG) 
    {
      TimeStampSD(DATALOG); 
      if ( TSL2561Error == 0 )
      {
        WriteTSL2561SD();
      }
      if ( BMP180Error == 0 )
      {
        WriteBMP180SD();
      }
      DATALOG.println();
      DATALOG.close();     
    } 
    else 
    {
      // if the file didn't open, print an error:
      Serial.println("error opening log file");
    }
    if (WiFiError == 0 && IOconnERROR == 0 )
    {
      if ( BMP180Error == 0 )
      {
        wdt_reset();
        WriteBMP180IO();
      }
      if ( TSL2561Error == 0 )
      {
        wdt_reset();
        WriteTSL2561IO();
      }
    }
    else 
    {
      if ( Serial )
      {
        Serial.println("Not attempting write to Adafruit IO (IOConnERROR = 1)");
      }
    }
  }

  if ( debug == 1)
  {
    if ( now.second() == 0 || now.second() == 30 && JustPrintRuntimeOnce == 0 )
    {
      LoopRuntime = (millis() - StartLoopRuntime);
      Serial.print("Loop runtime: "); Serial.print(LoopRuntime); Serial.println(" ms");
      JustPrintRuntimeOnce = 1;
    }
    else 
    {
      JustPrintRuntimeOnce == 0;
    }
  }
}

void TimeStampSD(File LogFile)  //Requires the name of the logfile to be passed, i.e. TimeStampSD(ERRORLOG);
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
  if ( WiFiError == 0 )
  {
  if (Serial)
  {
    Serial.print("Connecting to ");
    Serial.println(WIFI_SSID);
  }
  
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
        if (Serial)
        {
          Serial.println();
          Serial.println("WiFi connection failed");
          }
        ERRORLOG = SD.open("error.txt", FILE_WRITE);
        if (ERRORLOG)
        {
          TimeStampSD(ERRORLOG);
          ERRORLOG.println("WiFi connection failed");
          ERRORLOG.close();
        }
        WiFiError = 1;
        IOconnERROR = 1; //If no WiFi connection, prevent connection to Adafruit IO
        return;
       }
    }
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  }
}

