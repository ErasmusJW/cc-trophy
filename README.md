# The CC-Trophy :trophy:

Hackathon trophy for the sad departure of CC.

This is a trophy that is definitely \*not\* designed to induce FOMO or attachment issues.
Rather, this is a tribute to 12 years dedicated to an ever evolving product; from FieldServer to SMC Cloud to FieldServer Grid Manager.

Under the hood, your gift is controlled by an ESP32 that connects to an LED light strip with 10 individually programmable LEDs.
We connected the illuminated trophy to fieldpop.io so you'll always know when production is down -- which will hopefully only ever happen between 9 and 10 on a Tuesday morning! 

Hope you enjoy!

## Quickstart guide

1. Power on the device via a USB cable.
2. Use a PC to connect to the ESP32 access point called "CC Trophy" to set up an internet connection.
3. Navigate to 192.168.48.1 in your browser to get to the WiFi Configuration UI.
4. Select your WiFi network and enter your password. 
   - You can choose to save this information to skip the WiFI setup next time it boots. (recommended, but optional)
   - Once you've saved a WiFi configuration, the ESP32 access point will be removed and will not be recreated on start-up.
5. Once the device has WiFi access it will start to periodically GET www.fieldpop.io/get_time.
    - If the status code is 200, FieldPoP is ok and the trophy will be illuminated with a green to blue gradient. (reference to SMC Cloud and MSA)
    - If the status code is not 200, FieldPoP is not reachable and the trophy will pulse red.
    - If your device loses its WiFi connection, the trophy will pulse yellow.
7. Sit back, and enjoy the green-blue colored trophy :)

## LED Color Indicators

| Color          | Description                                  |
| :------------- | :------------------------------------------- |
| Rainbow        | On start-up (And who doesn't like rainbows?) |
| White          | WiFi configurable                            |
| Pulse yellow   | WiFi connection lost                         |
| Green gradient | FieldPoP is ok                               |
| Pulse red      | FieldPoP not ok, send help pls               |

## Serial Connection

Some basic input-output options to the ESP32 are available via a serial connection. One of the serial outputs on start-up is a quote of the day from the SIOSA office, just to remind you of your days working here.

> Baudrate is 115200

| Serial input | Function                                                                  |
| :----------- | :------------------------------------------------------------------------ |
| 'r'          | Reset saved WiFi configuration (Brings back the "CC Trophy" access point) |
| '+'          | Chainable, increase LED brightness                                        |
| '-'          | Chainable, decrease LED brightness                                        |
| 'c'          | forces a wiffi connection                                                 |

## Further Customization

This repo is part of this gift so you have free reign to customize it further, if you want to.

Some helpful tools:
- Arduino IDE as the development environment
- This [tutorial video](https://www.youtube.com/watch?v=hjJx6QOWVkU) to connect the ESP32 to your Arduino IDE
- Arduino 3rd party libraries:
  - SparkFun Qwiic LED Stick Library by SparkFun Electronics (released with version 1.0.5)
  - WiFiManager by tzapu (released with version 2.0.16-rc.2)
