#define F_CPU 16000000

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>

#define LED1 PD0
#define LED2 PD1
#define LED3 PD2
#define BUTTON1 PB0
#define BUTTON2 PB1
#define BUTTON3 PB2

void setUpIO() {
    DDRD |= (1 << LED1) | (1 << LED2) | (1 << LED3);
    DDRB &= ~((1 << BUTTON1) | (1 << BUTTON2) | (1 << BUTTON3));
    PORTB |= (1 << BUTTON1) | (1 << BUTTON2) | (1 << BUTTON3);
}

void setUpTimer() {
    TCCR1B |= (1 << WGM12) | (1 << CS12) | (1 << CS10);
    OCR1A = 78; // ~5ms
}

void setUpInterrupts() {
    TIMSK1 |= (1 << OCIE1A);
    sei();
}

bool isPressed(uint8_t pinb, uint8_t button) {
    return !(pinb & (1 << button));
}

bool anyPressed() {
    return isPressed(PINB, BUTTON1) || isPressed(PINB, BUTTON2) || isPressed(PINB, BUTTON3);
}

ISR(TIMER1_COMPA_vect) {
    static uint8_t buttonPressed = 0;
    static uint8_t buttonDebounced = 0x7;
    static uint8_t readyForRelease = 0;
    static uint8_t debounceCounter[8];
    static uint8_t pressedCounter[8];
    static uint8_t pinb;

    pinb = PINB;

    for (uint8_t i = 0; i <= 2; i++) {
        if ((!(buttonPressed & (1 << i))) && isPressed(pinb, i)) {
            buttonPressed |= (1 << i);
            buttonDebounced &= ~(1 << i);
            debounceCounter[i] = 5;
            pressedCounter[i] = 0;
        }

        if (buttonPressed & (1 << i)) {
            pressedCounter[i]++;
        }

        if ((buttonPressed & (1 << i)) && (!isPressed(pinb, i)) && (buttonDebounced & (1 << i))) {
            readyForRelease |= (1 << i);
            buttonDebounced &= ~(1 << i);
            debounceCounter[i] = 5;
        }

        if (debounceCounter[i] > 0) {
            debounceCounter[i]--;
        } else {
            buttonDebounced |= (1 << i);
            if (readyForRelease & (1 << i)) {
                readyForRelease &= ~(1 << i);
                buttonPressed &= ~(1 << i);
                PORTD ^= (1 << i);
            }
        }
    }
}

int main(void) {

    setUpIO();
    setUpTimer();
    setUpInterrupts();

    while (1) {
        
    }
}
