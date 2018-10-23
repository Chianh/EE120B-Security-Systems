/*
 * cwu048_Security.c
 *
 * Created: 3/3/2018 8:43:49 PM
 *  Author: chianh
 */ 


#include <avr/io.h>
#include <avr/eeprom.h> 
#include <util/delay.h>
#include <avr/interrupt.h>
#include "nokia5110.h"
#include "timer.h"
#include "keypad.h"
#include "Speaker.h"
#include "Motor.h"

#define read_eeprom_byte(address) eeprom_read_byte ((const uint8_t*)address)
#define write_eeprom_byte(address,value) eeprom_write_byte ((uint8_t*)address,(uint8_t)value)
#define update_eeprom_byte(address,value) eeprom_update_byte ((uint8_t*)address,(uint8_t)value)

unsigned char adminFlag = 0x00;
unsigned char motor_angle = 0x00;

enum SM_Lock_States{lock_start, lock_locked, lock_unlocked, lock_release, lock_reset} lock_state;
char code[4] = {'1','2','3','4'}; //temporary array holding 4 values, values will be overwritten from EEPROM
unsigned char code_index = 0x00;
unsigned char fail = 0x00;
unsigned char lock_status = 0x00;
void SM_Lock_Tick(){
	unsigned char keypad_entry = GetKeypadKey();
	switch(lock_state){
		case lock_start:
		nokia_lcd_set_cursor(0, 0);
		nokia_lcd_write_string("Input passcode", 1);
		nokia_lcd_render();
		code_index = 0;
		fail = 0;
		lock_status = 0;
		motor_angle = 0;
		if (motor_angle != 97) {
		lock_state = lock_unlocked;
		motor_angle = 97;
		}
		motor_unlock();

		break;
		
		case lock_locked:
		nokia_lcd_set_cursor(0, 0);
		nokia_lcd_write_string("Input passcode", 1);
		nokia_lcd_render();
		if (keypad_entry == '\0') {
			lock_state = lock_locked; //do nothing because no entry.
		}
		else {
			lock_state = lock_release; //begin process to unlock the lock.
			if (keypad_entry == code[code_index] && fail == 0) {
				nokia_lcd_set_cursor(code_index*10, 12);
				nokia_lcd_write_char(keypad_entry, 2);
				nokia_lcd_render();
				code_index++;
			}
			else{
				//Accepts wrong input
				nokia_lcd_set_cursor(code_index*10, 12);
				nokia_lcd_write_char(keypad_entry, 2);
				nokia_lcd_render();
				fail = 1;
				if (code_index == 3) {
					nokia_lcd_clear();
					nokia_lcd_render();
					nokia_lcd_set_cursor(0, 10);
					nokia_lcd_write_string("WRONG", 2);
					nokia_lcd_render();
					_delay_ms(5000);
					_delay_ms(5000);
					nokia_lcd_clear();
					nokia_lcd_render();
					fail = 1;
					code_index = -1;
					lock_state = lock_reset;
				}
				if(lock_status == 1) {
					nokia_lcd_set_cursor(2, 30);
					nokia_lcd_write_string("Locked! ", 1);
					nokia_lcd_drawSmile();
					nokia_lcd_render();
				}
				if (lock_status == 0) {
					nokia_lcd_set_cursor(2, 30);
					nokia_lcd_write_string("Unlocked!", 1);
					nokia_lcd_drawSmile();
					nokia_lcd_render();
					
				}
				
				code_index++;	
			}
		}
		break;
		
		case lock_release:
		if (keypad_entry != '\0') { //There should not be any entries into the keypad in this state.
			lock_state = lock_release;
		}
		else if (keypad_entry == '\0' && code_index <=3) { //Incomplete pass code entry. Go back to fetch the rest.
			lock_state = lock_locked;
		}
		else if (keypad_entry == '\0' && code_index == 4 && fail == 0) { //Password accepted.
			nokia_lcd_clear();
			nokia_lcd_render();
			nokia_lcd_set_cursor(2, 30);
			
			if (lock_status == 1) {
			nokia_lcd_write_string("Unlocked!", 1); //Lets the user know they have unlocked the system.
			nokia_lcd_drawSmile();
			nokia_lcd_render();

			if (motor_angle != 97) {
			motor_unlock();
			motor_angle = 97;
			}
			lock_state = lock_unlocked;
			lock_status = 0;
			code_index = 0;
			}
			
			else {
			nokia_lcd_clear(); //added clear command.
			nokia_lcd_set_cursor(2, 30);
			nokia_lcd_write_string("Locked! ", 1);
			nokia_lcd_drawSmile();
			nokia_lcd_render();
			if (motor_angle != 206) {
			motor_lock();
			motor_angle = 206;
			}
			lock_state = lock_unlocked;
			lock_status = 1;
			code_index = 0;
			}
		}
		else {
			lock_state = lock_reset;
		}
		break;
		
		case lock_reset:
		fail = 0;
		lock_state = lock_locked;
		break;
		
		
		case lock_unlocked:
		nokia_lcd_set_cursor(0, 0);
		nokia_lcd_write_string("Input passcode", 1);
		nokia_lcd_render();
		if (keypad_entry == '\0') {
			lock_state = lock_unlocked; //do nothing because no entry.
		}
		else {
			lock_state = lock_release; //begin process to unlock the lock.
			if (keypad_entry == code[code_index]) {
				nokia_lcd_set_cursor(code_index*10, 12);
				nokia_lcd_write_char(keypad_entry, 2);
				nokia_lcd_render();
				code_index++;
				
				if(lock_status == 1) {
					nokia_lcd_set_cursor(2, 30);
					nokia_lcd_write_string("Locked! ", 1);
					nokia_lcd_drawSmile();
					nokia_lcd_render();
					
				}
				if (lock_status == 0) {
					nokia_lcd_set_cursor(2, 30);
					nokia_lcd_write_string("Unlocked!", 1);
					nokia_lcd_drawSmile();
					nokia_lcd_render();
					
				}
			}
			else{
				//ADDED THIS:
				nokia_lcd_set_cursor(code_index*10, 12);
				nokia_lcd_write_char(keypad_entry, 2);
				nokia_lcd_render();
				//Accepts wrong inputs.
				//nokia_lcd_clear();
				//nokia_lcd_render();
				if(lock_status == 1) {
					nokia_lcd_set_cursor(2, 30);
					nokia_lcd_write_string("Locked! ", 1);
					nokia_lcd_drawSmile();
					nokia_lcd_render();
					
				}
				if (lock_status == 0) {
					nokia_lcd_set_cursor(2, 30);
					nokia_lcd_write_string("Unlocked!", 1);
					nokia_lcd_drawSmile();
					nokia_lcd_render();
					
				}
				
				code_index++;
			}
		}
		break;
	}
	
}


enum SM_Motion_States{NoMotion,YesMotion}motion_state;
unsigned char motion_detect = 0x00;
void SM_Sensor_Tick(){
	unsigned motion_detect = (PIND & 0x01); //PIR sensor detects motion: Returns 1; PIR sensor does not detect motion: Returns 0;
	switch(motion_state){		
		case NoMotion:
		if(motion_detect){
			motion_state = YesMotion;
			if (lock_status == 1) {
			nokia_lcd_clear();
			nokia_lcd_render();
			nokia_lcd_set_cursor(0, 0);
			nokia_lcd_write_string("Input passcode", 1);
			nokia_lcd_render();
			nokia_lcd_set_cursor(0,40);
			nokia_lcd_write_string("DANGER!", 1);
			nokia_lcd_render();
			code_index = 0;
			
			}
		}
		else {
		motion_state = NoMotion;
		}
		break;
		
		case YesMotion:
		if(motion_detect){
			motion_state = YesMotion;
		}
		else {
		motion_state = NoMotion;
		}
		break;
	}
	switch(motion_state){		
		case NoMotion:
		break;
		case YesMotion:
		break;
	}
}

enum sm_Speaker_States{speakerOn, speakerOff} speaker_state;
void SM_Speaker_Tick() {
	switch(speaker_state) {
		case speakerOn:
		if (lock_status == 0) {
		speaker_state = speakerOff;
		nokia_lcd_clear();
		nokia_lcd_render();
		nokia_lcd_set_cursor(2, 10);
		nokia_lcd_write_string("DISARMED", 1);
		nokia_lcd_render();
		_delay_ms(5000);
		nokia_lcd_clear();
		nokia_lcd_set_cursor(2, 30);
		nokia_lcd_write_string("Unlocked! ", 1);
		nokia_lcd_drawSmile();
		nokia_lcd_render();
		nokia_lcd_set_cursor(0, 0);
		nokia_lcd_write_string("Input passcode", 1);
		nokia_lcd_render();
		}
		else {
			speaker_state = speakerOn;
		}
		break;
		
		case speakerOff:
		if (lock_status == 1 && motion_state == YesMotion) { //if system is currently locked, trigger the alarm.
			speaker_state = speakerOn;
		}
		else {
			speaker_state = speakerOff;
		}
		break;
	}
	
	switch(speaker_state){
		case speakerOn:
		set_PWM(800);
		break;
		case speakerOff:
		set_PWM(0);
		break;
	}
	};
	/////////////////////////////////////////
	/////////////////////////////////////////
	unsigned char counter = 0;
	unsigned char newcodeIndex = 0x00;
	unsigned char target = 0x00;
	unsigned char writeflag = 0;
	unsigned char correct = 0;
	unsigned char stateFlag = 0;
	enum SM_EEPROM_STATES {EEPROM_Start,EEPROM_NewCode, EEPROM_Reset, EEPROM_Release, EEPROM_Release_Start, EEPROM_Done, EEPROM_Release_Correct} EEPROM_STATE;
	//=============================
	//=============================
	void SM_EEPROM_tick() {
		unsigned char x = GetKeypadKey();
		target = (char)read_eeprom_byte(1000+100*correct);
		switch(EEPROM_STATE) {
			//============================================
			//     DEBUG STATE SHOULD NOT ENTER
			//============================================
			case EEPROM_Start:
			stateFlag = 0;
			nokia_lcd_set_cursor(0,0);
			nokia_lcd_write_string("Enter passcode", 1);
			nokia_lcd_render();
			if (x == '\0') {
				EEPROM_STATE = EEPROM_Start;
				// CHECKS PASSCODE ENTRY LOGIC || 
				if(correct == 1) {
					nokia_lcd_set_cursor(40,30);
					nokia_lcd_write_string("1", 1);
					nokia_lcd_render();
				}
				if(correct == 2) {
					nokia_lcd_set_cursor(50, 30);
					nokia_lcd_write_string("2", 1);
					nokia_lcd_render();
				}
				if(correct == 3) {
					nokia_lcd_set_cursor(60, 30);
					nokia_lcd_write_string("3", 1);
					nokia_lcd_render();
				}
				if (correct == 4) {
					nokia_lcd_set_cursor(70, 30);
					nokia_lcd_write_string("4", 1);
					nokia_lcd_render();
				}
			}
			else if (x == target) {
				nokia_lcd_set_cursor(correct*10, 10);
				nokia_lcd_write_char(x, 2);
				nokia_lcd_render();
				EEPROM_STATE = EEPROM_Release_Correct;
			}
			
			else if (x != target) {
				nokia_lcd_set_cursor(counter*10, 10);
				nokia_lcd_write_char(x, 2);
				nokia_lcd_render();
				counter++;
				correct = 0;
				if (counter == 4) {
					counter = 0;
					nokia_lcd_clear();
					nokia_lcd_render();
					nokia_lcd_set_cursor(0,0);
					nokia_lcd_write_string("WRONG", 2);
					nokia_lcd_render();
					_delay_ms(1000);
					nokia_lcd_clear();
					nokia_lcd_render();
				}
				EEPROM_STATE = EEPROM_Release_Start;
			}
			
			if (~PINB & 0x01) {
				EEPROM_STATE = EEPROM_NewCode;
				nokia_lcd_clear();
				nokia_lcd_render();
				newcodeIndex = 0;
				correct = 0;
				counter = 0;
			}
			
			if (~PINB & 0x02) {
				EEPROM_STATE = EEPROM_Reset;
				correct = 0;
				counter = 0;
			}
			break;
			//=============================
			//         STATE MACHINE HERE
			//=============================
			case EEPROM_NewCode:
			nokia_lcd_set_cursor(0,0);
			nokia_lcd_write_string("CREATE NEW", 1);
			nokia_lcd_set_cursor(0, 12);
			nokia_lcd_write_string("4 DIGIT CODE", 1);
			nokia_lcd_render();
			if (x == '\0') {
				EEPROM_STATE = EEPROM_NewCode;
			}
			else {
				EEPROM_STATE = EEPROM_Release;
				write_eeprom_byte(1000 + (100*newcodeIndex), x);
				newcodeIndex++;
				if (newcodeIndex <= 3) {
					nokia_lcd_set_cursor(newcodeIndex*10, 30);
					nokia_lcd_write_char(x, 1);
					nokia_lcd_render();
				}
				else if (newcodeIndex == 4){
					EEPROM_STATE = EEPROM_Done;
					nokia_lcd_clear();
					nokia_lcd_render();
					newcodeIndex = 0;
					code_index = 0;
				}
			}
			_delay_ms(1000);
			break;
			//=============================
			//=============================
			case EEPROM_Reset:
			write_eeprom_byte(1000, '1');  // write value 1 to position 1000 of the eeprom
			write_eeprom_byte(1100, '2');  // write value 2 to position 1100 of the eeprom
			write_eeprom_byte(1200, '3');  // write value 3 to position 1200 of the eeprom
			write_eeprom_byte(1300, '4');  // write value 4 to position 1300 of the eeprom
			EEPROM_STATE = EEPROM_Done;
			break;
			//=============================
			//=============================
			case EEPROM_Release:
			if (x != '\0') {
				EEPROM_STATE = EEPROM_Release;
			}
			else if (x == '\0') {
				EEPROM_STATE = EEPROM_NewCode;

			}
			break;
			//=============================
			//=============================
			case EEPROM_Release_Start:
			if (x != '\0') {
				EEPROM_STATE = EEPROM_Release_Start;
			}
			else if (x == '\0') {
				EEPROM_STATE = EEPROM_Start;
			}
			break;
			//=============================
			//=============================
			case EEPROM_Done:
			EEPROM_STATE = EEPROM_Done;
			break;
			//=============================
			//=============================
			case EEPROM_Release_Correct:
			if (x != '\0') {
				EEPROM_STATE = EEPROM_Release_Correct;
			}
			else if (x == '\0') {
				counter++;
				correct++;
				EEPROM_STATE = EEPROM_Start;
				if (correct == 4) {
					EEPROM_STATE = EEPROM_Done;
				}
			}
			
		}
		
		
		switch(EEPROM_STATE) {
			case EEPROM_Start:
			break;
			case EEPROM_NewCode:
			break;
			case EEPROM_Reset:
			break;
			case EEPROM_Release:
			break;
			case EEPROM_Release_Start:
			break;
			case EEPROM_Done:
			adminFlag = 0;
			code[0] = (char)read_eeprom_byte(1000);
			code[1] = (char)read_eeprom_byte(1100);
			code[2] = (char)read_eeprom_byte(1200);
			code[3] = (char)read_eeprom_byte(1300);
			nokia_lcd_set_cursor(0,0);
			nokia_lcd_write_string("Input passcode", 1);
			nokia_lcd_render();
			break;
			case EEPROM_Release_Correct:
			break;
		}
	};
	

int main(void)
{
	DDRA = 0xF0; PORTA = 0x0F;
	DDRB = 0xFC; PORTB = 0x03;
	DDRD = 0xFE; PORTD = 0x01;
	motor_on(); 
	PWM_on();
	TimerOn();
	TimerSet(50);
	
	nokia_lcd_init();
	nokia_lcd_clear();
	nokia_lcd_set_cursor(2,15);
	nokia_lcd_write_string("Welcome!",1);
	nokia_lcd_render();
	_delay_ms(10000);
	nokia_lcd_clear();
	nokia_lcd_render();
	nokia_lcd_set_cursor(0,0);
	nokia_lcd_write_string("Input passcode", 1);
	nokia_lcd_render();
	//_delay_ms(10000);
	code[0] = (char)read_eeprom_byte(1000);
	code[1] = (char)read_eeprom_byte(1100);
	code[2] = (char)read_eeprom_byte(1200);
	code[3] = (char)read_eeprom_byte(1300);
	
	lock_state = lock_start;
	motion_state = NoMotion;
	speaker_state = speakerOff;
	while(1)
    {
        //TODO:: Please write your application code
	if (adminFlag == 0) {
		SM_Lock_Tick();
		SM_Sensor_Tick();
		SM_Speaker_Tick();
	}
	if (~PINB & 0x01 && speaker_state == speakerOff && lock_status == 0) {
		nokia_lcd_clear();
		adminFlag = 1;
		EEPROM_STATE = EEPROM_NewCode;	
	}
	else if (~PINB & 0x02 && speaker_state == speakerOff && lock_status == 0 && adminFlag == 0) {
		adminFlag = 1;
		EEPROM_STATE = EEPROM_Reset;
	}
	if (adminFlag == 1) {
		SM_EEPROM_tick();
	}
	while(!TimerFlag) {}
	TimerFlag = 0;
    }
	}