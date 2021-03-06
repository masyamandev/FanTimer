
#define BTN_IN (1 << PB1) // Button
#define PWR_IN (1 << PB2) // Power (use LEDS if 1)
#define PWR_MASK (1 << PCINT2) // Power interrupt (Same as previous line)
#define LED_OUT_R (1 << PB4) // LED
#define LED_OUT_G (1 << PB0) // LED
#define FAN_OUT (1 << PB3) // FAN

#define STATE_LONG_CLICK (5)
#define STATE_LONG_OFF (6)
const struct state STATES[] PROGMEM = {
		{0, 0x00, 0x00,      60 * TICKS_PER_SECOND, 0, 1}, // 0, off
		{1, 0xFF, 0x00,  5 * 60 * TICKS_PER_SECOND, 2, 3}, // 1, red on for 5 minutes
		{1, 0xCC, 0x00,      10 * TICKS_PER_SECOND, 3, 1}, // 2, red on for 10 seconds
		{1, 0x00, 0xFF, 20 * 60 * TICKS_PER_SECOND, 4, 0}, // 3, green on for 20 minutes
		{1, 0x00, 0xCC,      10 * TICKS_PER_SECOND, 0, 3}, // 4, green on for 10 seconds
		{1, 0x55, 0xAA,      60 * TICKS_PER_SECOND, 5, 0}, // 5, switched on forever
		{0, 0x00, 0x00,           TICKS_PER_SECOND, 6, 1}, // 6, off, no long click
};
