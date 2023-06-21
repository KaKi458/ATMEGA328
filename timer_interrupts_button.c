#define F_CPU 16000000

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#define LED PD7
#define BUTTON PB0

volatile uint8_t button_clicked = 0;
volatile uint8_t led_state = 0;

void setUpIO() {
	DDRD |= (1 << LED);
	DDRB &= ~(1 << BUTTON);
	PORTB |= (1 << BUTTON);
}

void setUpTimer() {
	TCCR1B |=  (1 << WGM12) | (1 << CS12) | (1 << CS10);
	OCR1A = 312; // ~20ms
}

void setUpInterrupts() {
	TIMSK1 |= (1 << OCIE1A);
	sei();
}

void controlOutputs() {
  if (led_state == 0) {
    // if led was off then now it will be on
    led_state = 1;
    PORTD |= (1 << LED);
  } else {
    // if led was on then now it will be off
    led_state = 0;
    PORTD &= ~(1 << LED);
  }
}

int main(void) {
	
	setUpIO();
	setUpTimer();
	setUpInterrupts();
	
	while (1);
}

ISR(TIMER1_COMPA_vect) {
	static uint8_t counter = 0;

	if ((button_clicked == 0) && !(PINB & (1 << BUTTON))) {
		button_clicked = 1;
		counter = 20; // ~400ms
    	controlOutputs();
	} 
		
	if (button_clicked == 1) {
		if (counter > 0) {
			counter--;
		} else {
			button_clicked = 0;
		}
	}
}



