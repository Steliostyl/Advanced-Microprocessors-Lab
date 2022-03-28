#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdlib.h>

#define pin0 1		// Pin 0 = Big Road
#define pin1 2		// Pin 1 =	Pedestrians
#define pin2 4		// Pin 2 = Small Road
#define sensTimer 1	// Time to wait before rechecking the sensor's data
#define pedTimer 4	// Pedestrian crossing timer

int btf;		// Is 1 when big traffic light is on
int sensorData;	/* Data from the small road's traffic sensor. 
				When its value is 0, 5 or 8, it means there's 
				traffic on the small road. */
int counterOn = 1;

int main(void) {
	PORTD.DIR |=  7;	// Pin 0,1,2 = Output
	PORTD.OUT |= 6;		// Initialize traffic lights (Only the Big Road's traffic light is on)
	// Pull-up enable and Interrupt enabled with sense on both edges
	PORTF.PIN5CTRL |= PORT_PULLUPEN_bm | PORT_ISC_BOTHEDGES_gc;
	sei();	// Enable interrupts
	while (1){
		sensorData = rand()%10;	// Change the sensor's data
		// If there's traffic on the small road
		if ( (sensorData == 0) || (sensorData == 5) || (sensorData == 8) ){
			PORTD.OUTSET = pin0; // Turn off the Big Road's traffic light
			PORTD.OUTCLR = pin2; // Turn on the Small Road's traffic light
			do {
				_delay_ms(sensTimer);	// Wait before reading the sensor's data
				sensorData = rand()%10;	// Change the sensor's data
			} while((sensorData == 0) || (sensorData == 5) || (sensorData == 8));
			// ^ While there's traffic on the Small Road ^
			PORTD.OUTSET = pin2;	// Turn off the Small Road's traffic light
			PORTD.OUTCLR = pin0;	// Turn on the Big Road's traffic light
			_delay_ms(sensTimer);	// Wait before reading the sensor's data
		}
	}
	cli();	// Disable interrupts (useless because we're using while(1))
}


ISR(PORTF_PORT_vect){
	int intflags = PORTF.INTFLAGS;
	PORTF.INTFLAGS = intflags;
	
	btf = PORTD.OUT % 2;	// Save current status of the Big Road's traffic lights

	if (!btf){	// Check if the Big Road's traffic light is on
		PORTD.OUT = pin0;		// Turn off the Big Road's traffic light
		PORTD.OUTCLR = pin2;	// Turn on the Small Road's traffic light
	}

	counterOn = 1;
	TCA0.SINGLE.CNT = 0;	// clear counter
	TCA0.SINGLE.CTRLB = 0;	// normal mode (counter)
	TCA0.SINGLE.CMP0 = 10;	// When counter reaches this value -> interrupt clock frequency/1024
	TCA0.SINGLE.CTRLA = TCA_SINGLE_CLKSEL_DIV1024_gc;	// frequency of counter
	TCA0.SINGLE.CTRLA |= 1;	// Enable
	TCA0.SINGLE.INTCTRL = TCA_SINGLE_CMP0_bm; // Interrupt Enable for counter
	sei();
	
	PORTD.OUTCLR = pin1;		// Turn on the Pedestrian traffic light
	
	while (counterOn){
	}
	
	PORTD.OUTSET = pin1;		// Turn off the Pedestrian traffic light
	if (!btf){	// If the Big Road's traffic light was on before the interrupt
		PORTD.OUTSET = pin2; // Turn off the Small Road's traffic light
		PORTD.OUTCLR = pin0; // Turn on the Big Road's traffic light
	}
}

ISR(TCA0_CMP0_vect){
	TCA0.SINGLE.CTRLA = 0; // Disable counter
	int intflags = TCA0.SINGLE.INTFLAGS;
	TCA0.SINGLE.INTFLAGS = intflags;
	counterOn = 0;
}