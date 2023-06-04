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

volatile uint8_t BUTTON_CLICKED = 0;
volatile uint8_t LED_STATE = 0;

void setUpIO() {
	DDRD |= (1 << LED1) | (1 << LED2) | (1 << LED3);
	DDRB &= ~((1 << BUTTON1) | (1 << BUTTON2) | (1 << BUTTON3));
	PORTB |= (1 << BUTTON1) | (1 << BUTTON2) | (1 << BUTTON3);
}

void setUpTimer() {
	TCCR1B |=  (1 << WGM12) | (1 << CS12) | (1 << CS10);
  // ~20ms
	OCR1A = 312;
}

void setUpInterrupts() {
	TIMSK1 |= (1 << OCIE1A);
	sei();
}

bool isPressed(uint8_t pinb, int button) {
	return !(pinb & (1 << button));
}

bool isNotPressed(uint8_t pinb, int button) {
	return (pinb & (1 << button));
}

void toggle(int led) {
  PORTD ^= (1 << led);
}


void controlOutputs(uint8_t pinb) {

  if ( isPressed(pinb, BUTTON1) && isNotPressed(pinb, BUTTON2) && isNotPressed(pinb, BUTTON3) ) {
  
  toggle(LED1);
  
  } else if ( isNotPressed(pinb, BUTTON1) && isPressed(pinb, BUTTON2) && isNotPressed(pinb, BUTTON3) ) {
  
	toggle(LED2);
  
  } else if ( isNotPressed(pinb, BUTTON1) && isNotPressed(pinb, BUTTON2) && isPressed(pinb, BUTTON3) ) {
  
	toggle(LED3);
  
  } else if ( isPressed(pinb, BUTTON1) && isPressed(pinb, BUTTON2) && isNotPressed(pinb, BUTTON3) ) {
	
	toggle(LED3);
  
  }
}

int main(void) {
	
	setUpIO();
	setUpTimer();
	setUpInterrupts();
	
	while (1) {

	}
}

ISR(TIMER1_COMPA_vect) {
	
	static uint8_t counter = 0;
	
	uint8_t pinb = PINB;

	if (!BUTTON_CLICKED && ( isPressed(pinb, BUTTON1) || isPressed(pinb, BUTTON2) || isPressed(pinb, BUTTON3) )) {
		BUTTON_CLICKED = 1;
		counter = 20; // ~400ms
		controlOutputs(pinb);
	} 
		
	if (BUTTON_CLICKED == 1) {
		if (counter > 0) {
			counter--;
		} else {
			BUTTON_CLICKED = 0;
		}
	}
}



