#include <avr/io.h>
#include <avr/interrupt.h>

#define right_wheel = PORTD.PIN0_bm
#define left_wheel = PORTD.PIN1_bm
#define standby_led = PORTD.PIN2_bm
#define standard_freq = 1

int turning = 0;

int main(){
	PORTD.DIR |= PIN1_bm;	// PIN is output
	initialize_TCA();		// Initialize TCA0 for PWM mode
	initialize_ADC();		// Initialize ADC for Free Running Mode
	sei();
	while(1){
		
	}
}

// Counter Overflow ISR
ISR(TCA0_OVF_vect){
	// Clear the interrupt flag
	int intflags = TCA0.SINGLE.INTFLAGS;
	TCA0.SINGLE.INTFLAGS = intflags;
	PORTD.OUT |= PIN1_bm; // PIN is on
}

// Counter ISR
ISR(TCA0_CMP0_vect){
	// Clear the interrupt flag
	int intflags = TCA0.SINGLE.INTFLAGS;
	TCA0.SINGLE.INTFLAGS = intflags;
	PORTD.OUTCLR |= PIN1_bm; // PIN is off
}

// ADC ISR
ISR(ADC0_WCOMP_vect){
	int intflags = ADC0.INTFLAGS;
	ADC0.INTFLAGS = intflags;
	stop_moving();
}

// Switch ISR
ISR (PORTF_PORT_vect){
	int intflags = PORTF.INTFLAGS;
	PORTF.INTFLAGS = intflags;
	
	// Is 1 only when bit 5 = 1 (otherwise it's 0)
	turn(PORTF.INTFLAGS / 16 % 2);
}


void initialize_TCA(){
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
}


// Initialize the ADC for Free-Running mode
void initialize_ADC(){
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

void turn(int direction){
	// Stop turning
	//if(turning==1){
	//	// Stop turning
	//	turning = 0;
	//	return;
	//}
	turning = 1;
	// Start turning right
	if(direction==1){
		
	}
	// Start turning left
	else{
		
	}
}

void stop_moving(){
	TCA0.SINGLE.CNT = 0;		// Clear counter
	TCA0.SINGLE.CTRLA = 0;		// Enable
	PORTD.OUT |= standby_led;	// Stop moving
}

void stop_turning(){
	// Left PWM = Right PWMs
}