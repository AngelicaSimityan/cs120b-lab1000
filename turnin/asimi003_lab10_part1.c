/*      Author: Angelica Simityan
 *  Partner(s) Name: 
 *      Lab Section:
 *      Assignment: Lab #  Exercise #
 *      Exercise Description: [optional - include for your own benefit]
 *
 *     I acknowledge all content contained herein, excluding template or example
 *     code, is my own original work.
 *    Demo:
 */

#include <avr/io.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#include "timer.h"
#include "keypad.h"
#include "task.h"
#include "doorlock.h"
#include "pause.h"
#include "gcd.h"
#endif

volatile unsigned char TimerFlag = 0;

unsigned long _avr_timer_M = 1;
unsigned long _avr_timer_cntcurr = 0;

void TimerOn ()
{
	TCCR1B = 0x0B;
	OCR1A = 125;
	TIMSK1 = 0x02;
	TCNT1 = 0;
	_avr_timer_cntcurr = _avr_timer_M;
	SREG |= 0x80;
}

void TimerOff()
{
	TCCR1B = 0x00;
}

void TimerISR()
{
	TimerFlag = 1;
}

ISR(TIMER1_COMPA_vect)
{
	_avr_timer_cntcurr--;
	if (_avr_timer_cntcurr == 0)
	{ 
		TimerISR();
		_avr_timer_cntcurr = _avr_timer_M;
	}
}

void TimerSet(unsigned long M)
{
	_avr_timer_M = M;
	_avr_timer_cntcurr = _avr_timer_M;
}

typedef struct _task
{
	signed char State1;
	unsigned long int period;
	unsigned long int elapedTime;
	int (*TickFnct)(int);
} task; 

unsigned char x = 0x00;

enum STATES1{Begin, Pause, Type, NotPressed}STATE;

int KeyFunction(int State1)
{
	x = GetKeypadKey();

	switch(State1)
	{
		case Begin:
			State1 = Pause;
			break;

		case Pause:
			if(x)
			{
				State1 = Type;
			}
			else
			{
				State1 = Pause;
			}
			break;

		case Type:
			State1 = NotPressed;
			break;

		case NotPressed:
			if(x)
			{
				State1 = NotPressed;
			}
			else
			{
				State1 = Pause;
			}
			break;

		default:
			State1 = Begin;
			break;
	}

	switch(State1)
	{
		case Type:
			switch(x)
			{
				case '\0':
					PORTB = 0x1F;
					break;

				case '1':
					PORTB = 0x01 | 0x80;
					break;

				case '2':
					PORTB = 0x02 | 0x80;
					break;

				case '3':
					PORTB = 0x03 | 0x80;
					break;

				case '4':
					PORTB = 0x04 | 0x80;
					break;

				case '5':
					PORTB = 0x05 | 0x80;
					break;

				case '6':
					PORTB = 0x06 | 0x80;
					break;

				case '7':
					PORTB = 0x07 | 0x80;
					break;

				case '8':
					PORTB = 0x08 | 0x80;
					break;

				case '9':
					PORTB = 0x09 | 0x80;
					break;

				case 'A':
					PORTB = 0x0A | 0x80;
					break;

				case 'B':
					PORTB = 0x0B | 0x80;
					break;

				case 'C':
					PORTB = 0x0C | 0x80;
					break;

				case 'D':
					PORTB = 0x0D | 0x80;
					break;

				case '*':
					PORTB = 0x0E | 0x80;
					break;

				case '0':
					PORTB = 0x00 | 0x80;
					break;

				case '#':
					PORTB = 0x0F | 0x80;
					break;

				default:
					PORTB = 0x1B;
					break;
			}
			break;

		case NotPressed:
			0x00 = 1;
			break;
		
		Begin:
		Pause:
		Type:
		default:
			break;
	}

	if(0x00)
	{
		PORTB = 0x80;
	}
	else
	{
		PORTB = 0x00;
	}
	return State1;
}

int main()
{
	/* 0x00 */
	DDRB = 0xFF;
	PORTB = 0x00;

	/* Output */
	DDRC = 0xF0;
	PORTC = 0x0F;
	
	int TaskNumCount = 1;
	
	task TaskNum[1];
	TaskNum[0].State1 = Begin;
	TaskNum[0].period = 100;
	TaskNum[0].elapedTime = TaskNum[0].period;
	TaskNum[0].TickFnct = &KeyFunction;

	TimerSet(100);
	TimerOn();

	while (1)
	{
		for(unsigned int i = 0; i < TaskNumCount; i++)
		{
			if(TaskNum[i].elapedTime >= TaskNum[i].period)
			{
				TaskNum[i].State1 = TaskNum[i].TickFnct(TaskNum[i].State1);
				TaskNum[i].elapedTime = 0;
			}
			TaskNum[i].elapedTime += 100;
		}
	
		while(!TimerFlag);
		TimerFlag = 0;
	}
	return 1;
}
