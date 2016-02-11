#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/pgmspace.h>

// for 1.2MHz
#define TICKS_PER_SECOND (F_CPU / 256) // 4687.5 (586 if timer frequency is divided by 8) for 1.2MHz
//#define PATTERN_PULSE_LENGTH (TICKS_PER_SECOND / 8)
#define PATTERN_PULSE_BIT_SCALE (9)
//#define FAN_PULSE_LENGTH (TICKS_PER_SECOND / 125) // 8 ms
#define FAN_PULSE_LENGTH (TICKS_PER_SECOND / 10) // 100 ms

#define LONG_CLICK_DURATION (3 * TICKS_PER_SECOND)
#define LONG_CLICK_OFF (10 * TICKS_PER_SECOND)

struct state
{
	uint8_t fan, red, green; // patterns for fan and leds
	uint32_t time; // timeout length
	uint8_t time_over, button; // state switch
};

//#include "states_orig.h"
//#include "states_new_gr.h"
#include "states_new_rg.h"

const struct state *currentState;

volatile uint32_t tickCounter; // counter of interrupts
volatile uint8_t fanTickCounter; // counter of interrupts

main()
{
    tickCounter = 0;
    fanTickCounter = 0;
    currentState = &STATES[0];

//    patternTickCounter = 0;
//    patternBit = 1;

    // init timer and interrupts

    DDRB = LED_OUT_R | LED_OUT_G | FAN_OUT;
    PORTB = BTN_IN;

    // init timer
    // prescale timer to 1/8th (1<<CS01) or 1/1st (1<<CS00) the clock rate
    TCCR0B |= (1<<CS00);
    // enable timer overflow interrupt
    TIMSK0 |= (1<<TOIE0);

	// init pc0 interrupt
	// Any logical change on INT0 generates an interrupt request.
	MCUCR |= (0 << ISC01) | (1 << ISC00);
	// External Interrupt Request 0 Enable
	GIMSK |= (1 << PCIE);
	// Set pin mask
	PCMSK = PWR_MASK;

    sei();

    uint8_t prevButton = PINB & BTN_IN;
    uint32_t prevCounter = tickCounter;

    while (1)
    {
    	if (tickCounter != prevCounter)
    	{
    		// check state
    		uint8_t button = !(PINB & BTN_IN);
    		if (button && prevButton && tickCounter >= LONG_CLICK_OFF)
			{
				currentState = &STATES[STATE_LONG_OFF];
			}
			else if (button && prevButton && tickCounter >= LONG_CLICK_DURATION)
			{
				currentState = &STATES[STATE_LONG_CLICK];
			}
			else if (button && !prevButton)
    		{
    			currentState = &STATES[pgm_read_byte(&(currentState->button))];
    			tickCounter = 0; tickCounter = 0;
//    			patternTickCounter = 0; patternBit = 0;
    		}
    		else if (prevCounter >= pgm_read_dword(&(currentState->time)))
    		{
    			currentState = &STATES[pgm_read_byte(&(currentState->time_over))];
    			tickCounter = 0; tickCounter = 0;
//    			patternTickCounter = 0; patternBit = 0;
    		}

    		// output
    		//uint8_t patternBit = 0xFF;//1 << (tickCounter * 8 / TICKS_PER_SECOND % 8);
    		uint8_t patternBit = 1 << (((uint16_t) prevCounter >> PATTERN_PULSE_BIT_SCALE) & 0b111);
    		uint8_t port = BTN_IN;
    		if (PINB & PWR_IN)
    		{
				if (pgm_read_byte(&(currentState->red)) & patternBit) {port |= LED_OUT_R;}
				if (pgm_read_byte(&(currentState->green)) & patternBit) {port |= LED_OUT_G;}
    		}
    		if (fanTickCounter > 0) {
    			if (pgm_read_byte(&(currentState->fan))) {port |= FAN_OUT;}
    			fanTickCounter --;
    		}
    		PORTB = port;

    		// save prev values
    		prevButton = button;
    		prevCounter = tickCounter;
    	}
    }
}


// Timer interrupt
ISR(TIM0_OVF_vect)
{
    tickCounter ++;
//
//    if (patternTickCounter == 0)
//    {
//    	patternTickCounter = PATTERN_PULSE_LENGTH;
//    	patternBit = patternBit << 1;
//    	if (patternBit == 0) {
//    		patternBit = 1;
//    	}
//    }
//    patternTickCounter --;
}


// Power change interrupt
ISR(PCINT0_vect)
{
	fanTickCounter = FAN_PULSE_LENGTH;
}
