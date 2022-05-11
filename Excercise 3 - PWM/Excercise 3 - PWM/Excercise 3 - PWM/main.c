#include <avr/io.h>
#include <avr/interrupt.h>

#define STD_PER 127

int turning = 0;
int tca0_int_cnt = 0;

void stop_moving(){
	TCA0.SINGLE.CNT = 0;		// Clear counter
	PORTD.OUT |= 8;				// Turn on standby LED
}

void turn(int direction){
	if(turning==1){
		// Stop turning
		turning = 0;
		return;
	}
	turning = 1;
	// Start turning right
	if(direction==1){
		initialize_TCA(STD_PER*2);
		initialize_TCB(STD_PER);
	}
	// Start turning left
	else{
		initialize_TCA(STD_PER);
		initialize_TCB(STD_PER*2);
	}
}

// Counter Overflow ISR
ISR(TCA0_OVF_vect){
	tca0_int_cnt ++ ;
	// Clear the interrupt flag
	int intflags = TCA0.SINGLE.INTFLAGS;
	TCA0.SINGLE.INTFLAGS = intflags;
	if(tca0_int_cnt % 2 == 0)
		PORTD.OUTCLR = 1;	// Turn LED 0 on
	else
		PORTD.OUTSET = 1;	// Turn LED 0 off 
	TCA0.SINGLE.CNT = 0;	// Clear counter
}

// ADC ISR
ISR(ADC0_WCOMP_vect){
	int intflags = ADC0.INTFLAGS;
	ADC0.INTFLAGS = intflags;
	// Stop moving
	TCA0.SINGLE.CNT = 0;		// Clear counter
	PORTD.OUT |= 8;				// Turn on standby LED
}

// Switch ISR
ISR (PORTF_PORT_vect){
	// If the standby led (LED 2 or 3rd bit of PORTD.OUT) 
	// is not on, don't do anything
	if(PORTD.OUT / 7 % 2)
		break;
	int intflags = PORTF.INTFLAGS;
	PORTF.INTFLAGS = intflags;
	
	turn(PORTF.INTFLAGS / 16 % 2);
}


void initialize_TCA(int period){
	TCA0.SINGLE.CTRLA = TCA_SINGLE_CLKSEL_DIV1024_gc;	// Prescaler = 1024
	TCA0.SINGLE.PER = period;		// Select the resolution
	TCA0.SINGLE.CMP0 = period/2;	// Select the duty cycle	
	TCA0.SINGLE.CTRLB |= TCA_SINGLE_WGMODE_SINGLESLOPE_gc;	// Select Single_Slope_PWM
	TCA0.SINGLE.INTCTRL = TCA_SINGLE_OVF_bm;	// Enable interrupt overflow
	TCA0.SINGLE.INTCTRL |= TCA_SINGLE_CMP0_bm;	// Enable interrupt COMP0
	TCA0.SINGLE.CTRLA |= TCA_SINGLE_ENABLE_bm;	// Enable
}

void initialize_TCB(int period){
	TCA0.SINGLE.CTRLA = TCA_SINGLE_CLKSEL_DIV1024_gc;	// Prescaler = 1024
	TCA0.SINGLE.PER = period;		// Select the resolution
	TCA0.SINGLE.CMP0 = period/2;	// Select the duty cycle
	TCA0.SINGLE.CTRLB |= TCA_SINGLE_WGMODE_SINGLESLOPE_gc;	// Select Single_Slope_PWM
	TCA0.SINGLE.INTCTRL = TCA_SINGLE_OVF_bm;	// Enable interrupt overflow
	TCA0.SINGLE.INTCTRL |= TCA_SINGLE_CMP0_bm;	// Enable interrupt COMP0
	TCA0.SINGLE.CTRLA |= TCA_SINGLE_ENABLE_bm;	// Enable
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




int main(){
	PORTD.DIR |= PIN1_bm;	// PIN is output
	initialize_TCA(STD_PER);		// Initialize TCA0 for PWM mode
	initialize_ADC();		// Initialize ADC for Free Running Mode
	sei();
	PORTD.OUT = 7;
	while(1){
	}
}