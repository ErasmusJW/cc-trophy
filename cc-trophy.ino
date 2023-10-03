#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager
#include <WiFi.h>
#include <Wire.h>
#include "Qwiic_LED_Stick.h" // Click here to get the library: http://librarymanager/All#SparkFun_Qwiic_LED_Stick
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <EEPROM.h> // Include the EEPROM library
#include <functional>
// #include <map>
#include "LED-functions.h"
#include <tuple>
#include <array>

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

typedef enum e_ledMode 
{   
  SOLID_BRU ,
  PULSING_BRU 
};


e_status APP_STATE = E_BOOT;
e_ledMode LED_MODE = SOLID_BRU;


WiFiManager wm;
LED LEDStick; //Create an object of the LED class

const char* quotes[] = {
  "*Ping-pong, ping-pong* - Lunch room",
  "╭∩╮(O_O)╭∩╮",
  "\"Birds aren't real!\" - Bianca",
  "\"Don't call me Shirley\" - Jacques",
  "\"Oh, so it's kinda like a trophy? Nice!\" - Cyrus",
  "\"You know what the office needs, Craig?\n    A kebab machine\" - Dylan",
  "\"fieldpoop.io is still available!\" - Dylan",
  "\"But why???\" - Cordell",
  "\"Please sir, may I have some context?\" - Cordell",
  "\"Please sir, may I have a review?\" - Bianca",
  "\"One does not simply update tunnels\" - Everyone",
  "\"Ugh, Tuesdays. Am I right?\" - Bianca",
  "\"Not my problem anymore!\" - Cordell",
  "\"Ai, maar ons kan kak praat\" - Cordell",
  "\"It's not that simple\" - Cordell",
  "\"1. fieldpop, 2. ???, 3. profit!\" - Cordell"
};

// LED SETTINGS
#define BRIGHTNESS_MAX 31
#define BRIGHTNESS_MIN 6




unsigned long secTimer=0;



void setAppStatus(uint8_t status) {
  switch(status){
    case E_BOOT :
      WalkingRainbow(30, 10, 100); // will this delay the boot time, but looks cool
      WalkingRainbow(30, 10, 100);
      break;
    case E_WIFI_NOT_CONFIGURED:
      LEDStick.setLEDColor(BS_APPORVED_NICE_RED);
      LED_MODE = SOLID_BRU;
      break;
    case E_WIFI_NOT_CONNECTED:
      LEDStick.setLEDColor(BS_APPORVED_NICE_BLUE);
      LED_MODE = PULSING_BRU;
      break;
    case E_WIFI_CONNECTED :
    
       LED_MODE = SOLID_BRU;
      break;
    case E_FIELPOP_ONLINE:
      colorGradient(0, 255, 0, 0, 0, 255, 10);
      LED_MODE = SOLID_BRU;
      break;
    case E_FIELDPOP_OFFLINE:
      LEDStick.setLEDColor(BS_APPORVED_NICE_RED);
      LED_MODE = PULSING_BRU;
      break;


  }
  APP_STATE = static_cast<e_status>(status);


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

void storeLedBrightness(uint8_t level){
  EEPROM.write(eepromSetting::ui_brightnessAdd, level);
  EEPROM.commit();
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

    // Print startup banner
    Serial.println("===================================================");
    Serial.println();
    Serial.println("                  FieldServer ft.");
    Serial.println();
    Serial.println("      ____                             _ __      ");
    Serial.println("     /  _/___  _________  ____ _____  (_) /_____ ");
    Serial.println("     / // __ \\/ ___/ __ \\/ __ `/ __ \\/ / __/ __ \\");
    Serial.println("   _/ // / / / /__/ /_/ / /_/ / / / / / /_/ /_/ /");
    Serial.println("  /___/_/ /_/\\___/\\____/\\__, /_/ /_/_/\\__/\\____/ ");
    Serial.println("                       /____/                    ");
    Serial.println();
    Serial.println("===================================================");
    Serial.println();

    // Print quote-of-the-day
    uint8_t randomQuoteIndex = random(sizeof(quotes));
    Serial.println(quotes[randomQuoteIndex]);
    Serial.println();

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
    } else {
      Serial.println("E_FIELDPOP_OFFLINE");
      setAppStatus(E_FIELDPOP_OFFLINE);
    }

  } else {
   
    if( wm.getWiFiIsSaved()){
      setAppStatus(E_WIFI_NOT_CONNECTED);
      WiFi.reconnect();
      Serial.println("Not connected to Wi-Fi!");
    }

  }
  statusPrevious = status;
}

// std::map<const char *,std::function<void(int)>> serrialCommands;

void serviceSerial(uint8_t new_brightness){
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
}






void updateLED(){
  static uint8_t previous_mode = -1;

  switch(LED_MODE){
    case SOLID_BRU :
      break;
    case PULSING_BRU:
      effects::pulse.run();
      break;
  }



}

void loop() {
  wm.process(); // update the manager
  uint8_t new_brightness = g_brightness;
  auto now = millis();
  if(now - secTimer > 10000){
    Serial.println("I;m alive");
    secTimer = now;
    checkWifiStatus();
  }
  serviceSerial(new_brightness);


  if(new_brightness != g_brightness){
      g_brightness = new_brightness;
      LEDStick.setLEDBrightness(g_brightness);
      storeLedBrightness(g_brightness);
      Serial.println(g_brightness);
  }
  updateLED();

}
