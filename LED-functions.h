#ifndef _LED_FUNCTIONS

#define _LED_FUNCTIONS

#include "Qwiic_LED_Stick.h"

extern LED LEDStick; 

extern uint8_t g_brightness ;

#define BS_APPORVED_NICE_RED 255, 0, 10
#define BS_APPORVED_NICE_PURPLE 125, 0, 180
#define BS_APPORVED_NICE_BLUE 0, 100, 255
#define BS_APPORVED_SMC_BLUE 76, 76, 255
#define BS_APPORVED_MSA_GREEN 0, 125, 20


// lifted from example
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


// lifted from example
//Display a linear gradient from color 1 (r1,g1,b1) to color 2 (r2,g2,b2) on LED strip of length LEDLength
void colorGradient(byte r1, byte g1, byte b1, byte r2, byte g2, byte b2, byte LEDLength) {
  //These will store the slope for a linear equation between
  //r/g/b1 and r/g/b2
  float rSlope, gSlope, bSlope;
  //Subtract 1 from LEDLength because there is one less
  //transition color than length of LEDs
  LEDLength--;
  //Calculate the slope of the line between r/g/b1 and r/g/b2
  rSlope = (r2 - r1) / (float)LEDLength;
  gSlope = (g2 - g1) / (float)LEDLength;
  bSlope = (b2 - b1) / (float)LEDLength;
  //Will repeat for each pixel in your LED Stick
  for (byte i = 0; i <= LEDLength; i++) {
    //Evaluate the ith point on the line between r/g/b1 and r/g/b2
    byte rValue = r1 + rSlope * i;
    byte gValue = g1 + gSlope * i;
    byte bValue = b1 + bSlope * i;
    //Set the (i+1)th pixel to the calculated color
    //the first LED corresponds to the 0th point on the line
    LEDStick.setLEDColor(i, rValue, gValue, bValue);
  }
}


template <std::size_t N>
struct stepWithDelay {
    int currentStep;
    unsigned long stepChanged;
    std::array<std::tuple<unsigned long, std::function<void()>>, N> steps;
    void run(){
      unsigned long timeWantedToBeInStep = std::get<0>(steps[currentStep]);
      unsigned long timeInStep = millis() - stepChanged;
      if(timeWantedToBeInStep < timeInStep){
        // call exit step action
        std::get<1>(steps[currentStep])();
        currentStep = currentStep >= N - 1 ? 0 : ++currentStep;
        stepChanged = millis();
      }

    }
};


namespace effects {
  
#define PULSE_DELTA 5

stepWithDelay<8> pulse = {
  0,
  0,
  {
    std::make_tuple(2000, []() { LEDStick.setLEDBrightness(g_brightness - PULSE_DELTA); }),
    std::make_tuple(100, []() { LEDStick.setLEDBrightness(g_brightness); }),
    std::make_tuple(100, []() { LEDStick.setLEDBrightness(g_brightness - PULSE_DELTA); }),
    std::make_tuple(100, []() { LEDStick.setLEDBrightness(g_brightness); }),
    std::make_tuple(100, []() { LEDStick.setLEDBrightness(g_brightness - PULSE_DELTA); }),
    std::make_tuple(100, []() { LEDStick.setLEDBrightness(g_brightness); }),
    std::make_tuple(100, []() { LEDStick.setLEDBrightness(g_brightness - PULSE_DELTA); }),
    std::make_tuple(100, []() { LEDStick.setLEDBrightness(g_brightness) ; }),
  }
};
}


#endif