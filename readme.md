<<<<<<< Updated upstream
# Overview

<table>
  <tr>
    <td width="300px">
      This is a LoRa APRS Tracker working on the 433 MHz band.<br/>
      It features an Access Point for real time configuration.<br/>
      Its main use is to send a periodic beacon containing: 
      <ul>
        <li>Callsign</li>
        <li>GPS or fixed coordinates</li>
        <li>Altitude</li>
        <li>Battery voltage and current</li>
      </ul>
      Any received LoRa APRS packet, including
      the one repeated by a local digipeater,
      is displayed in the (very) small 0.96" display.<br/>
      <img src="images/display.png" width="300" />
    </td>
    <td><img src="images/my_LILYGO_TBEAM.jpg" width="600"></td>
  </tr>
</table>



### About the fork 

This LoRa APRS Tracker is derived from (https://github.com/lora-aprs/LoRa_APRS_Tracker) by [peterus](https://github.com/lora-aprs/LoRa_APRS_Tracker/commits?author=peterus)

I added: 
- an Access Point and Captive Portal for easy configuration 
- many comments to the source code to keep it more readable

# Block diagram
![TTGO T-Beam](images/block_diagram.png)

# Supported boards

You can use one of the Lora32 boards:

* TTGO T-Beam V0.7 (433MHz SX1278)
The original software is designed for it, also, but I never tried.

* TTGO T-Beam V1 (433MHz SX1278)
This board costs around 35 Euros and includes a small 0.96" display
Keep in mind: you need a 433MHz version!

* Or you can build your own. See below
<td></td>

## You can use a LILYGO TTGO TBEAM

<table>
  <tr>
    <td>Front view</td><td>Rear view</td>
  </tr>
  <tr>
    <td><img src="images/LILYGO-TBEAM.png" width="400">
  </td>
  <td>
    <img src="images/LILYGO-TBEAM-GPS-antenna.png" width="400">
  </td>
  </tr>
  
</table>

## ... or you can build your own board
<table>
  <tr>
    <td>
      <img src="images/my_esp32_lora_aprs_tracker_portal.jpg" width="400">
    </td>
    <td>
    <img src="images/MyLoRa_APRS_GPS_schematic.png" width="400">
    </td>
  </tr>
</table>  
 

### Partlist
<ul>
  <li>
    <a href="https://www.amazon.it/gp/product/B093GQGJCV/">ESP32 microcontroller</a>
  </li>
<li>
    <a href="https://www.amazon.it/gp/product/B088LR3488/">NEO6M GPS</a>
</li>
<li>
    <a href="https://www.amazon.it/gp/product/B07RD2JV7Y/">LoRa 432MHz transceiver</a>
</li>
<li>
    <a href="https://www.amazon.it/ARCELI-pollici-SSD1306-auto-luminoso-Raspberry/dp/B07J2QWF43/" >OLED 0.96"</a>
</li>
</ul>  

### Please note that I changed the small original GPS antenna with a most performing one.

<table>
  <tr>
    <td>The original antenna</td><td>The most performing antenna</td><td width="300px"><a href="https://www.amazon.it/gp/product/B01BML4XMQ/">The amplified GPS magnetic antenna with a 3m cable</a> <br/>very useful when the tracker is inside a vehicle. Yes, the TBEAM or the NEO6M breakboard supply the GPS amplifier with 3.3V by the same antenna cable!
    </td>
  </tr>
  <tr>
    <td><img src="images/original_GPS_antenna.png" width="300">
  </td>
  <td>
    <img src="images/LILYGO-TBEAM-GPS-antenna.png" width="300">
  </td>
  <td>
    <img src="images/amplified_GPS_antenna.png" width="300">
  </td>
  </tr>
  
</table>


# Compiling and configuring


# How to compile

The best success is to use PlatformIO (and it is the only platform where I can support you). 

* Go to [PlatformIO](https://platformio.org/) download and install the IDE. 
* If installed open the IDE, go to the left side and click on 'extensions' then search for 'PatformIO' and install.
* When installed click 'the ant head' on the left and choose import the project on the right.
* Just open the folder and you can compile the Firmware.

# Configuration

* Press the service button (the middle one) for more than 5 seconds and a WiFi hotspot will be alive (SSID: ESP32_APRS)
* Connect to it with your smartphone or PC using the password 12345678
* After connection navigate to http://192.168.4.1 and the settings web page will show

# Settings
Be very careful when you change them!

![TTGO T-Beam](images/general_settings.png)

## Beacon modes
activate only one!!!
- I suggest to start with fixed_beacon (that don't use GPS and send the location set)
- Then try beacon (use GPS location). 
- Finally you can try smart_beacon in a moving vehicle. 

### Fixed beacon
* These are the settings for a beacon transmitting fixed coordinates (not using GPS) 
![TTGO T-Beam](images/fixed_beacon_settings.png)

### Plain Beacon
* These are the settings for a beacon transmitting the GPS coordinates every 20 minutes
![TTGO T-Beam](images/beacon_settings.png)

### Smart Beacon
* These are the settings for a beacon transmitting the GPS coordinates more frequently if the tracker moves at a higher speed 
![TTGO T-Beam](images/smart_beacon_settings.png)

## PTT (Push To Talk)
* These are the settings for an external transmitter connected by its Push To Talk 

![TTGO T-Beam](images/PTT_settings_save.png)

* You can save the settings or restore default settings
=======
# Overview

<table>
  <tr>
    <td width="300px">
      This is a LoRa APRS Tracker working on the 433 MHz band.<br/>
      It features an Access Point for real time configuration.<br/>
      Its main use is to send a periodic beacon containing: 
      <ul>
        <li>Callsign</li>
        <li>GPS or fixed coordinates</li>
        <li>Altitude</li>
        <li>Battery voltage and current</li>
      </ul>
      Any received LoRa APRS packet, including
      the one repeated by a local digipeater,
      is displayed in the (very) small 0.96" display.<br/>
      <img src="images/display.png" width="300" />
    </td>
    <td><img src="images/my_LILYGO_TBEAM.jpg" width="600"></td>
  </tr>
</table>



### About the fork 

This LoRa APRS Tracker is derived from (https://github.com/lora-aprs/LoRa_APRS_Tracker) by [peterus](https://github.com/lora-aprs/LoRa_APRS_Tracker/commits?author=peterus)

I added: 
- an Access Point and Captive Portal for easy configuration 
- many comments to the source code to keep it more readable

# Block diagram
![TTGO T-Beam](images/block_diagram.png)

# Supported boards

You can use one of the Lora32 boards:

* TTGO T-Beam V0.7 (433MHz SX1278)
The original software is designed for it, also, but I never tried.

* TTGO T-Beam V1 (433MHz SX1278)
This board costs around 35 Euros and includes a small 0.96" display
Keep in mind: you need a 433MHz version!

* Or you can build your own. See below
<td></td>

## You can use a LILYGO TTGO TBEAM

<table>
  <tr>
    <td>Front view</td><td>Rear view</td>
  </tr>
  <tr>
    <td><img src="images/LILYGO-TBEAM.png" width="400">
  </td>
  <td>
    <img src="images/LILYGO-TBEAM-GPS-antenna.png" width="400">
  </td>
  </tr>
  
</table>

## ... or you can build your own board
<table>
  <tr>
    <td>
      <img src="images/my_esp32_lora_aprs_tracker_portal.jpg" width="400">
    </td>
    <td>
    <img src="images/MyLoRa_APRS_GPS_schematic.png" width="400">
    </td>
  </tr>
</table>  
 

### Partlist
<ul>
  <li>
    <a href="https://www.amazon.it/gp/product/B093GQGJCV/">ESP32 microcontroller</a>
  </li>
<li>
    <a href="https://www.amazon.it/gp/product/B088LR3488/">NEO6M GPS</a>
</li>
<li>
    <a href="https://www.amazon.it/gp/product/B07RD2JV7Y/">LoRa 432MHz transceiver</a>
</li>
<li>
    <a href="https://www.amazon.it/ARCELI-pollici-SSD1306-auto-luminoso-Raspberry/dp/B07J2QWF43/" >OLED 0.96"</a>
</li>
</ul>  

### Please note that small original GPS antenna with a most performing one.

<table>
  <tr>
    <td>The original antenna</td><td>The most performing antenna</td><td width="300px"><a href="https://www.amazon.it/gp/product/B01BML4XMQ/">The amplified GPS magnetic antenna with a 3m cable</a> <br/>very useful when the tracker is inside a vehicle. Yes, the TBEAM or the NEO6M breakboard supply the GPS amplifier with 3.3V by the same antenna cable!
    </td>
  </tr>
  <tr>
    <td><img src="images/original_GPS_antenna.png" width="300">
  </td>
  <td>
    <img src="images/LILYGO-TBEAM-GPS-antenna.png" width="300">
  </td>
  <td>
    <img src="images/amplified_GPS_antenna.png" width="300">
  </td>
  </tr>
  
</table>


# Compiling and configuring


# How to compile

The best success is to use PlatformIO (and it is the only platform where I can support you). 

* Go to [PlatformIO](https://platformio.org/) download and install the IDE. 
* If installed open the IDE, go to the left side and click on 'extensions' then search for 'PatformIO' and install.
* When installed click 'the ant head' on the left and choose import the project on the right.
* Just open the folder and you can compile the Firmware.

# Configuration

* Press the service button (the middle one) for more than 5 seconds and a WiFi hotspot will be alive (SSID: ESP32_APRS)
* Connect to it with your smartphone or PC using the password 12345678
* After connection navigate to http://192.168.4.1 and the settings web page will show

# Settings
Be very careful when you change them!

![TTGO T-Beam](images/general_settings.png)

## Beacon modes
activate only one!!!
- I suggest to start with fixed_beacon (that don't use GPS and send the location set)
- Then try beacon (use GPS location). 
- Finally you can try smart_beacon in a moving vehicle. 

### Fixed beacon
* These are the settings for a beacon transmitting fixed coordinates (not using GPS) 
![TTGO T-Beam](images/fixed_beacon_settings.png)

### Plain Beacon
* These are the settings for a beacon transmitting the GPS coordinates every 20 minutes
![TTGO T-Beam](images/beacon_settings.png)

### Smart Beacon
* These are the settings for a beacon transmitting the GPS coordinates more frequently if the tracker moves at a higher speed 
![TTGO T-Beam](images/smart_beacon_settings.png)

## PTT (Push To Talk)
* These are the settings for an external transmitter connected by its Push To Talk 
* Pleas note that you can save the settings or restore default settings
![TTGO T-Beam](images/PTT_settings_save.png)
>>>>>>> Stashed changes
