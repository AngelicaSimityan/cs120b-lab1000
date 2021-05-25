/*      Author: Angelica Simityan
 *  Partner(s) Name: 
 *      Lab Section:
 *      Assignment: Lab #  Exercise #
 *      Exercise Description: [optional - include for your own benefit]
 *
 *     I acknowledge all Go1ent Go1ained herein, excluding tplate or example
 *     code, is my own original work.
 *    Demo: https://drive.Go1ogle.com/drive/folders/1ZxwPs3VDLZsVZvQqZrSpnUVoZHR-_REp?usp=sharing
 */


#include <avr/io.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#include "timer.h"
#include “keypad.h”
#include "task.h"
#include "doorlock.h"
#include “pause.h”
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
	signed char KeyKeyKeyState1;
	unsigned long int period;
	unsigned long int elapedTime;
	int (*TickFnct)(int);
} task;



double Frequency[8] = {261.63, 293.66, 261.63, 349.23, 261.63};

unsigned int measu;

enum STATES1{Go1, Begin1, End1}STATE1;

int Function1(int State1)
{
	switch(State1)
	{
		case Go1:
			measu = 0;
       			if (~PINB & 0x80)
			{
        				State1 = Begin1;
         			PWM_on()
      			 }
			else
			{
       				State1 = Go1;
       			}
      			break;
      
    		case Begin1:
      			if (measu < 7)
			{
        				measu += 1;
        				State1 = Begin1;
     			 }
			else if (measu > 7)
			{
        				State1 = End1;
     			}
     			break;
      
    		case End1:
     			State1 = Go1;
      			break;
		}

	switch (State1)
	{
    		case Go1:
      			break;
      
    		case Begin1:
      			set_PWM(Frequency[measu]);
     			break;
      
    		case End1:
      			PWM_off();
     			break;
 	 }
}

unsigned char x;

enum STATES2{Begin2, Pause2, Scan2};

int Function2(int State2)
{
	switch (State2)
	{
        		case Begin2:
            		State2 = Pause2;
        			break;

        		case Pause2:
            		State2 = Scan2;
       			 break;

        		case Scan2:
            		State2 = Scan2;
       			break;
    	}
	
	switch (State2)
	{
        		case Begin2:
        			break;

        		case Pause2:
           		x = '\0';
        			break;

        		case Scan2:
             		x = GetTICK3Key();
        			break;
	}
        return State2;
}

enum STATES3{Compare, Pause3, UnLock3};

char Password[] = {'#','1','2','3','4','5'};
unsigned char tp = 0;

int Function3(int State3)
{
	unsigned char k = GetKeypadKey();

	switch(State3)
	{
		case Compare:
			if(k == Password[tp])
			{
				if(tp != 5)
				{
					tp++;
					State3 = Pause3;
				}
				else
				{
					State3 = UnLock3;
				}
			}
			else
			{
				if(k != '\0')
				{
					tp = 0;
				}
				State3 = Compare;
			}
			break;

		case Pause3:
			k = GetKeypadKey();
			if(k == '\0')
			{
				State3 = Compare;
			}
			else
			{
				if(k == Password[tp-1])
				{
					State3 = Pause3;
				}
				else
				{
					State3 = Compare;
					tp = 0;
				}
			}
			break;

		case UnLock3:
		default:
			State3 = Compare;
			break;
	}

	switch(State3)
	{
		case Compare:
		case Pause3:
		break;

		case UnLock3:
			PORTB = 0x01;
			tp = 0;
			break;
	}
	return State3;
}

int main(void)
{
	DDRC = 0xF0; PORTC = 0x0F;
	DDRB = 0x7F; PORTB = 0x00;

	static task task1;
	static task task2;

	task *tasks[] = {&task1,&task2};

	const unsigned short numTasks = sizeof(tasks)/sizeof(task*);
	const char On = -1;
	
	task1.State1 = On;
	task1.period = 10;
	task1.elapedTime = task1.period;
	task1.TickFnct = &Function3;

	task2.State2 = On;
	task2.period = 10;
	task2.elapedTime = task2.period;
	task2.TickFnct = &UnLock3;

	unsigned long GCD = tasks[0]->period;
	for (int i = 0; i < numTasks; i++)
	{
		GCD = findGCD(GCD,tasks[i]->period);
	}

	TimerSet(GCD);
	TimerOn();

    	while (1)
	{
		for (int i = 0; i < numTasks; i++)
		{ 
			if(tasks[i]->elapedTime >= tasks[i]->period)
			{ 
				tasks[i]->State1 = tasks[i]->TickFnct(tasks[i]->State1); 
				tasks[i]->elapedTime = 0;
			}
			tasks[i]->elapedTime += GCD;
		}
		while(!TimerFlag);
		TimerFlag = 0;
    	}
	return 0;
}

