void displayBMP180SensorDetails(void)
{
  sensor_t sensor;
  bmp.getSensor(&sensor);
  Serial.println("------------------------------------");
  Serial.print  ("Sensor:       "); Serial.println(sensor.name);
  Serial.print  ("Driver Ver:   "); Serial.println(sensor.version);
  Serial.print  ("Unique ID:    "); Serial.println(sensor.sensor_id);
  Serial.print  ("Max Value:    "); Serial.print(sensor.max_value); Serial.println(" hPa");
  Serial.print  ("Min Value:    "); Serial.print(sensor.min_value); Serial.println(" hPa");
  Serial.print  ("Resolution:   "); Serial.print(sensor.resolution); Serial.println(" hPa");  
  Serial.println("------------------------------------");
  Serial.println("");
  delay(500);
}

void ReadBMP180( bool BMP180Error )
{
  /* Baro sensor event */
  sensors_event_t event;
  bmp.getEvent(&event);   

  if (event.pressure)
  {
    PressureVal = event.pressure;
    float temperature;
    bmp.getTemperature(&temperature);
    TemperatureVal = temperature; 
  }
  else
  {
    if ( Serial )
    {
      Serial.println("Sensor error");
    }
    ERRORLOG = SD.open("error.txt", FILE_WRITE);
    if (ERRORLOG)
    {
      SDTimeStamp(ERRORLOG);
      ERRORLOG.println("BMP180 Sensor Error: Failure to initialize");
      ERRORLOG.close();
    }
  }
}

void SerialWriteBMP180()
{
  Serial.print("Pressure:    ");
  Serial.print(PressureVal);
  Serial.println(" hPa");
  Serial.print("Temperature: ");
  Serial.print(TemperatureVal);
  Serial.println(" C");
}

void SDWriteBMP180()
{
  DATALOG.print(TemperatureVal);
  DATALOG.print(",");
  DATALOG.print(PressureVal);
}

void IOWriteBMP180()
{
  if ( connERROR == 0 )   //If connection failed, don't try to write to Adafruit IO
    {
      tempFeed->save(TemperatureVal);
      presFeed->save(PressureVal);
    }
}
