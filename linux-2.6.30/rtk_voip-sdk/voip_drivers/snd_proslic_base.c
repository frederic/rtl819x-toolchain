#include <linux/kernel.h>
#include <linux/timer.h>
#include <linux/sched.h>
#include <linux/delay.h>
#ifdef CONFIG_DEFAULTS_KERNEL_2_6
#include <linux/interrupt.h>
#endif
#include "snd_define.h"
#include "con_register.h"

#include "rtk_voip.h"
#include "spi.h"
#include "voip_types.h"
#include "voip_control.h"

#include "snd_proslic_type.h"


#define ts2count( ts )	( 1 + ( ts ) * 8 )		// Time slot to silab's count 

// --------------------------------------------------------
// proslic allocate / init function  
// --------------------------------------------------------

#ifdef CONFIG_RTK_VOIP_DRIVERS_SLIC_SI3226
#define PHONE_CHECK_CONNECT_COUNT_SI3226	18	// for Si3226  on RTL8972B
#define PHONE_CHECK_TIME_OUT_COUNT_SI3226	60
#define VOLT_TR_THRESHOL_SI3226	0x2E80000	// Vtr > 45V(0x2E20000) , 44V(0x2D00000)		
#define VOLT_TIP_THRESHOL_SI3226	0x3200000
#endif

#ifdef CONFIG_RTK_VOIP_DRIVERS_SLIC_SI3226x
#define PHONE_CHECK_CONNECT_COUNT_SI3226x        18      // for Si3226  on RTL8972B
#define PHONE_CHECK_TIME_OUT_COUNT_SI3226x       60
#define VOLT_TR_THRESHOL_SI3226x 0x2E80000       // Vtr > 45V(0x2E20000) , 44V(0x2D00000)
#define VOLT_TIP_THRESHOL_SI3226x        0x3200000
#endif


#ifdef CONFIG_RTK_VOIP_DRIVERS_SLIC_SI3217x
#define PHONE_CHECK_CONNECT_COUNT_SI3217X	11	// for Si3217x on RTL8972B
#define PHONE_CHECK_TIME_OUT_COUNT_SI3217X	60
#define VOLT_TR_THRESHOL_SI3217X	0x3000000	
#define VOLT_TIP_THRESHOL_SI3217X	0x3300000
#endif

void proslic_init_user_objs( ProslicContainer_t container[], 
		int size, int type )
{
	int i;
	
	// init user
	for( i = 0; i < size; i ++ ) {
		container[ i ].user.hookStatus = INVALID_HOOK_STATUS;
		container[ i ].user.pcm_law_save = 0xFF;
		
		switch( type ) {
#ifdef CONFIG_RTK_VOIP_DRIVERS_SLIC_SI3217x
		case SI3217X_TYPE:
			container[ i ].line.connect_count = PHONE_CHECK_CONNECT_COUNT_SI3217X;
			container[ i ].line.timeout_count = PHONE_CHECK_TIME_OUT_COUNT_SI3217X;
			container[ i ].line.TR_thres = VOLT_TR_THRESHOL_SI3217X;
			container[ i ].line.TIP_thres = VOLT_TIP_THRESHOL_SI3217X;
			break;
#endif
#ifdef CONFIG_RTK_VOIP_DRIVERS_SLIC_SI3226
		case SI3226_TYPE:
			container[ i ].line.connect_count = PHONE_CHECK_CONNECT_COUNT_SI3226;
			container[ i ].line.timeout_count = PHONE_CHECK_TIME_OUT_COUNT_SI3226;
			container[ i ].line.TR_thres = VOLT_TR_THRESHOL_SI3226;
			container[ i ].line.TIP_thres = VOLT_TIP_THRESHOL_SI3226;
			break;
#endif
#ifdef CONFIG_RTK_VOIP_DRIVERS_SLIC_SI3226x
                case SI3226X_TYPE:
                        container[ i ].line.connect_count = PHONE_CHECK_CONNECT_COUNT_SI3226x;
                        container[ i ].line.timeout_count = PHONE_CHECK_TIME_OUT_COUNT_SI3226x;
                        container[ i ].line.TR_thres = VOLT_TR_THRESHOL_SI3226x;
                        container[ i ].line.TIP_thres = VOLT_TIP_THRESHOL_SI3226x;
                        break;
#endif
		default:
		case SI321X_TYPE:
		case SI324X_TYPE:
		case SI3220_TYPE:
		//case SI3226X_TYPE:
			PRINT_R( "Undefined Proslic type\n" );
			break;
		}
	}
}

void proslic_alloc_objs( ProslicContainer_t container[], 
		int size, int devices, int chan, int type )
{
	// once call this function, decide:
	//  - one control interface 
	//  - n devices (chip) - 'devices'
	//  - m proslic channel (FXS) - 'chan'
	//  - at most one daa (if size-chan==1 )
	
	// -------------------------------------------------------
	//                                       chanPerDevice
	// Si32176 x 1:
	//   size = 1, devices = 1, chan = 1 -->       1
	// Si32178 x 1:
	//   size = 2, devices = 1, chan = 1 -->       1 
	// Si3226 x 1:
	//   size = 2, devices = 1, chan = 2 -->       2 
	// Si3226 x 2:
	//   size = 4, devices = 2, chan = 4 -->       2 
	// Si32176 + Si32178
	//   size = 3, devices = 2, chan = 2 -->       1 
	
	int i, j;
	controlInterfaceType *ProHWIntf;
	ProslicDeviceType *ProSLICDevices;
	const int oneDaaIdx = ( size - chan == 1 ? size - 1 : -1 );
	const int chanPerDevice = ( chan / devices );	// I think all devices have same channel 
	
	// check some thing 
	if( size <= 0 || size < devices || size < chan || 
		( size != chan && size != ( chan + 1 ) ) ) 
	{
		PRINT_R( "proslic allocate objects error!!\n" );
		PRINT_R( "size=%d, devicdes=%d, chan=%d\n", size, devices, chan );
		return;
	}
	
	if( chanPerDevice * devices != chan ) 
	{
		PRINT_R( "proslic_alloc_objs can't deal with this case!!\n" );
		PRINT_R( "\tchanPerDevice=%d, devices=%d, size=%d, chan=%d\n", chanPerDevice, devices, size, chan );
	}
	
	// one control interface 
	ProSLIC_createControlInterface( &ProHWIntf );
	
	for( i = 0; i < size; i ++ )
		container[ i ].ProHWIntf = ProHWIntf;
	
	// n device 
	for( i = 0, j = chanPerDevice; i < chan; i ++, j ++ ) {
		if( j == chanPerDevice ) {
			j = 0;
			ProSLIC_createDevice( &ProSLICDevices );
		} 
		
		container[ i ].ProSLICDevices = ProSLICDevices;		
	}
	
	if( oneDaaIdx >= 0 )
		container[ oneDaaIdx ].ProSLICDevices = ProSLICDevices;	// DAA
	
	// m proslic channel
	for( i = 0; i < chan; i ++ ) {
		ProSLIC_createChannel( &container[ i ].ProObj );
		ProSLIC_SWInitChan( container[ i ].ProObj, i, type/*SI3217X_TYPE*/, container[ i ].ProSLICDevices, container[ i ].ProHWIntf );
	}
	
	// at most one daa 
	if( oneDaaIdx >= 0 ) {
#ifdef CONFIG_RTK_VOIP_DRIVERS_FXO
		Vdaa_createChannel( &container[ oneDaaIdx ].daas );
		Vdaa_SWInitChan( container[ oneDaaIdx ].daas, oneDaaIdx, type/*SI3217X_TYPE*/, container[ oneDaaIdx ].ProSLICDevices, container[ oneDaaIdx ].ProHWIntf );
#else
		PRINT_R( "proslic_alloc_objs doesn't have VDAA function\n" );
#endif
	}
}

#ifdef CONFIG_RTK_VOIP_DRIVERS_FXO
#ifndef CONFIG_RTK_VOIP_DRIVERS_VIRTUAL_DAA
static int proslic_initializeDAA( const voip_snd_t p_snds[], 
		vdaaChanType* port[], int pcm_mode)
{
	const unsigned char i = 0;
	
	//for (i=0;i<NUM_OF_DAA;i++){
		Vdaa_setSWDebugMode (port[i],TRUE);

	//}
	if (Vdaa_Init(port,1/*NUM_OF_DAA*/))
	{
		PRINT_R("ERROR: Initialization failed\n");
		return 1;
	}

	//for (i=0;i<NUM_OF_DAA;i++){
	
		//Vdaa_EnableInterrupts(port[i]);
#if 0 //def CONFIG_RTK_VOIP_MULTIPLE_SI32178
		Vdaa_PCMTimeSlotSetup(port[i], 1+2*(i+DAA_CHANNEL_OFFSET)*8, 1+2*(i+DAA_CHANNEL_OFFSET)*8);
#else
		//Vdaa_PCMTimeSlotSetup(port[i], 1+2*SLIC_CH_NUM*8, 1+2*SLIC_CH_NUM*8);
		Vdaa_PCMTimeSlotSetup(port[i], ts2count( p_snds[ i ].TS1 ), ts2count( p_snds[ i ].TS1 ));
#endif
		//printk("fxo-%d: 0x%p, %d, %d\n", i, port[i], 1+(i+DAA_CHANNEL_OFFSET)*8, 1+(i+DAA_CHANNEL_OFFSET)*8);
		
		switch(pcm_mode)
		{
			case 0:// linear
			case 3:
				Vdaa_PCMSetup(port[i], 2, 1/* enable PCM */);
				break;
			case 1:// a-law
			case 4:
				Vdaa_PCMSetup(port[i], 0, 1/* enable PCM */);
				break;
			case 2:// u-law
			case 5:
				Vdaa_PCMSetup(port[i], 1, 1/* enable PCM */);
				break;
			default:// a-law
				Vdaa_PCMSetup(port[i], 0, 1/* enable PCM */);
				break;
		}
		Vdaa_TXAudioGainSetup(port[i], 3);
		//printk("preset gain 3");
	//}
	
	return 0;
}
#endif
#endif

void proslic_init( const voip_snd_t p_snds[],
		ProslicContainer_t container[],
		int size,
		const proslic_args_t *p_args, 
		int pcm_mode)
{
	// container includes at most one daa
	int i=0;
	//int ret = 0;
	extern int SPI_Init (ctrl_S *hSpi);
	unsigned char reg_val;

	extern unsigned char R_reg_dev(rtl_spi_dev_t *pdev, unsigned char chid, unsigned char regaddr);
	extern void W_reg_dev(rtl_spi_dev_t *pdev, unsigned char chid, unsigned char regaddr, unsigned char data);
	extern void W_ram_dev(rtl_spi_dev_t *pdev, unsigned char chid, unsigned short reg, unsigned int data);
	extern int ctrl_ResetWrapper (ctrl_S *hSpiGci, int status);
	extern int ctrl_WriteRegisterWrapper (ctrl_S *hSpiGci, uInt8 channel, uInt8 regAddr, uInt8 data);
	extern uInt8 ctrl_ReadRegisterWrapper (ctrl_S *hSpiGci, uInt8 channel, uInt8 regAddr);
	extern int ctrl_WriteRAMWrapper (ctrl_S *hSpiGci, uInt8 channel, uInt16 ramAddr, ramData data);	
	extern ramData ctrl_ReadRAMWrapper (ctrl_S *hSpiGci, uInt8 channel, uInt16 ramAddr);
	extern int time_DelayWrapper (void *hTimer, int timeInMs);	
	extern int time_TimeElapsedWrapper (void *hTimer, timeStamp *startTime, int *timeInMs);
	extern int time_GetTimeWrapper (void *hTimer, timeStamp *time);	
	
	ctrl_S * const spiGciObj = container[ 0 ].spiGciObj;
	controlInterfaceType * const ProHWIntf = container[ 0 ].ProHWIntf;
	proslicChanType * const ProObj0 = container[ 0 ].ProObj;
	
	const int daaIdx = ( container[ size - 1 ].daas ? size - 1 : -1 );
	const int prochan_num = ( daaIdx < 0 ? size : size - 1 );
	
	proslicChanType_ptr arrayOfProslicChans[ prochan_num /*TOTAL_NUM_OF_CH*/ ];
	proslicChanType *pSlic;
	
	printk("Start SLIC init....\n");
	SPI_Init (spiGciObj);	// Initialize SPI interface(including SLIC reset)
	
	/*** Initialize timer ***/
	//TimerInit(&timerObj);

	ProSLIC_setControlInterfaceCtrlObj (ProHWIntf, spiGciObj);

	/*** Set control functions ***/
	ProSLIC_setControlInterfaceReset (ProHWIntf, ( ctrl_Reset_fptr )ctrl_ResetWrapper);
	ProSLIC_setControlInterfaceWriteRegister (ProHWIntf, ( ctrl_WriteRegister_fptr )ctrl_WriteRegisterWrapper);
	ProSLIC_setControlInterfaceReadRegister (ProHWIntf, ( ctrl_ReadRegister_fptr )ctrl_ReadRegisterWrapper);
	ProSLIC_setControlInterfaceWriteRAM (ProHWIntf, ( ctrl_WriteRAM_fptr )ctrl_WriteRAMWrapper);
	ProSLIC_setControlInterfaceReadRAM (ProHWIntf, ( ctrl_ReadRAM_fptr )ctrl_ReadRAMWrapper);

	/*** Set timer functions ***/
	//ProSLIC_setControlInterfaceTimerObj (ProHWIntf, &timerObj);
	ProSLIC_setControlInterfaceDelay (ProHWIntf, time_DelayWrapper);
	ProSLIC_setControlInterfaceTimeElapsed (ProHWIntf, ( system_timeElapsed_fptr )time_TimeElapsedWrapper);
	ProSLIC_setControlInterfaceGetTime (ProHWIntf, ( system_getTime_fptr )time_GetTimeWrapper);
	ProSLIC_setControlInterfaceSemaphore (ProHWIntf, NULL);
	ProSLIC_Reset(ProObj0);	//Reset the ProSLIC(s) before we begin
	
	for (i=0;i<prochan_num;i++)
	{
		//ProSLIC_setSWDebugMode (ports[i].ProObj, TRUE);	// turn on debug message
		proslicChanType * const ProObj = container[ i ].ProObj;

		arrayOfProslicChans[i] = ProObj;		//create array of channel pointers (for init)		
		ProSLIC_setSWDebugMode (ProObj, FALSE);	// turn off debug message
		//ProSLIC_setSWDebugMode (ProObj, TRUE);	// turn off debug message
	}

	/*** Always wirte ram 1571 first before any SPI access if PFD is enabled. ***/
#ifdef SI32178_PFD
	???
	for (i=0;i<NUMBER_OF_CHAN;i++)
	{
		printk("Enable PFD mode for Si32178 #%d\n", i);
		W_reg_dev(&spiGciObj ->spi_dev, i, 1, 0x1);		// Reset SLIC Channel
		mdelay(300);
		
		pSlic = ports[i].ProObj;
		ProSLIC_SetUserMode(pSlic, 1);	// enable user mode
		W_ram(&spiGciObj ->spi_dev, i, 1571, 0x200000);	// set register 80 to 0x1F
		ProSLIC_SetUserMode(pSlic, 0);	// disable user mode
	}
#endif

	/*** Get SLIC ID ***/
	for (i=0; i<prochan_num; i++)
	{
		printk("SLIC%d ID: ", i);
		
		if( p_snds[ i ].snd_type == SND_TYPE_FXS )
			p_snds[ i ].fxs_ops ->SLIC_show_ID( ( voip_snd_t * )&p_snds[ i ] );
	}
	
	if (ProSLIC_Init(arrayOfProslicChans,prochan_num))
	{
		PRINT_R ("ERROR: proslic SLIC Initialization failed\n");
		goto proslic_err;
	}


	/*** Initialize the channel state for each channel ***/
	//for (i=0;i<NUMBER_OF_CHAN;i++){
	//	pSlic = ports[i].ProObj;
		//InitStateMachine(&(ports[i])); //initialize the call state machine
		//ProSLIC_InitializeDialPulseDetect(&(ports[i].pulseDialData),&(ports[i].offHookTime),&(ports[i].onHookTime));
	//}

	for (i=0; i<prochan_num; i++)
	{
		// Check master state global  register
		reg_val = R_reg_dev(&spiGciObj ->spi_dev, i, 3);
		
		if (reg_val != 0x1F)
			PRINT_Y("SLIC%d warning! master state = 0x%x (should be 0x1F )\n", i, reg_val);
		else
			printk("SLIC%d master state check pass!\n", i);
	}

	for (i=0;i<prochan_num;i++)
	{
		pSlic = container[i].ProObj;
		
//#define SLIC_LOOP_TEST
#ifdef SLIC_LOOP_TEST
		if (i == 0)
			ProSLIC_PCMTimeSlotSetup(pSlic, 1, 17);
		else if (i == 1)
			ProSLIC_PCMTimeSlotSetup(pSlic, 17, 1);
		ProSLIC_PCMSetup(pSlic, pcm_mode);
		ProSLIC_PCMStart(pSlic); //enable SLIC pcm
#else
		if (pSlic->debugMode)
			PRINT_MSG("SLIC PCM Setup for CH%d ...", i);
		//ProSLIC_PCMTimeSlotSetup(pSlic, 1+2*i*8, 1+2*i*8);
		ProSLIC_PCMTimeSlotSetup(pSlic, ts2count( p_snds[ i ].TS1 ), ts2count( p_snds[ i ].TS1 ));
		ProSLIC_PCMSetup(pSlic, pcm_mode);
		container[i].user.pcm_law_save = ( unsigned char )pcm_mode;
		ProSLIC_PCMStart(pSlic); //enable SLIC pcm
		if (pSlic->debugMode)
			PRINT_MSG("OK\n");
#endif		
		
		ProSLIC_RingSetup(pSlic, p_args ->ring_setup_preset/*2*/);	// 20Hz, 48VRMS
		ProSLIC_DCFeedSetup(pSlic, 0);
		
		if (pSlic->debugMode)
			PRINT_MSG("Set to LF_FWD_ACTIVE\n");
		ProSLIC_SetLinefeedStatus(pSlic, LF_FWD_ACTIVE);
		
#if 0
		/* Disable Si3217x Automatic Common Mode Control */
		ProSLIC_SetUserMode(pSlic, 1);	// enable user mode
		W_reg_dev(&spiGciObj ->spi_dev, i, 80, R_reg(&spiGciObj ->spi_dev, i, 80)&(~0x20));
		ProSLIC_SetUserMode(pSlic, 0);	// disable user mode
		if (pSlic->debugMode)
			PRINT_MSG("Disable Si3217x Automatic Common Mode Control[%d].\n", i);
#else
		W_ram_dev(&spiGciObj ->spi_dev, i, 750, 0x624F00); // skip Vbat tracking below 6V Vtr variation
#endif


#ifdef SI32178_PFD
		???
		W_reg_dev(&spiGciObj ->spi_dev, i, 126, 0x2);
		W_reg_dev(&spiGciObj ->spi_dev, i, 126, 0x6);
		W_reg_dev(&spiGciObj ->spi_dev, i, 126, 0x10);
#endif
	}

	printk("Init OK!\n");

#ifdef CONFIG_RTK_VOIP_DRIVERS_FXO
#ifndef CONFIG_RTK_VOIP_DRIVERS_VIRTUAL_DAA

	if( daaIdx < 0 )
		goto label_skip_init_daa;

	/* Enable DAA channel */	
	// Channel 0
#if 0//def CONFIG_RTK_VOIP_MULTIPLE_SI32178
	for (i=0; i<NUMBER_OF_CHAN; i++)
	{
		W_reg_dev(&spiGciObj ->spi_dev, i, 74, 0x20);		//reset the DAA
		mdelay(500);
		W_reg_dev(&spiGciObj ->spi_dev, i, 74, 0x01);		//enable the DAA
	}
#else
	W_reg_dev(&spiGciObj ->spi_dev, prochan_num - 1, 74, 0x20);	//reset the DAA
	mdelay(500);
	W_reg_dev(&spiGciObj ->spi_dev, prochan_num - 1, 74, 0x01);	//enable the DAA
#endif

	// Si3217x Rev 0.1 Daa driver
	printk("Start DAA init....\n");
	if (proslic_initializeDAA( &p_snds[ daaIdx ], &container[ daaIdx ].daas, pcm_mode))
	{
		PRINT_R ("ERROR: Proslic DAA Initialization failed\n");
		goto proslic_err;
	}

#if 0//def CONFIG_RTK_VOIP_MULTIPLE_SI32178
	for (i=0; i<NUMBER_OF_CHAN; i++)
		DAA_OnHook_Line_Monitor_Enable(i+DAA_CHANNEL_OFFSET);
#else
	//DAA_OnHook_Line_Monitor_Enable(SLIC_CH_NUM);
#endif

label_skip_init_daa:

	for( i = 0; i < size; i ++ ) {
	
		voip_snd_t * const p_snd = &p_snds[ i ];
		
		if( p_snd ->snd_type != SND_TYPE_DAA )
			continue;
		
		p_snd ->daa_ops ->DAA_OnHook_Line_Monitor_Enable( p_snd );
	}
	
	printk("Init OK!\n");

#endif
#endif
	//printk("<<<<<<<<<<<<<<<<<<<<<<<<>>>>>>>>>>>>>>>>>>>>>>\n\n");

	return;
proslic_err:
	PRINT_R("Init error!\n");
	mdelay(3000);
}

// --------------------------------------------------------
// proslic ops 
// --------------------------------------------------------

static void FXS_Ring_proslic(voip_snd_t *this, unsigned char ringset )
{
	ProslicContainer_t * const container = ( ProslicContainer_t * )this ->priv;
	proslicChanType * const pSlic = container ->ProObj;
	//pSlic = ports[chid].ProObj;
	if (ringset)
		ProSLIC_RingStart(pSlic);
	else
		ProSLIC_RingStop(pSlic);
}

static unsigned char FXS_Check_Ring_proslic(voip_snd_t *this)
{
	unsigned char ringer; //0: ring off, 1: ring on
	ProslicContainer_t * const container = ( ProslicContainer_t * )this ->priv;
	proslicChanType * const pSlic = container ->ProObj;

	//pSlic = ports[chid].ProObj;
	ProSLIC_GetLinefeedStatus(pSlic, &ringer);
	if ((ringer&0x70)==0x40)
		ringer = 1;
	else
		ringer = 0;

	return ringer;
}

static void Set_SLIC_Tx_Gain_proslic(voip_snd_t *this, int tx_gain)
{
	printk("Not implemented! Support unity gain only!\n");
}

static void Set_SLIC_Rx_Gain_proslic(voip_snd_t *this, int rx_gain)
{
	printk("Not implemented! Support unity gain only!\n");
}

static void SLIC_Set_Ring_Cadence_proslic(voip_snd_t *this, unsigned short OnMsec, unsigned short OffMsec)
{
	ProslicContainer_t * const container = ( ProslicContainer_t * )this ->priv;
	proslicChanType * const pSlic = container ->ProObj;
	
	//pSlic = ports[chid].ProObj;
	ProSLIC_Set_Ring_Cadence_ON(pSlic, OnMsec);
	ProSLIC_Set_Ring_Cadence_OFF(pSlic, OffMsec);

}

static void SLIC_Set_Ring_Freq_Amp_proslic(voip_snd_t *this, char preset)
{
	ProslicContainer_t * const container = ( ProslicContainer_t * )this ->priv;
	proslicChanType * const pSlic = container ->ProObj;
	//pSlic = ports[chid].ProObj;
	ProSLIC_RingSetup(pSlic, preset);
}

static void SLIC_Set_Impendance_Country_proslic(voip_snd_t *this, unsigned short country, unsigned short impd)
{
	ProslicContainer_t * const container = ( ProslicContainer_t * )this ->priv;
	proslicChanType * const pSlic = container ->ProObj;
	//pSlic = ports[chid].ProObj;
	ProSLIC_Set_Impendance_Silicon(pSlic, country, impd);
	PRINT_MSG("Set SLIC impedance according to the country...\n");
}

static void SLIC_Set_Impendance_proslic(voip_snd_t *this, unsigned short preset)
{
	ProslicContainer_t * const container = ( ProslicContainer_t * )this ->priv;
	proslicChanType * const pSlic = container ->ProObj;
	//pSlic = ports[chid].ProObj;
	ProSLIC_Set_Impendance(pSlic, preset);
}

#if 0
static void SLIC_GenProcessTone(unsigned int chid, genTone_struct *gen_tone)
{
}
#endif


static void OnHookLineReversal_proslic(voip_snd_t *this, unsigned char bReversal) //0: Forward On-Hook Transmission, 1: Reverse On-Hook Transmission
{
	ProslicContainer_t * const container = ( ProslicContainer_t * )this ->priv;
	proslicChanType * const pSlic = container ->ProObj;
	//pSlic = ports[chid].ProObj;
	if (bReversal)
		ProSLIC_SetLinefeedStatus(pSlic, LF_REV_OHT);
	else
		ProSLIC_SetLinefeedStatus(pSlic, LF_FWD_OHT);
}

static void SLIC_Set_LineVoltageZero_proslic(voip_snd_t *this)
{
	ProslicContainer_t * const container = ( ProslicContainer_t * )this ->priv;
	proslicChanType * const pSlic = container ->ProObj;
	//pSlic = ports[chid].ProObj;
	ProSLIC_SetLinefeedStatus(pSlic, LF_OPEN);
	//ProSLIC_SetLinefeedStatus(pSlic, LF_TIP_OPEN);
	//ProSLIC_SetLinefeedStatus(pSlic, LF_RING_OPEN);
}

static uint8 SLIC_CPC_Gen_proslic(voip_snd_t *this)
{
#if 0	// con_polling.c: SLIC_CPC_Gen_cch() do this 
	extern void HookPollingDisable(int cch);

	if (slic_cpc[chid].cpc_start != 0)
	{
		PRINT_R("SLIC CPC gen not stop, ch=%d\n", chid);
		return;
	}
#endif
	uint8 pre_linefeed;
	extern void W_reg_dev(rtl_spi_dev_t *pdev, unsigned char chid, unsigned char regaddr, unsigned char data);

	ProslicContainer_t * const container = ( ProslicContainer_t * )this ->priv;
	proslicChanType * const pSlic = container ->ProObj;
	//pSlic = ports[chid].ProObj;
	ProSLIC_GetLinefeedStatus(pSlic, &(pre_linefeed)); // save current linefeed status
	//printk("get linefeed=0x%x\n", (slic_cpc[chid].pre_linefeed)&0xF);
#if 1
	ProSLIC_SetLinefeedStatus(pSlic, LF_OPEN);
	//ProSLIC_SetLinefeedStatus(pSlic, LF_TIP_OPEN);
	//ProSLIC_SetLinefeedStatus(pSlic, LF_RING_OPEN);
#else
	W_reg_dev(pdev, chid, 30, 0);
#endif
	
	return pre_linefeed;

#if 0	// con_polling.c: SLIC_CPC_Gen_cch() do this 
	slic_cpc[chid].cpc_timeout = jiffies + (HZ*time_in_ms_of_cpc_signal/1000);
	slic_cpc[chid].cpc_start = 1;
	slic_cpc[chid].cpc_stop = 0;
	//HookPollFlag[chid] = 0; // disable hook pooling
	HookPollingDisable( chid );
#endif
}

static void SLIC_CPC_Check_proslic(voip_snd_t *this, uint8 pre_linefeed)	// check in timer
{
#if 0	// con_polling.c: ENTRY_SLIC_CPC_Polling() do this 
	extern void HookPollingEnable(int cch);
	
	if (slic_cpc[chid].cpc_start == 0)
		return;
#endif

	extern void W_reg_dev(rtl_spi_dev_t *pdev, unsigned char chid, unsigned char regaddr, unsigned char data);
	ProslicContainer_t * const container = ( ProslicContainer_t * )this ->priv;
	proslicChanType * const pSlic = container ->ProObj;
	
	// Stop wink function
#if 0
	if ((slic_cpc[chid].cpc_stop == 0) && (timetick_after(timetick, slic_cpc[chid].cpc_timeout)))
#endif
	{
		//pSlic = ports[chid].ProObj;
		//printk("set linefeed=0x%x\n", (slic_cpc[chid].pre_linefeed)&0xF);
		switch((pre_linefeed)&0xF)
		{
#if 1
			case 1:
				ProSLIC_SetLinefeedStatus(pSlic, LF_FWD_ACTIVE);	// restore linefeed status
				break;
			case 5:
				ProSLIC_SetLinefeedStatus(pSlic, LF_REV_ACTIVE);	// restore linefeed status
				break;
			default:
				ProSLIC_SetLinefeedStatus(pSlic, LF_FWD_ACTIVE);	// restore linefeed status
				printk("pre_linefeed = 0x%x, Set default linefeed status to LF_FWD_ACTIVE\n", (pre_linefeed)&0xF);
				break;
#else
			case 1:
				W_reg_dev(pdev, chid, 30, 1);
				break;
			case 5:
				W_reg_dev(pdev, chid, 30, 1);
				break;
			default:
				W_reg_dev(pdev, chid, 30, 1);
				printk("pre_linefeed = 0x%x, Set default linefeed status to LF_FWD_ACTIVE\n", (pre_linefeed)&0xF);
				break;
#endif
		}

#if 0	// con_polling.c: ENTRY_SLIC_CPC_Polling() do this 
		slic_cpc[chid].cpc_timeout2 = jiffies + (HZ*200/1000);
		slic_cpc[chid].cpc_stop = 1;
#endif
	}

	
#if 0	// con_polling.c: ENTRY_SLIC_CPC_Polling() do this 
	if ((slic_cpc[chid].cpc_stop == 1) && (timetick_after(timetick, slic_cpc[chid].cpc_timeout2)))
	{
		slic_cpc[chid].cpc_start = 0;
		//HookPollFlag[chid] = 1; // enable hook pooling
		HookPollingEnable( chid );
	}
#endif
}

/*  return value:
	0: Phone dis-connect, 
	1: Phone connect, 
	2: Phone off-hook, 
	3: Check time out ( may connect too many phone set => view as connect),
	4: Can not check, Linefeed should be set to active state first.
*/
//static inline unsigned char SLIC_Get_Hook_Status( int chid );

static inline unsigned int FXS_Line_Check_proslic( voip_snd_t *this )	// Note: this API may cause watch dog timeout. Should it disable WTD?
{
	unsigned long flags;
	//unsigned int v_tip, v_ring, tick=0;
	unsigned int v_tip, v_ring, tick = 0;
	unsigned int connect_flag = 0, time_out_flag = 0;
	unsigned char linefeed, rev_linefeed;
	ProslicContainer_t * const container = ( ProslicContainer_t * )this ->priv;
	proslicChanType * const pSlic = container ->ProObj;

	if ( 1 == this ->fxs_ops ->SLIC_Get_Hook_Status( this, 1 ) )
	{
		//PRINT_MSG("%s: Phone 0ff-hook\n",__FUNCTION__);
		return 2;
	}

#if 1
	if( container ->line.connect_count < 10 ||
		container ->line.connect_count > 20 ||
		container ->line.timeout_count < 50 ||
		container ->line.timeout_count > 70 )
	{
		PRINT_R( "Don't initialize values for FXS_Line_Check_proslic??\n" );
	}
#endif


	//pSlic = ports[chid].ProObj;
	ProSLIC_GetLinefeedStatus(pSlic, &linefeed);
	linefeed = linefeed >> 4;
	//printk("Get linefeed: %d\n", linefeed);

	if ( (linefeed == LF_FWD_ACTIVE) || (linefeed == LF_REV_ACTIVE) )
	{
		if (linefeed == LF_FWD_ACTIVE)
		{
			rev_linefeed = LF_REV_ACTIVE;
		}
		else if (linefeed == LF_REV_ACTIVE)
		{
			rev_linefeed = LF_FWD_ACTIVE;
		}
#if 0
		SLIC_read_ram(chid, 12, &v_tip);	//~3.8V
		SLIC_read_ram(chid, 13, &v_ring);	//~ 49V
		printk("v_ring=0x%x, v_tip=0x%x, v_tr=0x%x\n", v_ring, v_tip, v_ring - v_tip);
#endif
		save_flags(flags); cli();
		
		ProSLIC_SetLinefeedStatus(pSlic, rev_linefeed);

		while (1)
		{
			tick++;
			
			if (linefeed == LF_FWD_ACTIVE)
			{
				this ->fxs_ops ->SLIC_read_ram(this, 12, &v_tip);
				this ->fxs_ops ->SLIC_read_ram(this, 13, &v_ring);
			}
			else if (linefeed == LF_REV_ACTIVE)
			{
				this ->fxs_ops ->SLIC_read_ram(this, 12, &v_ring);
				this ->fxs_ops ->SLIC_read_ram(this, 13, &v_tip);
			}

			if (((v_tip - v_ring) > container ->line.TR_thres/*VOLT_TR_THRESHOL*/) && (v_tip > container ->line.TIP_thres/*VOLT_TIP_THRESHOL*/) )
			{
				PRINT_MSG("tick: %d\n", tick);
				
				//printk("v_tip=0x%x\n", v_tip);
				//printk("v_ring=0x%x\n", v_ring);

				if (tick > container ->line.connect_count /*PHONE_CHECK_CONNECT_COUNT*/)
				{
					connect_flag = 1;
				}
				break;
			}

			if ( tick > container ->line.timeout_count /*PHONE_CHECK_TIME_OUT_COUNT*/ )
			{
				time_out_flag = 1;
				//connect_flag = 1;
				break;
			}
		}
		
		ProSLIC_SetLinefeedStatus(pSlic, linefeed);
		restore_flags(flags);

	}
	else
	{
		PRINT_MSG("%s: Can't check. Linefeed should be set to active state first!\n", __FUNCTION__);
		return 4;
	}

	if (connect_flag == 1)
	{
		//PRINT_MSG("%s: Phone connected!\n",__FUNCTION__);
		//PRINT_MSG("Connect count: %d\n", PHONE_CHECK_CONNECT_COUNT);
		return 1;
	}
	else if (time_out_flag == 1)
	{
		//PRINT_MSG("%s: Check time out (may connect too many phone set)\n", __FUNCTION__);
		//PRINT_MSG("Connect count: %d\n", PHONE_CHECK_CONNECT_COUNT);
		//PRINT_MSG("Time out count: %d\n", PHONE_CHECK_TIME_OUT_COUNT);
		return 3;
	}
	else
	{
		//PRINT_MSG("%s: Phone dis-connected!\n",__FUNCTION__);
		//PRINT_MSG("Connect count: %d\n", PHONE_CHECK_CONNECT_COUNT);
		return 0;
	}

}


static void SendNTTCAR_proslic( voip_snd_t *this )
{
	ProslicContainer_t * const container = ( ProslicContainer_t * )this ->priv;
	proslicChanType * const pSlic = container ->ProObj;
	//pSlic = ports[chid].ProObj;
	ProSLIC_SendNTTCAR(pSlic);

}

static unsigned int SendNTTCAR_check_proslic(voip_snd_t *this, unsigned long time_out)
{
	ProslicContainer_t * const container = ( ProslicContainer_t * )this ->priv;
	proslicChanType * const pSlic = container ->ProObj;
	//pSlic = ports[chid].ProObj;
	return ProSLIC_SendNTTCAR_check(this->sch, pSlic, time_out);
	// this->sch can be ignored 
}

static void disableOscillators_proslic(voip_snd_t *this)
{
}

static void SetOnHookTransmissionAndBackupRegister_proslic(voip_snd_t *this) // use for DTMF caller id
{
	ProslicContainer_t * const container = ( ProslicContainer_t * )this ->priv;
	proslicChanType * const pSlic = container ->ProObj;
	unsigned char cid_reg64;
	
	//pSlic = ports[chid].ProObj;
	ProSLIC_GetLinefeedStatus(pSlic, &cid_reg64);

	if ( (cid_reg64 & 0x07) != 2 )  // force for DTMF CID display
	{
		container ->user.cid_reg64_prev = cid_reg64; // record it
		PRINT_MSG("Reg64 = 0x%02x\n", cid_reg64);
		this ->fxs_ops ->OnHookLineReversal(this, 0); //Forward On-Hook Transmission
	}

}

static inline void RestoreBackupRegisterWhenSetOnHookTransmission_proslic(voip_snd_t *this) // use for DTMF caller id
{
	ProslicContainer_t * const container = ( ProslicContainer_t * )this ->priv;
	proslicChanType * const pSlic = container ->ProObj;
	//pSlic = ports[chid].ProObj;
	ProSLIC_SetLinefeedStatus(pSlic, container ->user.cid_reg64_prev);
}

#define PCMLAW_OFFSET	3

CT_ASSERT( BUSDATFMT_PCM_WIDEBAND_LINEAR - BUSDATFMT_PCM_LINEAR == PCMLAW_OFFSET );
CT_ASSERT( BUSDATFMT_PCM_WIDEBAND_ALAW - BUSDATFMT_PCM_ALAW == PCMLAW_OFFSET );
CT_ASSERT( BUSDATFMT_PCM_WIDEBAND_ULAW - BUSDATFMT_PCM_ULAW == PCMLAW_OFFSET );

static void SLIC_PCMSetup_priv_ops( voip_snd_t *this, int enable )
{
	ProslicContainer_t * const container = ( ProslicContainer_t * )this ->priv;
	proslicChanType * const pSlic = container ->ProObj;
	
	int pcm_mode = -1;
	
	if( !enable )
		return;
	
	//printk( "Set_PCM_state:enable=%d,container ->user.pcm_law_save=%d\n", enable, container ->user.pcm_law_save );
	
	// check if need to switch between narrowband/wideband mode 
	switch( container ->user.pcm_law_save ) {
	case BUSDATFMT_PCM_LINEAR:
	case BUSDATFMT_PCM_ALAW:
	case BUSDATFMT_PCM_ULAW:
		if( enable == 2 ) {
			pcm_mode = container ->user.pcm_law_save + PCMLAW_OFFSET;
		}
		break;
		
	case BUSDATFMT_PCM_WIDEBAND_LINEAR:
	case BUSDATFMT_PCM_WIDEBAND_ALAW:
	case BUSDATFMT_PCM_WIDEBAND_ULAW:
		if( enable == 1 ) {
			pcm_mode = container ->user.pcm_law_save - PCMLAW_OFFSET;
		}
		break;
	}
	
	if( pcm_mode >= 0 ) {
		ProSLIC_PCMSetup(pSlic, pcm_mode);
		container ->user.pcm_law_save = ( unsigned char )pcm_mode;
	}	
}

static void SLIC_Set_PCM_state_proslic(voip_snd_t *this, int enable)
{
	ProslicContainer_t * const container = ( ProslicContainer_t * )this ->priv;
	proslicChanType * const pSlic = container ->ProObj;
	//pSlic = ports[chid].ProObj;
	
	if( enable )
		ProSLIC_PCMStart(pSlic);
	else
		ProSLIC_PCMStop(pSlic);
}

static unsigned char SLIC_Get_Hook_Status_proslic(voip_snd_t *this, int directly)
{
	unsigned char status = 0;
	ProslicContainer_t * const container = ( ProslicContainer_t * )this ->priv;
	proslicChanType * const pSlic = container ->ProObj;

	if( !directly && container ->user.hookStatus != INVALID_HOOK_STATUS ) {
		return container ->user.hookStatus;
	}
	
	//pSlic = ports[chid].ProObj;
	ProSLIC_ReadHookStatus(pSlic, &status); /* 1:off-hook  0:on-hook */

	container ->user.hookStatus = status;
	
	return status;
}

static void SLIC_read_reg_proslic(voip_snd_t *this, unsigned int num, unsigned char *len, unsigned char *val)
{
	extern unsigned char R_reg_dev(rtl_spi_dev_t *pdev,unsigned char chid, unsigned char regaddr);
	ProslicContainer_t * const container = ( ProslicContainer_t * )this ->priv;
	proslicChanType * const pSlic = container ->ProObj;
	ctrl_S * const ctrl = ( ctrl_S * )( 
		( container ->ProObj ? 
			container ->ProObj ->deviceId ->ctrlInterface ->hCtrl:
			container ->daas ->deviceId ->ctrlInterface ->hCtrl ) );
	rtl_spi_dev_t * const pdev = &ctrl ->spi_dev;
	const unsigned char chid = 
		( container ->ProObj ?
			container ->ProObj ->channel :
			container ->daas ->channel );

	if (num == 32261)
	{
		ProSLIC_SetUserMode( pSlic, 1); // enable user mode
		printk("Si3226 channel %d enter user mode.\n", chid);
	}
	else if (num == 32260)
	{
		ProSLIC_SetUserMode( pSlic, 0); // disable user mode
		printk("Si3226 channel %d leave user mode.\n", chid);
	}
	else
		*val = R_reg_dev(pdev, chid, (unsigned char)num);	

	*len = 1;
}


static void SLIC_write_reg_proslic(voip_snd_t *this, unsigned int num, unsigned char *len, unsigned char *val)
{
	extern void W_reg_dev(rtl_spi_dev_t *pdev, unsigned char chid, unsigned char regaddr, unsigned char data);
	ProslicContainer_t * const container = ( ProslicContainer_t * )this ->priv;
	ctrl_S * const ctrl = ( ctrl_S * )( 
		( container ->ProObj ? 
			container ->ProObj ->deviceId ->ctrlInterface ->hCtrl:
			container ->daas ->deviceId ->ctrlInterface ->hCtrl ) );
	rtl_spi_dev_t * const pdev = &ctrl ->spi_dev;
	const unsigned char chid = 
		( container ->ProObj ?
			container ->ProObj ->channel :
			container ->daas ->channel );
	
	W_reg_dev(pdev, chid, (unsigned char)num, *val);
}

static void SLIC_read_ram_proslic(voip_snd_t *this, unsigned short num, unsigned int *val)
{
	extern unsigned int R_ram_dev(rtl_spi_dev_t* pdev, unsigned char chid, unsigned short reg);
	ProslicContainer_t * const container = ( ProslicContainer_t * )this ->priv;
	ctrl_S * const ctrl = ( ctrl_S * )( 
		( container ->ProObj ? 
			container ->ProObj ->deviceId ->ctrlInterface ->hCtrl:
			container ->daas ->deviceId ->ctrlInterface ->hCtrl ) );
	rtl_spi_dev_t * const pdev = &ctrl ->spi_dev;
	const unsigned char chid = 
		( container ->ProObj ?
			container ->ProObj ->channel :
			container ->daas ->channel );
	
	*val = R_ram_dev(pdev, chid, num);
}


static void SLIC_write_ram_proslic(voip_snd_t *this, unsigned short num, unsigned int val)
{
	extern void W_ram_dev(rtl_spi_dev_t *pdev, unsigned char chid, unsigned short reg, unsigned int data);
	ProslicContainer_t * const container = ( ProslicContainer_t * )this ->priv;
	ctrl_S * const ctrl = ( ctrl_S * )( 
		( container ->ProObj ? 
			container ->ProObj ->deviceId ->ctrlInterface ->hCtrl:
			container ->daas ->deviceId ->ctrlInterface ->hCtrl ) );
	rtl_spi_dev_t * const pdev = &ctrl ->spi_dev;
	const unsigned char chid = 
		( container ->ProObj ?
			container ->ProObj ->channel :
			container ->daas ->channel );
			
	W_ram_dev(pdev, chid, num, val);
}

static void SLIC_dump_reg_proslic(voip_snd_t *this)
{
	unsigned char reg_val,reg_len;
	int i;

	reg_len = sizeof(reg_val);
	printk("Dump SLIC register:\n");
	for (i=0; i <= 126; i++)
	{
		this ->fxs_ops ->SLIC_read_reg(this, i, &reg_len, &reg_val);
		printk("%d: 0x%x\n", i, reg_val);
	}
}

static void SLIC_dump_ram_proslic(voip_snd_t *this)
{
	unsigned int ram_val;
	int i;

	printk("Dump SLIC ram:\n");
	for (i=0; i <= 1023; i++)
	{
		this ->fxs_ops ->SLIC_read_ram(this, i, &ram_val);
		printk("%d: 0x%x\n", i, ram_val);
	}
}

/* This API is workable only for Si32178, chid is SLIC's chid. */
static void FXS_FXO_DTx_DRx_Loopback_proslic(voip_snd_t *this, voip_snd_t *daa_snd, unsigned int enable)
{
	printk( "Not implement FXS_FXO loopback\n" );
}

static void SLIC_OnHookTrans_PCM_start_proslic(voip_snd_t *this)
{
	this ->fxs_ops ->SLIC_Set_PCM_state(this, SLIC_PCM_ON);
	this ->fxs_ops ->OnHookLineReversal(this, 0);		//Forward On-Hook Transmission
	PRINT_MSG("SLIC_OnHookTrans_PCM_start, ch = %d\n", this ->sch);
}

static int enable_proslic( voip_snd_t *this, int enable )
{
	SLIC_PCMSetup_priv_ops( this, enable );
	this ->fxs_ops ->SLIC_Set_PCM_state( this, enable );
	
	return 0;
}


// --------------------------------------------------------
// channel mapping architecture 
// --------------------------------------------------------

const snd_ops_fxs_t snd_proslic_fxs_ops = {
	// common operation 
	.enable = enable_proslic,
	
	// for each snd_type 
	//.SLIC_reset = SLIC_reset_proslic,
	.FXS_Ring = FXS_Ring_proslic,
	.FXS_Check_Ring = FXS_Check_Ring_proslic,
	.FXS_Line_Check = FXS_Line_Check_proslic,	// Note: this API may cause watch dog timeout. Should it disable WTD?
	.SLIC_Set_PCM_state = SLIC_Set_PCM_state_proslic,
	.SLIC_Get_Hook_Status = SLIC_Get_Hook_Status_proslic,
	
	.Set_SLIC_Tx_Gain = Set_SLIC_Tx_Gain_proslic,
	.Set_SLIC_Rx_Gain = Set_SLIC_Rx_Gain_proslic,
	.SLIC_Set_Ring_Cadence = SLIC_Set_Ring_Cadence_proslic,
	.SLIC_Set_Ring_Freq_Amp = SLIC_Set_Ring_Freq_Amp_proslic,
	.SLIC_Set_Impendance_Country = SLIC_Set_Impendance_Country_proslic, 
	.SLIC_Set_Impendance = SLIC_Set_Impendance_proslic,
	.OnHookLineReversal = OnHookLineReversal_proslic,	//0: Forward On-Hook Transmission, 1: Reverse On-Hook Transmission
	.SLIC_Set_LineVoltageZero = SLIC_Set_LineVoltageZero_proslic,
	
	.SLIC_CPC_Gen = SLIC_CPC_Gen_proslic,
	.SLIC_CPC_Check = SLIC_CPC_Check_proslic,	// check in timer
	
	.SendNTTCAR = SendNTTCAR_proslic,
	.SendNTTCAR_check = SendNTTCAR_check_proslic,
	
	.disableOscillators = disableOscillators_proslic,
	
	.SetOnHookTransmissionAndBackupRegister = SetOnHookTransmissionAndBackupRegister_proslic,	// use for DTMF caller id
	.RestoreBackupRegisterWhenSetOnHookTransmission = RestoreBackupRegisterWhenSetOnHookTransmission_proslic,	// use for DTMF caller id
	
	.FXS_FXO_DTx_DRx_Loopback = FXS_FXO_DTx_DRx_Loopback_proslic,
	.SLIC_OnHookTrans_PCM_start = SLIC_OnHookTrans_PCM_start_proslic,
	
	// read/write register/ram
	.SLIC_read_reg = SLIC_read_reg_proslic,
	.SLIC_write_reg = SLIC_write_reg_proslic,
	.SLIC_read_ram = SLIC_read_ram_proslic,
	.SLIC_write_ram = SLIC_write_ram_proslic,
	.SLIC_dump_reg = SLIC_dump_reg_proslic,
	.SLIC_dump_ram = SLIC_dump_ram_proslic,
	
	//.SLIC_show_ID = ??
};

