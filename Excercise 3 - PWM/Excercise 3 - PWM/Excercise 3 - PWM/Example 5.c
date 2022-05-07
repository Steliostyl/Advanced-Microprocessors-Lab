#include <avr/io.h>
#include <avr/interrupt.h>

int main(){
	PORTD.DIR |= PIN1_bm; // PIN is output
	// Prescaler = 1024
	TCA0.SINGLE.CTRLA = TCA_SINGLE_CLKSEL_DIV1024_gc;
	TCA0.SINGLE.PER = 254; // Select the resolution
	TCA0.SINGLE.CMP0 = 127; // Select the duty cycle
	// Select Single_Slope_PWM
	TCA0.SINGLE.CTRLB |= TCA_SINGLE_WGMODE_SINGLESLOPE_gc;
	// Enable interrupt overflow
	TCA0.SINGLE.INTCTRL = TCA_SINGLE_OVF_bm;
	// Enable interrupt COMP0
	TCA0.SINGLE.INTCTRL |= TCA_SINGLE_CMP0_bm;
	TCA0.SINGLE.CTRLA |= TCA_SINGLE_ENABLE_bm; // Enable
	sei();
	while(1){
		
	}
}

ISR(TCA0_OVF_vect){
	// Clear the interrupt flag
	int intflags = TCA0.SINGLE.INTFLAGS;
	TCA0.SINGLE.INTFLAGS = intflags;
	PORTD.OUT |= PIN1_bm; // PIN is on
}

ISR(TCA0_CMP0_vect){
	// Clear the interrupt flag
	int intflags = TCA0.SINGLE.INTFLAGS;
	TCA0.SINGLE.INTFLAGS = intflags;
	PORTD.OUTCLR |= PIN1_bm; // PIN is off
}