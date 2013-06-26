#ifndef _LCM_CHAR_16x2_H_
#define _LCM_CHAR_16x2_H_

typedef struct lcm_pixel_data_s
{
	unsigned char row0;
	unsigned char row1;
	unsigned char row2;
	unsigned char row3;
	unsigned char row4;
	unsigned char row5;
	unsigned char row6;
	unsigned char row7;
}lcm_pixel_data_t;

#define DISPLAY_CHARACTER 16
// lcm.c function prototype
void LCM_Software_Init(void);
void Clear_Display(unsigned char bit_interface);
void Return_Home(unsigned char bit_interface);
void Entry_mode_set(unsigned char address_shift,unsigned char bit_interface);
void Display_on_off(unsigned char display,unsigned char cursor,unsigned char blink,unsigned char bit_interface);
void Function_set(unsigned char data_length,unsigned char bit_interface);
void Set_CG_RAM_address(unsigned char CG_address,unsigned char bit_interface);
void Set_DD_RAM_address(unsigned char DD_address,unsigned char bit_interface);
void Write_Data_to_RAM(char character,unsigned char bit_interface);
void Write_Data_to_CGRAM(char character,unsigned char bit_interface);
void Write_string_to_LCM(char *strings,unsigned char start,unsigned char end,unsigned char line);
unsigned char Read_BF_AC(void);
unsigned char Read_Data_from_RAM(void);
void make_character(unsigned char number_char, lcm_pixel_data_t *data);

void lcm_draw_text( int x, int y, unsigned char *pszText, int len );
void lcm_move_cursor_position( int x, int y );

#endif	//_LCM_CHAR_16x2_H_

