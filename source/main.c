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

enum States{Start, Init, On, Wait}state;
double seq[18] = {261.63,261.63, 293.66, 329.63, 329.63, 293.66, 261.63, 261.63, 329.63, 329.63, 261.63, 261.63, 293.66, 261.63, 329.63, 293.66, 293.66, 261.63};
char beatCount[18] = {2, 2, 4, 2, 2, 4, 2, 2, 2, 2, 2, 2, 4, 2, 2, 4, 4, 2};
int tempCount[18] = {2, 2, 4, 2, 2, 4, 2, 2, 2, 2, 2, 2, 4, 2, 2, 4, 4, 2};
//{3, 3, 6, 3, 3, 6, 3, 3, 3, 3, 3, 3, 6, 3, 3, 6, 6, 3};
unsigned char counter = 0;
unsigned char j;

void Tick() {
	switch(state) {
		case Start:
			state = Init;
			set_PWM(0);
			break;
		case Init:
			for(unsigned char k = 0; k < 18; ++k) {
				tempCount[k] = beatCount[k];
			}
			if ((~PINA & 0x01) == 0x01) { state = On; }
			else { state = Init; }
			break;
		case On:
			if (counter >= 18) { state = Wait; }
			else { state = On; }
			break;
		case Wait:
			if ((~PINA & 0x01) == 0x00) { state = Init; }
			else { state = Wait; }
	}
	switch(state) {
		case Start:
			break;
		case Init:
			counter = 0;
			j = 0;
			break;
		case On:
			if (j % 2 == 0) {
				--tempCount[counter];
				set_PWM(seq[counter]);
				if(tempCount[counter] == 0) {
					++j;
					++counter;
				}
			}
			else { set_PWM(0); ++j; }
			break;
		case Wait:
			set_PWM(0);
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

