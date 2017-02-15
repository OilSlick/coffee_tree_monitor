void displayTSL2561SensorDetails(void)
{
  sensor_t sensor;
  tsl.getSensor(&sensor);
  Serial.println("------------------------------------");
  Serial.print  ("Sensor:       "); Serial.println(sensor.name);
  Serial.print  ("Driver Ver:   "); Serial.println(sensor.version);
  Serial.print  ("Unique ID:    "); Serial.println(sensor.sensor_id);
  Serial.print  ("Max Value:    "); Serial.print(sensor.max_value); Serial.println(" lux");
  Serial.print  ("Min Value:    "); Serial.print(sensor.min_value); Serial.println(" lux");
  Serial.print  ("Resolution:   "); Serial.print(sensor.resolution); Serial.println(" lux");  
  Serial.println("------------------------------------");
  delay(500);
}

/**************************************************************************/
/*
    Configures the gain and integration time for the TSL2561
*/
/**************************************************************************/
void configureSensor(void)
{
  /* You can also manually set the gain or enable auto-gain support */
  // tsl.setGain(TSL2561_GAIN_1X);      /* No gain ... use in bright light to avoid sensor saturation */
  // tsl.setGain(TSL2561_GAIN_16X);     /* 16x gain ... use in low light to boost sensitivity */
  tsl.enableAutoRange(true);            /* Auto-gain ... switches automatically between 1x and 16x */
  
  /* Changing the integration time gives you better sensor resolution (402ms = 16-bit data) */
  // tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_13MS);      /* fast but low resolution */
  // tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_101MS);  /* medium resolution and speed   */
   tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_402MS);  /* 16-bit data but slowest conversions */

  /* Update these values depending on what you've set above! 
  Serial.print  ("Gain:         "); Serial.println("Auto");
  Serial.print  ("Timing:       "); Serial.println("402 ms");
  Serial.println("------------------------------------"); */
}

void ReadTSL2561()
{
  /* Get a new sensor event */ 
  sensors_event_t event;
  tsl.getEvent(&event);   //TSL2561 Lux sensor
  
  /* Display the results (light is measured in lux) */
  if (event.light)
  {
    TSL2561Val = event.light;
    if ( TSL2561Val >= 17000 )
    {
      errorFeed->save("TSL2561 Sensor Error: lux above sensor max threshold");  //Log error to Adafruit IO
      ERRORLOG = SD.open("error.txt", FILE_WRITE);
      if (ERRORLOG)
    {
      TimeStampSD(ERRORLOG);
      ERRORLOG.println("TSL2561 Sensor Error: lux above sensor max threshold");
      ERRORLOG.close();
    }
    }
  }
  else
  {
    /* If event.light = 0 lux during sun-down, there is likely not enough light for the sensor to read */
    TSL2561Val = 0;
  }  
}

void WriteTSL2561Serial()
{
  Serial.print("Lux:         ");
  Serial.println(TSL2561Val);
}

void WriteTSL2561SD()
{
  DATALOG.print(TSL2561Val);
  DATALOG.print(",");
}

void WriteTSL2561IO()
{
  luxFeed->save(TSL2561Val);
}

