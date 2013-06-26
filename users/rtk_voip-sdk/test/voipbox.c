#include <stdio.h>

#ifdef __ECOS

#include <commands.h>

#else

typedef struct {
	char name[20];
	int (*func)(int, char **);
} TVoIPBox;

extern int dbg_main(int argc, char *argv[]);
extern int fskgen_main(int argc, char *argv[]);
extern int pulse_dial_main(int argc, char *argv[]);
extern int cp3_measure_main(int argc, char *argv[]);
extern int led_main(int argc, char *argv[]);
extern int phonerecord_main(int argc, char *argv[]);
extern int reg_main(int argc, char *argv[]);
extern int ram_main(int argc, char *argv[]);
extern int slic_reset_main(int argc, char *argv[]);
extern int switchmii_main(int argc, char *argv[]);
extern int test_main(int argc, char *argv[]);
extern int vmwigen_main(int argc, char *argv[]);
extern int voicerecord_main(int argc, char *argv[]);
extern int voiceplay_main(int argc, char *argv[]);
extern int ec_test_main(int argc, char *argv[]);
extern int rtcp_main(int argc, char *argv[]);
extern int rtcp_logger_main(int argc, char *argv[]);
extern int iphone_test_main(int argc, char *argv[]);
extern int ring_test_main(int argc, char *argv[]);
extern int clone_mac_main(int argc, char *arg[]);
extern int gpio_main(int argc, char *argv[]);
extern int bandwidth_mgr_main(int argc, char *argv[]);
extern int send_2833_main(int argc, char *argv[]);
extern int power_main(int argc, char *argv[]);
extern int netmask_main(int argc, char *argv[]);
extern int voip_cli_main(int argc, char *argv[]);
extern int tone_main(int argc, char *argv[]);
extern int lb_test_main(int argc, char *argv[]);
extern int voip_event_main(int argc, char *argv[]);
extern int dtmf_det_cfg_main(int argc, char *argv[]);

#endif // __ECOS

#ifdef VOIPBOX_GPIO
#ifdef __ECOS
CMD_DECL( gpio_main1 ) { return gpio_main( argc, argv ); }
CMD_DECL( gpio_main2 ) { return gpio_main( argc, argv ); }
CMD_DECL( gpio_main3 ) { return gpio_main( argc, argv ); }
#else
#define gpio_main1	gpio_main
#define gpio_main2	gpio_main
#define gpio_main3	gpio_main
#endif
#endif


#ifdef __ECOS
  	#define VOIP_BOX_DECLARE_BEGIN	
  	#define VOIP_BOX_DECLARE_END	
    #define VOIP_BOX_DECLARE( _name_, _func_ )	\
    	shell_old_cmd( _name_, "", "", _func_ );
#endif

#ifndef VOIP_BOX_DECLARE
  	#define VOIP_BOX_DECLARE_BEGIN				\
  		TVoIPBox voip_box[] = {
  	#define VOIP_BOX_DECLARE_END				\
  		};
	#define VOIP_BOX_DECLARE( _name_, _func_ )	\
		{ _name_, _func_ },
#endif

VOIP_BOX_DECLARE_BEGIN	// TVoIPBox voip_box[] = {
#ifdef VOIPBOX_DBG
	VOIP_BOX_DECLARE("dbg", dbg_main)
#endif
#ifdef VOIPBOX_FSKGEN
	VOIP_BOX_DECLARE("fskgen", fskgen_main)
#endif
#ifdef VOIPBOX_PULSE_DIAL
	VOIP_BOX_DECLARE("pulse_dial", pulse_dial_main)
#endif
#ifdef VOIPBOX_CP3_MEASURE
	VOIP_BOX_DECLARE("cp3_measure", cp3_measure_main)
#endif
#ifdef VOIPBOX_LED
	VOIP_BOX_DECLARE("led", led_main)
#endif
#ifdef VOIPBOX_PHONERECORD
	VOIP_BOX_DECLARE("phonerecord", phonerecord_main)
#endif
#ifdef VOIPBOX_VOICEPLAY
	VOIP_BOX_DECLARE("voiceplay", voiceplay_main)
#endif
#ifdef VOIPBOX_REG
	VOIP_BOX_DECLARE("reg", reg_main)
#ifdef VOIPBOX_RAM
	VOIP_BOX_DECLARE("ram", ram_main)
#endif
#endif
#ifdef VOIPBOX_SLIC_RESET
	VOIP_BOX_DECLARE("slic_reset", slic_reset_main)
#endif
#ifdef VOIPBOX_SWITCHMII
	VOIP_BOX_DECLARE("switchmii", switchmii_main)
#endif
#ifdef VOIPBOX_TEST
	VOIP_BOX_DECLARE("crash", test_main)
#endif
#ifdef VOIPBOX_VMWIGEN
	VOIP_BOX_DECLARE("vmwigen", vmwigen_main)
#endif
#ifdef VOIPBOX_VOICERECORD
	VOIP_BOX_DECLARE("voicerecord", voicerecord_main)
#endif
#ifdef VOIPBOX_EC_TEST
	VOIP_BOX_DECLARE("ec_test", ec_test_main)
#endif
#ifdef VOIPBOX_RTCP_STATISTIC
	VOIP_BOX_DECLARE("rtcp_statistic", rtcp_main)
#endif
#ifdef VOIPBOX_RTCP_LOGGER
	{"rtcp_logger", rtcp_logger_main},
#endif
#ifdef CONFIG_RTK_VOIP_DRIVERS_IP_PHONE
	VOIP_BOX_DECLARE("iphone_test", iphone_test_main)
#endif
#ifdef VOIPBOX_RING_TEST
	VOIP_BOX_DECLARE("ring_test", ring_test_main)
#endif
#ifdef VOIPBOX_CLONE_MAC
	VOIP_BOX_DECLARE("clone_mac", clone_mac_main)
#endif
#ifdef VOIPBOX_GPIO
	VOIP_BOX_DECLARE("gpio_init", gpio_main1)
	VOIP_BOX_DECLARE("gpio_read", gpio_main2)
	VOIP_BOX_DECLARE("gpio_write", gpio_main3)
#endif
#ifdef VOIPBOX_BANDWIDTH_MGR
	VOIP_BOX_DECLARE("bandwidth_mgr", bandwidth_mgr_main)
#endif
#ifdef VOIPBOX_SEND_2833
	VOIP_BOX_DECLARE("send_2833", send_2833_main)
#endif
#ifdef VOIPBOX_POWER
	VOIP_BOX_DECLARE("power", power_main)
#endif
#ifdef VOIPBOX_NETMASK
	VOIP_BOX_DECLARE("netmask", netmask_main)
#endif
#ifdef VOIPBOX_CLI
	VOIP_BOX_DECLARE("voipcli", voip_cli_main)
#endif
#ifdef VOIPBOX_TONE
	VOIP_BOX_DECLARE("tone", tone_main)
#endif
#ifdef VOIPBOX_LBTEST
	VOIP_BOX_DECLARE("lb_test", lb_test_main)
#endif
#ifdef VOIPBOX_VOIPEVENT
	VOIP_BOX_DECLARE("voip_event", voip_event_main)
#endif
#ifdef VOIPBOX_DTMF_DET_CFG
	VOIP_BOX_DECLARE("dtmf_det_cfg", dtmf_det_cfg_main)
#endif
#ifndef __ECOS
	VOIP_BOX_DECLARE("", NULL)
#endif
VOIP_BOX_DECLARE_END	//};

#ifndef __ECOS
int main(int argc, char *argv[])
{
	int i;

	for (i=0; voip_box[i].func; i++)
	{
		if (strcmp(argv[0], voip_box[i].name) == 0)
		{
			return voip_box[i].func(argc, argv);
		}
	}

	printf("voip box: cmd %s is not support\n", argv[0]);
	return 0;
}
#endif

