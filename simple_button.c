#define F_CPU 16000000

#include <avr/io.h>

#define LED  PD7
#define BUTTON  PB0

int main(void) {

    // Set LED pin as output
    DDRD |= (1 << LED);
  
    // Set button pin as input
    DDRB &= ~(1 << BUTTON);
  
    // Enable internal pull-up resistor for button pin
    PORTB |= (1 << BUTTON);

    while (1) {
        // Check if button is pressed
        if (!(PINB & (1 << BUTTON))) {
            // Turn on the LED
            PORTD |= (1 << LED);
        }
        else {
            // Turn off the LED
            PORTD &= ~(1 << LED);
        }
    }
}