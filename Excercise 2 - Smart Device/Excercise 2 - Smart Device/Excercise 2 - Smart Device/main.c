#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdio.h>

int corner_cnt;

int main(){
	PORTD.DIR |= 3; // Pin 0, Pin 1 are outputs
	
	// Initialize the ADC for Free-Running mode
	ADC0.CTRLA |= ADC_RESSEL_10BIT_gc; // 10-bit resolution
	ADC0.CTRLA |= ADC_FREERUN_bm; // Free-Running mode enabled
	ADC0.CTRLA |= ADC_ENABLE_bm; // Enable ADC
	ADC0.MUXPOS |= ADC_MUXPOS_AIN7_gc; // The bit
	// Enable Debug Mode
	ADC0.DBGCTRL |= ADC_DBGRUN_bm;
	// Window Comparator Mode
	ADC0.WINLT |= 24; // Set threshold
	ADC0.INTCTRL |= ADC_WCMP_bm; // Enable Interrupts for WCM
	ADC0.CTRLE |= ADC_WINCM0_bm; // Interrupt when Result < WINLT
	sei();
	
	corner_cnt = 0;
	PORTD.OUT = 2; // Only led 0 is on => Going straight
	ADC0.COMMAND |= ADC_STCONV_bm; // Start Conversion
	
	while(corner_cnt < 4){
		printf("Hello world");
	}
	PORTD.OUT |= 3; // Turn off all leds
	cli();
}

ISR(ADC0_WCOMP_vect){
	int intflags = ADC0.INTFLAGS;
	ADC0.INTFLAGS = intflags;
	
	PORTD.OUT |= 1;		// Stop moving
	PORTD.OUTCLR |= 2;	// Turn left
	_delay_ms(1);
	PORTD.OUT |= 2;		// Stop turning
	PORTD.OUTCLR |= 1;	// Start moving
	
	corner_cnt ++;
}

