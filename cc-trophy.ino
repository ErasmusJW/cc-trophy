#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager
#include <WiFi.h>
#include <Wire.h>
#include "Qwiic_LED_Stick.h" // Click here to get the library: http://librarymanager/All#SparkFun_Qwiic_LED_Stick
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <EEPROM.h> // Include the EEPROM library
#include <functional>
#include <map>

namespace wifiSettings {
  const auto s_accesPointName = "CC TROPHY";
  const auto s_accesPointPassword = "werfetter";
}

namespace eepromSetting {
  const uint16_t ui_EEPROMsize = 1;
  const uint16_t ui_brightnessAdd = 0;
}


uint8_t g_brightness = 0;
const auto g_targetUrl = "https://www.fieldpop.io/get_time";



WiFiManager wm;
LED LEDStick; //Create an object of the LED class




// LED SETTINGS
#define BRIGHTNESS_MAX 31
#define BRIGHTNESS_MIN 6

#define WHITE 255, 255, 255
#define SMC_BLUE 76, 128, 194
#define RED 255, 0, 0
#define GREEN 0,255,0
#define BLUE 0,0,255
#define YELLOW 255,255,0
#define PURPLE 125, 0 ,125
#define ROSE_RED 194, 30, 86
#define FIELD_SERVER_PURPLE 102,0,204 //accroding to riaan
#define aquamarine 127,255,212
#define olive 28,128,0


void WalkingRainbow(byte rainbowLength, byte LEDLength, int delayTime) {
  //Create three LEDs the same length as the LEDStick to store color values
  byte redArray[LEDLength];
  byte greenArray[LEDLength];
  byte blueArray[LEDLength];
  //This will repeat rainbowLength times, generating 3 arrays (r,g,b) of length LEDLength
  //This is like creating rainbowLength differnent rainbow arrays where the positions
  //of each color have changed by 1
  for (byte j = 0; j < rainbowLength; j++) {
    //This will repeat LEDLength times, generating 3 colors (r,g,b) per pixel
    //This creates the array that is sent to the LED Stick
    for (byte i = 0 ; i < LEDLength ; i++) {
      //there are n colors generated for the rainbow
      //the rainbow starts at the location where i and j are equal: n=1
      //the +1 accounts for the LEDs starting their index at 0
      //the value of n determines which color is generated at each pixel
      int n = i + 1 - j;
      //this will loop n so that n is always between 1 and rainbowLength
      if (n <= 0) {
        n = n + rainbowLength; 
      }
      //the nth color is between red and yellow
      if (n <= floor(rainbowLength / 6)) {
        redArray[i] = 255;
        greenArray[i] = floor(6 * 255 / (float) rainbowLength * n);
        blueArray[i] = 0;
      }
      //the nth color is between yellow and green
      else if (n <= floor(rainbowLength / 3)) {
        redArray[i] = floor(510 - 6 * 255 / (float) rainbowLength * n);
        greenArray[i] = 255;
        blueArray[i] = 0;
      }
      //the nth color is between green and cyan
      else if (n <= floor(rainbowLength / 2)) {
        redArray[i] = 0;
        greenArray[i] = 255;
        blueArray[i] = floor( 6 * 255 / (float) rainbowLength * n - 510);
      }
      //the nth color is between cyan and blue
      else if ( n <= floor(2 * rainbowLength / 3)) {
        redArray[i] = 0;
        greenArray[i] = floor(1020 - 6 * 255 / (float) rainbowLength * n);
        blueArray[i] = 255;
      }
      //the nth color is between blue and magenta
      else if (n <= floor(5 * rainbowLength / 6)) {
        redArray[i] = floor(6 * 255 / (float) rainbowLength * n - 1020);
        greenArray[i] = 0;
        blueArray[i] = 255;
      }
      //the nth color is between magenta and red
      else {
        redArray[i] = 255;
        greenArray[i] = 0;
        blueArray[i] = floor(1530 - (6 * 255 / (float)rainbowLength * n));;
      }
    }
    //sets all LEDs to the color values according to the arrays
    //the first LED corresponds to the first entries in the arrays
    LEDStick.setLEDColor(redArray, greenArray, blueArray, LEDLength);
    delay(delayTime);
  }
}

uint8_t getStoredLedBrightness(){
  uint8_t storedValue = EEPROM.read(eepromSetting::ui_brightnessAdd);
  if(storedValue > BRIGHTNESS_MAX){
    return BRIGHTNESS_MAX;
  } 

  if(storedValue < BRIGHTNESS_MIN){
    return BRIGHTNESS_MIN;
  }
  return storedValue;
}

void storeLedBrightness(uint8_t level){
  EEPROM.write(eepromSetting::ui_brightnessAdd, level);
  EEPROM.commit();
}

typedef enum e_status 
{   
  E_BOOT ,
  E_WIFI_NOT_CONFIGURED , 
  E_WIFI_NOT_CONNECTED ,
  E_WIFI_CONNECTED ,
  E_FIELPOP_ONLINE ,
  E_FIELDPOP_OFFLINE ,
  E_FIELDPOP_DEPLOYING
};


e_status APP_STATE = E_BOOT;

unsigned long secTimer=0;

void setAppStatus(uint8_t status) {
  switch(status){
    case E_BOOT :
      LEDStick.setLEDColor(WHITE);
      break;
    case E_WIFI_NOT_CONFIGURED:
      LEDStick.setLEDColor(RED);
      break;
    case E_WIFI_NOT_CONNECTED:
      LEDStick.setLEDColor(olive);
      break;
    case E_WIFI_CONNECTED :
      LEDStick.setLEDColor(PURPLE);
      break;
    case E_FIELPOP_ONLINE:
      LEDStick.setLEDColor(BLUE);
      break;
    case E_FIELDPOP_OFFLINE:
      LEDStick.setLEDColor(YELLOW);
    
      break;
    case E_FIELDPOP_DEPLOYING : 
      LEDStick.setLEDColor(YELLOW);

  }
  APP_STATE = static_cast<e_status>(status);


}


void initLed(){
    if (LEDStick.begin() == false){
      while(1){
        Serial.println("Qwiic LED Stick failed to begin. Please check wiring and try again!");
        delay(5000);
      }
    }

    //Start by resetting the state of the LEDs
    LEDStick.LEDOff();
    g_brightness = getStoredLedBrightness();

    LEDStick.setLEDBrightness(g_brightness) ;//value between 0-31
    WalkingRainbow(30, 10, 100); // will this delay the boot time, but looks cool
    WalkingRainbow(30, 10, 100);
}

bool initWifi(){
    //automatically connect using saved credentials if they exist
    //If connection fails it starts an access point with the specified name
    if(wm.autoConnect(wifiSettings::s_accesPointName,wifiSettings::s_accesPointPassword)){
        Serial.println("wifi auto connect failed");
        return true;
    }
    else {
        Serial.println("Configportal AP running");
        return false;
    }
}


void setup() {
    Wire.begin();
    EEPROM.begin(eepromSetting::ui_EEPROMsize);
    Serial.begin(115200);
    WiFi.mode(WIFI_STA);

    initLed();
    setAppStatus(E_BOOT);


    wm.setConfigPortalBlocking(false);
    Serial.println(xPortGetCoreID());
  

    if(initWifi()){
      setAppStatus(E_WIFI_CONNECTED);
      Serial.println("Initial wifi connection, skipping creating access point");
    }else{
      setAppStatus(E_WIFI_NOT_CONFIGURED);
      Serial.println("Initial wifi FAILED,  creating access point");
    }

    checkWifiStatus();

}

int getFieldpopStatus(){
 int httpCode;
  WiFiClientSecure *client = new WiFiClientSecure;
  if(client) {
    client -> setInsecure();

    {
      // Add a scoping block for HTTPClient https to make sure it is destroyed before WiFiClientSecure *client is 
      HTTPClient https;
  
      if (https.begin(*client, g_targetUrl)) {  // HTTPS
        httpCode = https.GET();
  
        // httpCode will be negative on error
        if (httpCode > 0) {
          // HTTP header has been send and Server response header has been handled
          Serial.printf("[HTTPS] GET... code: %d\n", httpCode);
        } else {
          Serial.printf("[HTTPS] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
        }
  
        https.end();
      } else {
        httpCode = -1;
        Serial.printf("[HTTPS] Unable to connect\n");
      }

      // End extra scoping block
    }
  
    delete client;
  } else {
    
    Serial.println("Unable to create client");
    httpCode = -1;
  }  
  return httpCode;
}

void checkWifiStatus(){

  static int statusPrevious = -1;
  int status = WiFi.status();

    // Check Wi-Fi connection status
  if (status == WL_CONNECTED) {
    // Connected to Wi-Fi
    Serial.println("Connected to Wi-Fi!");

    if(status != statusPrevious)
      setAppStatus(E_WIFI_CONNECTED);
    int status = getFieldpopStatus();
    //at least switch here ffs
    if(status == 200){
      Serial.println("E_FIELPOP_ONLINE");
      setAppStatus(E_FIELPOP_ONLINE);
    } else if( status == 503){
      Serial.println("E_FIELDPOP_OFFLINE");
      setAppStatus(E_FIELDPOP_OFFLINE);
    } else {
      Serial.println("E_FIELDPOP_DEPLOYING");
      setAppStatus(E_FIELDPOP_DEPLOYING);
    }

  } else {
    if( wm.getWiFiIsSaved()){
      WiFi.reconnect();
      setAppStatus(E_WIFI_NOT_CONNECTED);
      Serial.println("Not connected to Wi-Fi!");
    }

  }
  statusPrevious = status;
}

std::map<const char *,std::function<void(int)>> serrialCommands;


void loop() {
  wm.process(); // update the manager
  uint8_t new_brightness = g_brightness;
  auto now = millis();
  if(now - secTimer > 10000){
    secTimer = now;
    checkWifiStatus();
  }

  while (Serial.available() > 0) {
    char command = Serial.read();
    if (command == '+') {
      new_brightness = new_brightness >= BRIGHTNESS_MAX ? BRIGHTNESS_MAX : ++new_brightness;

    } else if (command == '-'){
      new_brightness = new_brightness < BRIGHTNESS_MIN ? BRIGHTNESS_MIN : --new_brightness;
    }  else if (command == 'R' || command == 'r') {
      Serial.println("Reset start");
      WiFiManager wm;
      Serial.println("Switching to AP mode on next reboot.");
      wm.resetSettings();
      Serial.println("Reset done");
      delay(1000); // Delay to allow the message to be printed
      ESP.restart(); // Reboot  to enter AP mode
    } else if (command == 'c' || command == 'c') {

        if(wm.autoConnect(wifiSettings::s_accesPointName,wifiSettings::s_accesPointPassword)){
            setAppStatus(E_WIFI_CONNECTED);
            Serial.println("initial wifi connect skipping creation of AP");

        }
        else {
            Serial.println("Configportal running");
            setAppStatus(E_WIFI_NOT_CONFIGURED);
        }

    }
     
  }
  if(new_brightness != g_brightness){
      g_brightness = new_brightness;
      LEDStick.setLEDBrightness(g_brightness);
      storeLedBrightness(g_brightness);
      Serial.println(g_brightness);
  }

}
