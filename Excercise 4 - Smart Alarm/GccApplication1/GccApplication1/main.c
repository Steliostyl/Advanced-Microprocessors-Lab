#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>

#define STD_PER 4
#define duration 1

int wrong_pwd = 0;
int pwd_digit = 1;
int state = 0;
int wrong_pwd_count = 0;
// State 0 = Initial State
// State 1 = Awaiting for deactivation/Siren is on

void initialize_TCA_Timer(int period){
	TCA0.SINGLE.CNT = 0;								// Clear counter
	TCA0.SINGLE.CTRLB = 0;								// Normal mode (counter)
	TCA0.SINGLE.CMP0 = duration;						// When counter reaches this value -> interrupt clock frequency/1024
	TCA0.SINGLE.CTRLA = TCA_SINGLE_CLKSEL_DIV1024_gc;	// Frequency of counter
	TCA0.SINGLE.CTRLA |= 1;								// Enable
	TCA0.SINGLE.INTCTRL = TCA_SINGLE_CMP0_bm;			// Interrupt Enable for counter

}

void initialize_TCA_PWM(int period){
	TCA0.SINGLE.CTRLA = TCA_SINGLE_CLKSEL_DIV64_gc;	// Prescaler = 64
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

// START OF ALARM FUNCTIONS

void activate_alarm(){
	initialize_TCA_Timer(); // Start timer
}

void deacvtivate_alarm(){
	state = 0;
	// Turn off led 0
	// Deactivate ADC
	// Deactivate TCA
}

// END OF ALARM FUNCTIONS

// TCA0 Overflow ISR
ISR(TCA0_OVF_vect){
	int intflags = TCA0.SINGLE.INTFLAGS;
	TCA0.SINGLE.INTFLAGS = intflags;
}

ISR(TCA0_CMP0_vect){
	TCA0.SINGLE.CTRLA = 0;
	int intflags = TCA0.SINGLE.INTFLAGS;
	TCA0.SINGLE.INTFLAGS = intflags;
	if(state==0){
		initialize_ADC();
		state++;
	}
	else
		initialize_TCA_PWM();
}

// ADC ISR
ISR(ADC0_WCOMP_vect){
	int intflags = ADC0.INTFLAGS;
	ADC0.INTFLAGS = intflags;
	// Turn on LED0
	initialize_TCA_Timer();
}

// Switch ISR
ISR(PORTF_PORT_vect){
	int intflags = PORTF.INTFLAGS;
	PORTF.INTFLAGS = intflags;
	// If the ISR is called from SW5
	if(intflags==0x20){
		// and current pwd_digit is not 1
		// or 2, turn on the wrong_pwd "flag"
		if(pwd_digit != 2 && pwd_digit != 3)
			wrong_pwd = 1;
	}
	// Otherwise, the ISR is called from SW6
	else
		if(pwd_digit != 1 && pwd_digit != 4)
			wrong_pwd = 1;
	
	// When the final digit is entered,
	// if it's the right digit and the
	// wrong_pwd digit "flag" hasn't
	// been activated, then the pwd is
	// correct.
	
	if(pwd_digit==4){
		if(wrong_pwd != 1){
			// PWD entered successfully
			wrong_pwd_count = 0;
			if(state==0)
				initialize_TCA_Timer();
			else
				deacvtivate_alarm();
		}
		else{
			wrong_pwd_count ++;
			if(state==1 && wrong_pwd_count==3)
				initialize_TCA_PWM();
		}
		pwd_digit = 1;
	}
	else pwd_digit++;
}

int main(){
	PORTF.PIN5CTRL |= PORT_PULLUPEN_bm | PORT_ISC_BOTHEDGES_gc; //Select pin 5
	PORTF.PIN6CTRL |= PORT_PULLUPEN_bm | PORT_ISC_BOTHEDGES_gc; //select pin 6
	
	sei();	// Enable interrupts
	
	while(1){
		printf("Waiting for input...");
	}
}