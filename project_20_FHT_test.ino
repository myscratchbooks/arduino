/*
fht_adc.pde
guest openmusiclabs.com 9.5.12
example sketch for testing the fht library.
it takes in data on ADC0 (Analog0) and processes them
with the fht. the data is sent out over the serial
port at 115.2kb.  there is a pure data patch for
visualizing the data.

// http://wiki.openmusiclabs.com/wiki/ArduinoFHT

//
*/

// https://cassiopeia.hk/spectrumanalyser/
//#define OCT_NORM 0        // 0: no normalisation, more high freq 1: divided by number of bins, less high freq
//#define OCTAVE 1
// https://github.com/ayavilevich/ArduinoSoundLevelMeter/blob/master/ArduinoSoundLevelMeter.ino
//#define LIN_OUT8 1 // use the linear byte output function
#define LOG_OUT 1             // use the log output function
#include <FHT.h>                    // include the library
#define FHT_N 256                                     // set to 256 point fht
#include <Wire.h>                                     // I2C library for OLED
#include <Adafruit_GFX.h>                             // graphics library for OLED
#include <Adafruit_SSD1306.h>                         // OLED driver
#define OLED_RESET 4                                  // OLED reset, not hooked up but required by library
Adafruit_SSD1306 display(OLED_RESET);                 // declare instance of OLED library called display
char x = 0, ylim = 60;                                // variables for drawing the graphics
const int noise_level = 60;                            // set 0 will have background noise spectrum displayed


void setup() {
  //Serial.begin(115200); // use the serial port

  display.begin(SSD1306_SWITCHCAPVCC,0x3C);           // begin OLED @ hex addy 0x3C
  display.setTextSize(1);                             // set OLED text size to 1 (1-6)
  display.setTextColor(WHITE);                        // set text color to white
  display.clearDisplay();                             // clear display
  analogReference(DEFAULT);                           // Use default (5v) aref voltage.
  setFreeRunMode();
}

void loop() {
  while(1) {                                // reduces jitter
    cli();                                  //  no for TIMING
       // unsigned long start_time = micros(); // yes for TIMING
    for (int i = 0 ; i < FHT_N ; i++) {     // save 128 samples
      while(!(ADCSRA & 0x10));              // wait for adc to be ready (ADIF)
      ADCSRA = 0xf5;                        // restart adc
      byte m = ADCL;                        // fetch adc data, the ADCL register first because reading the ADCH causes to ADC to update
      byte j = ADCH;
      int k = (j << 8) | m;                 // form into an int
      k -= 0x0200;                          // form into a signed int
      k <<= 6;                              // form into a 16b signed int
      fht_input[i] = k;                     // put real data into bins
    }


      // Serial.println(micros()-start_time); // yes for TIMING
    fht_window();                             // window the data for better frequency response
    fht_reorder();                            // reorder the data before doing the fht
    fht_run();                                // process the data in the fht
    // fht_mag_octave();                      // take the output of the fht
    fht_mag_log();                            // take the output of the fht
   // fht_mag_lin8();                         // take the output of the fht
    sei();                                    // no for TIMING
   // Serial.write(255);                      // send a start byte
   // Serial.write(fht_log_out, FHT_N/2);     // send out the data

   // OLED display
       display.clearDisplay();                                             // clear display
       for (int i=2; i< FHT_N; i+=1) {                            // output even bins because oled is 128x64 when FHT_N=256
                                                                  // i=2, because i've noise on first 2 bins
                                                                  // If FHT_N=128, then use for (int i=2; i<FHT_N; i++)
        int dat = max((fht_log_out[i] - noise_level) / 4, 0);     // scale the height
        display.drawLine(i + x, ylim, i + x, ylim-dat, WHITE);          // draw bar graphics for freqs 
        
      }                                                
      display.setCursor(0,0);                                           // set cursor to top of screen
      display.print("->Spectrum Analyzer<-");                           // print title to buffer
      display.display();         

  }  // end while   
} // end loop


void setFreeRunMode() {
  TIMSK0 = 0;                               // turn off timer0 for lower jitter
  ADCSRA = 0xE5;                            // "ADC Enable", "ADC Start Conversion", "ADC Auto Trigger Enable", 32 prescaler for 38.5 KHz
                                            // 0xE4, 16 prescaler
                                            // 0xE6, 64 prescaler
  ADMUX = 0x40;                             // b0100 0000; use adc0, right-align, use the full 10 bits resolution 
  DIDR0 = 0x01;                             // turn off the digital input for adc0  
}


