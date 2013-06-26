#include <stdio.h>
#include <unistd.h>
#include "ui_config.h"
#include "ioctl_lcd.h"
#include "ioctl_keypad.h"
#include "ioctl_timer.h"
#include "ioctl_led.h"
#include "flash_rw_api.h"
#include "ui_event_handler.h"
#include "ui_host.h"
#include "ui_exit.h"

#define _BLOCK_RCV_MSG_MODE

void InitializeDevicesOfUI( void )
{
	/*
	 * Initialize hardware / event handler / UI.
	 */
	TurnOffAllLEDsThroughGPIO();
	InitializeLCD();
	InitializeKeypad();
	InitializeTimer();

	InitializeFlash();
	
	InitializeEventHandler();
		
	UI_BasicInitialization();
}

void TerminateDevicesOfUI( void )
{
	/*
	 * Terminate hardware / graphic system / event handler.
	 */
	TerminateLCD();
	TerminateKeypad();
	TerminateTimer();

	TerminateFlash();
		 
	TerminateEventHandler();
	
	UI_Termination();
}

int main( int argc, char **argv )
{
	int ret;
	/*
	 * Parse arguments if necessary
	 */
	
	//printf( "phone book: %d\n", sizeof( phonebook_info_t ) );
	//printf( "call record: %d\n", sizeof( call_records_t ) );
	//printf( "total: %d\n", sizeof( ui_falsh_layout_t ) );
	
	/* Initalize UI devices */
	InitializeDevicesOfUI();
	
	/* Run it after all devices' initialization */
	InitializeExitHandler();
		
	/*
	 * Enter main loop to wait message.
	 */
	while( 1 ) {
		
#ifdef _BLOCK_RCV_MSG_MODE
		if( ( ret = UIEventLauncher( 0 /* No wait */ ) ) != RET_OK ) 
#else
		if( ( ret = UIEventLauncher( 1 /* No wait */ ) ) != RET_OK ) 
#endif
		{
			/* Ask exit or not initalize */
			if( ret != RET_ASK_EXIT )
				debug_out( "Program exit reason: %d\n", ret );
			break;
		}
		
		//printf( "S " );

#ifndef _BLOCK_RCV_MSG_MODE
		sleep( 1 );
#endif
	}
	
	/* Display exit message */
	DisplayExitMessage();

	/* Terminate UI devices */
	TerminateDevicesOfUI();

	return 0;	
}

