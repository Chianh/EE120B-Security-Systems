#ifndef __MOTOR_H__
#define __MOTOR_H__

#include <avr/io.h>

void motor_unlock() {
	OCR1A = 97;
}

void motor_lock() {
	OCR1A = 207;
}

void motor_on() {

	TCCR1A|=(1<<COM1A1)|(1<<COM1B1)|(1<<WGM11); 
	TCCR1B|=(1<<WGM13)|(1<<WGM12)|(1<<CS11)|(1<<CS10);
	motor_unlock();
}

void motor_off() {
	TCCR0A = 0x00;
	TCCR0B = 0x00;
}
#endif