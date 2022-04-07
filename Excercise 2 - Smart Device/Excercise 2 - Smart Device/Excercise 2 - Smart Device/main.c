#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdio.h>

#define forward 1
#define right_turn 2
#define left_turn 4
#define time_to_turn 1

int corner_cnt = 0;
int backtrack = 0; // Backtrack mode Activated = 1, Deactivated = 0
int wall_right = 0;
int intflags;
int turning;


int main(){
	PORTD.DIR |= 7;		// Pin 0, Pin 1, Pin 2 are outputs
	initialize_ADC();	// Initialize ADC for Free Running Mode
	sei();				// Enable interrupts
	
	PORTD.OUT = 2;					// Only led 0 is on => Moving forward
	ADC0.COMMAND |= ADC_STCONV_bm;	// Start Conversion
	
	while(corner_cnt < 8){
		// Wait until the device is done turning
		if(turning)
			continue;	// Skip to next iteration of while
			
		wall_right = rand()%1;			// Check if there's a wall on the right
		_delay_ms(2);
		if(!wall_right){
			PORTD.OUT |= forward;			// Stop moving
			PORTD.OUTCLR |= right_turn;		// Turn right
			initialize_Timer(time_to_turn);	
			corner_cnt ++;
		}
	}
	PORTD.OUT |= 3; // Turn off all leds
	cli();
}

ISR(ADC0_WCOMP_vect){
	intflags = ADC0.INTFLAGS;
	ADC0.INTFLAGS = intflags;

	PORTD.OUT |= forward;			// Stop moving
	
	if(backtrack == 0)
		PORTD.OUTCLR |= left_turn;	// Turn left
	else
		PORTD.OUTCLR |= right_turn;	// Turn right
		
	initialize_Timer(time_to_turn);
	corner_cnt ++;
}


ISR (PORTF_PORT_vect){
	intflags = PORTF.INTFLAGS;
	PORTF.INTFLAGS = intflags;
	
	PORTD.OUT |= forward;		// Stop moving
	PORTD.OUTCLR |= left_turn;	// Turn left twice
	initialize_Timer(2*time_to_turn);
	PORTD.OUT |= left_turn;		// Stop turning left
	PORTD.OUTCLR |= forward;	// Start moving
	
}

ISR(TCA0_CMP0_vect){
	TCA0.SINGLE.CTRLA = 0;
	intflags = TCA0.SINGLE.INTFLAGS;
	TCA0.SINGLE.INTFLAGS = intflags;
	
	PORTD.OUT |= 3;				// Stop turning
	PORTD.OUTCLR |= forward;	// Start moving
}

// Initialize the ADC for Free-Running mode
initialize_ADC(){
		ADC0.CTRLA |= ADC_RESSEL_10BIT_gc;	// 10-bit resolution
		ADC0.CTRLA |= ADC_FREERUN_bm;		// Free-Running mode enabled
		ADC0.CTRLA |= ADC_ENABLE_bm;		// Enable ADC
		ADC0.MUXPOS |= ADC_MUXPOS_AIN7_gc;	// The bit
		ADC0.DBGCTRL |= ADC_DBGRUN_bm;		// Enable Debug Mode
		// Window Comparator Mode
		ADC0.WINLT |= 24; // Set threshold
		ADC0.INTCTRL |= ADC_WCMP_bm; // Enable Interrupts for WCM
		ADC0.CTRLE |= ADC_WINCM0_bm; // Interrupt when Result < WINLT
}

// Initialize timer
initialize_Timer(int duration){
		TCA0.SINGLE.CNT = 0;								// Clear counter
		TCA0.SINGLE.CTRLB = 0;								// Normal mode (counter)
		TCA0.SINGLE.CMP0 = duration;						// When counter reaches this value -> interrupt clock frequency/1024
		TCA0.SINGLE.CTRLA = TCA_SINGLE_CLKSEL_DIV1024_gc;	// Frequency of counter
		TCA0.SINGLE.CTRLA |= 1;								// Enable
		TCA0.SINGLE.INTCTRL = TCA_SINGLE_CMP0_bm;			// Interrupt Enable for counter
}