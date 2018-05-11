void LCD_write_char(char *a);

void LCD_restart();

void LCD_init();

void LCD_command(int command);

void LCD_cursor_position(int line, int position);

void LCD_write_line(int line, char *string);

void LCD_cursor(bool state);

void LCD_write_menu_gen(int LineSel, unsigned p1, unsigned p2, unsigned p3, unsigned p4);

int LCD_menu_navigate(int position, int direction);

void LCD_menu_shift(int menu);

void LCD_clear();

void LCD_change_time(int min, int hour);

void LCD_menu_update(int menu);