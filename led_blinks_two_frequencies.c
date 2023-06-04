#define F_CPU 16000000

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#define LED PD7
#define BUTTON PB0

volatile uint8_t button_state;
volatile bool button_clicked;

void setUpIO() {
	DDRD |= (1 << LED);
	DDRB &= ~(1 << BUTTON);
	PORTB |= (1 << BUTTON);
}

void setUpTimer() {
	TCCR1B |= (1 << WGM12) | (1 << CS12) | (1 << CS10);
	OCR1A = 7812;
}

void setUpInterrupts() {
	TIMSK1 |= (1 << OCIE1A);
	sei();
}

int main(void) {
	
	setUpIO();
	setUpTimer();
	setUpInterrupts();

	
	while (1) {
		if (PINB != button_state) {
			_delay_ms(20);
			if (PINB != button_state) {
				button_state = PINB;
				TCNT1 = 0;
				if (!(PINB & (1 << BUTTON))) {
					OCR1A = 3124;
					} else {
					OCR1A = 7812;
				}
			}
		}
	}
}

ISR(TIMER1_COMPA_vect) {
	PORTD ^= (1 << LED);
}


