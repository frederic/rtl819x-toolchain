#ifndef INCLUDE_ULINKER_H
#define INCLUDE_ULINKER_H

#if defined(CONFIG_RTL_ULINKER)

#if 0
#define BDBG_FPRINT(format, arg...)		\
	fprintf(format , ## arg)
#else
#define BDBG_FPRINT(format, arg...)
#endif

#define ULINKER_AP_CL "/proc/ulinker_ap_cl"
#define ULINKER_AP_CL_SWITCHING "/proc/ulinker_ap_cl_switching"
#define ULINKER_AUTO_DHCP "/var/ulinker_auto_dhcp"

#define ULINKER_RESET_DOMAIN_NAME_QUERY "/var/ulinker_reset_domain"


#endif

#if defined(CONFIG_RTL_ULINKER)
/*----- define -----*/
#define __FUNC__  __FUNCTION__
#define ULINKER_DHCPS_DISCOVER_FLAG "/var/ulinker_dhcps_discover_flag"
#define ULINKER_MODE_FLAG "/var/ulinker_mode_flag"
#define ULINKER_CDC_RNDIS_STATUS_FLAG "/proc/ulinker_cdc_rndis_status"

#define AUTO_DETECT_RPEATER  1 /* 0:  auto detect can't have repeater mode */

typedef enum { ULINKER_BR_AP_MODE=0, ULINKER_BR_CL_MODE=1, ULINKER_AUTO_WAN_MODE=2, ULINKER_BR_USB_MODE=3,
				ULINKER_RT_DHCP_MODE=4, ULINKER_RT_PPPOE_MODE=5, ULINKER_BR_RPT_MODE=6 } ULINKER_OPMODE_T;
typedef enum { ULINKER_WL_CL=CLIENT_MODE, ULINKER_WL_AP=AP_MODE} ULINKER_WLAN_MODE_T;


/*----- debug -----*/
#define BDBG_PING_TEST      0
#define BDBG_RNDIS_CHECK    0
#define BDBG_DHCP_SERVER_DISCOVER  0
#define BDBG_ULINKER_DECIDE_DUT_MODE   0
#define BDBG_ULINKER_SET_DUT_TO_DECIDED_MODE 0
#define BDBG_ULINKER_SWAP_AP_CL_MIB 0
#define BDBG_ULINKER_BOOTUP 0
#define BDBG_ULINKER_DOES_DUT_MODE_CHANGED 0
#define BDBG_ULINKER_DOMAIN_NAME_QUERY 0


#if BDBG_PING_TEST
#define bdbg_ping_test(format, arg...)		\
	fprintf(format , ## arg)
#else
#define bdbg_ping_test(format, arg...)
#endif

#if BDBG_RNDIS_CHECK
#define bdbg_rndis_check(format, arg...)		\
	fprintf(format , ## arg)
#else
#define bdbg_rndis_check(format, arg...)
#endif

#if BDBG_DHCP_SERVER_DISCOVER
#define bdbg_dhcps_discover(format, arg...)		\
	fprintf(format , ## arg)
#else
#define bdbg_dhcps_discover(format, arg...)
#endif

#if BDBG_ULINKER_DECIDE_DUT_MODE
#define bdbg_decide_dut_mode(format, arg...)		\
	fprintf(format , ## arg)
#else
#define bdbg_decide_dut_mode(format, arg...)
#endif

#if BDBG_ULINKER_SET_DUT_TO_DECIDED_MODE
#define bdbg_set_decided_mode(format, arg...)		\
	fprintf(format , ## arg)
#else
#define bdbg_set_decided_mode(format, arg...)
#endif

#if BDBG_ULINKER_SWAP_AP_CL_MIB
#define bdbg_swap_ap_cl_mib(format, arg...)		\
	fprintf(format , ## arg)
#else
#define bdbg_swap_ap_cl_mib(format, arg...)
#endif

#if BDBG_ULINKER_BOOTUP
#define bdbg_ulinker_bootup(format, arg...)		\
	fprintf(format , ## arg)
#else
#define bdbg_ulinker_bootup(format, arg...)
#endif

#if BDBG_ULINKER_DOES_DUT_MODE_CHANGED
#define bdbg_ulinker_does_dut_mode_changed(format, arg...)		\
	fprintf(format , ## arg)
#else
#define bdbg_ulinker_does_dut_mode_changed(format, arg...)
#endif

#if BDBG_ULINKER_DOMAIN_NAME_QUERY
#define bdbg_ulinker_domain_name_query(format, arg...)		\
		fprintf(format , ## arg)
#else
#define bdbg_ulinker_domain_name_query(format, arg...)
#endif


/*----- inline -----*/
static inline int eth1_up(void)
{
	system("ifconfig eth1 0.0.0.0 2> /dev/null");
	return 1;
}

static inline int eth1_down(void)
{
	system("ifconfig eth1 0.0.0.0 2> /dev/null");
	system("ifconfig eth1 down 2> /dev/null");
	return 1;
}

static inline int usb0_up(void)
{
	system("ifconfig usb0 0.0.0.0 2> /dev/null");
	return 1;
}

static inline int usb0_down(void)
{
	system("ifconfig usb0 0.0.0.0 2> /dev/null");
	system("ifconfig usb0 down 2> /dev/null");

	return 1;
}

static inline int wlan_down(void)
{
	system("ifconfig wlan0 0.0.0.0 2> /dev/null");
	system("ifconfig wlan0 down 2> /dev/null");

	system("ifconfig wlan0-vxd 0.0.0.0 2> /dev/null");
	system("ifconfig wlan0-vxd down 2> /dev/null");

	return 1;
}

static inline int start_dhcpc(void)
{
#if BDBG_DHCP_SERVER_DISCOVER
	system("udhcpc -d 1 -i eth1 -p /etc/udhcpc/udhcpc-eth1.pid -s /usr/share/udhcpc/br0.bound");
#else
	system("udhcpc -d 1 -i eth1 -p /etc/udhcpc/udhcpc-eth1.pid -s /usr/share/udhcpc/br0.bound >/dev/null 2>&1");
#endif

	return 1;
}

static inline int stop_dhcpc(void)
{
	/* todo: search pid and then kill */
	system("killall udhcpc >/dev/null 2>&1");
	system("rm -f /etc/udhcpc/udhcpc-*.pid >/dev/null 2>&1");
	return 1;
}

static inline int stop_dhcpd(void)
{
	system("killall -9 udhcpd 2> /dev/null");
	system("rm -f /var/run/udhcpd.pid >/dev/null 2>&1");
	return 1;
}

static inline int kill_daemons(void)
{
	system("killall -9 lld2d >/dev/null 2>&1");
	system("killall -9 reload >/dev/null 2>&1");
	system("killall -9 iapp >/dev/null 2>&1");
	system("killall -9 wscd >/dev/null 2>&1");
	system("killall -9 dnrd >/dev/null 2>&1");
	system("killall -9 nmbserver >/dev/null 2>&1");
	system("killall -9 igmpproxy >/dev/null 2>&1");
	return 1;
}

static inline int clean_dhcps_discover(void)
{
	char *buf = calloc(sizeof(char), 64);

	sprintf(buf, "rm -f %s", ULINKER_DHCPS_DISCOVER_FLAG);
	system(buf);

	if (buf) free(buf);
	return 1;
}

static inline int export_decided_mode(int mode)
{
	char *buf = calloc(sizeof(char), 64);

	sprintf(buf, "echo %d > %s", mode, ULINKER_MODE_FLAG);
	system(buf);

	if (buf) free(buf);
	return 1;
}

static inline int clean_decided_mode(void)
{
	char *buf = calloc(sizeof(char), 128);

	sprintf(buf, "rm -f %s >/dev/null 2>&1", ULINKER_MODE_FLAG);
	system(buf);

	if (buf) free(buf);
	return 1;
}

static inline int enable_bridge_dhcp_filter(void)
{
	/* start to filter dhcp discover in bridge */
	system("echo 1 > /proc/pocket/en_filter");
	return 1;
}

static inline int disable_bridge_dhcp_filter(void)
{
	/* stop filter dhcp discover in bridge */
	system("echo 0 > /proc/pocket/en_filter");
	system("echo \"00000000 000000000000 0\" > /proc/pocket/filter_conf");
	return 1;
}

static inline int clean_auto_dhcp_flag(void)
{
	/* clean auot_dhcp flag */
	system("rm -f /var/ulinker_auto_dhcp >/dev/null 2>&1");
	return 1;
}

static inline int clean_up_layer_dns_flag(void)
{
	/* clean up layer dns flag */
	system("rm -f /var/ulinker_dns >/dev/null 2>&1");
	return 1;
}

static inline int reset_domain_query(void)
{
	/* notice ulinker_process to reset domain name query */
	system("echo '1' > /var/ulinker_reset_domain >/dev/null 2>&1");
	return 1;
}

static inline int system_initial_led(int running)
{
	if (running == 1)
		system("echo 1 > /proc/uled"); //system initating, blink led
	else
		system("echo 0 > /proc/uled"); //system init done, disable system led
	return 1;
}

#endif /* #if defined(CONFIG_RTL_ULINKER) */


#endif // INCLUDE_ULINKER_H
