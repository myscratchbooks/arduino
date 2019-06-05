/*
 *** references
http://forum.arduino.cc/index.php?topic=432991.msg2988230#msg2988230
http://audiojudgement.com/how-to-calculate-decibels/
https://www.luisllamas.es/medir-sonido-con-arduino-y-microfono-amplificado-max9812/

--- connections ------
MAX-9812  Arduino
VCC       5V
GND       GND
OUT       A0

 */

const int sensitivity = -58;     // microphone sensitivity in dB
const int gain = 20;             // op-amp gain dB
const int sensorPIN = A0;
const int sampleWindow = 1000;  //sample for 1000 milliseconds
unsigned int sample;

void setup()
{
   Serial.begin(9600);
}

void loop()
{
   
   unsigned long startMillis= millis();  // Start of sample window
   unsigned int peakToPeak = 0;         // peak-to-peak level
   unsigned int signalMax = 0;
   unsigned int signalMin = 1024;       // 10 bit ADC = 2^10

   // collect data for Sample window width in mS 
   while (millis() - startMillis < sampleWindow)
   {
      sample = analogRead(sensorPIN);

      if (sample < 1024)  // toss out spurious readings
      {
         // see if you have a new maxValue
         if (sample > signalMax)
         {
            signalMax = sample;  // save just the max levels
         }
         else if (sample < signalMin)
         {
            signalMin = sample;  // save just the min levels
         }
      }
   }
   peakToPeak = signalMax - signalMin;               // max - min = peak-peak amplitude
   
   double volts = ((peakToPeak * 5.0) / 1024);    // convert to volts,gain 20db  = 10V
                                                     // Uno/Mega analogRead() voltages between 0 and 5 volts into integer values between 0 and 1023
                                                     // change if have different board

                                                        
      // https://electronics.stackexchange.com/questions/96205/how-to-convert-volts-in-db-spl
      // The microphone sensitivity is -58db, so V RMS / PA is 0.001259
      // conversion using : http://www.sengpielaudio.com/calculator-gainloss.htm
   double volts_db = 20*log10(volts/0.012589); 
                                         
    
     //  94 is a 1 Pa expressed as dB SPL  
     // https://support.biamp.com/General/Audio/Microphone_sensitivity     
     // https://forums.adafruit.com/download/file.php?id=38278&sid=3f8400ec828f5b22ed3be3ef64eb4e86                             
   double spl_db = volts_db + 94 + sensitivity - gain;            

   // Output results
   Serial.print("volts: " + String(volts) );
   Serial.print("\t");                        // prints a tab
   Serial.print("dB: " + String(volts_db) );
   Serial.print("\t");
   Serial.println("dB-spl: " + String(spl_db) );
}
