#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>

#define STD_PER 4

int wrong_pwd = 0;
int pwd_digit = 0;

void initialize_TCA(int period){
	TCA0.SINGLE.CTRLA = TCA_SINGLE_CLKSEL_DIV64_gc;	// Prescaler = 64
	TCA0.SINGLE.PER = period;		// Select the resolution
	TCA0.SINGLE.CMP0 = period/2;	// Select the duty cycle
	TCA0.SINGLE.CTRLB |= TCA_SINGLE_WGMODE_SINGLESLOPE_gc;	// Select Single_Slope_PWM
	TCA0.SINGLE.INTCTRL = TCA_SINGLE_OVF_bm;	// Enable interrupt overflow
	TCA0.SINGLE.INTCTRL |= TCA_SINGLE_CMP0_bm;	// Enable interrupt COMP0
	TCA0.SINGLE.CTRLA |= TCA_SINGLE_ENABLE_bm;	// Enable
}

void initialize_TCB(int period){
	/* Enable writing to protected register */
	CPU_CCP = CCP_IOREG_gc;
	/* Enable Prescaler and set Prescaler Division to 64 */
	CLKCTRL.MCLKCTRLB = CLKCTRL_PDIV_64X_gc | CLKCTRL_PEN_bm;
	
	/* Enable writing to protected register */
	CPU_CCP = CCP_IOREG_gc;
	/* Select 32KHz Internal Ultra Low Power Oscillator (OSCULP32K) */
	CLKCTRL.MCLKCTRLA = CLKCTRL_CLKSEL_OSCULP32K_gc;
	
	
	/* Load CCMP register with the period and duty cycle of the PWM.
	
	Note:
	TCB turns on PORTB.PIN0 (left wheel) on each rising edge, 
	stays on for half the period time then turns off again.
	
	Therefore, in order for left wheel to move at the same speed
	as the right wheel, CCMPL+1 must be 4 times period.
	
	Since the period of the output pulse is defined by TCBn.CCMPL+1, 
	the value that must be loaded into the TCBn.CCMPL register is 0xFF.
	We also want to set CCMPH to 4 times period and therefore we have:*/
	TCB0.CCMP = 4*period*0x100 | 0xff;

	/* Enable TCB0 and Divide CLK_PER by 2 */
	TCB0.CTRLA |= TCB_ENABLE_bm;
	TCB0.CTRLA |= TCB_CLKSEL_CLKDIV2_gc;
	
	/* Enable Pin Output and configure TCB in 8-bit PWM mode */
	TCB0.CTRLB |= TCB_CCMPEN_bm;
	TCB0.CTRLB |= TCB_CNTMODE_PWM8_gc;
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
	
}

void deacvtivate_alarm(){
	
}

// END OF ALARM FUNCTIONS

// TCA0 Overflow ISR
ISR(TCA0_OVF_vect){
	int intflags = TCA0.SINGLE.INTFLAGS;
	TCA0.SINGLE.INTFLAGS = intflags;
	
}

// ADC ISR
ISR(ADC0_WCOMP_vect){
	int intflags = ADC0.INTFLAGS;
	ADC0.INTFLAGS = intflags;
}

// Switch ISR
ISR(PORTF_PORT_vect){
	int intflags = PORTF.INTFLAGS;
	PORTF.INTFLAGS = intflags;
	printf(intflags);
	// If the ISR is called from SW5
	if()
	// and current pwd_digit is not 1
	// or 2, turn on the wrong_pwd "flag"
		if(pwd_digit != 1 && pwd_digit != 2)
			wrong_pwd = 1;
	// Otherwise, the ISR is called from SW6
	else
		if()
			wrong_pwd = 1;
	
	// When the final digit is entered,
	// if it's the right digit and the 
	// wrong_pwd digit "flag" hasn't 
	// been activated, then the pwd is
	// correct.
	
	if(pwd_digit==3){
		// PWD entered successfully
		// 
		pwd_digit = 0;
	}
	else pwd_digit++;
}

int main(){
	sei();						// Enable interrupts
	
	while(1){
		printf("Waiting for input...");
	}
}