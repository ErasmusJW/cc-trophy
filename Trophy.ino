#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager
#include <WiFi.h>
#include <Wire.h>
#include "Qwiic_LED_Stick.h" // Click here to get the library: http://librarymanager/All#SparkFun_Qwiic_LED_Stick

#include <WiFiClientSecure.h> //fuck https, fuck fuck fuck

#include <HTTPClient.h>

// Define the target server and path
const char* server = "https://smc.fieldpop.io";
const uint16_t httpsPort = 443; // HTTPS port
const char* path = "/get_time"; // Root path


LED LEDStick; //Create an object of the LED class

#include <EEPROM.h> // Include the EEPROM library

/// EEPROM settings
#define EEPROM_SIZE 1
#define BRIGHTNESS_ADD 0


// LED SETTINGS
#define BRIGHTNESS_MAX 31

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

#define AP_PASSWORD "werfetter"

WiFiManager wm;

uint8_t brightness = 0;

uint8_t getStoredLedBrightness(){
  uint8_t storedValue = EEPROM.read(BRIGHTNESS_ADD);
  return storedValue > BRIGHTNESS_MAX ? 1 : storedValue;
}

void storeLedBrightness(uint8_t level){
  EEPROM.write(BRIGHTNESS_ADD, level);
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


void setup() {
    Wire.begin();
    EEPROM.begin(EEPROM_SIZE);
    Serial.begin(115200);
    WiFi.mode(WIFI_STA);

    if (LEDStick.begin() == false){
  
      Serial.println("Qwiic LED Stick failed to begin. Please check wiring and try again!");
      while(1);
    }

    //Start by resetting the state of the LEDs
    LEDStick.LEDOff();
    brightness = getStoredLedBrightness();

    LEDStick.setLEDBrightness(brightness) ; //value between 0-31
    setAppStatus(E_BOOT);


    wm.setConfigPortalBlocking(false);

    // WiFi.mode(WIFI_STA); // explicitly set mode, esp defaults to STA+AP
    // it is a good practice to make sure your code sets wifi mode how you want it.

    // put your setup code here, to run once:

    Serial.println(xPortGetCoreID());
    
    //WiFiManager, Local intialization. Once its business is done, there is no need to keep it around


    // reset settings - wipe stored credentials for testing
    // these are stored by the esp library
    // wm.resetSettings();




    //automatically connect using saved credentials if they exist
    //If connection fails it starts an access point with the specified name
    if(wm.autoConnect("CC TROPHY",AP_PASSWORD)){
        setAppStatus(E_WIFI_CONNECTED);
        Serial.println("connected...yeey :)");

    }
    else {
        Serial.println("Configportal running");
        setAppStatus(E_WIFI_NOT_CONFIGURED);
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
  
      if (https.begin(*client, "https://smc.fieldpop.io/get_time")) {  // HTTPS
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


void loop() {
  wm.process();
  uint8_t new_brightness = brightness;
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
      new_brightness = new_brightness == 0 ? 0 : --new_brightness;
    }  else if (command == 'R' || command == 'r') {
      Serial.println("Reset start");
      WiFiManager wm;
      Serial.println("Switching to AP mode on next reboot.");
      wm.resetSettings();
      Serial.println("Reset done");
      delay(1000); // Delay to allow the message to be printed
      ESP.restart(); // Reboot  to enter AP mode
    } else if (command == 'c' || command == 'c') {

        if(wm.autoConnect("CC TROPHY",AP_PASSWORD)){
            setAppStatus(E_WIFI_CONNECTED);
            Serial.println("connected...yeey :)");

        }
        else {
            Serial.println("Configportal running");
            setAppStatus(E_WIFI_NOT_CONFIGURED);
        }

    }
     
  }
  if(new_brightness != brightness){
      brightness = new_brightness;
      LEDStick.setLEDBrightness(brightness);
      storeLedBrightness(brightness);
      Serial.println(brightness);
  }

}
