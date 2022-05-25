/*
    Στυλιανάκης Στυλιανός 1059713
    Λεκαράκος Αλέξιος 1069367
*/

// When entering the password, we always
// wait for the user to enter 4 digits
// before we check if the password is correct.

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>

#define pwm_per 6
#define timer_length 30

// State 1 First password input
// State 2 Siren Activated
int state = 1;
int wrong_password = 0;
int wrong_password_count = 0;
int current_password_digit = 1;

// Initializing Functions Declaration
void initialize_TCA_as_Timer();
void initialize_TCA_as_PWM_gen();
void initialize_ADC();
void deactivate_alarm();

int main()
{
    PORTF.PIN5CTRL |= PORT_PULLUPEN_bm | PORT_ISC_BOTHEDGES_gc; // Select pin 5
    PORTF.PIN6CTRL |= PORT_PULLUPEN_bm | PORT_ISC_BOTHEDGES_gc; // select pin 6

    sei(); // Enable interrupts

    while (1)
    {
        printf("Waiting for input...");
    }
}

void initialize_TCA_as_Timer()
{
    TCA0.SINGLE.CNT = 0;                              // Clear counter
    TCA0.SINGLE.CTRLB = 0;                            // Normal mode (counter)
    TCA0.SINGLE.CMP0 = timer_length;                  // When counter reaches this value -> interrupt clock frequency/1024
    TCA0.SINGLE.CTRLA = TCA_SINGLE_CLKSEL_DIV1024_gc; // Frequency of counter
    TCA0.SINGLE.CTRLA |= 1;                           // Enable
    TCA0.SINGLE.INTCTRL = TCA_SINGLE_CMP0_bm;         // Interrupt Enable for counter
}

void initialize_TCA_as_PWM_gen()
{
    TCA0.SINGLE.CTRLA = TCA_SINGLE_CLKSEL_DIV64_gc;        // Prescaler = 64
    TCA0.SINGLE.PER = pwm_per;                             // Select the resolution
    TCA0.SINGLE.CMP0 = pwm_per / 2;                        // Select the duty cycle to be half the period
    TCA0.SINGLE.CTRLB |= TCA_SINGLE_WGMODE_SINGLESLOPE_gc; // Select Single_Slope_PWM
    TCA0.SINGLE.INTCTRL = TCA_SINGLE_OVF_bm;               // Enable interrupt overflow
    TCA0.SINGLE.INTCTRL |= TCA_SINGLE_CMP0_bm;             // Enable interrupt COMP0
    TCA0.SINGLE.CTRLA |= TCA_SINGLE_ENABLE_bm;             // Enable
}

// Initialize the ADC for Free-Running mode
void initialize_ADC()
{
    ADC0.CTRLA |= ADC_RESSEL_10BIT_gc; // 10-bit resolution
    ADC0.CTRLA |= ADC_FREERUN_bm;      // Free-Running mode enabled
    ADC0.CTRLA |= ADC_ENABLE_bm;       // Enable ADC
    ADC0.MUXPOS |= ADC_MUXPOS_AIN7_gc; // The bit
    ADC0.DBGCTRL |= ADC_DBGRUN_bm;     // Enable Debug Mode
    // Window Comparator Mode
    ADC0.WINLT |= 24;            // Set threshold
    ADC0.INTCTRL |= ADC_WCMP_bm; // Enable Interrupts for WCM
    ADC0.CTRLE |= ADC_WINCM0_bm; // Interrupt when Result < WINLT
    ADC0.COMMAND |= ADC_STCONV_bm;
}

void deactivate_alarm()
{
    state = 1;             // Set initial state.
    PORTD.OUTCLR = 1;      // LED 0 ON
    ADC0.CTRLA = 0;        // ADC OFF
    TCA0.SINGLE.CTRLA = 0; // TCA OFF
    TCA0.SINGLE.INTCTRL = 0;
}

// ISR Functions Declaration
// TCA0 Overflow ISR
ISR(TCA0_OVF_vect)
{
    int intflags = TCA0.SINGLE.INTFLAGS;
    TCA0.SINGLE.INTFLAGS = intflags;
    PORTD.OUTSET = 1; // LED 0 OFF
}

// TCA0 CMP ISR
ISR(TCA0_CMP0_vect)
{
    TCA0.SINGLE.CTRLA = 0;
    int intflags = TCA0.SINGLE.INTFLAGS;
    TCA0.SINGLE.INTFLAGS = intflags;
    if (state == 1)
    {
        initialize_ADC();
        state++; // State 2 (Siren Activated)
    }
    else
        initialize_TCA_as_PWM_gen();
}

// ADC ISR
ISR(ADC0_WCOMP_vect)
{
    int intflags = ADC0.INTFLAGS;
    ADC0.INTFLAGS = intflags;
    PORTD.OUTCLR = 1; // LED 0 ON
    initialize_TCA_as_Timer();
}

// Switch ISR
ISR(PORTF_PORT_vect)
{
    // Checking password 6556
    int intflags = PORTF.INTFLAGS;
    PORTF.INTFLAGS = intflags;
    // Checking which SW triggered the ISR

    // Triggered by SW5
    if (intflags == 0x20)
    {
        // Check digit position with current_password_digit
        // if 5 is inserted while not in one of the correct positions
        // raise wrong password flag
        if (current_password_digit != 2 && current_password_digit != 3)
            wrong_password = 1;
    }
    // Triggered by SW6
    // Same check as for SW5
    else if (current_password_digit != 1 && current_password_digit != 4)
        wrong_password = 1;

    // After all digits have been entered
    // accept password if wrong_password flag = 0
    // otherwise reject it.
    if (current_password_digit == 4)
    {
        if (wrong_password != 1)
        {
            // Password accepted.
            wrong_password_count = 0;
            if (state == 1)
                initialize_TCA_as_Timer();
            else
                deactivate_alarm();
        }
        else
        {
            wrong_password_count++;
            if (state == 2 && wrong_password_count == 3)
                initialize_TCA_as_PWM_gen();
        }
        current_password_digit = 1;
    }
    else
        current_password_digit++;
}