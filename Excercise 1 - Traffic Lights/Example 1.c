#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#define del 10
int interr=0;

int main(){
	PORTD.DIR |= PIN1_bm;
	PORTD.OUT |= PIN1_bm;
	PORTF.PIN5CTRL |= PORT_PULLUPEN_bm | PORT_ISC_BOTHEDGES_gc ;
	sei();
	while (interr==0){
		//_delay_ms(del);
		//_delay_ms(del);
	}
	cli();
}

ISR (PORTF_PORT_vect){
	int intflags = PORTF.INTFLAGS;
	PORTF.INTFLAGS = intflags;
	interr = 1;
	PORTD.OUTCLR = PIN1_bm;
}