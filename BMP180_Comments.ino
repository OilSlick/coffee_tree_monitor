/* This driver uses the Adafruit unified sensor library (Adafruit_Sensor),
   which provides a common 'type' for sensor data and some helper functions.
   
   To use this driver you will also need to download the Adafruit_Sensor
   library and include it in your libraries folder.

   You should also assign a unique ID to this sensor for use with
   the Adafruit Sensor API so that you can identify this particular
   sensor in any data logs, etc.  To assign a unique ID, simply
   provide an appropriate value in the constructor below (12345
   is used by default in this example).
   
   Connections
   ===========
   Connect SCL to analog 5
   Connect SDA to analog 4
   Connect VDD to 3.3V DC
   Connect GROUND to common ground
    
   History
   =======
   2013/JUN/17  - Updated altitude calculations (KTOWN)
   2013/FEB/13  - First version (KTOWN)
*/    

/* Calculating altitude with reasonable accuracy requires pressure    *
 * sea level pressure for your position at the moment the data is     *
 * converted, as well as the ambient temperature in degress           *
 * celcius.  If you don't have these values, a 'generic' value of     *
 * 1013.25 hPa can be used (defined as SENSORS_PRESSURE_SEALEVELHPA   *
 * in sensors.h), but this isn't ideal and will give variable         *
 * results from one day to the next.                                  *
 *                                                                    *
 * You can usually find the current SLP value by looking at weather   *
 * websites or from environmental information centers near any major  *
 * airport.                                                           *
 *                                                                    *
 * For example, for Paris, France you can check the current mean      *
 * pressure and sea level at: http://bit.ly/16Au8ol                   */

 
     
    
