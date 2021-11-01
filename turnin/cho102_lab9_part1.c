/*	Author: lab
 *  Partner(s) Name: Cindy Ho
 *	Lab Section:
 *	Assignment: Lab #9  Exercise #1
 *	Exercise Description: [optional - include for your own benefit]
 *	Demo Link: https://drive.google.com/file/d/1UC1WXGkgtgJDbu-6kyPnNq-kHRkQXsOo/view?usp=sharing
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 */
#include <avr/io.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
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

enum States{Start, Init, C_Press, D_Press, E_Press}state;

void Tick() {
	switch(state) {
		case Start:
			state = Init;
			break;
		case Init :
			if ((~PINA & 0x07) == 0x01) { 
				state = C_Press; 
			}
			else if ((~PINA & 0x07) == 0x02) { 
				state = D_Press; 
			}
			else if ((~PINA & 0x07) == 0x04) { 
				state = E_Press; 
			}
			else { 
				state = Init; 
			}
			break;
		case C_Press:
 			if ((~PINA & 0x07) == 0x01) { 
				state = C_Press; 
			}
			else { 
				state = Init; 
			}
			break;
		case D_Press:
			if ((~PINA & 0x07) == 0x02) { 
				state = D_Press; 
			}
			else { 
				state = Init; 
			}
			break;
		case E_Press:
			if ((~PINA & 0x07) == 0x04) { 
				state = E_Press; 
			}
			else { 
				state = Init; 
			}
			break;
		default:
			break;
	}
	switch(state) {
		case Start:
			break;
		case Init:
			set_PWM(0);
			break;
		case C_Press:
			set_PWM(261.63);
			break;
		case D_Press:
			set_PWM(293.66);
			break;
		case E_Press:
			set_PWM(329.63);
			break;
		default:
			break;
	}
}

int main(void) {
	/* Insert DDR and PORT initializations */
	DDRA = 0x00; PORTA = 0xFF;
	DDRB = 0xFF; PORTB = 0x00;

	/* Insert your solution below */
	state = Start;
	PWM_on();
	while (1) {
		Tick();
	}
	return 1;
}
