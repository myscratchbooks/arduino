/*
 * Arduino Spectrum Analizer
 * https://www.youtube.com/watch?v=5RmQJtE61zE
 * learnelectronics
 * 27 April 2017
 * black magic stolen from CBM80Amiga
 * 
 * code modified from: https://www.dropbox.com/sh/m6c40pu99fqxb5b/AABAQTv67pbYOl6gNozWwDNLa/spectrum?dl=0
 * 
 * Fix_FFT library available @ https://github.com/kosme/fix_fft
 * A library for implementing fixed-point in-place Fast Fourier Transform on Arduino. 
 * It sacrifices precision and instead it is way faster than floating-point implementations.
 * 
 * Facts:
 * Human hearing is 20 Hz to 20 KHz.
 * http://www.seaindia.in/blog/human-voice-frequency-range/
 * Fundamental frequency for a typical adult male covers  100Hz to 900Hz, 
 * and typical adult female is 350Hz to 3KHz 
 * 
 * 
 */


#include "fix_fft.h"                                  // library to perfom the fixed-point in-place Fast Fourier Transform
//#include <SPI.h>                                    // SPI library
#include <Wire.h>                                     // I2C library for OLED
#include <Adafruit_GFX.h>                             // graphics library for OLED
#include <Adafruit_SSD1306.h>                         // OLED driver

#define OLED_RESET 4                                  // OLED reset, not hooked up but required by library
Adafruit_SSD1306 display(OLED_RESET);                 // declare instance of OLED library called display

// https://arduino.stackexchange.com/questions/699/how-do-i-know-the-sampling-frequency
// http://www.dellascolto.com/bitwise/2017/05/25/audio-spectrum-analyzer/
// For a 16 MHz Arduino the ADC clock , default (prescaler=128) fs=9615 Hz=> fmax =fs/2
// if FFT_N=128, (9615 /2)/(128/2) = 75 hz per bin
// if FFT_N=256, (9615/2)/(266/2) = 37.6 hz per bin
const int FFT_N =256;                                 // The Arduino Uno does not have enough memory to support 256 samples.
                                                      // many samples you will get a larger resolution for the results.
                                                      // set FFT_N to 256 by using an Arduino Mega 2560.
char im[FFT_N], data[FFT_N];                          // variables for the FFT
char x = 0, ylim = 60;                                // variables for drawing the graphics
int i = 0, val,fall;                                       //counters
int dropMax = 0;
int curMax;

void setup()
{
  //Serial.begin(9600);                               // serial comms for debuging
  display.begin(SSD1306_SWITCHCAPVCC,0x3C);           // begin OLED @ hex addy 0x3C
  display.setTextSize(1);                             // set OLED text size to 1 (1-6)
  display.setTextColor(WHITE);                        // set text color to white
  display.clearDisplay();                             // clear display
  analogReference(DEFAULT);                           // Use default (5v) aref voltage.
};

void loop()
{
  int mini=1024, maxi=0;                               // set minumum & maximum ADC values
  for (i = 0; i < FFT_N; i++) {                       // take 128 or 256  point fft samples
    val = analogRead(A0);                             // get audio from Analog 0
    data[i] = val / 4 - 128;                          // the reason it is divided by 4 and subtracted by 128 is because the "data" is of type "char" which is an 8 bit variable (-128 to 127). 
                                                      // So because analogRead returns 0-1023, dividing it by 4 gives you 0-255, subtracting it by 128 gives you -128-127.
                                                      // scale from 10 bit (0-1023) to 8 bit (0-255)
    im[i] = 0;                                        //
    if(val>maxi) maxi=val;                              //capture maximum level
    if(val<mini) mini=val; 
    //if(fall<mini) mini=fall;                              //capture minimum level
  };

 
  /*
   Performs foward or inverse Fourier transform.
   // fix_fft (char fr[], char fi[], int m, int inverse)
   // fr is a real data set,
   // fi is the imaginary data set, im at 0 here
   // the 3rd parameter m, then there are 2 ^ m 'bins'; if m = 7 then so 128; only the first half (64) contain the usable values
   // m is log2(n) where n (FFT_N) is number of data points (log2 (128) = 7), (log2 (256) = 8) 
   // 0 is set for forward transform. 1 would be inverse transform. Apparently inverse does not work,
*/
    
  fix_fft(data, im, 8, 0);                            // perform the FFT on data for log2 (256) = 8 
  //fix_fft(data, im, 7, 0);                            // perform the FFT on data for log2 (128) = 7

// /*  display graphic, un-comment here
  
  display.clearDisplay();                                             // clear display
      // result array is a bin. 
      // interessted in the absolute value of the transformation
      // The number of bins you get is half the amount of samples spanning the frequency range from zero to half the sampling rate.
      // the first half of array elements contain the real values after fix_fft 
      // In the end we get N/2 bins, each covering a frequency range of sampling_rate/N Hz. 
  for (i = 1; i < FFT_N/2; i++) {                                     // full spectrum;  here 4.8kHz bandwith, has 128 bins each 37.5Hz
                                                                      // 1st bin = 37.5Hz, 2nd bin = 37.5*2 Hz  etc.
                                                                      // change for loop values for specific range of interested frequency
                                                                      // ex:  for (i = 1; i < 64; i++)    // intersted in first 63 bins
    int dat = sqrt(data[i] * data[i] + im[i] * im[i]);                // filter out noise and hum    
    dat = (2-dat/60)*dat;                                             // multiplication factor makes it look better with the microphone board (runs from 2 to 1)  
    display.drawLine(i*2 + x, ylim, i*2 + x, ylim - dat, WHITE);  // if using FFT_N=128; draw bar graphics for freqs 
    //display.drawLine(i + x, ylim, i + x, ylim - dat, WHITE);          // if using FFT_N=256; draw bar graphics for freqs 
  }                                                
  display.setCursor(0,0);                                           // set cursor to top of screen
  display.print("->Spectrum Analyzer<-");                           // print title to buffer
  display.display();                                               // show the buffer

//  */  display graphic, un-comment here

 
/*   // Another display graphic, un-comment here
// * 
// * 
// * 
//
//
// code from http://www.dellascolto.com/bitwise/category/arduino/page/2/

  display.clearDisplay();                                           //clear display
      // the first 32 array elements contain the real values after fix_fft
  for (i = 1; i < FFT_N/2; i++) {                          
    int dat = sqrt(data[i] * data[i] + im[i] * im[i]);              //filter out noise and hum
    dat = (2-dat/60)*dat;                                          // multiplication factor makes it look better with the microphone board (runs from 2 to 1)
    display.drawLine(0,63,128,63,WHITE);                              // baseline
        // the +5 ensures that the background noise becomes invisible over the full width
    // display.fillRect(i*4 + x, ylim-dat+5, 2, 63-ylim+dat, WHITE);     //  if FFT_N=64, draw bar graphics for freqs 
    display.fillRect(i + x, ylim-dat+5, 2, 63-ylim+dat, WHITE);          //  if FFT_N=256, draw bar graphics for freqs 
  };                                                
  display.setCursor(0,0);                             //set cursor to top of screen
  curMax=(dropMax+2*min((maxi-mini)/4-12,128))/3;     // weighted average between new and old value; partial factor 4 because max-min is in range 0-512; -12 is compensation
  display.fillRect(0,0,curMax,3,WHITE);
  if (dropMax>curMax+4) {
    dropMax=dropMax-3;
    display.fillRect(dropMax-4,0,3,3,WHITE);
  } else {
    dropMax=curMax;
  }
  display.display();         //show the buffer

*/   // Another display graphic, un-comment here


};  // end of loop()



