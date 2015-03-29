/*
This sketch listens to incoming midi clock messages on port RX and pulses output tempo_pin each quarter note,
i.e. every 24th clock tick

Written for Attiny85 but can be modified for other Atmega, Attiny MCU's

IMPORTANT! Don't use pins RX, TX for anything else other than the SoftwareSerial library
more details, schematics, etc.. here: http://ambinaut.wordpress.com/2014/07/14/digitech-jamman-stereo-midi-sync/
*/
#define F_CPU 8000000UL //Define the speed the clock is running at.  Used for the delay.h functions

#include <avr/io.h>     //Include the headers that handles Input/Output function for AVR chips
#include <util/delay.h> //Include the headers that allow for a delay function
#include <SoftwareSerial.h>  //Arduino Software Serial library
#include <avr/sleep.h> // Lib for sleep modes
#include <avr/interrupt.h> //Lib for interrupts


#define tempo_pin 1 //Physical pin 6, or PB1 on the Attiny85

byte midi_msg = 0; //stores incoming midi messages, only 1 byte needed, real time msgs are one byte long
byte clockpulse = 0; //stores current midi clock tick number
unsigned long last_midi_event_time = 0; //time of the last incoming midi event

SoftwareSerial mySerial(3, 4); // RX, TX Physical pins 2, 3

void init_io(void)
{
	DDRB |= _BV(tempo_pin); //set tempo_pin and test_led as outputs
        PORTB &= ~(_BV(tempo_pin)); //set the pins to LOW		
}

int main(void)
{
	init_io();	
	init(); 
	mySerial.begin(31250);
	
	while (1) {  //endless loop

		if (mySerial.available()) {
			midi_msg = mySerial.read();
			last_midi_event_time = millis();
			midi_clk_to_pulse(); //see below			
		}		
		//if there's no more incoming midi for longer than 1000ms, set tempo_pin LOW
		else   {
			unsigned long current_time = millis() - last_midi_event_time;
			if (current_time > 1000){		
				midi_msg = 0xFC; //midi stop message
				last_midi_event_time = 0;
				midi_clk_to_pulse();
                sleep();						
			}
		}	
	} //END while (1) - endless (main) loop
}

void midi_clk_to_pulse(){
			
	switch(midi_msg) {

		case 0xFC: //midi stop message
			clockpulse = 0;
			PORTB &= ~(_BV(tempo_pin)); //turns tempo_pin off
		break;
				
		case 0xFA: //midi start message
			clockpulse = 0;
			PORTB &= ~(_BV(tempo_pin)); //turns tempo_pin off
		break;
				
		case 0xF8: //midi clock pulse
			clockpulse = clockpulse + 1;
			switch (clockpulse){
				case 1:
				PORTB |= (_BV(tempo_pin)); //tempo_pin HIGH
				break;
					
				case 8:
				PORTB &= ~(_BV(tempo_pin)); //tempo_pin LOW
				break;
					
				case 24: // second quarter begin
				PORTB |= (_BV(tempo_pin)); //tempo_pin HIGH
				break;
					
				case 32:
				PORTB &= ~(_BV(tempo_pin)); //tempo_pin LOW
				break;
					
				case 48: // third quarter begin, middle of the measure
				PORTB |= (_BV(tempo_pin)); //tempo_pin HIGH
				break;
					
				case 56:
				PORTB &= ~(_BV(tempo_pin)); //tempo_pin LOW
				break;
					
				case 72: //fourth quarter begin
				PORTB |= (_BV(tempo_pin)); //tempo_pin HIGH
				break;
					
				case 80:
				PORTB &= ~(_BV(tempo_pin)); //tempo_pin LOW
				break;
					
				case 96:
				clockpulse = 0; // resets the clock pulse counter, 96 pulses = 1 measure in 4/4
			break; //END case 0xF8:
		} // END switch (clockpulse)
		break;				
	} // END switch(midi_msg)
} //END midi_clk_to_pulse()

void sleep() {

    GIMSK |= _BV(PCIE);                     // Enable Pin Change Interrupts
    PCMSK |= _BV(PCINT2);                   // Use PB2 as interrupt pin
    ADCSRA &= ~_BV(ADEN);                   // ADC off
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);    // replaces above statement

    sleep_enable();                         // Sets the Sleep Enable bit in the MCUCR Register (SE BIT)
    sei();                                  // Enable interrupts
    sleep_cpu();                            // sleep

    cli();                                  // Disable interrupts
    PCMSK &= ~_BV(PCINT2);                  // Turn off PB2 as interrupt pin
    sleep_disable();                        // Clear SE bit
    ADCSRA |= _BV(ADEN);                    // ADC on

    sei();                                  // Enable interrupts
    } // sleep
