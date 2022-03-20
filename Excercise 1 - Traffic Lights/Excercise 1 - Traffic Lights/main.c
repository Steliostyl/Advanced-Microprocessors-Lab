#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdlib.h>

#define pin0 1	// Pin 0 = Big Road Cars
#define pin1 2	// Pin 1 = Big Road Pedestrians
#define pin2 4	// Pin 2 = Small Road

int interr = 0;

ISR(PORTF_PORT_vect){
	int intflags = PORTF.INTFLAGS;
	PORTF.INTFLAGS=intflags;
	interr=1;
	PORTD.OUTSET = pin0;
	PORTD.OUTCLR = pin1;
}

int main(void)
{
	PORTD.DIR |=  7;	// Pin 0,1,2 = Output
	PORTD.OUT |= 6;		// Initialize traffic lights (Only 0 is on)
	int tmp = rand()%10;
	while (1){
		while ( (tmp == 0) | (tmp == 5) | (tmp == 8) ){
			PORTD.OUTSET = pin0; // Turn off the Big Road's traffic light
			PORTD.OUTCLR = pin2; // Turn on the Small Road's traffic light
			_delay_ms(1);
			tmp = rand()%10;
		}
		if (interr == 0){
			PORTD.OUTSET = pin2; // Turn off the Small Road's traffic light
			PORTD.OUTCLR = pin0; // Turn on the Big Road's traffic light
			tmp = rand()%10;
		}
	}
}