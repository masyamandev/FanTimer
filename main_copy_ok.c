#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

// for 1.2MHz
#define TICKS_PER_SECOND (F_CPU / 256) // 4687.5 (586 if timer frequency is divided by 8) for 1.2MHz
//#define PATTERN_PULSE_LENGTH (TICKS_PER_SECOND / 8)
#define PATTERN_PULSE_BIT_SCALE (9)
#define FAN_PULSE_LENGTH (TICKS_PER_SECOND / 500) // 2 ms

struct state
{
	uint8_t fan, red, green; // patterns for fan and leds
	uint32_t time; // timeout length
	uint8_t time_over, button; // state switch
};

const struct state STATES[] = {
		{0, 0x00, 0x00,      TICKS_PER_SECOND, 0, 1}, // 0, off
		{1, 0x00, 0xFF, 10 * TICKS_PER_SECOND, 2, 3}, // 1, green on for 20 minutes
		{1, 0x00, 0xCC,  3 * TICKS_PER_SECOND, 3, 1}, // 2, green on for 10 seconds
		{1, 0xFF, 0x00,  5 * TICKS_PER_SECOND, 4, 0}, // 3, red on for 3 minutes
		{1, 0xCC, 0x00,  5 * TICKS_PER_SECOND, 0, 3}  // 4, red on for 5 seconds
//		,{1, 0x55, 0xAA,      TICKS_PER_SECOND, 5, 0}  // 5, switched on forever
};
//const struct state STATES[] = {
//		{0, 0x00, 0x00,      TICKS_PER_SECOND, 0, 1}, // 0, off
//		{1, 0x00, 0xFF, 20 * 60 * TICKS_PER_SECOND, 2, 3}, // 1, green on for 20 minutes
//		{1, 0x00, 0xCC,      10 * TICKS_PER_SECOND, 3, 1}, // 2, green on for 10 seconds
//		{1, 0xFF, 0x00,  3 * 60 * TICKS_PER_SECOND, 4, 0}, // 3, red on for 3 minutes
//		{1, 0xCC, 0x00,      10 * TICKS_PER_SECOND, 0, 3}  // 4, red on for 10 seconds
////		,{1, 0x55, 0xAA,      TICKS_PER_SECOND, 5, 0}  // 5, switched on forever
//};
struct state *currentState;

volatile uint32_t tickCounter; // counter of interrupts
volatile uint8_t fanTickCounter; // counter of interrupts

//uint16_t patternTickCounter; // counter of interrupts
//volatile uint8_t patternBit;

#define BTN_IN (1 << PB1) // Button
#define PWR_IN (1 << PB0) // Power (use LEDS if 1)
#define LED_OUT_R (1 << PB3) // LED
#define LED_OUT_G (1 << PB2) // LED
#define FAN_OUT (1 << PB4) // FAN

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
	PCMSK = (1 << PCINT0);

    sei();

    uint8_t prevButton = PINB & BTN_IN;
    uint32_t prevCounter = tickCounter;

    while (1)
    {
    	if (tickCounter != prevCounter)
    	{
    		// check state
    		uint8_t button = PINB & BTN_IN;
    		if (prevButton && !button)
    		{
    			currentState = &STATES[currentState->button];
    			tickCounter = 0; tickCounter = 0;
//    			patternTickCounter = 0; patternBit = 0;
    		}
    		else if (prevCounter >= currentState->time)
    		{
    			currentState = &STATES[currentState->time_over];
    			tickCounter = 0; tickCounter = 0;
//    			patternTickCounter = 0; patternBit = 0;
    		}

    		// output
    		//uint8_t patternBit = 0xFF;//1 << (tickCounter * 8 / TICKS_PER_SECOND % 8);
    		uint8_t patternBit = 1 << (((uint16_t) prevCounter >> PATTERN_PULSE_BIT_SCALE) & 0b111);
    		uint8_t port = BTN_IN;
    		if (PINB & PWR_IN)
    		{
				if (currentState->red & patternBit) {port |= LED_OUT_R;}
				if (currentState->green & patternBit) {port |= LED_OUT_G;}
    		}
    		if (fanTickCounter > 0) {
    			if (currentState->fan) {port |= FAN_OUT;}
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
