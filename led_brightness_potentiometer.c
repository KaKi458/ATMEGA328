#define F_CPU 16000000

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>

#define LED1 PD3
#define LED2 PD5
#define LED3 PD6
#define BUTTON1 PB0
#define BUTTON2 PB1
#define BUTTON3 PB2

volatile uint16_t adcValue; 
volatile uint8_t analogMode = 0;

void setUpIO() {
  DDRD |= (1 << LED1) | (1 << LED2) | (1 << LED3);
  DDRB &= ~((1 << BUTTON1) | (1 << BUTTON2) | (1 << BUTTON3));
  PORTB |= (1 << BUTTON1) | (1 << BUTTON2) | (1 << BUTTON3);
}

void setUpTimer() {
  TCCR1B |= (1 << WGM12) | (1 << CS12) | (1 << CS10);
  OCR1A = 78;  // ~5ms
}

void setUpADC() {
  ADMUX |= (1 << REFS0) | (1 << MUX2) | (1 << MUX0);
  ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
  ADCSRA |= (1 << ADEN) | (1 << ADSC);;
}

void setUpInterrupts() {
  TIMSK1 |= (1 << OCIE1A);
  ADCSRA |= (1 << ADIE);
  sei();
}

void setUpPWM() {
    TCCR0A |= (1 << WGM01) | (1 << WGM00);
    TCCR0A |= (1 << COM0A1) | (1 << COM0A0) | (1 << COM0B1) | (1 << COM0B0);
    TCCR0B |= (1 << CS01);
    OCR0A = 255;
    OCR0B = 255;

    TCCR2A |= (1 << WGM20) | (1 << WGM21);
    TCCR2A |= (1 << COM2B1) | (1 << COM0B0);
    TCCR2B |= (1 << CS21);
    OCR2B = 255;
}

uint8_t getRegister(uint8_t i) { 
  if (i == 0) {
    return &OCR2B;
  } else if (i == 1) {
    return &OCR0B;
  } else {
    return &OCR0A;
  }
}

void toggleLed(uint8_t i) {
  uint8_t *ledRegister = getRegister(i);
  if (*ledRegister == 255) {
    *ledRegister = 0;
  } else {
    *ledRegister = 255;
  }
}

bool isPressed(uint8_t pinb, uint8_t button) {
  return !(pinb & (1 << button));
}

void changeState(uint8_t i) {
  if (analogMode & (1 << i)) {
    analogMode &= ~(1 << i);
    uint8_t *reg = getRegister(i);
    *reg = 255;
  } else {
    toggleLed(i);
  }
}

void turnOnAnalogMode(uint8_t i) {
  analogMode |= (1 << i);
  uint8_t *reg = getRegister(i);
  *reg = adcValue;
}

ISR(TIMER1_COMPA_vect) {
  static uint8_t buttonPressed = 0;
  static uint8_t buttonDebounced = 0x7;
  static uint8_t readyForRelease = 0;
  static uint8_t debounceCounter[8];
  static uint8_t pressedCounter[8];
  uint8_t pinb;

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
        if (pressedCounter[i] > 50 && !(analogMode & (1 << i))) {
          turnOnAnalogMode(i);
        } else {
          changeState(i);
        }
      }
    }
  }
}

ISR(ADC_vect) {
    adcValue = ADC >> 2;    
    ADCSRA |= (1 << ADSC);
}

int main(void) {

  setUpIO();
  setUpTimer();
  setUpADC();
  setUpPWM();
  setUpInterrupts();

  while (1) {
    for (uint8_t i = 0; i <= 2; i++) {
      if (analogMode & (1 << i)) {
        uint8_t *reg = getRegister(i);
        *reg = adcValue;
      }
    }
  }
}
