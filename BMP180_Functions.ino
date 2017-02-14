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
      Serial.println("BMP180 sensor error");
    }
    ERRORLOG = SD.open("error.txt", FILE_WRITE);
    if (ERRORLOG)
    {
      TimeStampSD(ERRORLOG);
      ERRORLOG.println("BMP180 Sensor Error: Failure to initialize");
      ERRORLOG.close();
    }
  }
}

void WriteBMP180Serial()
{
  Serial.print("Pressure:    ");
  Serial.print(PressureVal);
  Serial.println(" hPa");
  Serial.print("Temperature: ");
  Serial.print(TemperatureVal);
  Serial.println(" C");
}

void WriteBMP180SD()
{
  DATALOG.print(TemperatureVal);
  DATALOG.print(",");
  DATALOG.print(PressureVal);
}

void WriteBMP180IO()
{
  tempFeed->save(TemperatureVal);
  presFeed->save(PressureVal);
}
