/*      Author: Angelica Simityan
 *  Partner(s) Name: 
 *      Lab Section:
 *      Assignment: Lab #  Exercise #
 *      Exercise Description: [optional - include for your own benefit]
 *
 *     I acknowledge all content contained herein, excluding template or example
 *     code, is my own original work.
 *    Demo: https://drive.google.com/drive/folders/1ZxwPs3VDLZsVZvQqZrSpnUVoZHR-_REp?usp=sharing
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
	signed char KeyState;
	unsigned long int period;
	unsigned long int elapedTime;
	int (*TickFnct)(int);
} task;

unsigned long int findGCD (unsigned long int a, unsigned long int b) {
	unsigned long int c;
	while(1){
		c = a%b;
		if(c == 0){
			return b;
		}
		a = b;
		b = c;
	}
	return 0;
}

//Keypad
unsigned char x;

enum STATES1{Begin1, Pause1, Scan1};

int Function1(int State1)
{
	switch (State1)
	{
 		case Begin1:
			State1 = Pause1;
			break;

		case Pause1:
			State1 = Scan1;
			break;

		case Scan1:
			State1 = Scan1;
			break;
    }
	switch (State1)
	{
		case Begin1:
			break;

		case Pause1:
			x = '\0';
			break;
		
		case Scan1:
			x = GetKeypadKey();
			break;
    }

    return State1;
}

//Lock
char Password[] = {'#','1','2','3','4','5'};

enum STATES2{Lock2, Pause2};

int Function2(int State2)
{
	unsigned char input = ~PINB & 0x80;
	
	switch(State2)
	{
		case Pause2:
			if(input == 0x80)
			{
				State2 = Lock2;
			}
			else
			{
				State2 = Pause2;
			}
			break;

		case Lock2:
			if(input == 0x80)
			{
				State2 = Lock2;
			}
			else
			{
				State2 = Pause2;
			}
			break;

		default:
			State2 = Pause2;
			break;
	}

	switch(State2)
	{
		case Pause2:
			break;

		case Lock2:
			PORTB = 0x00;
			break;
	}
	return State2;
}

int main(void)
{
	DDRC = 0xF0; PORTC = 0x0F;
	DDRB = 0x7F; PORTB = 0x00;

	static task Tasks1;
	static task Tasks2;

	task *tasks[] = {&Tasks1,&Tasks2};
	const unsigned short numTasks = sizeof(tasks)/sizeof(task*);
	const char start = -1;
	

	Tasks1.State1 = start;
	Tasks1.period = 10;
	Tasks1.elapsedTime = Tasks1.period;
	Tasks1.TickFct = &Function1;

	Tasks2.State2 = start;
	Tasks2.period = 10;
	Tasks2.elapsedTime = Tasks2.period;
	Tasks2.TickFct = &Function2;

	unsigned long GCD = tasks[0]->period;

	for (int i = 0; i < numTasks; i++)
	{
		GCD = findGCD(GCD,tasks[i]->period);
	}

	TimerSet(GCD);
	TimerOn();

	 unsigned short i;

	while (1)
	{
		for (i = 0; i < numTasks; i++)
		{
			if (tasks[i]->elapsedTime == tasks[i]->period)
			{
				tasks[i]->KeyState = tasks[i]->TickFct(tasks[i]->KeyState);
				tasks[i]->elapsedTime = 0;
			}
			tasks[i]->elapsedTime += GCD;
		}
		while (!TimerFlag);
		TimerFlag = 0;
	}
	return 0;

}
