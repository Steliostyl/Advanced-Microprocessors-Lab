#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#define ped 5

int interr = 0;

int main(){
	PORTD.DIR |= PIN1_bm;
	PORTD.OUTCLR |= PIN1_bm;
	
	TCA0.SINGLE.CNT = 0;	// clear counter
	TCA0.SINGLE.CTRLB = 0;	// normal mode (counter)
	TCA0.SINGLE.CMP0 = ped;	// When counter reaches this value -> interrupt clock frequency/1024
	TCA0.SINGLE.CTRLA = TCA_SINGLE_CLKSEL_DIV1024_gc;	// frequency of counter
	TCA0.SINGLE.CTRLA |= 1;	// Enable
	TCA0.SINGLE.INTCTRL = TCA_SINGLE_CMP0_bm; // Interrupt Enable for counter
	
	sei();
	while(interr==0){
	}
	PORTD.OUT |= PIN1_bm;
	cli();
}

ISR(TCA0_CMP0_vect){
	TCA0.SINGLE.CTRLA = 0;
	int intflags = TCA0.SINGLE.INTFLAGS;
	TCA0.SINGLE.INTFLAGS = intflags;
	interr = 1;
}