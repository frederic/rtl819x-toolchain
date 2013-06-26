#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/init.h>
#include <linux/sched.h>
#include "../gpio/gpio.h"
#include "base_gpio.h"
#include "keyscan_tri.h"
#include "keypad_map.h"
#include "voip_timer.h"


//-----------------------------
#if 1	//0: kernel used(udelay), 1: test program used(for(;;)) 
#define __test_program_used__
#else
#define __kernel_used__
#endif

//----------------------- keypad -------------------------------------------
#define _KEYPAD_PIN_	//GPIO pin
#define KEYPAD_RATING_FACTOR	4	//debounce check
#define KEYPAD_ROW_NUMBER	6	//input 
#define KEYPAD_COLUMN_NUMBER	7	//output
//The GPIO's interrupt of 8186 is edge-trigger. So when u want to use interrupt mode, 
//PLEASE CHOOSE MACRO KEYPAD_INTERRUPT_ENABLE
#if 1		
#define INPUT_PULL_HIGH_RESISTOR	//input pull-high resistor.for interrupt mode or polling mode
#define CHECK_FLAG	0
#else		
#define INPUT_PULL_LOW_RESISTOR		//input pull-low resistor.for interrupt mode or polling mode 
#define CHECK_FLAG	1
#endif
#if 1		//enable interrupt
#define KEYPAD_INTERRUPT_ENABLE
#else		//disable interrupt
#define KEYPAD_INTERRUPT_DISABLE
#endif
//-----------------------------------------------------------------------------

//----------------------- tri-keypad -------------------------------------------
#define _TRI_KEYPAD_PIN_	//GPIO pin
#define TRI_KEYPAD_RATING_FACTOR	1	//debounce check
#define TRI_KEYPAD_ROW_NUMBER	10	//input/output
#define TRI_CHECK_FLAG	0		//interrupt pin :pull high
#define _POLLING_INTERRUPT_ 0	//_POLLING_INTERRUPT_:0 -> polling mode, _POLLING_INTERRUPT_:1 ->interrupt mode
#if _POLLING_INTERRUPT_
#define _TRI_KEYPAD_INTERRUPT_
#else
#define _TRI_KEYPAD_POLLING_
#endif
//-----------------------------------------------------------------------------

/************************ KEYPAD data struct *******************************************/
struct rtl8186_keypad_dev_s
{
	unsigned int row0;		//input
	unsigned int row1;		//input
	unsigned int row2;		//input
	unsigned int row3;		//input
	unsigned int row4;		//input
	unsigned int row5;		//input
	unsigned int column0;		//output
	unsigned int column1;		//output
	unsigned int column2;		//output
	unsigned int column3;		//output
	unsigned int column4;		//output
	unsigned int column5;		//output
	unsigned int column6;		//output
};	

typedef struct rtl8186_keypad_dev_s rtl8186_keypad_dev_t;
/*******************************************************************/

/************************ TRI_KEYPAD data struct *******************************************/
struct rtl8186_tri_keypad_dev_s
{
	unsigned int tri_row0;		//input/output
	unsigned int tri_row1;		//input/output
	unsigned int tri_row2;		//input/output
	unsigned int tri_row3;		//input/output
	unsigned int tri_row4;		//input/output
	unsigned int tri_row5;		//input/output
	unsigned int tri_row6;		//input/output
	unsigned int tri_row7;		//input/output
	unsigned int tri_row8;		//input/output
	unsigned int tri_row9;		//input/output
	
	unsigned int tri_interrupt;		//input	
};	

typedef struct rtl8186_tri_keypad_dev_s rtl8186_tri_keypad_dev_t;
/*******************************************************************/

#ifdef _KEYPAD_PIN_
#define GPIO_KEYPAD "C"
#define KEYPAD_ROW_0		GPIO_ID(GPIO_PORT_C,7)	//input
#define KEYPAD_ROW_1		GPIO_ID(GPIO_PORT_C,8)	//input
#define KEYPAD_ROW_2		GPIO_ID(GPIO_PORT_C,9)	//input
#define KEYPAD_ROW_3		GPIO_ID(GPIO_PORT_C,10)	//input
#define KEYPAD_ROW_4		GPIO_ID(GPIO_PORT_C,11)	//input
#define KEYPAD_ROW_5		GPIO_ID(GPIO_PORT_C,12)	//input
#define KEYPAD_COLUMN_0		GPIO_ID(GPIO_PORT_C,0)	//output
#define KEYPAD_COLUMN_1		GPIO_ID(GPIO_PORT_C,1)	//output
#define KEYPAD_COLUMN_2		GPIO_ID(GPIO_PORT_C,2)	//output
#define KEYPAD_COLUMN_3		GPIO_ID(GPIO_PORT_C,3)	//output
#define KEYPAD_COLUMN_4		GPIO_ID(GPIO_PORT_C,4)	//output
#define KEYPAD_COLUMN_5		GPIO_ID(GPIO_PORT_C,5)	//output
#define KEYPAD_COLUMN_6		GPIO_ID(GPIO_PORT_C,6)	//output
#endif

#ifdef _TRI_KEYPAD_PIN_
#if defined(CONFIG_RTK_VOIP_GPIO_IPP_100)
#if 0
#define GPIO_TRI_KEYPAD "C"
#define TRI_KEYPAD_ROW_0		GPIO_ID(GPIO_PORT_C,6)	//input/output
#define TRI_KEYPAD_ROW_1		GPIO_ID(GPIO_PORT_C,7)	//input/output
#define TRI_KEYPAD_ROW_2		GPIO_ID(GPIO_PORT_C,8)	//input/output
#define TRI_KEYPAD_ROW_3		GPIO_ID(GPIO_PORT_C,9)	//input/output
#define TRI_KEYPAD_ROW_4		GPIO_ID(GPIO_PORT_C,10)	//input/output
#define TRI_KEYPAD_ROW_5		GPIO_ID(GPIO_PORT_C,11)	//input/output
#define TRI_KEYPAD_ROW_6		GPIO_ID(GPIO_PORT_C,12)	//input/output
#define TRI_KEYPAD_ROW_7		GPIO_ID(GPIO_PORT_C,13)	//input/output
#define TRI_KEYPAD_ROW_8		GPIO_ID(GPIO_PORT_C,14)	//input/output
#define TRI_KEYPAD_ROW_9		GPIO_ID(GPIO_PORT_C,15)	//input/output
#define TRI_INTERRUPT			GPIO_ID(GPIO_PORT_C,5)	//input
#else
#define GPIO_TRI_KEYPAD "A"
#define TRI_KEYPAD_ROW_0		GPIO_ID(GPIO_PORT_C,13)//GPIO_ID(GPIO_PORT_E,0)	//input/output
#define TRI_KEYPAD_ROW_1		GPIO_ID(GPIO_PORT_A,8)	//input/output
#define TRI_KEYPAD_ROW_2		GPIO_ID(GPIO_PORT_A,7)	//input/output
#define TRI_KEYPAD_ROW_3		GPIO_ID(GPIO_PORT_A,6)	//input/output
#define TRI_KEYPAD_ROW_4		GPIO_ID(GPIO_PORT_C,14)//GPIO_ID(GPIO_PORT_E,1)	//input/output
#define TRI_KEYPAD_ROW_5		GPIO_ID(GPIO_PORT_C,12)//GPIO_ID(GPIO_PORT_E,2)	//input/output
#define TRI_KEYPAD_ROW_6		GPIO_ID(GPIO_PORT_A,3)	//input/output
#define TRI_KEYPAD_ROW_7		GPIO_ID(GPIO_PORT_A,2)	//input/output
#define TRI_KEYPAD_ROW_8		GPIO_ID(GPIO_PORT_A,1)	//input/output
#define TRI_KEYPAD_ROW_9		GPIO_ID(GPIO_PORT_A,0)	//input/output
#define TRI_INTERRUPT			GPIO_ID(GPIO_PORT_D,0)	//input
#endif
#elif defined(CONFIG_RTK_VOIP_GPIO_IPP_101)
#define GPIO_TRI_KEYPAD "C"
#define TRI_KEYPAD_ROW_0		GPIO_ID(GPIO_PORT_C,0)	//input/output
#define TRI_KEYPAD_ROW_1		GPIO_ID(GPIO_PORT_C,1)	//input/output
#define TRI_KEYPAD_ROW_2		GPIO_ID(GPIO_PORT_C,2)	//input/output
#define TRI_KEYPAD_ROW_3		GPIO_ID(GPIO_PORT_C,3)	//input/output
#define TRI_KEYPAD_ROW_4		GPIO_ID(GPIO_PORT_C,4)	//input/output
#define TRI_KEYPAD_ROW_5		GPIO_ID(GPIO_PORT_C,5)	//input/output
#define TRI_KEYPAD_ROW_6		GPIO_ID(GPIO_PORT_C,6)	//input/output
#define TRI_KEYPAD_ROW_7		GPIO_ID(GPIO_PORT_C,7)	//input/output
#define TRI_KEYPAD_ROW_8		GPIO_ID(GPIO_PORT_C,8)	//input/output
#define TRI_KEYPAD_ROW_9		GPIO_ID(GPIO_PORT_C,9)	//input/output
#define TRI_INTERRUPT			GPIO_ID(GPIO_PORT_C,10)	//input
#endif
#endif

/********************* keypad data struct **********************/
static rtl8186_keypad_dev_t keypad_dev;
keypad_dev_t keypad_data_pool={0,0};
//-------------- keypad api-------------------------------------//
void init_keypad_gpio(void)
{
	printk("( GPIO %s ) ", GPIO_KEYPAD);
	
	keypad_dev.row0 = KEYPAD_ROW_0;
	keypad_dev.row1 = KEYPAD_ROW_1;
	keypad_dev.row2 = KEYPAD_ROW_2;
	keypad_dev.row3 = KEYPAD_ROW_3;
	keypad_dev.row4 = KEYPAD_ROW_4;
	keypad_dev.row5 = KEYPAD_ROW_5;
	keypad_dev.column0 = KEYPAD_COLUMN_0;
	keypad_dev.column1 = KEYPAD_COLUMN_1;
	keypad_dev.column2 = KEYPAD_COLUMN_2;
	keypad_dev.column3 = KEYPAD_COLUMN_3;
	keypad_dev.column4 = KEYPAD_COLUMN_4;
	keypad_dev.column5 = KEYPAD_COLUMN_5;
	keypad_dev.column6 = KEYPAD_COLUMN_6;
	#ifdef KEYPAD_INTERRUPT_ENABLE
	_rtl8186_initGpioPin(keypad_dev.row0, GPIO_DIR_IN, GPIO_INT_ENABLE);
	_rtl8186_initGpioPin(keypad_dev.row1, GPIO_DIR_IN, GPIO_INT_ENABLE);
	_rtl8186_initGpioPin(keypad_dev.row2, GPIO_DIR_IN, GPIO_INT_ENABLE);
	_rtl8186_initGpioPin(keypad_dev.row3, GPIO_DIR_IN, GPIO_INT_ENABLE);
	_rtl8186_initGpioPin(keypad_dev.row4, GPIO_DIR_IN, GPIO_INT_ENABLE);
	_rtl8186_initGpioPin(keypad_dev.row5, GPIO_DIR_IN, GPIO_INT_ENABLE);
	#endif
	#ifdef KEYPAD_INTERRUPT_DISABLE
	_rtl8186_initGpioPin(keypad_dev.row0, GPIO_DIR_IN, GPIO_INT_DISABLE);
	_rtl8186_initGpioPin(keypad_dev.row1, GPIO_DIR_IN, GPIO_INT_DISABLE);
	_rtl8186_initGpioPin(keypad_dev.row2, GPIO_DIR_IN, GPIO_INT_DISABLE);
	_rtl8186_initGpioPin(keypad_dev.row3, GPIO_DIR_IN, GPIO_INT_DISABLE);
	_rtl8186_initGpioPin(keypad_dev.row4, GPIO_DIR_IN, GPIO_INT_DISABLE);
	_rtl8186_initGpioPin(keypad_dev.row5, GPIO_DIR_IN, GPIO_INT_DISABLE);
	#endif
	_rtl8186_initGpioPin(keypad_dev.column0, GPIO_DIR_OUT, GPIO_INT_DISABLE);
	_rtl8186_initGpioPin(keypad_dev.column1, GPIO_DIR_OUT, GPIO_INT_DISABLE);
	_rtl8186_initGpioPin(keypad_dev.column2, GPIO_DIR_OUT, GPIO_INT_DISABLE);
	_rtl8186_initGpioPin(keypad_dev.column3, GPIO_DIR_OUT, GPIO_INT_DISABLE);
	_rtl8186_initGpioPin(keypad_dev.column4, GPIO_DIR_OUT, GPIO_INT_DISABLE);
	_rtl8186_initGpioPin(keypad_dev.column5, GPIO_DIR_OUT, GPIO_INT_DISABLE);
	_rtl8186_initGpioPin(keypad_dev.column6, GPIO_DIR_OUT, GPIO_INT_DISABLE);
	printk("For KEYPAD port init=> OK !\n");
	
	return;
}

void keyscan(void)
{
	#ifdef INPUT_PULL_HIGH_RESISTOR
	const unsigned char column_port_data[KEYPAD_COLUMN_NUMBER] = {0x7e,0x7d,0x7b,0x77,0x6f,0x5f,0x3f};
	#endif
	#ifdef INPUT_PULL_LOW_RESISTOR
	const unsigned char column_port_data[KEYPAD_COLUMN_NUMBER] = {0x01,0x02,0x04,0x08,0x10,0x20,0x40};
	#endif
	unsigned char column_data, row_data;
	unsigned char column_temp, row_temp, input_data_temp;
	int i;
	unsigned int buf;	
	static unsigned char counter = 0;
	
	for (column_temp=0; column_temp<KEYPAD_COLUMN_NUMBER; column_temp++)
	{
			
		_rtl8186_setGpioDataBit(keypad_dev.column0,(column_port_data[0]>>column_temp));
		_rtl8186_setGpioDataBit(keypad_dev.column1,(column_port_data[1]>>column_temp));
		_rtl8186_setGpioDataBit(keypad_dev.column2,(column_port_data[2]>>column_temp));
		_rtl8186_setGpioDataBit(keypad_dev.column3,(column_port_data[3]>>column_temp));
		_rtl8186_setGpioDataBit(keypad_dev.column4,(column_port_data[4]>>column_temp));
		_rtl8186_setGpioDataBit(keypad_dev.column5,(column_port_data[5]>>column_temp));
		_rtl8186_setGpioDataBit(keypad_dev.column6,(column_port_data[6]>>column_temp));
		column_data = column_temp;
		//WaitKey();
		for (row_temp=0; row_temp<KEYPAD_ROW_NUMBER; row_temp++)
		{
			row_data = row_temp;
			_rtl8186_getGpioDataBit(*(&keypad_dev.row0+row_temp),&buf);	
			
			if (buf == CHECK_FLAG)
			{
				//debounce check
				#ifdef __kernel_used__
				mdelay(1*KEYPAD_RATING_FACTOR);
				#endif
				#ifdef __test_program_used__
				for (i=0;i<KEYPAD_RATING_FACTOR*100000;i++);
				#endif
				_rtl8186_getGpioDataBit(*(&keypad_dev.row0+row_temp),&buf);
				
				if (buf == CHECK_FLAG)	//finish check 
				{
					printk("  row=%d,column=%d  ",row_data,column_data);	       
					input_data_temp = (row_data<<4) | column_data;         
					if (keypad_data_pool.flags == 0)                       
					{                                                      
						keypad_data_pool.data_string = input_data_temp;
						keypad_data_pool.flags = 1;                    
					}                                                      
				}	
			}//end if	
		}//end for
	}//end for	
		
	

	
#ifdef INPUT_PULL_HIGH_RESISTOR	
	_rtl8186_setGpioDataBit(keypad_dev.column0,0);
	_rtl8186_setGpioDataBit(keypad_dev.column1,0);
	_rtl8186_setGpioDataBit(keypad_dev.column2,0);
	_rtl8186_setGpioDataBit(keypad_dev.column3,0);
	_rtl8186_setGpioDataBit(keypad_dev.column4,0);
	_rtl8186_setGpioDataBit(keypad_dev.column5,0);
	_rtl8186_setGpioDataBit(keypad_dev.column6,0);
#endif

#ifdef INPUT_PULL_LOW_RESISTOR	
	_rtl8186_setGpioDataBit(keypad_dev.column0,1);
	_rtl8186_setGpioDataBit(keypad_dev.column1,1);
	_rtl8186_setGpioDataBit(keypad_dev.column2,1);
	_rtl8186_setGpioDataBit(keypad_dev.column3,1);
	_rtl8186_setGpioDataBit(keypad_dev.column4,1);
	_rtl8186_setGpioDataBit(keypad_dev.column5,1);
	_rtl8186_setGpioDataBit(keypad_dev.column6,1);
#endif
	
	return;
}		
        
/********************* tri_keypad data struct **********************/
static rtl8186_tri_keypad_dev_t tri_keypad_dev;
//keypad_dev_t tri_keypad_data_pool={0,0};
//-------------- tri_keypad api-------------------------------------//
void init_tri_keypad_gpio(void)
{
	printk("( GPIO %s ) ", GPIO_TRI_KEYPAD);
	
	tri_keypad_dev.tri_row0 = TRI_KEYPAD_ROW_0;
	tri_keypad_dev.tri_row1 = TRI_KEYPAD_ROW_1;
	tri_keypad_dev.tri_row2 = TRI_KEYPAD_ROW_2;
	tri_keypad_dev.tri_row3 = TRI_KEYPAD_ROW_3;
	tri_keypad_dev.tri_row4 = TRI_KEYPAD_ROW_4;
	tri_keypad_dev.tri_row5 = TRI_KEYPAD_ROW_5;
	tri_keypad_dev.tri_row6 = TRI_KEYPAD_ROW_6;
	tri_keypad_dev.tri_row7 = TRI_KEYPAD_ROW_7;
	tri_keypad_dev.tri_row8 = TRI_KEYPAD_ROW_8;
	tri_keypad_dev.tri_row9 = TRI_KEYPAD_ROW_9;
	tri_keypad_dev.tri_interrupt = TRI_INTERRUPT;
	
	_rtl8186_initGpioPin(tri_keypad_dev.tri_row0, GPIO_DIR_OUT, GPIO_INT_DISABLE);
	_rtl8186_initGpioPin(tri_keypad_dev.tri_row1, GPIO_DIR_OUT, GPIO_INT_DISABLE);
	_rtl8186_initGpioPin(tri_keypad_dev.tri_row2, GPIO_DIR_OUT, GPIO_INT_DISABLE);
	_rtl8186_initGpioPin(tri_keypad_dev.tri_row3, GPIO_DIR_OUT, GPIO_INT_DISABLE);
	_rtl8186_initGpioPin(tri_keypad_dev.tri_row4, GPIO_DIR_OUT, GPIO_INT_DISABLE);
	_rtl8186_initGpioPin(tri_keypad_dev.tri_row5, GPIO_DIR_OUT, GPIO_INT_DISABLE);
	_rtl8186_initGpioPin(tri_keypad_dev.tri_row6, GPIO_DIR_OUT, GPIO_INT_DISABLE);
	_rtl8186_initGpioPin(tri_keypad_dev.tri_row7, GPIO_DIR_OUT, GPIO_INT_DISABLE);
	_rtl8186_initGpioPin(tri_keypad_dev.tri_row8, GPIO_DIR_OUT, GPIO_INT_DISABLE);
	_rtl8186_initGpioPin(tri_keypad_dev.tri_row9, GPIO_DIR_OUT, GPIO_INT_DISABLE);

#ifdef _TRI_KEYPAD_INTERRUPT_	
	_rtl8186_initGpioPin(tri_keypad_dev.tri_interrupt, GPIO_DIR_IN, GPIO_INT_ENABLE);
#endif	
	//Initial state of all data pin(exclusive interrupt pin) is output and set to 0.
	_rtl8186_setGpioDataBit(tri_keypad_dev.tri_row0,0);
	_rtl8186_setGpioDataBit(tri_keypad_dev.tri_row1,0);
	_rtl8186_setGpioDataBit(tri_keypad_dev.tri_row2,0);
	_rtl8186_setGpioDataBit(tri_keypad_dev.tri_row3,0);
	_rtl8186_setGpioDataBit(tri_keypad_dev.tri_row4,0);
	_rtl8186_setGpioDataBit(tri_keypad_dev.tri_row5,0);
	_rtl8186_setGpioDataBit(tri_keypad_dev.tri_row6,0);
	_rtl8186_setGpioDataBit(tri_keypad_dev.tri_row7,0);
	_rtl8186_setGpioDataBit(tri_keypad_dev.tri_row8,0);
	_rtl8186_setGpioDataBit(tri_keypad_dev.tri_row9,0);	
	printk("For TRI_KEYPAD port init=> OK !\n");
	return;
}

void tri_keyscan(void)
{
	unsigned char column_data, row_data;
	unsigned char column_temp, row_temp, input_data_temp;
	int i;
	unsigned int buf, one_more_trigger = 0;
#ifdef _TRI_KEYPAD_POLLING_
	unsigned long flags;
	save_flags(flags); cli();
#endif
		
	for (row_temp=0 ;row_temp<(TRI_KEYPAD_ROW_NUMBER-1) ;row_temp++) {
		_rtl8186_initGpioPin(*(&tri_keypad_dev.tri_row0+row_temp), GPIO_DIR_OUT, GPIO_INT_DISABLE);
		_rtl8186_setGpioDataBit(*(&tri_keypad_dev.tri_row0+row_temp),TRI_CHECK_FLAG);
		row_data = row_temp;
		for (i=row_temp+1 ;i<TRI_KEYPAD_ROW_NUMBER ;i++) {
			_rtl8186_setGpioDataBit(*(&tri_keypad_dev.tri_row0+i),!TRI_CHECK_FLAG);
		}
		
		for (column_temp=row_temp+1 ;column_temp<TRI_KEYPAD_ROW_NUMBER ;column_temp++) {
			column_data = column_temp;
			_rtl8186_initGpioPin(*(&tri_keypad_dev.tri_row0+column_temp), GPIO_DIR_IN, GPIO_INT_DISABLE);
			_rtl8186_getGpioDataBit(*(&tri_keypad_dev.tri_row0+column_temp),&buf);	
			//printk(" buf=%d ",buf);
			if (buf == TRI_CHECK_FLAG) {
				//debounce check
				#ifndef CONFIG_RTK_VOIP_IP_PHONE
				#ifdef __kernel_used__
				mdelay(1*TRI_KEYPAD_RATING_FACTOR);
				#endif
				#ifdef __test_program_used__
				for (i=0;i<TRI_KEYPAD_RATING_FACTOR*100000;i++);
				#endif
				#endif
				_rtl8186_getGpioDataBit(*(&tri_keypad_dev.tri_row0+column_temp),&buf);
				
				if (buf == TRI_CHECK_FLAG)	{//finish check 
					//printk("  row=%d,column=%d  ",row_data,column_data);	       
					input_data_temp = (row_data<<4) | column_data;         
					one_more_trigger++;
					//goto label;
				}	
			}//end if
		}//end for
		_rtl8186_setGpioDataBit(*(&tri_keypad_dev.tri_row0+row_data),!TRI_CHECK_FLAG);
	}//end for
//label:
	for (row_temp=0 ;row_temp<TRI_KEYPAD_ROW_NUMBER ;row_temp++) {
		//Initial state of all data pin(exclusive interrupt pin) is output and set to 0.
		_rtl8186_initGpioPin(*(&tri_keypad_dev.tri_row0+row_temp), GPIO_DIR_OUT, GPIO_INT_DISABLE);
		_rtl8186_setGpioDataBit(*(&tri_keypad_dev.tri_row0+row_temp),TRI_CHECK_FLAG);
	}
#ifdef _TRI_KEYPAD_POLLING_
	//printk("@");
	restore_flags(flags);
#endif		
	if (keypad_data_pool.flags == 0 && one_more_trigger == 1) {                                                      
			keypad_data_pool.data_string = input_data_temp;
			keypad_data_pool.flags = 1;
			//printk("  row=%d,column=%d  ",input_data_temp>>4,input_data_temp&0x0f);                    

#ifdef CONFIG_RTK_VOIP_IP_PHONE
			{
				/* signal to UI */
				extern void keypad_polling_signal_target( keypad_dev_t *keypad_data_pool );
				keypad_polling_signal_target( &keypad_data_pool );
			}
#endif /* CONFIG_RTK_VOIP_IP_PHONE */
	}  

#ifdef CONFIG_RTK_VOIP_IP_PHONE
	{
		extern void keypad_polling_scan_done( void );
		keypad_polling_scan_done();
	}
#endif
	
	return;
}

#ifdef _TRI_KEYPAD_POLLING_
void tri_keypad_polling()
{
	init_tri_keypad_gpio();
	
	register_timer_10ms( ( fn_timer_t )tri_keyscan, NULL );
	
	return;
}

//module_init(tri_keypad_polling);
#endif
