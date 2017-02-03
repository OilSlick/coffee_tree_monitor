# Coffee Tree Monitor
An Arduino-based sensor network to monitor environmental conditions of coffee trees.

Intended to compare the differences between two coffee trees; one shaded and one unshaded. 

### Basic Operation

Collect and compare the following data from each tree:

1. Amount of light at tree (still deciding where to measure this)
2. Temperature at the sensor (presumably attached low on main stem)
3. Soil moisture content 

**As of February 3, 2017 this project is still being developed.** I am currently prototyping the circuit and all of the sensors, which I'm still researching and sourcing.  

Current components included in the circuit:

1. <a target="blank" href="https://learn.adafruit.com/adafruit-feather-32u4-fona?view=all">Adafruit Feather 32u4 Fona</a>
2. BMP180 Barometric Pressure/Temperature/Altitude Sensor from <a target="blank" href="https://www.adafruit.com/product/1603">Adafruit</a>
3. TSL2561 Luminosity Sensor from <a target="blank" href="https://learn.adafruit.com/tsl2561?view=all">Adafruit</a>
4. DS3231 RTC (A cheap-ass clone bought locally)
5. SHARP Memory Display Breakout from <a target="blank" href="https://www.adafruit.com/product/1393">Adafruit</a> (may replace for a true ePaper display)
6. <a target="blank" href="https://www.amazon.com/Panasonic-NCR18650B-3400mAh-Rechargeable-Battery-Green/dp/B00DHXY72O">NCR18650B Li-ion battery</a> (hoping with solar power I can instead use a smaller 2200mAh LiPo)

### Hardware To-Do
* [x] Add SD Card adapter
* [ ] Add soil sensor
* [ ] Add solar panel
* [ ] Add BLE for sensor network comms
* [ ] Organize circuit for project box
* [ ] Evaluate battery life and consider LiPo to replace current battery

### Software To-Do
* [x] Configure for SD Card adapter
* [x] Add lowpower/sleep routines
* [ ] Power down sensors between data collection (this is causing problems with TSL2561)
* [ ] Configure Fona routines for data upload
* [ ] Add error checking and logging to SD, www, and via LED blink