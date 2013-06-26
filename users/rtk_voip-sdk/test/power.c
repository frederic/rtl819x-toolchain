#include <unistd.h>

#include "voip_manager.h"

#define GPIO_ID(port,pin) ((uint32)port<<16|(uint32)pin)

#define PIN_VOIP1_LED   GPIO_ID(3/*GPIO_PORT_D*/,1)  //output
#define PIN_VOIP2_LED   GPIO_ID(3/*GPIO_PORT_D*/,2)  //output
#define PIN_VOIP3_LED   GPIO_ID(3/*GPIO_PORT_D*/,3)  //output
#define PIN_VOIP4_LED   GPIO_ID(3/*GPIO_PORT_D*/,4)  //output
#define PIN_PSTN_LED    GPIO_ID(5/*GPIO_PORT_F*/,5)  //output

static void print_help( int bad )
{
	if( bad ) {
		printf( "Bad parameter %d!\n", bad );
	}

	printf( "This tool is used to emulate power cut-off, and functions contains\n" );
	printf( "interval of power on and power off, and repeat times.\n" );
	printf( "\n" );
	printf( "Usage and parameters:\n" );
	printf( "power [d|u] n t_on t_off rep \n" );
	printf( "	d: predefined gpio PIN\n" );
	printf( "	u: user define gpio PIN\n" );
	printf( "	n: PIN number\n" );
	printf( "	   - predefined gpio PIN:\n" );
	printf( "	     1. PIN_VOIP1_LED (D1)\n" );
	printf( "	     2. PIN_VOIP2_LED (D2)\n" );
	printf( "	     3. PIN_VOIP3_LED (D3)\n" );
	printf( "	     4. PIN_VOIP4_LED (D4)\n" );
	printf( "	   - user define gpio PIN:\n" );
	printf( "	     GPIO ID ( port << 16 | pin )\n" );
	printf( "	t_on: power on interval (in unit of ms)\n" );
	printf( "	t_off: power off interval (in unit of ms)\n" );
	printf( "	rep: repeat times (0:infinite)\n" );
	printf( "	pol: polarity (power on is 0 or 1)\n" );
	printf( "\n" );
	printf( "Example:\n" );
	printf( "power d 4 200 100 10 0\n" );
	printf( "	GPIO = PIN_VOIP4_LED (D4)\n" );
	printf( "	power on interval is 200ms\n" );
	printf( "	power off interval is 100ms\n" );
	printf( "	repeat times is 10\n" );
	printf( "	GPIO polarity is 0\n" );
}

int power_main(int argc, char *argv[])
{
	unsigned long gpio;
	unsigned long ret;
	unsigned long t_on, t_off, rep, pol;
	unsigned long t;
	unsigned long i;
	unsigned long result;

	/* check parameters */
	if( argc != 6 + 1 ) {
		print_help( 0 );
		return 0;
	}
	
	if( argv[ 1 ][ 0 ] == 'd' ) {
	
		t = atoi( argv[ 2 ] );
		
		switch( t ) {
		case 1:
			gpio = PIN_VOIP1_LED;
			break;
		case 2:
			gpio = PIN_VOIP2_LED;
			break;
		case 3:
			gpio = PIN_VOIP3_LED;
			break;
		case 4:
			gpio = PIN_VOIP4_LED;
			break;
		default:
			print_help( 2 );
			return 1;
		}
		
	} else if( argv[ 1 ][ 0 ] == 'u' ) {
		
		gpio = atoi( argv[ 2 ] );
	
	} else {
		print_help( 1 );
		return 1;
	}
	
	t_on = atol( argv[ 3 ] );
	t_off = atol( argv[ 4 ] );
	rep = atol( argv[ 5 ] );
	pol = atol( argv[ 6 ] );
	
	/* print message for testing */
	printf( "Now, we are going to test GPIO %c-%d, ", ( gpio >> 16 ) + 'A', gpio & 0xFFFF );
	printf( "and power on and off intervals are %lu and %lu respectively.\n", t_on, t_off );
	printf( "Repeat times is %lu.\n", rep );
	
	/* It is time to control GPIO */
	rtk_gpio( 0, gpio, 0 | ( 1 << 2 ) | ( 0 << 3 ), &result );	
						// bit0~1: GPIO_PERIPHERAL 
						// bit2: GPIO_DIRECTION 
						// bit3: GPIO_INTERRUPT_TYPE 
	
	for( i = 0; rep == 0 || i < rep; i ++ ) {
		
		/* power on */
		rtk_gpio( 2, gpio, ( pol ? 1 : 0 ), &result );

		if( t_on > 1000 )
			printf( "On - %lu.%lu sec\n", t_on / 1000, t_on % 1000 );			
		else
			printf( "On - %lu ms\n", t_on );
		
		if( t_on < ( 0x7FFFFFFF / 1000 ) )
			usleep( t_on * 1000 );
		else
			sleep( t_on / 1000 );
		
		/* power off */
		rtk_gpio( 2, gpio, ( pol ? 0 : 1 ), &result );
		
		if( t_off > 1000 )
			printf( "Off - %lu.%lu sec\n", t_off / 1000, t_off % 1000 );
		else
			printf( "Off - %lu ms\n", t_off );
		
		if( t_off < ( 0x7FFFFFFF / 1000 ) )
			usleep( t_off * 1000 );
		else
			sleep( t_off / 1000 );
	}
	
	return 0;
}

