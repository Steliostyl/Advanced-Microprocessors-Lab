#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>

#define STD_PER 4

// Note to self:
// TCA controls RIGHT wheel
// TCB controls LEFT wheel

// LED 0 = Right wheel
// LED 1 = Left wheel
// LED 2 = Standby LED

int turning = 0;
int tca0_int_cnt = 0;
int tcb_int_cnt = 0;

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
	PORTB_DIR |= PIN2_bm;
	PORTB_OUT |= PIN2_bm;
	
	/* Enable writing to protected register */
	CPU_CCP = CCP_IOREG_gc;
	/* Enable Prescaler and set Prescaler Division to 64 */
	CLKCTRL.MCLKCTRLB = CLKCTRL_PDIV_64X_gc | CLKCTRL_PEN_bm;
	
	/* Enable writing to protected register */
	CPU_CCP = CCP_IOREG_gc;
	/* Select 32KHz Internal Ultra Low Power Oscillator (OSCULP32K) */
	CLKCTRL.MCLKCTRLA = CLKCTRL_CLKSEL_OSCULP32K_gc;
	
	
	/* Load CCMP register with the period and duty cycle of the PWM */
	TCB0.CCMPL = 2*period*0x100 + 0xff;

	/* Enable TCB3 and Divide CLK_PER by 2 */
	TCB0.CTRLA |= TCB_ENABLE_bm;
	TCB0.CTRLA |= TCB_CLKSEL_CLKDIV2_gc;
	
	/* Enable Pin Output and configure TCB in 8-bit PWM mode */
	TCB0.CTRLB |= TCB_CCMPEN_bm;
	TCB0.CTRLB |= TCB_CNTMODE_PWM8_gc;
	
	/*TCA0.SINGLE.CTRLA = TCA_SINGLE_CLKSEL_DIV1024_gc;	// Prescaler = 1024
	TCA0.SINGLE.PER = period;		// Select the resolution
	TCA0.SINGLE.CMP0 = period/2;	// Select the duty cycle
	TCA0.SINGLE.CTRLB |= TCA_SINGLE_WGMODE_SINGLESLOPE_gc;	// Select Single_Slope_PWM
	TCA0.SINGLE.INTCTRL = TCA_SINGLE_OVF_bm;	// Enable interrupt overflow
	TCA0.SINGLE.INTCTRL |= TCA_SINGLE_CMP0_bm;	// Enable interrupt COMP0
	TCA0.SINGLE.CTRLA |= TCA_SINGLE_ENABLE_bm;	// Enable
	*/
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

void move_forward(){
	initialize_ADC();			// Initialize ADC for Free Running Mode
	initialize_TCA(STD_PER);	// Initialize TCA0 for PWM mode
	initialize_TCB(STD_PER);	// Initialize TCB for PWM mode
}

void stop_moving(){
	TCA0.SINGLE.CNT = 0;		// Clear counter
	PORTD.OUT |= 8;				// Turn on standby LED
								// Disable TCA
								// Disable TCB
								// Disable ADC
}

void turn(int direction){	
	// If the standby led (LED 2 or 3rd bit of PORTD.OUT)
	// is not on, don't do anything
	if(!(PORTD.OUT / 7 % 2))
		return;
		
	// Stop turning and start moving forward again
	if(turning==1){
		turning = 0;
		PORTD.OUT |= 8; // Turn LED 2 (STANDBY LED) off
		move_forward();	// Start moving forward
		return;			// Exit function
	}
	// If the device was turning right, 
	// then the left_wheel speed is 2 x STD_PER.
	if(direction){
		// In order to start going straight again,
		// set left_wheel speed to STD_PER.
		initialize_TCB(STD_PER);
	}
	else {
		// Similarly, if it was turning left,
		// set the right_wheel speed to STD_PER.
		initialize_TCA(STD_PER);
	}
	turning = 1;
	// Start turning right
	if(direction==1){
		initialize_TCA(STD_PER*2);	// Right wheel's period doubles => moves slower
		initialize_TCB(STD_PER);	// Left wheel's period is set to default speed
	}
	// Start turning left
	else{
		initialize_TCA(STD_PER);	// Left wheel's period doubles => moves slower
		initialize_TCB(STD_PER*2);	// Right wheel's period is set to default speed
	}
}

// TCA0 Overflow ISR
ISR(TCA0_OVF_vect){
	tca0_int_cnt ++ ;
	// Clear the interrupt flag
	int intflags = TCA0.SINGLE.INTFLAGS;
	TCA0.SINGLE.INTFLAGS = intflags;
	
	// Every 2 PULSES, move the right wheel.
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
	int intflags = PORTF.INTFLAGS;
	PORTF.INTFLAGS = intflags;	
	turn(PORTF.INTFLAGS / 16 % 2);
}

int main(){
	PORTD.DIR |= 7;				// PIN 0, 1 and 2 is output
	move_forward();
	sei();						// Enable interrupts
	PORTD.OUT = 7;				// Turn off all 3 LEDs (0, 1 and 2)
	while(1){
		printf("Hello world");		
	}
}