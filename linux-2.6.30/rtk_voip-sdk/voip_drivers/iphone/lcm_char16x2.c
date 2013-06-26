//It's something wrong about AC increment by 1 in 4-bit interface.
//So some trick is included in Write_Data_to_RAM function.

#include <linux/kernel.h>
#include <linux/string.h>
#include "../gpio/gpio.h"
#include "base_gpio.h"
#include "lcm_char16x2.h"

//-----------------------------
#if 1	//0: kernel used(udelay), 1: test program used(for(;;)) 
#define __test_program_used__
#else
#define __kernel_used__
#endif

//---------------------- LCM --------------------------------------------
#define _LCM_PIN_
#ifdef CONFIG_RTK_VOIP_GPIO_IPP_8972B_V00
#define RATING_FACTOR	3
#else
#define RATING_FACTOR	2	//Adjust LCM read and write rating.
#endif
//cli() protection for kernel used. Don't define MACRO LCM_CLI_PROTECT in test program.
//#define LCM_CLI_PROTECT

/********************* LCM data struct *************************************/

struct lcm_char16x2_dev_s
{
	unsigned int reg_select;	//output
	unsigned int enable;		//output
	unsigned int read_write;	//output
	unsigned int DB7;		//input or output
	unsigned int DB6;		//input or output
	unsigned int DB5;		//input or output
	unsigned int DB4;		//input or output
	/*unsigned int DB3;		//input or output
	unsigned int DB2;		//input or output
	unsigned int DB1;		//input or output
	unsigned int DB0;		//input or output
	*/
};

typedef struct lcm_char16x2_dev_s lcm_char16x2_dev_t;

enum REGISTER_SELECT
{
	INSTRUCT_REGISTER = 0,
	DATA_REGISTER,	
};

enum BUSY_FLAG
{
	READ_FLAG = 0,
	NOT_READ_FLAG,
};

//---------------------- LCM PIN --------------------------------------------

#ifdef _LCM_PIN_  
#if defined(CONFIG_RTK_VOIP_GPIO_IPP_100)
#if 0
#define GPIO_LCM "C"
#define PIN_RS		GPIO_ID(GPIO_PORT_C,6)	//output
#define PIN_ENABLE	GPIO_ID(GPIO_PORT_C,7)	//output
#define PIN_R_W_	GPIO_ID(GPIO_PORT_C,8)	//output
#define PIN_DB7		GPIO_ID(GPIO_PORT_C,9)	//output or input
#define PIN_DB6		GPIO_ID(GPIO_PORT_C,10)	//output or input
#define PIN_DB5		GPIO_ID(GPIO_PORT_C,11)	//output or input
#define PIN_DB4		GPIO_ID(GPIO_PORT_C,12)	//output or input
#else
#define GPIO_LCM "E"
#define PIN_RS		GPIO_ID(GPIO_PORT_E,6)	//output
#define PIN_ENABLE	GPIO_ID(GPIO_PORT_E,4)	//output
#define PIN_R_W_	GPIO_ID(GPIO_PORT_C,15)	//output
#define PIN_DB7		GPIO_ID(GPIO_PORT_E,0)	//output or input
#define PIN_DB6		GPIO_ID(GPIO_PORT_E,1)	//output or input
#define PIN_DB5		GPIO_ID(GPIO_PORT_E,2)	//output or input
#define PIN_DB4		GPIO_ID(GPIO_PORT_E,3)	//output or input
#endif
#elif defined(CONFIG_RTK_VOIP_GPIO_IPP_101)
#define GPIO_LCM "ACE"
#define PIN_RS		GPIO_ID(GPIO_PORT_E,6)	//output
#define PIN_ENABLE	GPIO_ID(GPIO_PORT_E,4)	//output
#define PIN_R_W_	GPIO_ID(GPIO_PORT_C,15)	//output
#define PIN_DB7		GPIO_ID(GPIO_PORT_E,0)	//output or input
#define PIN_DB6		GPIO_ID(GPIO_PORT_A,9)	//output or input
#define PIN_DB5		GPIO_ID(GPIO_PORT_A,5)	//output or input
#define PIN_DB4		GPIO_ID(GPIO_PORT_A,4)	//output or input
#elif defined(CONFIG_RTK_VOIP_GPIO_IPP_8972B_V00)
#define GPIO_LCM ""
#define PIN_RS		GPIO_ID(GPIO_PORT_D,4)	//output
#define PIN_ENABLE	GPIO_ID(GPIO_PORT_D,7)	//output
#define PIN_R_W_	GPIO_ID(GPIO_PORT_E,0)	//output
#define PIN_DB7		GPIO_ID(GPIO_PORT_D,3)	//output or input
#define PIN_DB6		GPIO_ID(GPIO_PORT_D,2)	//output or input
#define PIN_DB5		GPIO_ID(GPIO_PORT_D,0)	//output or input
#define PIN_DB4		GPIO_ID(GPIO_PORT_C,4)	//output or input
#endif
#endif
/*******************************************************************/

#ifdef __kernel_used__
#include <linux/delay.h>	//udelay(),mdelay()
#endif

static void init_lcm_char16x2_gpio(int lcm_gpio);
static void write_lcm_char16x2_IR(unsigned char data,enum BUSY_FLAG bf_flag,unsigned char bit_interface);
static unsigned char read_lcm_char16x2_IR(void);
static void write_lcm_char16x2_DR(unsigned char data,enum BUSY_FLAG bf_flag,unsigned char bit_interface);
static unsigned char read_lcm_char16x2_DR(void);

void LCM_Software_Init(void)
{
	unsigned int i;
	init_lcm_char16x2_gpio(0);
	//delay 15 ms
	#ifdef __kernel_used__
	mdelay(15);
	#endif
	#ifdef __test_program_used__
	for (i=0;i<1000000;i++);
	#endif
	write_lcm_char16x2_IR(0x30,NOT_READ_FLAG,1);//Function_set
	//delay 4.1 ms
	#ifdef __kernel_used__
	mdelay(5);
	#endif
	#ifdef __test_program_used__
	for (i=0;i<500000;i++);
	#endif
	write_lcm_char16x2_IR(0x30,NOT_READ_FLAG,1);//Function_set
	//delay 100us
	#ifdef __kernel_used__
	udelay(100);
	#endif
	#ifdef __test_program_used__
	for (i=0;i<40000;i++);
	#endif
	write_lcm_char16x2_IR(0x30,NOT_READ_FLAG,1);//Function_set
	//delay 100us
	#ifdef __kernel_used__
	udelay(100);
	#endif
	#ifdef __test_program_used__
	for (i=0;i<40000;i++);
	#endif

	Function_set(0,1);//set interface to 4-bit
	Function_set(0,0);//set number of lines and character font
	Display_on_off(0,0,0,0);//display off ,cursor off and no blink
	Clear_Display(0);//clear display
	Entry_mode_set(1,0);//entry mode set
	Display_on_off(1,0,0,0);//display on ,cursor off and no blink
	
	return;
}

//bit_interface: 0-> 4-bit data length, 1-> 8-bit data_length.	
void Clear_Display(unsigned char bit_interface)
{
	write_lcm_char16x2_IR(0x01,READ_FLAG,bit_interface);
	return;
}

void Return_Home(unsigned char bit_interface)
{
	write_lcm_char16x2_IR(0x02,READ_FLAG,bit_interface);
	return;
}

//address_shift: 0->decrement DD_RAM address by 1, 1->increment DD_RAM address by 1.
void Entry_mode_set(unsigned char address_shift,unsigned char bit_interface)
{
	if (address_shift > 1) {
		printk("Wrong LCM address decrement or increment\n");
		return;
	}		
	write_lcm_char16x2_IR(0x04|(address_shift<<1),READ_FLAG,bit_interface);
	return;
}

//display: 0->display off, 1->display on.
//cursor: 0->cursor don't display, 1->cursor display.
//blink: 0->cursor don't blink, 1->cursor blink.
void Display_on_off(unsigned char display,unsigned char cursor,unsigned char blink,unsigned char bit_interface)
{
	if ((display > 1) || (cursor > 1) || (blink > 1)) {
		printk("Wrong input to Display_on_off\n");
		return;
	}	
	write_lcm_char16x2_IR(0x08|(display<<2)|(cursor<<1)|blink,READ_FLAG,bit_interface);	
	return;
}

//data_length: 0->data sent or received in 4-bit length, 1->data sent or received in 8-bit length.
void Function_set(unsigned char data_length,unsigned char bit_interface)
{
	if (data_length > 1) {
		printk("Wrong data interface\n");
		return;
	}
	write_lcm_char16x2_IR(0x28|(data_length<<4),READ_FLAG,bit_interface);	
	return;
}

void Set_CG_RAM_address(unsigned char CG_address,unsigned char bit_interface)
{
	write_lcm_char16x2_IR(0x40|CG_address,READ_FLAG,bit_interface);
	return;
}

void Set_DD_RAM_address(unsigned char DD_address,unsigned char bit_interface)
{
	write_lcm_char16x2_IR(0x80|DD_address,READ_FLAG,bit_interface);
	return;
}

void Write_Data_to_RAM(char character,unsigned char bit_interface)
{
	write_lcm_char16x2_DR(character,READ_FLAG,bit_interface);
	Set_DD_RAM_address(((Read_BF_AC()&0x7F)-1),0);
	return;
}

void Write_Data_to_CGRAM(char character,unsigned char bit_interface)
{
	//printk("bef Read_BF_AC()=%x\n",Read_BF_AC());
	write_lcm_char16x2_DR(character,READ_FLAG,bit_interface);
	
	//Set_CG_RAM_address((Read_BF_AC()-1),0);
	return;
}

unsigned char Read_BF_AC(void)
{
	return read_lcm_char16x2_IR();
}

unsigned char Read_Data_from_RAM(void)
{
	return read_lcm_char16x2_DR();
}

#if 0
static unsigned char check_string_number(char *strings)
{
	int i=0;
	while (strings[i++] != '\x0');
	return (i-1);
}	
#endif

//line: 1-> line 1, 2-> line2. 
void Write_string_to_LCM(char *strings,unsigned char start,unsigned char end,unsigned char line)
{
	int i=0;
	
	#if 0
	if (check_string_number(strings) > DISPLAY_CHARACTER)  {
		printk("The string is over the display\n");
		return;
	}
	#endif
	if (end < start) {
		printk("Wrong parameter input\n");
		return;
	}
		
	if (start == 0 && end == 0) {
		if (line == 1)
			Set_DD_RAM_address(0,0);
		else if (line == 2)
			Set_DD_RAM_address(0x40,0);
		else
			printk("Wrong LCM line number\n");
		while (strings[i] != '\x0') 
			Write_Data_to_RAM(strings[i++],0);			
	} else {
		if (line == 1)
			Set_DD_RAM_address(start,0);
		else if (line == 2)
			Set_DD_RAM_address(0x40|start,0);
		else
			printk("Wrong LCM line number\n");
		while (strings[i] != '\x0') {
			Write_Data_to_RAM(strings[i++],0);
			if (i == end+1)
				break;
		}		
	}		
	return;	
}

void make_character(unsigned char number_char, lcm_pixel_data_t *data)
{
	if (number_char > 6) {
		printk("Over CGRAM address\n");
		return;
	}
	
	Set_CG_RAM_address((0x40+8*number_char),0);
	Write_Data_to_CGRAM(data->row0,0);
	Set_CG_RAM_address((0x40+8*number_char+1),0);
	Write_Data_to_CGRAM(data->row1,0);
	Set_CG_RAM_address((0x40+8*number_char+2),0);
	Write_Data_to_CGRAM(data->row2,0);
	Set_CG_RAM_address((0x40+8*number_char+3),0);
	Write_Data_to_CGRAM(data->row3,0);
	Set_CG_RAM_address((0x40+8*number_char+4),0);
	Write_Data_to_CGRAM(data->row4,0);
	Set_CG_RAM_address((0x40+8*number_char+5),0);
	Write_Data_to_CGRAM(data->row5,0);
	Set_CG_RAM_address((0x40+8*number_char+6),0);
	Write_Data_to_CGRAM(data->row6,0);
	Set_CG_RAM_address((0x40+8*number_char+7),0);
	Write_Data_to_CGRAM(data->row7,0);	 

	return;
}	

#define LCM_WIDTH		DISPLAY_CHARACTER	/* 16 */
#define LCM_HEIGHT		2

void lcm_draw_text( int x, int y, unsigned char *pszText, int len )
{
	int abs_x;
	
	if( y < 0 || y >= LCM_HEIGHT || x >= LCM_WIDTH || len <= 0 )
		return;

	if( x < 0 ) {
		
		abs_x = x * ( -1 );
		
		if( abs_x > len )	/* too left */
			return;

		x = 0;
		pszText += abs_x;
		len -= abs_x;
	}
	
	if( x + len > LCM_WIDTH )	/* too long */
		len = LCM_WIDTH - x;
		
	/*
	 * Now, it normalize to 
	 *  0 <= x < LCM_WIDTH
	 *  0 <= y < LCM_HEIGHT
	 *  1 <= x + len < LCM_WIDTH
	 */
	if( y == 0 )
		Set_DD_RAM_address( 0x00 + x, 0 );
	else
		Set_DD_RAM_address( 0x40 + x, 0 );
	
	while( len -- )
		Write_Data_to_RAM( *pszText ++, 0 );
}

void lcm_move_cursor_position( int x, int y )
{
	if( y < 0 || y >= LCM_HEIGHT || x < 0 || x >= LCM_WIDTH )
		return;
	
	/* set AC address */
	if( y == 0 )
		Set_DD_RAM_address( 0x00 + x, 0 );
	else
		Set_DD_RAM_address( 0x40 + x, 0 );
}

/************************************* Set GPIO Pin to LCM ***********************************************************/
/*
@func int | _lcm_char16x2_data_port_init 
@parm lcm_char16x2_dev_t* | pLCM_Dev | Structure to store device information
*/
static void _lcm_char16x2_data_port_init( lcm_char16x2_dev_t* pLCM_Dev,unsigned char in_out)
{
		
	if (in_out == 1) {//config DB7~DB4 output
		__lcm_initGpioPin( pLCM_Dev->DB7, GPIO_DIR_OUT, GPIO_INT_DISABLE );
		__lcm_initGpioPin( pLCM_Dev->DB6, GPIO_DIR_OUT, GPIO_INT_DISABLE );
		__lcm_initGpioPin( pLCM_Dev->DB5, GPIO_DIR_OUT, GPIO_INT_DISABLE );
		__lcm_initGpioPin( pLCM_Dev->DB4, GPIO_DIR_OUT, GPIO_INT_DISABLE );
	} else if (in_out ==0) {//config DB7~DB4 input	
		__lcm_initGpioPin( pLCM_Dev->DB7, GPIO_DIR_IN, GPIO_INT_DISABLE );
		__lcm_initGpioPin( pLCM_Dev->DB6, GPIO_DIR_IN, GPIO_INT_DISABLE );
		__lcm_initGpioPin( pLCM_Dev->DB5, GPIO_DIR_IN, GPIO_INT_DISABLE );
		__lcm_initGpioPin( pLCM_Dev->DB4, GPIO_DIR_IN, GPIO_INT_DISABLE );
	}	
	return;
}


/*
@func void | _lcm_char16x2_rawRead | Read several bits from LCM
@parm lcm_char16x2_dev_t* | pLCM_Dev | Structure containing device information
@parm unsigned int* | pData | Pointer to store data
@parm unsigned int | bits | Number bits of data wanted to read
@comm
*/
//rs_flag: 0->instruction register, 1->data register.
static void _lcm_char16x2_rawRead( lcm_char16x2_dev_t* pLCM_Dev, void* pData, int bits, enum REGISTER_SELECT rs_flag)
{
	unsigned char* pch = pData;
	unsigned int buf,i;
	unsigned char j=0;
	
	
	_lcm_char16x2_data_port_init(pLCM_Dev,0);//config DB7~DB4 input
	*pch = 0;
	for (j=0;j<2;j++) {
		if (rs_flag == INSTRUCT_REGISTER)
			__lcm_setGpioDataBit( pLCM_Dev->reg_select, 0); /* fall down reg_select*/
		else if (rs_flag == DATA_REGISTER)
			__lcm_setGpioDataBit( pLCM_Dev->reg_select, 1); /* raise reg_select*/
		__lcm_setGpioDataBit( pLCM_Dev->read_write, 1); /* raise read_write*/
		//delay 1 us.
		#ifdef __kernel_used__
		udelay(1*RATING_FACTOR);
		#endif
		#ifdef __test_program_used__
		for (i=0;i<1000*RATING_FACTOR;i++);
		#endif
		__lcm_setGpioDataBit( pLCM_Dev->enable, 1); /* raise enable*/
		//delay 0.2 us.
		#ifdef __kernel_used__
		udelay(1*RATING_FACTOR);
		#endif
		#ifdef __test_program_used__
		for (i=0;i<200*RATING_FACTOR;i++);
		#endif
		//read data from DB7~DB4
		__lcm_getGpioDataBit( pLCM_Dev->DB7, &buf );
		*pch |= (buf<<(7-4*j));
		__lcm_getGpioDataBit( pLCM_Dev->DB6, &buf );
		*pch |= (buf<<(7-4*j-1));
		__lcm_getGpioDataBit( pLCM_Dev->DB5, &buf );
		*pch |= (buf<<(7-4*j-2));
		__lcm_getGpioDataBit( pLCM_Dev->DB4, &buf );
		*pch |= (buf<<(7-4*j-3));
		//delay 0.25 us
		#ifdef __kernel_used__
		udelay(1*RATING_FACTOR);
		#endif
		#ifdef __test_program_used__
		for (i=0;i<250*RATING_FACTOR;i++);
		#endif
		__lcm_setGpioDataBit( pLCM_Dev->enable, 0); /* fall down enable*/
		//delay 0.25 us
		#ifdef __kernel_used__
		udelay(1*RATING_FACTOR);
		#endif
		#ifdef __test_program_used__
		for (i=0;i<250*RATING_FACTOR;i++);
		#endif
		if (rs_flag == INSTRUCT_REGISTER)
			__lcm_setGpioDataBit( pLCM_Dev->reg_select, 1); /* raise reg_select*/
		else if (rs_flag == DATA_REGISTER)
			__lcm_setGpioDataBit( pLCM_Dev->reg_select, 0); /* fall down reg_select*/
	}
	_lcm_char16x2_data_port_init(pLCM_Dev,1);//config DB7~DB4 output
	return;
}


/*
@func int | _lcm_char16x2_rawWrite | Write several bits from LCM
@parm lcm_char16x2_dev_t* | pLCM_Dev | Structure containing device information
@parm unsigned int* | pData | Pointer to data
@parm unsigned int | bits | Number bits of data wanting to write
@comm
*/
//rs_flag: 0->instruction register, 1->data register.
//bf_flag: 0->read busy flag(DB7), 1->don't read busy flag
//bit_interface: 0-> 4-bit data length, 1-> 8-bit data_length.
static void _lcm_char16x2_rawWrite( lcm_char16x2_dev_t* pLCM_Dev, 
									void* pData, 
									int bits, 
									enum REGISTER_SELECT rs_flag, 
									enum BUSY_FLAG bf_flag,
									unsigned char bit_interface)
{
	unsigned char* pch = pData;
	unsigned char flag_DB7, j;
	unsigned int i;
	
	if ( pData == NULL ) {
		printk("no data will be written\n");
		return;
	}
	
	for (j=0;j<(2-bit_interface);j++) {
		if (rs_flag == INSTRUCT_REGISTER)
			__lcm_setGpioDataBit( pLCM_Dev->reg_select, 0); /* fall down reg_select*/
		else if (rs_flag == DATA_REGISTER)
			__lcm_setGpioDataBit( pLCM_Dev->reg_select, 1); /* raise reg_select*/
		__lcm_setGpioDataBit( pLCM_Dev->read_write, 0); /* fall down read_write*/
		//delay 1 us.
		#ifdef __kernel_used__
		udelay(1*RATING_FACTOR);
		#endif
		#ifdef __test_program_used__
		for (i=0;i<1000*RATING_FACTOR;i++);
		#endif
		__lcm_setGpioDataBit( pLCM_Dev->enable, 1); /* raise enable*/
		//write data to pin
		__lcm_setGpioDataBit( pLCM_Dev->DB7, ((*pch)>>(7-4*j))&0x00000001);
		__lcm_setGpioDataBit( pLCM_Dev->DB6, ((*pch)>>(7-4*j-1))&0x00000001);
		__lcm_setGpioDataBit( pLCM_Dev->DB5, ((*pch)>>(7-4*j-2))&0x00000001);
		__lcm_setGpioDataBit( pLCM_Dev->DB4, ((*pch)>>(7-4*j-3))&0x00000001);
		//delay 0.25 us
		#ifdef __kernel_used__
		udelay(1*RATING_FACTOR);
		#endif
		#ifdef __test_program_used__
		for (i=0;i<250*RATING_FACTOR;i++);
		#endif
		__lcm_setGpioDataBit( pLCM_Dev->enable, 0); /* fall down enable*/
		//delay 0.25 us
		#ifdef __kernel_used__
		udelay(1*RATING_FACTOR);
		#endif
		#ifdef __test_program_used__
		for (i=0;i<250*RATING_FACTOR;i++);
		#endif
		if (rs_flag == INSTRUCT_REGISTER)
			__lcm_setGpioDataBit( pLCM_Dev->reg_select, 1); /* raise reg_select*/
		else if (rs_flag == DATA_REGISTER)
			__lcm_setGpioDataBit( pLCM_Dev->reg_select, 0); /* fall down reg_select*/
		
	}
	
	if (bf_flag == READ_FLAG) {
		do{
			_lcm_char16x2_rawRead(pLCM_Dev,&flag_DB7,bits,rs_flag);
		} while (flag_DB7&0x80);
	}	
	return;
}

/************************* LCM API ****************************************/

static lcm_char16x2_dev_t lcm_dev;

//------------------------------------------

static void gpio_pin_config(unsigned int reg_select, unsigned int enable, 
	unsigned int read_write, unsigned int DB7, unsigned int DB6,unsigned int DB5, 
	unsigned int DB4,unsigned char in_out)
{
	
	lcm_dev.reg_select = reg_select;
	lcm_dev.enable = enable;
	lcm_dev.read_write = read_write;
	lcm_dev.DB7 = DB7;
	lcm_dev.DB6 = DB6;
	lcm_dev.DB5 = DB5;
	lcm_dev.DB4 = DB4;
	
	__lcm_initGpioPin(lcm_dev.reg_select, GPIO_DIR_OUT, GPIO_INT_DISABLE);
	__lcm_initGpioPin(lcm_dev.enable, GPIO_DIR_OUT, GPIO_INT_DISABLE);
	__lcm_initGpioPin(lcm_dev.read_write, GPIO_DIR_OUT, GPIO_INT_DISABLE);
	
	if (in_out == 1)//config DB7~DB4 output 
		_lcm_char16x2_data_port_init(&lcm_dev,in_out);	
	else if (in_out == 0)//config DB7~DB4 input
		_lcm_char16x2_data_port_init(&lcm_dev,in_out);
	else
		printk("Wrong data pin type\n");
		
	return;	

}

//------------------------------------------

static void init_lcm_char16x2_gpio(int lcm_gpio)
{	
	printk("( GPIO %s )  ", GPIO_LCM );
	if (lcm_gpio == 0)
	{
		gpio_pin_config(PIN_RS,PIN_ENABLE,PIN_R_W_,PIN_DB7,PIN_DB6,PIN_DB5,PIN_DB4,1);
		printk("For LCM port init=> OK !\n");
	}
	return;	
}

/************************* Read & Write LCM ***********************************/

static unsigned char read_lcm_char16x2_IR(void)
{
	unsigned char buf;
#ifdef LCM_CLI_PROTECT	
	unsigned long flags;
	save_flags(flags); cli();
#endif	
	_lcm_char16x2_rawRead(&lcm_dev,&buf,8,INSTRUCT_REGISTER);
#ifdef LCM_CLI_PROTECT	
	restore_flags(flags);
#endif	
	return buf;
}	

static void write_lcm_char16x2_IR(unsigned char data,enum BUSY_FLAG bf_flag,unsigned char bit_interface)
{
#ifdef LCM_CLI_PROTECT	
	unsigned long flags;
	save_flags(flags); cli();
#endif	
	_lcm_char16x2_rawWrite(&lcm_dev,&data,8,INSTRUCT_REGISTER,bf_flag,bit_interface);
#ifdef LCM_CLI_PROTECT	
	restore_flags(flags);
#endif	
	return;
}

static unsigned char read_lcm_char16x2_DR(void)
{
	unsigned char buf;
#ifdef LCM_CLI_PROTECT	
	unsigned long flags;
	save_flags(flags); cli();
#endif	
	_lcm_char16x2_rawRead(&lcm_dev,&buf,8,DATA_REGISTER);
#ifdef LCM_CLI_PROTECT	
	restore_flags(flags);
#endif	
	return buf;
}	

static void write_lcm_char16x2_DR(unsigned char data,enum BUSY_FLAG bf_flag,unsigned char bit_interface)
{
#ifdef LCM_CLI_PROTECT	
	unsigned long flags;
	save_flags(flags); cli();
#endif	
	_lcm_char16x2_rawWrite(&lcm_dev,&data,8,DATA_REGISTER,bf_flag,bit_interface);
#ifdef LCM_CLI_PROTECT	
	restore_flags(flags);
#endif	
	return;
}
				
