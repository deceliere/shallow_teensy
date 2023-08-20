#include <Arduino.h>
#include "shallow.h"

static const unsigned maxSpeed = 10000000ULL; //10 MHz
static const unsigned maxSpeedBeforeDelay = 392000000ULL; //max F_CPU_ACTUAL before doing delays (measured for 10MHz, -O2)

void shiftOut_lsbFirst_rd(uint8_t dataPin, uint8_t clockPin, uint8_t value)
{
	uint8_t mask;
	uint8_t cycles_multiply;

	cycles_multiply = 3;
	if (F_CPU_ACTUAL > maxSpeedBeforeDelay) {
		// DPRINTLN("OVER max speed");
		uint32_t cycles = (F_CPU_ACTUAL / 2 / maxSpeed);
		cycles *= cycles_multiply;
		uint32_t t = ARM_DWT_CYCCNT;
		for (mask = 0x01; mask; mask <<= 1) {
		    digitalWrite(dataPin, value & mask);
		    do {;} while(ARM_DWT_CYCCNT - t < cycles);
			DPRINTLN(cycles);
		    t += cycles / 2;

		    digitalWrite(clockPin, HIGH);
		    do {;} while(ARM_DWT_CYCCNT - t < cycles);
		    t += cycles;

		    digitalWrite(clockPin, LOW);
		    do {;} while(ARM_DWT_CYCCNT - t < cycles);
		    t += cycles / 2;
		}
	}
	else
	{
		// DPRINTLN("under max speed");
		for (mask=0x01; mask; mask <<= 1) {
			digitalWrite(dataPin, value & mask);
			digitalWrite(clockPin, HIGH);
			digitalWrite(clockPin, LOW);
		}
	}
}

void shiftOut_msbFirst_rd(uint8_t dataPin, uint8_t clockPin, uint8_t value)
{
	uint32_t v;
	asm volatile ("rbit %0, %1" : "=r" (v) : "r" (value) );
	shiftOut_lsbFirst_rd(dataPin, clockPin, v >> 24);
}
