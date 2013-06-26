#include <linux/slab.h>
#include <linux/interrupt.h>
#include <asm/uaccess.h>
#include <linux/delay.h>
#include <net/sock.h>

#include "rtk_voip.h"
#include "voip_types.h"
#include "voip_control.h"
#include "voip_params.h"
#include "voip_mgr_define.h"
#include "voip_mgr_netfilter.h"
#ifndef CONFIG_DEFAULTS_KERNEL_2_6
#include "voip_mgr_do_debug.h"
#endif

#ifdef CONFIG_RTK_VOIP_IP_PHONE
  #if defined(CONFIG_RTK_VOIP_DRIVERS_CODEC_WM8510)
    #include "../voip_drivers/iphone/WM8510.h"	/* for type of AI_AO_select() */
    #include "../voip_drivers/iphone/base_i2c_WM8510.h"
  #elif defined(CONFIG_RTK_VOIP_DRIVERS_CODEC_ALC5621)
    #include "../voip_drivers/iphone/ALC5621.h"
    #include "../voip_drivers/iphone/base_i2c_ALC5621.h"
  #endif
  #if defined( CONFIG_RTK_VOIP_GPIO_IPP_100 ) || defined( CONFIG_RTK_VOIP_GPIO_IPP_101 )
  #elif defined( CONFIG_RTK_VOIP_GPIO_IPP_8972_V00 ) || defined( CONFIG_RTK_VOIP_GPIO_IPP_8972_V01 ) 
    #include "../voip_drivers/iphone/PT6961.h"
  #elif defined( CONFIG_RTK_VOIP_GPIO_IPP_8952_V00 )
    #include "../voip_drivers/iphone/led_directgpio.h"
  #endif
#endif

int dsp_init_first = ( int )NULL;
int g_voip_debug = 0;

#ifndef CONFIG_RTK_VOIP_IPC_ARCH
#ifdef RTK_VOICE_RECORD
extern TstVoipdataget stVoipdataget[];
TstVoipdataget_o stVoipdataget_o;
#endif //#ifdef RTK_VOICE_RECORD
#endif

#ifndef CONFIG_RTK_VOIP_IPC_ARCH
#ifdef RTK_VOICE_PLAY
extern TstVoipdataput stVoipdataput[];
TstVoipdataput_o stVoipdataput_o;
#endif //#ifdef RTK_VOICE_PLAY
#endif

//#define TEST_UNALIGN
#ifdef TEST_UNALIGN
#include "../include/dmem_stack.h"
char test_data[8] = {0, 1, 2, 3, 4, 5, 6, 7};
int *test_p;
#endif

#ifdef CONFIG_RTK_VOIP_IPC_ARCH

#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
//static unsigned int mgr_chid;
#endif

#ifdef CONFIG_RTK_VOIP_IPC_ARCH
#include "ipc_arch_tx.h"
#endif

#endif // CONFIG_RTK_VOIP_IPC_ARCH

// Force control gloable variable
int g_force_codec = -1;
int g_force_vad = -1;
int g_force_ptime = -1;
int g_force_PcmMode = -1;
int g_force_g7111mode = -1;

//----- below IO Ctrl is for test -------//
int do_mgr_VOIP_MGR_INIT_GNET( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
#if 0
	unsigned long flags;
	
	PRINT_MSG("INIT GNET\n");
	save_flags(flags); cli();
	//DSP_init();
	PRINT_MSG("useless now\n");
	restore_flags(flags);
	PRINT_MSG("INIT GNET FINISH\n");
#endif
	return 0;
}

int do_mgr_VOIP_MGR_INIT_GNET2( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
#if 0
	PRINT_MSG("INIT GNET2\n");
	if(dsp_init_first == ( int )NULL )
	{
		//rtk_voip_dsp_init();
		dsp_init_first = 1;
	}
	PRINT_MSG("INIT GNET2 FINISH\n");
#endif
	return 0;
}

int do_mgr_VOIP_MGR_LOOPBACK( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
#if 0
	TstVoipTest stVoipTest;
	
	PRINT_MSG("SET LOOPBACK MODE\n");
	copy_from_user(&stVoipTest, (TstVoipTest *)user, sizeof(TstVoipTest));
	//DSP_internal_loopback(stVoipTest.ch_id, stVoipTest.enable);
	PRINT_MSG("useless now\n");
	PRINT_MSG("chid = %d, enable = %d \n", stVoipTest.ch_id, stVoipTest.enable );
	PRINT_MSG("SET LOOPBACK MODE FINISH\n");
#endif
	return 0;
}

int do_mgr_VOIP_MGR_GNET( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
#if 0
	unsigned int data[4];
	
	//PRINT_MSG("GNET IOCTL ENTER\n");
	copy_from_user(data, (unsigned int *)user, 16);
	/*
	for(i=0;i<4;i++)
	PRINT_MSG("kernel space:data[%d]=%d\n", i , data[i]);
	data[3] = 101;
	copy_to_user(user, data, 16);
	*/
	//DSP_ioctl(data);
	PRINT_MSG("GNET IOCTL CLOSE\n");
	//PRINT_MSG("GNET IOCTL EXIT\n");
#endif
	return 0;
}

int do_mgr_VOIP_MGR_SIGNALTEST( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
#if 0
#ifdef CONFIG_RTK_VOIP_DRIVERS_PCM8186
	kill_fasync(&( ((struct pcm_priv*)pcmctrl_devices)->pcm_fasync), SIGIO, POLL_IN);
#endif
#endif
	return 0;
}

int do_mgr_VOIP_MGR_DSPSETCONFIG( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
#if 0
	//test only
	unsigned long flags;
	TstVoipValue stVoipValue;
	
	copy_from_user(&stVoipValue, (TstVoipValue *)user, sizeof(TstVoipValue));
	PRINT_MSG("VOIP_MGR_DSPSETCONFIG:value=%d\n", stVoipValue.value);
	//DSP_SetConfig(0, false, rtpPayloadPCMU, 0);
	save_flags(flags); cli();
	//DSP_SetConfig(0, false, stVoipValue.value, 0, 5, 8, 7);
	restore_flags(flags);
#endif
	return 0;
}

int do_mgr_VOIP_MGR_DSPCODECSTART( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
#if 0
	//test only
	unsigned long flags;
	TstVoipValue stVoipValue;
	
	copy_from_user(&stVoipValue, (TstVoipValue *)user, sizeof(TstVoipValue));
	PRINT_MSG("VOIP_MGR_DSPCODECSTART:value=%d\n", stVoipValue.value);
	save_flags(flags); cli();
	//	DspcodecStart(0, stVoipValue.value);
	restore_flags(flags);
#endif
	return 0;
}

/**
 * @ingroup VOIP_DEBUG
 * @brief For various debug purpose 
 * @param TstVoipValue.value Debug level 
 * @see VOIP_MGR_DEBUG TstVoipValue 
 */
int do_mgr_VOIP_MGR_DEBUG( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	unsigned long flags;
	TstVoipValue stVoipValue;
	int ret = 0;
	extern int g_disable_report_fax;
	extern int g_disable_announce_fax;
	extern int g_enable_fax_dis;

	COPY_FROM_USER(&stVoipValue, (TstVoipValue *)user, sizeof(TstVoipValue));
	if( ( ret = NO_COPY_TO_USER( cmd, seq_no ) ) < 0 )
		return ret;

#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	//int ret;
	// Send Control Packet and wait Response Packet (global setting for every DSP)
	ret = ipcSentControlPacketNoChannel(cmd, &stVoipValue, sizeof(TstVoipValue));
	
	PRINT_MSG("VOIP_MGR_DEBUG: dbg=%d\n", stVoipValue.value);
	save_flags(flags); cli();
	rtk_dbg_level = stVoipValue.value;
	g_voip_debug = stVoipValue.value;
	//gpio_debug = stVoipValue.value;
	restore_flags(flags);
	

#else

	PRINT_MSG("VOIP_MGR_DEBUG: dbg=%d,watchdog=%d\n", stVoipValue.value,stVoipValue.value1);
	save_flags(flags); cli();
	rtk_dbg_level = stVoipValue.value;
	g_voip_debug = stVoipValue.value;
	//gpio_debug = stVoipValue.value;
  #if 0 // ec128 test mode code
		stVoipdataget[0].write_enable=0;
		extern int ec128_pattern_index;
		extern int ec128_start_count;

		ec128_pattern_index=0;
		ec128_start_count=0;
		stVoipdataget[0].tx_readindex=0;
		stVoipdataget[0].tx_writeindex=0;
		stVoipdataget[0].rx_readindex=0;
		stVoipdataget[0].rx_writeindex=0;
		stVoipdataget[0].rx_readindex2=0;
		stVoipdataget[0].rx_writeindex2=0;
  #endif
	restore_flags(flags);

#ifdef CONFIG_RTL865X_WTDOG
	if( stVoipValue.value1 == 0 ) {
		extern void plat_disable_watchdog( void );
		plat_disable_watchdog();
		PRINT_MSG("Disable watchdog\n" );
	} else if( stVoipValue.value1 == 1 ) {
		extern void plat_enable_watchdog( void );
		plat_enable_watchdog();
		PRINT_MSG("Enable watchdog\n");
	}
#elif defined( CONFIG_RTL_WTDOG )
	if( stVoipValue.value1 == 0 ) {
		extern void bsp_disable_watchdog( void );
		bsp_disable_watchdog();
		PRINT_MSG("Disable watchdog\n" );
	} else if( stVoipValue.value1 == 1 ) {
		extern void bsp_enable_watchdog( void );
		bsp_enable_watchdog();
		PRINT_MSG("Enable watchdog\n");
	}
#else
	PRINT_MSG("CONFIG_RTL865X_WTDOG is undefined\n");
#endif

#if 0
	extern short test1(void);
	extern short test2(void);
	extern short test2_1(void);
	extern short test2_2(void);
	short w, x, y, z;

	if (g_voip_debug == 73)
	{
		w = test1();
		x = test2();
		y = test2_1();
		z = test2_2();

		printk(" %d, %d, %d, %d\n", w, x, y, z);

	}
#endif
	
	/* Force codec control */

	if (g_voip_debug == 112) {
		g_enable_fax_dis=0;
		printk("g_enable_fax_dis=0\n");
	} else if (g_voip_debug == 113) {
		g_enable_fax_dis=1;
		printk("g_enable_fax_dis=1\n");
	}

	if (g_voip_debug == 114) {
		g_disable_announce_fax=0;
		printk("g_disable_announce_fax=0\n");
	} else if (g_voip_debug == 115) {
		g_disable_announce_fax=1;
		printk("g_disable_announce_fax=1\n");
	}

	if (g_voip_debug == 116) {
		g_disable_report_fax=0;
		printk("g_disable_report_fax=0\n");
	} else if (g_voip_debug == 117) {
		g_disable_report_fax=1;
		printk("g_disable_report_fax=1\n");
	}

	if (g_voip_debug == 118) {
	} else if (g_voip_debug == 119) {
	}


	if (g_voip_debug == 11)
		g_force_codec = 8;
	else if (g_voip_debug == 23)	// force codec to G.723
		g_force_codec = 4;
	else if (g_voip_debug == 22)	// force codec to G.722
		g_force_codec = 9;
	else if (g_voip_debug == 29)	// force codec to G.729
		g_force_codec = 18;
	else if (g_voip_debug == 100)	// force codec to Silence
		g_force_codec = 35;
	else if (g_voip_debug == 38)	// force codec to T.38
		g_force_codec = 110;
	else if (g_voip_debug == 123)	// force codec to AMR-NB
		g_force_codec = 123;
#ifdef CONFIG_RTK_VOIP_PCM_LINEAR_8K
	else if (g_voip_debug == 8)	//PCM_Linear_8K
		g_force_codec = 119;
#endif
#ifdef CONFIG_RTK_VOIP_PCM_LINEAR_16K
	else if (g_voip_debug == 16)	//PCM_Linear_16K
		g_force_codec = 120;
#endif
	

	else if (g_voip_debug == 111)
		g_force_codec = 102;	// force codec to G.7111-U
	else if (g_voip_debug == 112)
		g_force_codec = 103;	// force codec to G.7111-A
		
	else if (g_voip_debug == 101)
		g_force_g7111mode = 1;	// force G.711.1 to R1 mode
	else if (g_voip_debug == 102)
		g_force_g7111mode = 2;	// force G.711.1 to R2a mode
	else if (g_voip_debug == 103)
		g_force_g7111mode = 3;	// force G.711.1 to R2b mode
	else if (g_voip_debug == 104)
		g_force_g7111mode = 4;	// force G.711.1 to R3 mode

	/* Force PCM mode control */
	if (g_voip_debug == 105)
		g_force_PcmMode = 0;	// force PCM mode to no action
	else if (g_voip_debug == 106)
		g_force_PcmMode = 1;	// force PCM mode to DSP auto mode
	else if (g_voip_debug == 107)
		g_force_PcmMode = 2;	// force PCM mode to NB mode
	else if (g_voip_debug == 108)
		g_force_PcmMode = 3;	// force PCM mode to WB auto mode

	/* Force VAD control */
	else if (g_voip_debug == 120)
		g_force_vad = 0;	// force VAD disable
	else if (g_voip_debug == 121)
		g_force_vad = 1;	// force VAD enable

	/* Force Packet Time control */
	else if (g_voip_debug == 10)
		g_force_ptime = 1;	// force frame per packet to 1
	else if (g_voip_debug == 20)
		g_force_ptime = 2;	// force frame per packet to 2
	else if (g_voip_debug == 30)
		g_force_ptime = 3;	// force frame per packet to 3
	else if (g_voip_debug == 40)
		g_force_ptime = 4;	// force frame per packet to 4
	else if (g_voip_debug == 50)
		g_force_ptime = 5;	// force frame per packet to 5
	else if (g_voip_debug == 60)
		g_force_ptime = 6;	// force frame per packet to 6
	else if (g_voip_debug == 70)
		g_force_ptime = 20;	// force frame per packet to 20


	/* Disable all force control */
	if (g_voip_debug == 127)
	{
		g_force_codec = -1;
		g_force_vad = -1;
		g_force_ptime = -1;
		g_force_PcmMode = -1;
		g_force_g7111mode = -1;
	}

	if (g_voip_debug == 77)
	{
		extern char ModemFlag[];
		extern char FaxFlag[];
		printk("M=%d, F=%d\n", ModemFlag[0], FaxFlag[0]);
	}
		
#ifdef CONFIG_CRYPTO_DEV_REALTEK_TEST
	if (g_voip_debug >= 10 && g_voip_debug <= 99)
	{
		extern void rtl_ipsec_test();
		rtl_ipsec_test(g_voip_debug);
	}
#endif

#if 0
	if (g_voip_debug == 20)
	{
	// test 8306 led
	#define rtl8306_page_select(page)	do { int pagenumber; pagenumber = (page + 2) & 0x03; \
		MII_write(0,16,(MII_read(0,16,0)&0x7ffd)|(pagenumber&0x02)|((pagenumber&0x01)<<15),0); } while (0)
		int port, flag, val;

		// LED controlled by CPU
        rtl8306_page_select(3);
	    MII_write(2, 21, MII_read(2 ,21, 0)|0x780, 0);

		flag = 0; // disable_led
		while (flag < 2)
		{
			for (port=0; port<5; port++)
			{
				val = (1 << port) | (1 << (port + 5));
				if (flag == 0)	
					MII_write(3, 24, MII_read(3 ,24, 0)&~val, 0); 
				else
					MII_write(3, 24, MII_read(3 ,24, 0)|val, 0); 

				MII_read(4, 0, 0); // switch to MII to wan port to fix Eth auto link issue
				mdelay(300);
			}
				
			flag++;	// enable_led
			mdelay(1000);
		}

		// LED controlled by 8306
        rtl8306_page_select(3);
	    MII_write(2, 21, MII_read(2 ,21, 0) & (~0x780), 0);
	}
		
	if (g_voip_debug == 21)
	{
	// test wlan led
	#define _9346CR_		0x50
	#define _CFG0_			0x51
	#define _PSR_			0x5e
	#define WLAN_CTRL(reg)	(*(volatile unsigned char *)((unsigned int)(reg + 0xbd400000)))

		// enable sw led
		WLAN_CTRL(_9346CR_) = 0xc0;	// enable config register write
		WLAN_CTRL(_CFG0_) = WLAN_CTRL(_CFG0_) | 0x10;	// turn off HW led control
		// led test
		WLAN_CTRL(_PSR_) = WLAN_CTRL(_PSR_) & (~BIT(4));	// LED0 on
		WLAN_CTRL(_PSR_) = WLAN_CTRL(_PSR_) & (~BIT(5));	// LED1 on
		mdelay(1000);
		WLAN_CTRL(_PSR_) = WLAN_CTRL(_PSR_) | BIT(4);	// LED0 off
		WLAN_CTRL(_PSR_) = WLAN_CTRL(_PSR_) | BIT(5);	// LED1 off
		// disable sw led
		WLAN_CTRL(_9346CR_) = 0xc0;	// enable config register write
		WLAN_CTRL(_CFG0_) = WLAN_CTRL(_CFG0_) & (~0x10);	// turn on HW led control
	}
#endif
		
#ifdef TEST_UNALIGN
	if (g_voip_debug == 1)
	{
		test_p = (int *)(test_data + 3);
		printk("data = %p, p = %p\n", test_data, test_p);
		printk("*p = %d\n", *test_p);
	}
	else if (g_voip_debug == 2)
	{
		extern void set_DMEM(unsigned long start, unsigned long size);

		save_flags(flags); cli();
		set_DMEM(&__sys_dmem_start, SYS_DMEM_SSIZE);
		sys_dmem_sp = &(sys_dmem_stack[SYS_DMEM_SSIZE]);
		entry_dmem_stack(&sys_orig_sp, &sys_dmem_sp);
		printk("enter: sys_orig_sp = %x\n", sys_orig_sp);

		test_p = (int *)(test_data + 3);
		printk("data = %p, p = %p\n", test_data, test_p);
		printk("*p = %d\n", *test_p);

		leave_dmem_stack(&sys_orig_sp);
		printk("leave1: sys_orig_sp = %x\n", sys_orig_sp);
		restore_flags(flags);
	}
#endif
#endif
	return ret;
}

/**
 * @ingroup VOIP_DEBUG
 * @brief Get T.38 input PCM data 
 * @see VOIP_MGR_GET_T38_PCMIN TstT38PcmIn 
 */
#if ! defined (AUDIOCODES_VOIP)
int do_mgr_VOIP_MGR_GET_T38_PCMIN( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
#if 0	/* debug T.38 */
 #ifdef CONFIG_RTK_VOIP_T38
	TstT38PcmIn stT38PcmIn;
	extern uint32 T38Dump_GetPcm( unsigned short *pPcmIn, uint32 *priPcm );

	copy_from_user(&stT38PcmIn, (TstT38PcmIn *)user, sizeof(TstT38PcmIn));

	stT38PcmIn.ret = T38Dump_GetPcm( stT38PcmIn.pcmIn, &stT38PcmIn.snPcm );

	copy_to_user(user, &stT38PcmIn, sizeof(TstT38PcmIn));
 #endif /* CONFIG_RTK_VOIP_T38 */
#endif /* debug T.38 */
	return 0;
}
#else
int do_mgr_VOIP_MGR_GET_T38_PCMIN( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	PRINT_MSG("VOIP_MGR_GET_T38_PCMIN: NOT support for ACMW\n");
	return 0;
}
#endif

/**
 * @ingroup VOIP_DEBUG
 * @brief Get T.38 input packet 
 * @see VOIP_MGR_GET_T38_PACKETIN TstT38PacketIn
 */
#if ! defined (AUDIOCODES_VOIP)
int do_mgr_VOIP_MGR_GET_T38_PACKETIN( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
#if 0	/* debug T.38 */
 #ifdef CONFIG_RTK_VOIP_T38
	TstT38PacketIn stT38PacketIn;
	extern uint32 T38Dump_GetPacket( uint32 snPcm, unsigned char *pPacket );

	copy_from_user(&stT38PacketIn, (TstT38PacketIn *)user, sizeof(TstT38PacketIn));

	stT38PacketIn.nSize = T38Dump_GetPacket( stT38PacketIn.snPcm,
											 stT38PacketIn.packetIn );

	copy_to_user(user, &stT38PacketIn, sizeof(TstT38PacketIn));
 #endif /* CONFIG_RTK_VOIP_T38 */
#endif /* debug T.38 */
	return 0;
}
#else
int do_mgr_VOIP_MGR_GET_T38_PACKETIN( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	PRINT_MSG("VOIP_MGR_GET_T38_PACKETIN: NOT support for ACMW\n");
	return 0;
}
#endif		

/**
 * @ingroup VOIP_DEBUG
 * @brief Get voice data in many stages 
 * @see VOIP_MGR_SET_GETDATA_MODE TstVoipdataget_o
 */
int do_mgr_VOIP_MGR_SET_GETDATA_MODE( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
#ifndef CONFIG_RTK_VOIP_IPC_ARCH
#ifdef RTK_VOICE_RECORD
	COPY_FROM_USER(&stVoipdataget_o, (TstVoipdataget_o *)user, sizeof(TstVoipdataget_o) - 1120);

#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	int ret;
	// Send Control Packet and wait Response Packet
	mgr_chid = stVoipdataget_o.ch_id;
	stVoipdataget_o.ch_id = API_get_DSP_CH(cmd, stVoipdataget_o.ch_id);	// convert to DSP chid
	ret = ipcSentControlPacket(cmd, mgr_chid, &stVoipdataget_o, sizeof(TstVoipdataget_o));
	
	// Ckeck Response Packet (need for copy_to_user)
	unsigned short dsp_id;
	ipcCheckRespPacket(cmd, &stVoipdataget_o, &dsp_id);
	stVoipdataget_o.ch_id = API_get_Host_CH( dsp_id, stVoipdataget_o.ch_id);/* Get Host chid */
	
	//stVoipdataget_o.ret_val = ret; // update ret_val must after check response ack    
	
#else

	int temp_readindex,temp_writeindex;

	stVoipdataget[stVoipdataget_o.ch_id].write_enable=stVoipdataget_o.write_enable;

	if(!stVoipdataget_o.write_enable)
	{
  #if 0 //ec128 test mode code
		extern int ec128_pattern_index;
		extern int ec128_start_count;

		ec128_pattern_index=0;
		ec128_start_count=0;
  #endif
		stVoipdataget[stVoipdataget_o.ch_id].tx_readindex=0;
		stVoipdataget[stVoipdataget_o.ch_id].tx_writeindex=0;
		stVoipdataget[stVoipdataget_o.ch_id].rx_readindex=0;
		stVoipdataget[stVoipdataget_o.ch_id].rx_writeindex=0;
		stVoipdataget[stVoipdataget_o.ch_id].rx_readindex2=0;
		stVoipdataget[stVoipdataget_o.ch_id].rx_writeindex2=0;
	}

	if(stVoipdataget_o.mode&0x1)//tx
	{
		temp_readindex=stVoipdataget[stVoipdataget_o.ch_id].tx_readindex;
		temp_writeindex=stVoipdataget[stVoipdataget_o.ch_id].tx_writeindex;

		if(   ((temp_writeindex>temp_readindex) && ((temp_writeindex-temp_readindex)>=1120))
		   || ((temp_writeindex<temp_readindex) && ((DATAGETBUFSIZE - temp_readindex + temp_writeindex)>=1120))  )
		{
			if(temp_writeindex>temp_readindex)
				stVoipdataget_o.length = temp_writeindex-temp_readindex;
			else//(temp_writeindex<temp_readindex)
				stVoipdataget_o.length = DATAGETBUFSIZE - temp_readindex + temp_writeindex;

			memcpy(stVoipdataget_o.buf,&stVoipdataget[stVoipdataget_o.ch_id].txbuf[temp_readindex],1120);
			temp_readindex=(temp_readindex+1120)%DATAGETBUFSIZE;
		}
		else
			stVoipdataget_o.length=0;

		stVoipdataget[stVoipdataget_o.ch_id].tx_readindex=temp_readindex;
	}
	else if(stVoipdataget_o.mode&0x2)	// rx2
	{
		temp_readindex=stVoipdataget[stVoipdataget_o.ch_id].rx_readindex2;
		temp_writeindex=stVoipdataget[stVoipdataget_o.ch_id].rx_writeindex2;

		if(   ((temp_writeindex>temp_readindex) && ((temp_writeindex-temp_readindex)>=1120))
		   || ((temp_writeindex<temp_readindex) && ((DATAGETBUFSIZE - temp_readindex + temp_writeindex)>=1120))  )
		{
			if(temp_writeindex>temp_readindex)
				stVoipdataget_o.length = temp_writeindex-temp_readindex;
			else//(temp_writeindex<temp_readindex)
				stVoipdataget_o.length = DATAGETBUFSIZE - temp_readindex + temp_writeindex;

			memcpy(stVoipdataget_o.buf,&stVoipdataget[stVoipdataget_o.ch_id].rxbuf2[temp_readindex],1120);
			temp_readindex=(temp_readindex+1120)%DATAGETBUFSIZE;
		}
		else
			stVoipdataget_o.length=0;

		stVoipdataget[stVoipdataget_o.ch_id].rx_readindex2=temp_readindex;
	}
	else
	{
		temp_readindex=stVoipdataget[stVoipdataget_o.ch_id].rx_readindex;
		temp_writeindex=stVoipdataget[stVoipdataget_o.ch_id].rx_writeindex;

		if(   ((temp_writeindex>temp_readindex) && ((temp_writeindex-temp_readindex)>=1120))
		   || ((temp_writeindex<temp_readindex) && ((DATAGETBUFSIZE - temp_readindex + temp_writeindex)>=1120))  )
		{
			if(temp_writeindex>temp_readindex)
				stVoipdataget_o.length = temp_writeindex-temp_readindex;
			else//(temp_writeindex<temp_readindex)
				stVoipdataget_o.length = DATAGETBUFSIZE - temp_readindex + temp_writeindex;

			memcpy(stVoipdataget_o.buf,&stVoipdataget[stVoipdataget_o.ch_id].rxbuf[temp_readindex],1120);
			temp_readindex=(temp_readindex+1120)%DATAGETBUFSIZE;
		}
		else
			stVoipdataget_o.length=0;

		stVoipdataget[stVoipdataget_o.ch_id].rx_readindex=temp_readindex;
	}

#endif
	//stVoipdataget_o.ret_val = 0;
	
	if(stVoipdataget_o.length>=1120)
		return COPY_TO_USER(user, &stVoipdataget_o, sizeof(TstVoipdataget_o), cmd, seq_no);
	else
		return COPY_TO_USER(user, &stVoipdataget_o, sizeof(TstVoipdataget_o)-1120+stVoipdataget_o.length, cmd, seq_no);
#else
	
	PRINT_MSG("please define RTK_VOICE_RECORD in rtk_voip.h\n");
#endif //#ifdef RTK_VOICE_RECORD
#endif
	return 0;
}

#define BUFFER_FULL 1
#define BUFFER_NOTFULL 0
/**
 * @ingroup VOIP_DEBUG
 * @brief voice play 
 * @see VOIP_MGR_VOICE_PLAY TstVoipdataput_o
 */
int do_mgr_VOIP_MGR_VOICE_PLAY( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
#ifndef CONFIG_RTK_VOIP_IPC_ARCH
#ifdef RTK_VOICE_PLAY
	COPY_FROM_USER(&stVoipdataput_o, (TstVoipdataput_o *)user, sizeof(TstVoipdataput_o));

	int temp_readindex,temp_writeindex;

	stVoipdataput[stVoipdataput_o.ch_id].write_enable=stVoipdataput_o.write_enable;

	if (!stVoipdataput_o.write_enable) {
		stVoipdataput[stVoipdataput_o.ch_id].readindex=0;
		stVoipdataput[stVoipdataput_o.ch_id].writeindex=0;
	}

	stVoipdataput_o.ret_val = BUFFER_NOTFULL;

	if (stVoipdataput_o.mode&0x1) { //tx
		temp_readindex=stVoipdataput[stVoipdataput_o.ch_id].readindex;
		temp_writeindex=stVoipdataput[stVoipdataput_o.ch_id].writeindex;

		if (   ((temp_writeindex>=temp_readindex) && ((DATAPUTBUFSIZE-temp_writeindex+temp_readindex)>EACH_DATAPUTBUFSIZE))
		   || ((temp_writeindex<temp_readindex) && ((temp_readindex-temp_writeindex)>EACH_DATAPUTBUFSIZE))  ) {
			if(temp_writeindex>=temp_readindex)
				stVoipdataput_o.length = DATAPUTBUFSIZE-temp_writeindex+temp_readindex;
			else//(temp_writeindex<temp_readindex)
				stVoipdataput_o.length = temp_readindex-temp_writeindex;

			memcpy(&stVoipdataput[stVoipdataput_o.ch_id].txbuf[temp_writeindex], stVoipdataput_o.buf, EACH_DATAPUTBUFSIZE);
			temp_writeindex=(temp_writeindex+EACH_DATAPUTBUFSIZE)%DATAPUTBUFSIZE;
		} else {
			stVoipdataput_o.length=0;
			stVoipdataput_o.ret_val = BUFFER_FULL;
		}
		stVoipdataput[stVoipdataput_o.ch_id].writeindex=temp_writeindex;
	}
	
	COPY_TO_USER(user, &stVoipdataput_o, sizeof(TstVoipdataput_o)-EACH_DATAPUTBUFSIZE, cmd, seq_no);
#else
	
	PRINT_MSG("please define RTK_VOICE_PLAY in rtk_voip.h\n");
#endif //#ifdef RTK_VOICE_PLAY
#else
	PRINT_R("Not support, %s\n", __FUNCTION__);
#endif
	return 0;
}

/**
 * @ingroup VOIP_DEBUG
 * @brief IP phone test function to manipulate codec and LED 
 * @see VOIP_MGR_IPHONE_TEST IPhone_test 
 */
int do_mgr_VOIP_MGR_IPHONE_TEST( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
#ifndef CONFIG_RTK_VOIP_IPC_ARCH
#ifdef CONFIG_RTK_VOIP_DRIVERS_IP_PHONE
	unsigned long flags;
	IPhone_test iphone;

	COPY_FROM_USER(&iphone, (IPhone_test *)user, sizeof(IPhone_test));
	NO_COPY_TO_USER( cmd, seq_no );
	save_flags(flags); cli();
	if (iphone.function_type == 0) {
#if defined(CONFIG_RTK_VOIP_DRIVERS_CODEC_WM8510)
		write_WM8510(iphone.reg, iphone.value);
#elif defined(CONFIG_RTK_VOIP_DRIVERS_CODEC_ALC5621)
		write_ALC5621(iphone.reg, iphone.value);
#endif
	} else if (iphone.function_type == 1) {
#if defined(CONFIG_RTK_VOIP_DRIVERS_CODEC_WM8510)
		WM8510_fake_read(iphone.reg);
#elif defined(CONFIG_RTK_VOIP_DRIVERS_CODEC_ALC5621)
		ALC5621_fake_read(iphone.reg);
#endif
	} else if (iphone.function_type == 2) {
#if defined(CONFIG_RTK_VOIP_DRIVERS_CODEC_WM8510)
		Audio_interface_loopback_test(1);
#elif defined(CONFIG_RTK_VOIP_DRIVERS_CODEC_ALC5621)
		ALC5621_loopback( 1 );
#endif
	} else if (iphone.function_type == 3) {
#if defined(CONFIG_RTK_VOIP_DRIVERS_CODEC_WM8510)
		Audio_interface_loopback_test(0);
#elif defined(CONFIG_RTK_VOIP_DRIVERS_CODEC_ALC5621)
		ALC5621_loopback( 0 );
#endif
	} else if (iphone.function_type == 4) {
#if defined( CONFIG_RTK_VOIP_GPIO_IPP_100 ) || defined( CONFIG_RTK_VOIP_GPIO_IPP_101 )
		led_shower(iphone.reg);
#elif defined( CONFIG_RTK_VOIP_GPIO_IPP_8972_V00 ) || defined( CONFIG_RTK_VOIP_GPIO_IPP_8972_V01 ) 
		pt6961_SetDisplay( iphone.reg );
#elif defined( CONFIG_RTK_VOIP_GPIO_IPP_8952_V00 ) || defined( CONFIG_RTK_VOIP_GPIO_IPP_8972B_V00 )
		LED_DirectGPIO_SetDisplay( iphone.reg );
#endif
	}
	restore_flags(flags);
#endif //CONFIG_RTK_VOIP_DRIVERS_IP_PHONE
#endif
	return 0;
}

/**
 * @ingroup VOIP_DEBUG
 * @brief Print debug message according to its level 
 * @see VOIP_MGR_PRINT rtk_print_cfg
 */
int do_mgr_VOIP_MGR_PRINT( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
	rtk_print_cfg cfg;
	int ret;

	if (len != sizeof(cfg))
	{
		DBG_ERROR("%s", "invalid len\n");
		return -EFAULT;
	}

	if (COPY_FROM_USER(&cfg, user, len))
	{
		DBG_ERROR("%s", "copy_from_user failed\n");
		return -EFAULT;
	}
	if( ( ret = NO_COPY_TO_USER( cmd, seq_no ) ) < 0 )
		return ret;

#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	// Host auto forward 
#endif

	DBG_PRINT(cfg.level, cfg.module, "%s", cfg.msg);
	return 0;
}

/**
 * @ingroup VOIP_DEBUG
 * @brief Config the COP3 parameters for VoIP
 * @see VOIP_MGR_COP3_CONIFG st_CP3_VoIP_param
 */
#ifdef CONFIG_VOIP_COP3_PROFILE
	st_CP3_VoIP_param cp3_voip_param = {0};
	unsigned int gCp3Params = 0;
#endif

int do_mgr_VOIP_MGR_COP3_CONIFG( int cmd, void *user, unsigned int len, unsigned short seq_no )
{
#ifdef CONFIG_VOIP_COP3_PROFILE

#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
	// Host auto forward 
#else
	int ret;

	COPY_FROM_USER(&cp3_voip_param, user, len);
	if( ( ret = NO_COPY_TO_USER( cmd, seq_no ) ) < 0 )
		return ret;

#ifdef CONFIG_ARCH_CPU_RLX5281
	gCp3Params = /* Counter0 */((cp3_voip_param.cp3_counter1)<< 0) |
	             /* Counter1 */((cp3_voip_param.cp3_counter2)<< 8) |
	             /* Counter2 */((cp3_voip_param.cp3_counter3)<<16) |
	             /* Counter3 */((cp3_voip_param.cp3_counter4)<<24);
#else
	gCp3Params = /* Counter0 */((0x10|cp3_voip_param.cp3_counter1)<< 0) |
	             /* Counter1 */((0x10|cp3_voip_param.cp3_counter2)<< 8) |
	             /* Counter2 */((0x10|cp3_voip_param.cp3_counter3)<<16) |
	             /* Counter3 */((0x10|cp3_voip_param.cp3_counter4)<<24);
#endif
	PRINT_MSG("VOIP_MGR_COP3_CONIFG:\n");
	PRINT_MSG(" - counter1: %d\n", cp3_voip_param.cp3_counter1);
	PRINT_MSG(" - counter2: %d\n", cp3_voip_param.cp3_counter2);
	PRINT_MSG(" - counter3: %d\n", cp3_voip_param.cp3_counter3);
	PRINT_MSG(" - counter4: %d\n", cp3_voip_param.cp3_counter4);
	PRINT_MSG(" - dump period: %d\n", cp3_voip_param.cp3_dump_period);

#endif

#endif
	return 0;
}


