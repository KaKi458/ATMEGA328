#define F_CPU 16000000

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#define LED1 PD0
#define LED2 PD1
#define LED3 PD2
#define BUTTON1 PB0
#define BUTTON2 PB1
#define BUTTON3 PB2

volatile uint8_t LED_STATE = 0;

void setUpIO() {
	DDRD |= (1 << LED1) | (1 << LED2) | (1 << LED3);
	DDRB &= ~((1 << BUTTON1) | (1 << BUTTON2) | (1 << BUTTON3));
	PORTB |= (1 << BUTTON1) | (1 << BUTTON2) | (1 << BUTTON3);
}

void setUpTimers() {
	TCCR1B |=  (1 << WGM12) | (1 << CS12) | (1 << CS10);
	OCR1A = 78; // ~5ms

  	TCCR0B |= (1 << CS02) | (1 << CS00); 
}

void setUpInterrupts() {
	TIMSK1 |= (1 << OCIE1A);
	sei();
}

bool isPressed(uint8_t pinb, int button) {
	return !(pinb & (1 << button));
}

bool anyPressed() {
  	return isPressed(PINB, BUTTON1) || isPressed(PINB, BUTTON2) || isPressed(PINB, BUTTON3);
}

void toggle(int led) {
  	PORTD ^= (1 << led);
}

void enableTimer0Interrupts() {
  	TIMSK0 |= (1 << TOIE0);
}

void disableTimer0Interrupts() {
  	TIMSK0 &= ~(1 << TOIE0);
}

void reset() {
	if (LED_STATE != 0) {
		disableTimer0Interrupts();
		LED_STATE = 0;
		PORTD = 0;
	}
}

void changeState(uint8_t pinb) {

	if ( isPressed(pinb, BUTTON1) && !isPressed(pinb, BUTTON2) && !isPressed(pinb, BUTTON3) ) {
		reset();
		toggle(LED1);
	
	} else if ( !isPressed(pinb, BUTTON1) && isPressed(pinb, BUTTON2) && !isPressed(pinb, BUTTON3) ) {
		reset();
	  	toggle(LED2);
	
	} else if ( !isPressed(pinb, BUTTON1) && !isPressed(pinb, BUTTON2) && isPressed(pinb, BUTTON3) ) {
		reset(); 
		toggle(LED3);
	
	} else if ( isPressed(pinb, BUTTON1) && isPressed(pinb, BUTTON2) && !isPressed(pinb, BUTTON3) ) {
		if (LED_STATE != 1) {
	  		PORTD = 0;
			LED_STATE = 1;
		  	enableTimer0Interrupts();
		}
	
	} else if ( isPressed(pinb, BUTTON1) && !isPressed(pinb, BUTTON2) && isPressed(pinb, BUTTON3) ) {
		if (LED_STATE != 2) {
			PORTD = 0;
			LED_STATE = 2; 
			enableTimer0Interrupts();
		}
	} 
}

void toggleAllLeds() {
	PORTD ^= (1 << LED1) | (1 << LED2) | (1 << LED3);
}

void blinkLedsOneByOne() {
	if (PORTD == (1 << LED1)) {
		PORTD = 0;
		PORTD |= (1 << LED2);
	} else if (PORTD == (1 << LED2)) {
		PORTD = 0;
		PORTD |= (1 << LED3);
	} else {
		PORTD = 0;
		PORTD |= (1 << LED1);
	}
}

int main(void) {
	
	setUpIO();
	setUpTimers();
	setUpInterrupts();
	
	while (1);
}

ISR(TIMER1_COMPA_vect) {
	
	static uint8_t counter = 0;
	static bool buttonClicked = false;
	static bool buttonDebounced = false;
	static bool actionPerformed = false;
	
	uint8_t pinb = PINB;
	
	if (!buttonClicked && anyPressed()) {
		buttonClicked = true;
		buttonDebounced = false;
		actionPerformed = false;
		counter = 80; // ~400ms
	}
		
	if (buttonClicked) {
		if (counter > 0) {
			counter--;
		  	if (counter == 76) {
				buttonDebounced = true;
			}
		} else {
			buttonClicked = false;
		}
	}
	
	if (!actionPerformed && buttonClicked && buttonDebounced) {
		pinb = PINB;
		changeState(pinb);
		actionPerformed = true;
	} 
}
	
ISR(TIMER0_OVF_vect) {
	static uint8_t counts = 0;
	if (counts == 30) { // ~500ms
		if (LED_STATE == 1) {
		toggleAllLeds();
		} else if (LED_STATE == 2) {
			blinkLedsOneByOne();
		}
		counts = 0;
	} else {
		counts++;
	}
}
