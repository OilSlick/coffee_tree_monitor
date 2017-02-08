# Coffee Tree Monitor
An Arduino-based sensor network to monitor environmental conditions of coffee trees.

Intended to compare the differences between two types of microclimate for coffee trees; shaded and unshaded environments. 

### Basic Unit Operation

Collect and compare the following data from each tree:

1. Amount of light measured at the top of the tree canopy
2. Air temperature at the sensor (presumably attached low on main stem)
3. Soil moisture content 

**As of February 3, 2017 this project is still being developed.** I am currently prototyping the circuit and all of the sensors, which I'm still researching and sourcing.  

Current unit configurations and tests are being documented in the [wiki](https://github.com/OilSlick/coffee_tree_monitor/wiki)

### Hardware To-Do
* [x] Add SD Card adapter
* [ ] Add soil sensor
* [ ] Add solar panel (February 8, 2017: ordered and en route )
* [ ] Determine method for posting data online
* [ ] Organize circuit for project/gang box
* [ ] Evaluate battery life and consider physically smaller LiPo to replace current Li-ion battery

### Software To-Do
* [x] Configure for SD Card adapter
* [x] Add lowpower/sleep routines
* [ ] Power down sensors between data collection (this is causing problems with TSL2561)
* [ ] Configure Fona routines for data upload
* [ ] Add error checking and logging to SD, www, and via LED blink