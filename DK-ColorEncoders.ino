#define DEBUG

#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(16, 10, NEO_GRB + NEO_KHZ800);


// Use PJRC multi-encoder library downloaded to /libraries folder
#include <Encoder.h>
// Change these pin numbers to the pins connected to your encoder.
//   Best Performance: both pins have interrupt capability
//   Good Performance: only the first pin has interrupt capability
//   Low Performance:  neither pin has interrupt capability
Encoder encoderA(2, 4);
Encoder encoderB(3, 5);
#define buttonA 8
#define buttonB 15



void setup() {
  pinMode(buttonA, INPUT_PULLUP);
  pinMode(buttonB, INPUT_PULLUP);
  
  #ifdef DEBUG
  Serial.begin(9600);
  #endif

  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
}


long oldPositionA  = -999;
long oldPositionB  = -999;
uint8_t i, kA, kB;
uint8_t algoA = 2, algoB = 2;
uint8_t paramA, paramB;
long prevTimeA, prevTimeB = 0;

// todo: idea blink onboard led for every encoder increment (for debug/feedback purposes)


void loop() {
  // READ ENCODER A
  long newPosition = encoderA.read();

  // Detect clockwise/counterclockwise step from quadrature encoder
  if (newPosition >= oldPositionA + 4 || newPosition <= oldPositionA - 4) {    
    if(newPosition >= oldPositionA + 4) {
      paramA+=2; // double increment/decrement rate
    }
    if(newPosition <= oldPositionA - 4) {
      paramA-=2;
    }

    oldPositionA = newPosition;
    #ifdef DEBUG
    Serial.print("positionA ");
    Serial.println(newPosition);
    Serial.print("paramA ");
    Serial.println(paramA);
    #endif

    delay(10); // too long of a delay interferes with encoder
  }

  // READ ENCODER B
  newPosition = encoderB.read();

  // Detect clockwise/counterclockwise step from quadrature encoder
  if (newPosition >= oldPositionB + 4 || newPosition <= oldPositionB - 4) {    
    if(newPosition >= oldPositionB + 4) {
      paramB+=2; // double increment/decrement rate
    }
    if(newPosition <= oldPositionB - 4) {
      paramB-=2;
    }

    oldPositionB = newPosition;
    #ifdef DEBUG
    Serial.print("positionB ");
    Serial.println(newPosition);
    Serial.print("paramB ");
    Serial.println(paramB);
    #endif

    delay(10); // too long of a delay interferes with encoder
  }

  // Change algorithm when encoder button is pressed
  if(digitalRead(buttonA) == LOW) {    
    
    algoA++;
    if(algoA > 3) algoA = 0;
    
    #ifdef DEBUG
    Serial.println("Button A pressed");
    Serial.print("algoA ");
    Serial.println(algoA);
    #endif
    
    delay(200);
  }

  if(digitalRead(buttonB) == LOW) {    
    
    algoB++;
    if(algoB > 3) algoB = 0;

    #ifdef DEBUG
    Serial.println("Button B pressed");
    Serial.print("algoB ");
    Serial.println(algoB);
    #endif
    
    delay(200);
  }


  // LIGHT ALGORITHMS
  unsigned long currentTime = millis();

  // Algorithm A: Even LEDs
  switch(algoA) {
    
    case 0: // Half color wheel with 2 zoomy dots, param controlling speed
      if(currentTime - prevTimeA > paramA) {
        prevTimeA = currentTime;
        kA++; // update keyframe
        if(kA > 128) kA = 0;
        for(i=0; i<strip.numPixels(); i+=2) {
            strip.setPixelColor(i, Wheel((i+kA) & 255));
        }
      }
      strip.setPixelColor(((16-kA) + 1) % 16, strip.Color(0, 255, 255));
      strip.setPixelColor(kA % 16, strip.Color(255, 0, 0));
      strip.show();
    break;
    
    case 1: // Partial color wheel, different somehow
      if(currentTime - prevTimeA > paramA) {
        prevTimeA = currentTime;
        kA++; // update keyframe
        if(kA > 255) kA = 128;
        for(i=0; i<strip.numPixels(); i+=2) {
            strip.setPixelColor(i, Wheel((i+kA) & kA));
        }
        strip.show(); 
      } 
    break;

    case 2: // Full color wheel, smoother with param controlling color
      if(currentTime - prevTimeA > 50) {
        prevTimeA = currentTime;
        kA++; // update keyframe
        if(kA > 255) kA = 0;
        for(i=0; i<strip.numPixels(); i+=2) {
            strip.setPixelColor(i, Wheel((i+kA) & paramA));
        }
      }
      strip.show();
    break;

    case 3: // 2 zoomy dots, with param controlling 3 speed steps
      if(currentTime - prevTimeA > 100 * (paramA%3)) {
        prevTimeA = currentTime;
        kA++; // update keyframe
      }
      strip.setPixelColor(((16-kA) + 1) % 16, Wheel((i+kA) & 255));
      strip.setPixelColor(kA % 16, Wheel((i+kA) & 255));
      strip.show();
    break;
  }
  

  // Algorithm B: Odd LEDs
  switch(algoB) {
    
    case 0: // Other half of color wheel, param controlling speed 
      if(currentTime - prevTimeB > paramB * 3) {
        prevTimeB = currentTime;
    
        // Update keyframe
        kB++;
        if(kB > 255) kB = 128;
        for(i=1; i<strip.numPixels(); i+=2) {
            strip.setPixelColor(i, Wheel((i+kB) & 255));
        }
        strip.show(); 
      }
    break;

    case 1: // Rapid blinking
      if(currentTime - prevTimeB > paramB) {
          prevTimeB = currentTime;
          kB++; // update keyframe
          if(kB > 64) kB = 0;
          for(i=1; i<strip.numPixels(); i+=2) {
              strip.setPixelColor(i, Wheel((i+kB) & 128));
          }
          strip.show(); 
      } else {
          for(i=1; i<strip.numPixels(); i+=2) {
              strip.setPixelColor(i, strip.Color(0,0,0));
          }  
          strip.show(); 
      }
    break;

    case 2: // Full color wheel, smoother with param controlling color
      if(currentTime - prevTimeB > 70) {
        prevTimeB = currentTime;
    
        // Update keyframe
        kB++;
        if(kB > 255) kB = 0;
        for(i=1; i<strip.numPixels(); i+=2) {
            strip.setPixelColor(i, Wheel((i+kB) & paramB));
        }
        strip.show(); 
      }
    break;

    case 3: // Random black out x2 + 1 white twinkle, param controlling speed
      if(currentTime - prevTimeB > paramB*2) {
        prevTimeB = currentTime;
    
        strip.setPixelColor(random(strip.numPixels()), strip.Color(0,0,0));
        strip.setPixelColor(random(strip.numPixels()), strip.Color(0,0,0));
        strip.setPixelColor(random(strip.numPixels()), strip.Color(255,255,255));
        strip.show(); 
      }
    break;    
  }
  
}





// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return strip.Color(128 - WheelPos , 0, WheelPos );
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(WheelPos , 128 - WheelPos , 0);
  }
  WheelPos -= 170;
  return strip.Color(0, WheelPos , 128 - WheelPos );
}


void clearStrip() {
    for(uint16_t i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, strip.Color(0, 0, 0));
    }
    strip.show();
}
