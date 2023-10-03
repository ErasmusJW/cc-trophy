# The CC-Trophy 🏆

Hackathon trophy for the sad departure of CC.

This is a trophy that is definitely _*not_* designed to induce FOMO or attachment issues.
Rather, this is a tribute to 12 years dedicated to an ever evolving product; from FieldServer to SMC Cloud to FieldServer Grid Manager.

Under the hood, your gift is controlled by an ESP32 that connects to an LED light strip with 10 individually programmable LEDs.
We connected the illuminated trophy to fieldpop.io so you'll always know when production is down -- which will hopefully only ever happen between 9 and 10 on a Tuesday morning! 

Hope you enjoy!

## Quickstart guide

1. Power on the device via a USB cable.
2. Connect to the ESP32 access point called "CC Trophy" to set up an internet connection.
3. Navigate to 192.168.8.1 in your browser to get to the WiFi Configuration UI.
4. Select your WiFi network and enter your password. 
   - You can choose to save this information to skip the WiFI setup next time it boots. (recommended, but optional)
   - Once you've saved a WiFi configuration, the ESP32 access point will be removed and will not be recreated on start-up.
5. Once the device has WiFi access it will start to periodically GET www.fieldpop.io/get_time.
    - If the status code is 200, FieldPoP is ok and the trophy will be illuminated with a green to blue gradient. (reference to SMC Cloud and MSA)
    - If the status code is not 200, FieldPoP is not reachable and the trophy will pulse red.
    - If your device loses its WiFi connection, the trophy will pulse blue.
7. Sit back, and enjoy the green-blue colored trophy :)

## LED Color Indicators

| Color          | Description                                  |
| :------------- | :------------------------------------------- |
| Rainbow        | On start-up (And who doesn't like rainbows?) |
| White          | WiFi configurable                            |
| Pulse blue     | WiFi connection lost                         |
| Green gradient | FieldPoP is ok                               |
| Pulse red      | FieldPoP not ok, send help pls               |
