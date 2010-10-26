//      -=- Angular Deplexer -=-
// A little chip to take multiplexed PPM
//  signals, and split them up over the
//   digital pins, for RC things, and
//   fun little cassette tape robots!
//                             <3 Bluebie

#define F_CPU 9600000UL  // 9.6 MHz
#include <stdlib.h>
#include <avr/io.h>
#include <util/delay.h>

// For the niceness
#define bit(number) _BV(number)
#define becomeInput(pin) DDRB &= ~_BV(pin);
#define becomeOutput(pin) DDRB |= _BV(pin); 
#define pinOn(pin) PORTB |= _BV(pin);
#define pinOff(pin) PORTB &= ~_BV(pin);
#define delay(ms) _delay_ms(ms);
#define digitalRead(pin) ((PINB & _BV(pin)) > 0)
#define digitalWrite(pin, state) if (state) pinOn(pin) else pinOff(pin);
#define comparator ((ACSR & 0b00100000) > 1)

// Note to self, PB0 and PB1 are AIN0 (+) and AIN1 (-) for comparator
// PB2, PB3, PB4, and PB5 are our servo signalers!

// And now for some settings...
#define outputs 3
#define first_output 2
#define max_output first_output + outputs - 1
#define reset_gap_minimum 2500
#define minimum_gap 10

unsigned long pollComparatorUntil(bool value) {
  unsigned long count = 1;
  while (comparator != value) count++;
  return count;
}

int main() {
  // Setup the comparator
  becomeInput(0);
  becomeInput(1);
  pinOff(0);
  pinOff(1);
  
  // Setup our outputs
  becomeOutput(2);
  becomeOutput(3);
  //becomeOutput(4);
  becomeInput(4);
  // Not using this, it's the reset pin!
  //becomeOutput(5);
  
  // Todo: See if the internal bandgap is a good enough voltage for comparator
  
  // Magic analog comparator configuration voodoo:
  // ACD=0, ACBG=0, ACO=0 ACI=0 ACIE=0 ACIC=0 ACIS1, ACIS0
  // - interrupt on output toggle
  ACSR = 0b00000000;
  // ADEN=1
  ADCSRA = 0b10000000;
  // ACME=0 (on) ADEN=0 MUX = b000 for use of AIN1
  ADCSRB = 0b00000000;
  // Save some power by turning off the digital input on AIN0 and AIN1
  DIDR0 = 0b00111100;
  // Voodoo done.
  
  //while(true) { digitalWrite(4, comparator); }
  
  // Do a little dance!
  unsigned char output = first_output;
  unsigned long gap = 0;
  while(true) {
    gap = pollComparatorUntil(false); // wait till the thingy goes low (i.e. pulse done!)
    gap += pollComparatorUntil(true); // wait till it does high, and then...
    
    pinOff(output); // turn off the last servo
    if (gap > minimum_gap) output++; // change to the next one!
    
    if (output > max_output || gap > reset_gap_minimum)
      output = first_output;
    
    pinOn(output); // and turn it on, then wait till the next pulse to change over again!
  }
  
  return 0;
}
