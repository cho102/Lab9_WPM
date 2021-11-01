/*	Author: lab
 *  Partner(s) Name: Cindy Ho
 *	Lab Section:
 *	Assignment: Lab #9  Exercise #2
 *	Exercise Description: [optional - include for your own benefit]
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 */
#include <avr/io.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#include <avr/interrupt.h>

#endif

void set_PWM(double frequency) {
	static double current_frequency;
	if (frequency != current_frequency) {
		if (!frequency) { TCCR3B &= 0x08; }
		else { TCCR3B |= 0x03; }
		if (frequency < 0.954) { OCR3A = 0xFFFF; }
		else if (frequency > 31250) { OCR3A = 0x0000; }
		else { OCR3A = (short)(8000000 / (128 * frequency)) - 1; }
		TCNT3 = 0;
		current_frequency = frequency;
	}
}

void PWM_on() {
	TCCR3A = (1 << COM3A0);
	TCCR3B = (1 << WGM32) | (1 << CS31) | (1 << CS30);
	set_PWM(0);
}

void PWM_off() {
	TCCR3A = 0x00;
	TCCR3B = 0x00;
}


volatile unsigned char TimerFlag = 0;

unsigned long _avr_timer_M = 1;
unsigned long _avr_timer_cntcurr = 0;

void TimerOn() {
	TCCR1B = 0x0B;
	OCR1A = 125;
	TIMSK1 = 0x02;
	TCNT1 = 0;
	_avr_timer_cntcurr = _avr_timer_M;
	SREG |= 0x80;
}

void TimerOff() {
	TCCR1B = 0x00;
}

void TimerISR() {
	TimerFlag = 1;
}

ISR(TIMER1_COMPA_vect) {
	_avr_timer_cntcurr--;
	if (_avr_timer_cntcurr == 0) {
		TimerISR();
		_avr_timer_cntcurr = _avr_timer_M;
	}
}

void TimerSet(unsigned long M) {
	_avr_timer_M = M;
	_avr_timer_cntcurr = _avr_timer_M;
}

enum States{Start, Init, On, Increment, Decrement, Wait}state;

double counter = 0;
unsigned char power = 0;

void Tick() {
	switch(state) {
		case Start:
			state = Init;
			set_PWM(0);
			break;
		case Init:
			if ((~PINA & 0x07) == 0x01) 
			{ 
				state = On; 
			}
			else { 
				state = Init; 
			}
			break;
		case On:
 			if ((~PINA & 0x07) == 0x01) { 
				state = Init; 
			}
			else if ((~PINA & 0x07) == 0x02) { 
				state = Increment; 
			}
			else if ((~PINA & 0x07) == 0x04) { 
				state = Decrement; 
			}
			else { state = On; }
			break;
		case Increment:
			//if (counter < 8) {
			//	++counter;
			//}
			state = Wait;
			break;
		case Decrement:
			//if (counter > 1) {
			//	++counter;
			//}
			state = Wait;
			break;
		case Wait:
			if ((~PINA & 0x07) == 0x00) {
				if (power) { state = On; }
				else { state = Init; }
			}
			else { state = Wait; }
			break;
	}
	switch(state) {
		case Start:
			break;
		case Init:
			power = 0;
			set_PWM(0);
			counter = 261.63;
			break;
		case On:
			power = 1;
	//		counter = 1;
			set_PWM(counter);
			break;
		case Increment:
			if (counter == 261.63) { 
				counter = 293.66; 
			}
			else if (counter == 293.66) { 
				counter = 329.63; 
			}
			else if (counter == 329.63) { 
				counter = 349.23; 
			}
			else if (counter == 349.23) { 
				counter = 392.00; 
			}
			else if (counter == 392.00) { 
				counter = 440.00; 
			}
			else if (counter == 440.00) { 
				counter = 493.88; 
			}
			else if (counter == 493.88) { 
				counter = 523.25; 
			}
			if (power) { 
				set_PWM(counter);
			}
			break;
		case Decrement:
			if (counter == 523.25) { 
				counter = 493.88;
			}
			else if (counter == 493.88) { 
				counter = 440.00; 
			}
			else if (counter == 440.00) { 
				counter = 392.00; 
			}
			else if (counter == 392.00) { 
				counter = 349.23; 
			}
			else if (counter == 349.23) { 
				counter = 329.63; 
			}
			else if (counter == 329.63) { 
				counter = 293.66; 
			}
			else if (counter == 293.66) { 
				counter = 261.63; 
			}
			if (power) { 
				set_PWM(counter); 
			}
			break;
		case Wait:
			break;
	}
}

int main(void) {
	DDRA = 0x00; PORTA = 0xFF;
	DDRB = 0xFF; PORTB = 0x00;
	state = Start;
	PWM_on();
	TimerSet(100);
	TimerOn();
	while (1) {
		Tick();
		while (!TimerFlag) { }
		TimerFlag = 0;
	}
	return 1;
}

