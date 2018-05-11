#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <string.h>

#define BITSET(a,b) ((a) |= (1<<(b)))
#define BITCLEAR(a,b) ((a) &= ~(1<<(b)))
#define BITFLIP(a,b) ((a) ^= (1<<(b)))

#define EN PB0
#define RW PB1
#define RS PB2



void LCD_command(int command);


void LCD_write_char(char *a) {
	//Write single character in the selected position
	BITSET(PORTB, RS);             // => RS = 1
	PORTA = a;
	BITSET(PORTB, EN);             // => E = 1
	_delay_us(40);
	BITCLEAR(PORTB, EN);             // => E = 04
	_delay_us(40);
}

void LCD_restart() {
	//Clear screen
	LCD_command(0x01);
	_delay_ms(2);
	
	//Make cursor visible
	LCD_command(0x0C);
	_delay_us(50);
	
	//Set display to 8-bit interfacing mode
	LCD_command(0x38);
}

void LCD_init() {
	//Clear screen
	LCD_command(0x01);
	_delay_ms(2);
	
	//Make cursor visible
	LCD_command(0x0C);
	_delay_ms(1);
	
	//Set display to 8-bit interfacing mode
	LCD_command(0x38);
}

void LCD_command(int command) {
	BITCLEAR(PORTB, RS);
	//Set up command
	PORTA = command;
	BITSET(PORTB, EN);
	_delay_us(100);
	
	//Execute command
	BITCLEAR(PORTB, EN);
	_delay_us(100);
}


void LCD_cursor_position(int line, int position) {
	//Set position to line, tile using 8-bit addresses
	switch(line) {
		case 1:
			LCD_command(0x80 + (position));
			break;
		case 2:
			LCD_command(0xC0 + (position));
			break;
		case 3:
			LCD_command(0x90 + (position));
			break;
		case 4:
			LCD_command(0xD0 + (position));
			break;
	}
	_delay_us(200);
}


void LCD_write_line(int line, char *string) {
	//Write entire line using character function
	LCD_cursor_position(line, 0);
	
	
	for(int i=0; i < strlen(string); i++) {
		LCD_write_char(string[i]);
	}
	_delay_ms(1);
}


void LCD_clear() {
	LCD_command(0x01);
	_delay_us(50);
}


void LCD_cursor(int state) {
	//Turn on underline cursor
	if(state) {
		LCD_command(0x0E);
	}
	
	//Turn off cursor
	else {
		LCD_command(0x0C);
	}
	_delay_us(40);
}

#define Main1 "  Time"
#define Main2 "  Alarms"
#define Main3 "  Clock Speed"
#define Main4 "  "
#define Time1 "      :    "
#define Time2 "  Back"
#define Time3 "  "
#define Time4 "  "
#define Alarm1 "  Alarm 1       "
#define Alarm2 "  Alarm 2       "
#define Alarm3 "  Alarm 3       "
#define Alarm4 "  Back"


void LCD_write_menu_gen(int LineSel, unsigned p1, unsigned p2, unsigned p3, unsigned p4) {
	//Generator function, for creating all menus
	LCD_write_line(1, p1);
	LCD_write_line(2, p2);
	LCD_write_line(3, p3);
	LCD_write_line(4, p4);
		
	LCD_cursor_position(1, 0);
	LCD_write_char('~');
}


void LCD_write_menu_main(int LineSel) {
	//Write main menu
	LCD_clear();
	LCD_write_menu_gen(LineSel, Main1, Main2, Main3, Main4);
}


void LCD_change_time(int line, int min, int hour) {
	//Update displayed time on LCD
	//And change the symbols next to the numbers, so they match the selected tile
	extern int selected_menu;
	extern int ChangeTimeState;
	extern int ChangeAlarmState;
	extern int AlarmOn1;
	extern int AlarmOn2;
	extern int AlarmOn3;
	char prefix1;
	char prefix12;
	char prefix2;
	char prefix22;
	char AlarmPrefix1;
	char AlarmPrefix2;
	switch(ChangeTimeState) {
		case 0:
			prefix1 = ' ';
			prefix12 = ' ';
			prefix2 = ' ';
			prefix22 = ' ';
			AlarmPrefix1 = ' ';
			AlarmPrefix2 = ' ';
			break;
		case 1:
			prefix1 = '[';
			prefix12 = ']';
			prefix2 = ' ';
			prefix22 = ' ';
			AlarmPrefix1 = ' ';
			AlarmPrefix2 = ' ';
			break;
		case 2:
			prefix1 = ' ';
			prefix12 = ' ';
			prefix2 = '[';
			prefix22 = ']';
			AlarmPrefix1 = ' ';
			AlarmPrefix2 = ' ';
			break;
	}
	if(selected_menu == 3) {
		switch(ChangeAlarmState) {
			case 0:
				prefix1 = ' ';
				prefix12 = ' ';
				prefix2 = ' ';
				prefix22 = ' ';
				AlarmPrefix1 = ' ';
				AlarmPrefix2 = ' ';
				break;
			case 1:
				prefix1 = '[';
				prefix12 = ']';
				prefix2 = ' ';
				prefix22 = ' ';
				AlarmPrefix1 = ' ';
				AlarmPrefix2 = ' ';
				break;
			case 2:
				prefix1 = ' ';
				prefix12 = ' ';
				prefix2 = '[';
				prefix22 = ']';
				AlarmPrefix1 = ' ';
				AlarmPrefix2 = ' ';
				break;
			case 3:
				prefix12 = ' ';
				prefix2 = ' ';
				prefix22 = ' ';
				AlarmPrefix1 = '[';
				AlarmPrefix2 = ']';
				break;
		}
	}
	
	
	LCD_cursor_position(line, 2);
	LCD_write_char(prefix1);
	LCD_write_char((hour/10)%10 + 48);
	LCD_write_char(hour%10 + 48);
	LCD_write_char(prefix12);
	LCD_write_char(':');
	LCD_write_char(prefix2);
	LCD_write_char((min/10)%10 + 48);
	LCD_write_char(min%10 + 48);
	LCD_write_char(prefix22);
	LCD_write_char(AlarmPrefix1);
	if(selected_menu == 3) {
		if(line == 1) {
			if(AlarmOn1) {
				LCD_write_char('O');
				LCD_write_char('N');
				LCD_write_char(AlarmPrefix2);
				LCD_write_char(' ');
			}
			else {
				LCD_write_char('O');
				LCD_write_char('F');
				LCD_write_char('F');
				LCD_write_char(AlarmPrefix2);
			}
		}
		else if(line == 2) {
			if(AlarmOn2) {
				LCD_write_char('O');
				LCD_write_char('N');
				LCD_write_char(AlarmPrefix2);
				LCD_write_char(' ');
			}
			else {
				LCD_write_char('O');
				LCD_write_char('F');
				LCD_write_char('F');
				LCD_write_char(AlarmPrefix2);
			}
		}
		else if(line == 3) {
			if(AlarmOn3) {
				LCD_write_char('O');
				LCD_write_char('N');
				LCD_write_char(AlarmPrefix2);
				LCD_write_char(' ');
			}
			else {
				LCD_write_char('O');
				LCD_write_char('F');
				LCD_write_char('F');
				LCD_write_char(AlarmPrefix2);
			}
		}
	}
}


void LCD_write_menu_Time(int LineSel) {
	//Write time menu
	extern int min;
	extern int hour;
	LCD_clear();
	LCD_write_menu_gen(LineSel, Time1, Time2, Time3, Time4);
	LCD_change_time(1, min, hour);
}
	
	
void LCD_write_menu_Alarm(int LineSel) {
	//Write alarm menu
	LCD_clear();
	LCD_write_menu_gen(LineSel, Alarm1, Alarm2, Alarm3, Alarm4);
}


int LCD_menu_navigate(int position, int direction) {
	//Set mark on the right line, and wrap menu at right line
	int MenuSize = 0;
	extern int selected_menu;
	switch(selected_menu) {
		case 1:
			MenuSize = 3;
			break;
		case 2:
			MenuSize = 2;
			break;
		case 3:
			MenuSize = 4;
	}
	switch(direction) {
		case 0:
			position -= 1;
			break;
		case 1:
			position++;
			break;
	}
	if(position > MenuSize) {
		position -= MenuSize;
	}
	else if(position < 1) {
		position += MenuSize;
	}
	
	for(int i = 1; i < 5; i++) {
		LCD_cursor_position(i, 0);
		LCD_write_char(' ');
	}
	LCD_cursor_position(position, 0);
	LCD_write_char('~');
	return position;
}


int LCD_menu_shift(int menu, int position) {
	//Enter or exit new menus, depending on active menu
	switch(menu) {
		case 1:
			switch(position) {
				case 1:
					LCD_write_menu_Time(1);
					return 2;
					break;
				case 2:
					LCD_write_menu_Alarm(1);
					return 3;
					break;
				case 3:
					LCD_write_menu_Time(1);
					return 2;
					break;
				case 4:
					LCD_write_menu_Time(1);
					return 2;
					break;
				
			}
			break;
		case 2:
			switch(position) {
				case 2:
					LCD_write_menu_main(1);
					return 1;
					break;
				default:
					return 2;
					break;
			}
			break;
		
		case 3:
			switch(position) {
				case 4:
					LCD_write_menu_main(1);
					return 1;
					break;
				default:
					return 2;
					break;
			}
			break;
			
	}
}


void LCD_time_menu_position(int line, int position) {
	//Set symbols next to the right number in the time menu (also used when changing alarms)
	switch(position) {
			case 0:
				LCD_cursor_position(line, 2);
				LCD_write_char(' ');
				LCD_cursor_position(line, 5);
				LCD_write_char(' ');
				LCD_cursor_position(line, 7);
				LCD_write_char(' ');
				LCD_cursor_position(line, 10);
				LCD_write_char(' ');
				break;
			case 1:
				LCD_cursor_position(line, 2);
				LCD_write_char('[');
				LCD_cursor_position(line, 5);
				LCD_write_char(']');
				LCD_cursor_position(line, 7);
				LCD_write_char(' ');
				LCD_cursor_position(line, 10);
				LCD_write_char(' ');
				break;
			case 2:
				LCD_cursor_position(line, 2);
				LCD_write_char(' ');
				LCD_cursor_position(line, 5);
				LCD_write_char(' ');
				LCD_cursor_position(line, 7);
				LCD_write_char('[');
				LCD_cursor_position(line, 10);
				LCD_write_char(']');
				break;
		}
}


void LCD_menu_update(int menu) {
	//An update run whenever called in main
	//Exeutes different actions depending on active menu
	extern int min;
	extern int hour;
	extern int AlarmMin1;
	extern int AlarmHour1;
	
	extern int AlarmMin2;
	extern int AlarmHour2;
	
	extern int AlarmMin3;
	extern int AlarmHour3;
	
	extern int selected_line;
	extern int ChangeAlarmState;
	switch(menu) {
		case 1:
			break;
		case 2:
			LCD_change_time(1, min, hour);
			break;
		case 3:
			switch(ChangeAlarmState) {
				case 0:
					//BAD IDEA:
					switch(selected_line) {
						case 1:
							LCD_write_line(1, Alarm1);
							break;
						case 2:
							LCD_write_line(2, Alarm2);
							break;
						case 3:
							LCD_write_line(3, Alarm3);
							break;
						//makes displays flicker
					}
					LCD_cursor_position(selected_line, 0);
					LCD_write_char('~');
					break;
				default:
					switch(selected_line) {
						case 1:
							LCD_change_time(selected_line, AlarmMin1, AlarmHour1);
							break;
						case 2:
							LCD_change_time(selected_line, AlarmMin2, AlarmHour2);
							break;
						case 3:
							LCD_change_time(selected_line, AlarmMin3, AlarmHour3);
							break;
					}
					break;
			}
			break;
	}
}