/* Name: main.c
 * Author: <Julian Brandt>
 */

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <string.h>

#define ZERO ~0x3F
#define ONE ~0x06
#define TWO ~0x5B
#define THREE ~0x4F
#define FOUR ~0x66
#define FIVE ~0x6D
#define SIX ~0x7D
#define SEVEN ~0x07
#define EIGHT ~0x7F
#define NINE ~0x6F
#define rZERO 0x3F
#define rONE 0x06
#define rTWO 0x5B
#define rTHREE 0x4F
#define rFOUR 0x66
#define rFIVE 0x6D
#define rSIX 0x7D
#define rSEVEN 0x07
#define rEIGHT 0x7F
#define rNINE 0x6F

#define DISP_1 0b00100000
#define DISP_2 0b00010000
#define DISP_3 0b00001000
#define DISP_4 0b00000100
#define DISP_5 0b00000010
#define DISP_6 0b00000001

#define AlarmP1 PD6
#define AlarmP2 PD7
#define AlarmP3 PB3

#define BITSET(a,b) ((a) |= (1<<(b)))
#define BITCLEAR(a,b) ((a) &= ~(1<<(b)))
#define BITFLIP(a,b) ((a) ^= (1<<(b)))


int min = 0;
int hour = 0;

int AlarmMin1 = 0;
int AlarmHour1 = 0;
int AlarmMin2 = 0;
int AlarmHour2 = 0;
int AlarmMin3 = 0;
int AlarmHour3 = 0;
int AlarmOn1 = 0;
int AlarmOn2 = 0;
int AlarmOn3 = 0;
int UpdateMinTo = 0;
int UpdateHourTo = 0;

int speedup = 0;

int AlarmDisabled = 61;
int AlarmExecution = 0;

int SelectedAlarm = 0;
int ChangeAlarmState = 0;
int ChangeTimeState = 0;

int time = 0;


int selected_line = 1;
int selected_menu = 1;


void port_init() {
	//Set input/output ports
	DDRC = 0xFF;
	DDRD = 0x3F;
	DDRA = 0xFF;
	DDRB = 0x0F;
}

int t0_overflow = 0;
int timeunits = 0;
int change_hour = 0;

void timer_init(){
	//Timer0 
	TIMSK = (1<<TOIE0);
	TCCR0 = (1<<CS00); //prescaling = 0
}


ISR(TIMER0_OVF_vect){
	//Number of overflows per 1 second = 30731
	//8 MHz / 256 = 31250 (+- finetuning)
	t0_overflow++;
	if(t0_overflow > 30731 - speedup) {
		timeunits++;
		t0_overflow = 0;
	}
	if(timeunits >= 60) {
			min++;
			timeunits = 0;
		}
	if(min >= 60) {
		if(!ChangeTimeState) {
			hour++;
		}
		min -= 60;
	}
	if(hour >= 24){
		hour -= 24;
	}
}


void disp_select(int number) {
	//Set all PORTD pins except PD6 and PD7
	//1-2 = CC
	//3-6 = CA
	switch(number) {
		case 1:
			BITCLEAR(PORTD, PD0);
			BITCLEAR(PORTD, PD1);
			BITCLEAR(PORTD, PD2);
			BITCLEAR(PORTD, PD3);
			BITSET(PORTD, PD4);
			BITCLEAR(PORTD, PD5);
			break;
		case 2:
			BITCLEAR(PORTD, PD0);
			BITCLEAR(PORTD, PD1);
			BITCLEAR(PORTD, PD2);
			BITCLEAR(PORTD, PD3);
			BITSET(PORTD, PD5);
			BITCLEAR(PORTD, PD4);
			break;
		case 3:
			BITCLEAR(PORTD, PD0);
			BITCLEAR(PORTD, PD1);
			BITCLEAR(PORTD, PD2);
			BITSET(PORTD, PD4);
			BITSET(PORTD, PD5);
			BITSET(PORTD, PD3);
			break;
		case 4:
			BITCLEAR(PORTD, PD0);
			BITCLEAR(PORTD, PD1);
			BITCLEAR(PORTD, PD3);
			BITSET(PORTD, PD4);
			BITSET(PORTD, PD5);
			BITSET(PORTD, PD2);
			break;
		case 5:
			BITCLEAR(PORTD, PD0);
			BITCLEAR(PORTD, PD2);
			BITCLEAR(PORTD, PD3);
			BITSET(PORTD, PD4);
			BITSET(PORTD, PD5);
			BITSET(PORTD, PD1);
			break;
		case 6:
			BITCLEAR(PORTD, PD1);
			BITCLEAR(PORTD, PD2);
			BITCLEAR(PORTD, PD3);
			BITSET(PORTD, PD4);
			BITSET(PORTD, PD5);
			BITSET(PORTD, PD0);
			break;
	}
}


unsigned concat(unsigned x, unsigned y) {
	//Shift y 2 digits, insert x
    return y * 100 + x;
}


void btn1() {
	//Change selected line unless a timer is selected. If so, change the selected timer by 1
	switch(selected_menu) {
		case 1:
			selected_line = LCD_menu_navigate(selected_line, 0);
			break;
		case 2:
			switch(ChangeTimeState) {
				case 0:
					selected_line = LCD_menu_navigate(selected_line, 0);
					break;
				case 1:
					hour += 1;
					break;
				case 2:
					min += 1;
					break;
			}
			break;
		case 3:
			switch(ChangeAlarmState) {
				case 0:
					selected_line = LCD_menu_navigate(selected_line, 0);
					break;
				case 1:
					switch(selected_line) {
						case 1:
							if(AlarmHour1 == 0) {
								AlarmHour1 += 23;
							}
							else {
								AlarmHour1 -= 1;
							}
							break;
						case 2:
							if(AlarmHour2 == 0) {
								AlarmHour2 += 23;
							}
							else {
								AlarmHour2 -= 1;
							}
							break;
						case 3:
							if(AlarmHour3 == 0) {
								AlarmHour3 += 23;
							}
							else {
								AlarmHour3 -= 1;
							}
							break;
					}
					break;
				case 2:
					switch(selected_line) {
						case 1:
							if(AlarmMin1 == 0) {
								AlarmMin1 += 23;
							}
							else {
								AlarmMin1 -= 1;
							}
							break;
						case 2:
							if(AlarmMin2 == 0) {
								AlarmMin2 += 23;
							}
							else {
								AlarmMin2 -= 1;
							}
							break;
						case 3:
							if(AlarmMin3 == 0) {
								AlarmMin3 += 23;
							}
							else {
								AlarmMin3 -= 1;
							}
							break;
					}
					break;
				case 3:
					switch(selected_line) {
						case 1:
							if(AlarmOn1) {
								AlarmOn1 = 0;
							}
							else {
								AlarmOn1 = 1;
							}
							break;
						case 2:
							if(AlarmOn2) {
								AlarmOn2 = 0;
							}
							else {
								AlarmOn2 = 1;
							}
							break;
						case 3:
							if(AlarmOn3) {
								AlarmOn3 = 0;
							}
							else {
								AlarmOn3 = 1;
							}
							break;
					}
					break;
			}
			break;
	}
}


void btn2() {
	//Change selected line unless a timer is selected. If so, change the selected timer by 1
	switch(selected_menu) {
		case 1:
			selected_line = LCD_menu_navigate(selected_line, 1);
			break;
		case 2:
			switch(ChangeTimeState) {
				case 0:
					selected_line = LCD_menu_navigate(selected_line, 1);
					break;
				case 1:
					if(hour == 0) {
						hour += 23;
					}
					else {
						hour -= 1;
					}
					break;
				case 2:
					if(min == 0) {
						min += 59;
					}
					else {
						min -= 1;
					}
					break;
			}
			break;
		case 3:
			switch(ChangeAlarmState) {
				case 0:
					selected_line = LCD_menu_navigate(selected_line, 1);
					break;
				case 1:
					switch(selected_line) {
						case 1:
							AlarmHour1 += 1;
							break;
						case 2:
							AlarmHour2 += 1;
							break;
						case 3:
							AlarmHour3 += 1;
							break;
					}
					break;
				case 2:
					switch(selected_line) {
						case 1:
							AlarmMin1 += 1;
							break;
						case 2:
							AlarmMin2 += 1;
							break;
						case 3:
							AlarmMin3 += 1;
							break;
					}
					break;
				case 3:
					switch(selected_line) {
						case 1:
							if(AlarmOn1) {
								AlarmOn1 = 0;
							}
							else {
								AlarmOn1 = 1;
							}
							break;
						case 2:
							if(AlarmOn2) {
								AlarmOn2 = 0;
							}
							else {
								AlarmOn2 = 1;
							}
							break;
						case 3:
							if(AlarmOn3) {
								AlarmOn3 = 0;
							}
							else {
								AlarmOn3 = 1;
							}
							break;
					}
					break;
			}
			break;
	}
}


void btn3() {
	//Navigate to next or previous menu. If a timer is selected, navigate to next tile
	switch(selected_menu) {
		case 2:
			switch(selected_line) {
				case 1:
					ChangeTimeState++;
					if(ChangeTimeState > 2) {
						ChangeTimeState = 0;
					}
					LCD_time_menu_position(1, ChangeTimeState);
					LCD_change_time(1, min, hour);
					break;
					
				default:
					selected_menu = LCD_menu_shift(selected_menu, selected_line);
					selected_line = 1;
					break;
			}
			break;
		case 3:
			switch(selected_line) {
				case 1:
					SelectedAlarm = 1;
					ChangeAlarmState++;
					if(ChangeAlarmState > 3) {
						ChangeAlarmState = 0;
						
					}
					LCD_time_menu_position(1, ChangeAlarmState);
					LCD_change_time(1, AlarmMin1, AlarmHour1);
					LCD_menu_update(3);
					break;
					
				case 2:
					SelectedAlarm = 2;
					ChangeAlarmState++;
					if(ChangeAlarmState > 3) {
						ChangeAlarmState = 0;
					}
					LCD_time_menu_position(2, ChangeAlarmState);
					LCD_change_time(2, AlarmMin2, AlarmHour2);
					LCD_menu_update(3);
					break;
					
				case 3:
					SelectedAlarm = 3;
					ChangeAlarmState++;
					if(ChangeAlarmState > 3) {
						ChangeAlarmState = 0;
					}
					LCD_time_menu_position(3, ChangeAlarmState);
					LCD_change_time(3, AlarmMin3, AlarmHour3);
					LCD_menu_update(3);
					break;
					
				default:
					selected_menu = LCD_menu_shift(selected_menu, selected_line);
					selected_line = 1;
					break;
			}
			break;
		default:
			if(selected_line == 3 && selected_menu == 1) {
				if(speedup > 30600) {
					speedup = 0;
				}
				else if(speedup) {
					speedup = 30720;
				}
				else {
					speedup = 30500;
				}
			}
			else {
				selected_menu = LCD_menu_shift(selected_menu, selected_line);
				selected_line = 1;
			}
			break;
	}
}


void btn4() {
	//Turn off alarm for the given minute
	AlarmDisabled = min;
}


int pushed1 = 0;
int pushed2 = 0;
int pushed3 = 0;
int pushed4 = 0;
void button_pushed() {
	//Check each button and set their variable accordingly, so they will not activate until they are unpressed once
	if(~PINB & (1<<PB5)){
		if(pushed1 == 0) {
			pushed1 = 1;
			btn1();
		}
	}
	else {
		pushed1 = 0;
	}
	if(~PINB & (1<<PB6)){
		if(pushed2 == 0) {
			pushed2 = 1;
			btn2();
		}
	}
	else {
		pushed2 = 0;
	}
	if(~PINB & (1<<PB7)){
		if(pushed3 == 0) {
			pushed3 = 1;
			btn3();
		}
	}
	else {
		pushed3 = 0;
	}
	if(~PINB & (1<<PB4)){
		if(pushed4 == 0) {
			pushed4 = 1;
			btn4();
		}
	}
	else {
		pushed4 = 0;
	}
}


int alarm_execute() {
	//Checks which alarms are active and sets their corresponding ports accordingly
		int active1 = 0;
		int active2 = 0;
		int active3 = 0;
		if(AlarmMin1 == min && AlarmHour1 == hour && AlarmOn1 && AlarmDisabled != min) {
			AlarmDisabled = 61;
			BITSET(PORTD, AlarmP1);
			active1 = 1;
		}
		else {
			BITCLEAR(PORTD, AlarmP1);
			active1 = 0;
		}
		if(AlarmMin2 == min && AlarmHour2 == hour && AlarmOn2 && AlarmDisabled != min) {
			AlarmDisabled = 61;
			BITSET(PORTD, AlarmP2);
			active2 = 1;
		}
		else {
			BITCLEAR(PORTD, AlarmP2);
			active2 = 0;
		}
		if(AlarmMin3 == min && AlarmHour3 == hour && AlarmOn3 && AlarmDisabled != min) {
			AlarmDisabled = 61;
			BITSET(PORTB, AlarmP3);
			active3 = 1;
		}
		else {
			BITCLEAR(PORTB, AlarmP3);
			active3 = 0;
		}
		
		if(active1 || active2 || active3) {
			return 1;
		}
		else {
			return 0;
		}
}


void calc_clock() {
	//Set time to be a 4-digit number, constructed from the min and hour timer concatenated
	time = concat(min, hour);
}


void display_number(int number) {
	//Multiplexes a 4-digit number over 4 displays. 2 of the displays are CC, the 2 others are CA.
	int one = number % 10;
	int ten = (number / 10) % 10;
	int hundred = (number / 100) % 10;
	int thousand = (number / 1000) % 10;
	
	//Makes it possible to multiplex between CC and CA displays, by making different arrays
	int display_numbers[20] = {ZERO, ONE, TWO, THREE, FOUR, FIVE, SIX, SEVEN, EIGHT, NINE, rZERO, rONE, rTWO, rTHREE, rFOUR, rFIVE, rSIX, rSEVEN, rEIGHT, rNINE};
	int digits[4] = {display_numbers[thousand], display_numbers[hundred], display_numbers[ten], display_numbers[one]};
	int reversedigits[4] = {display_numbers[thousand+10], display_numbers[hundred+10], display_numbers[ten+10], display_numbers[one+10]};
	
	int i = 0;
	
	while (i < 4) {
		//while loop that multiplexes over 4 displays, where disp1 and disp2 are CC, so they use the CC arrays
		if(i < 2) {
			PORTC = reversedigits[i];
		}
		else {
			PORTC = digits[i];
		}
		disp_select(i+1);
		_delay_ms(1);
		i++;
	}
	display_seconds(timeunits);
}


void display_seconds(int number) {
	//Same as display number, but for the last 2 displays, since the number int overflows with 6-digit numbers
	int one = number % 10;
	int ten = (number / 10) % 10;
	
	int display_numbers[10] = {ZERO, ONE, TWO, THREE, FOUR, FIVE, SIX, SEVEN, EIGHT, NINE};
	int digits[2] = {display_numbers[ten], display_numbers[one]};
	
	int i = 0;
	
	while (i < 2) {
		PORTC = digits[i];
		disp_select(i+5);
		_delay_ms(1);
		i++;
	}
}


int main(void)
{
	//init
	sei();
	port_init();
	timer_init();
	LCD_init();
	
	//time = 0
	min = 0;
	hour = 0;
	
	//write menu
	LCD_write_menu_main(selected_line);
	while(1){
		//Set time variable and display
		calc_clock();
		display_number(time);
		
		//Check for pushed buttons
		button_pushed();
		
		//Check alarm states
		AlarmExecution = alarm_execute();
		
		//Check if time on LCD is offset
		if((min != UpdateMinTo || hour != UpdateHourTo) && selected_menu == 2) {
			LCD_menu_update(2);
			UpdateMinTo = min;
			UpdateHourTo = hour;
			
		}
		//Check if alarms are being altered
		if(ChangeAlarmState != 0) {
			LCD_menu_update(3);
		}
		
	}
	return 0;
}