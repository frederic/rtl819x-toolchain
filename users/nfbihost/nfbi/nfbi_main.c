#include <stdio.h> 
#include <fcntl.h> 
#include <sys/types.h> 
#include <sys/uio.h> 
#include <unistd.h> 
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <linux/if.h>
#include <linux/if_ether.h>
#include <netinet/in.h>

#include "../../../linux-2.6.30/drivers/char/rtl_nfbi/rtl_nfbi.h"
#include "nfbi_api.h"

#define DEV_NAME  ("/dev/"DRIVER_NAME)

#define NFBI_VERSION	"1.4"

typedef struct rtk_wireless_info {
    unsigned char ap[8];// first 6 bytes indicated as associated AP MAC/BSSID address, last two bytes
                        // are padding data. An address equal to 00:00:00:00:00:00 means that the module 
                        // failed in association
    unsigned char ssid[36];// SSID of associated AP/IBSS
	int freq;			// channel number of associated AP/IBSS
	int link_quality;
	int signal_level;
	int noise_level;
	int rx_invalid_cypt;	// error frame for decryption
	int rx_invalid_frag;	// error frame for decode fragmentation
	int tx_excessive_retry;	// error frame to transmission
	int invalid_misc; 	// other packets lost in relation with specific wireless operations. 
    int missed_beacon;	// number of periodic beacons from the Access Point/ we have missed.
} rtk_wireless_info;

typedef struct bss_info {
    unsigned char bssid[8];	// first 6 bytes indicate as BSS ID/AP mac address, last 2 bytes are padding data
    unsigned char ssid[34];	// SSID in string
    unsigned short channel;
    unsigned char network;	// network type, bit-mask value: 
                            // 1 ¡V 11b, 2 ¡V 11g, 4 ¡V 11a-legacy, 8 ¡V 11n, 16 ¡V 11a-n
    unsigned char type;	// AP or Ad-hoc. 0 - AP, 1 - Ad-hoc
    unsigned char encrypt;// encryption type. 0 ¡V open, 1 ¡V WEP, 2 ¡V WPA, 3 ¡VWPA2, 
                            // 4 ¡VMixed Mode (WPA+WPA2)
    unsigned char rssi;		// received signal strength. 1-100
} bss_info;

typedef struct scan_result {
    unsigned char index;      // indicate the position of bssdb[0] in the scanning result
    unsigned char number;	// number bss_info existed in bssdb[] array
    unsigned char more;      // 0:no more scanning result, 1:there are still scanning results after bssdb[4]
    unsigned char pad;	    // padding field, not use
    bss_info bssdb[5];
} scan_result;

static int got_sigusr1 = 0;

extern int nfbi_fd;

void debug_out(unsigned char *label, unsigned char *data, int data_length)
{
    int i,j;
    int num_blocks;
    int block_remainder;

    num_blocks = data_length >> 4;
    block_remainder = data_length & 15;

	if (label) {
	    printf("%s\n", label);
	}

	if (data==NULL || data_length==0)
		return;

    for (i=0; i<num_blocks; i++)
    {
        printf("\t");
        for (j=0; j<16; j++)
        {
            printf("%02x ", data[j + (i<<4)]);
        }
        printf("\n");
    }

    if (block_remainder > 0)
    {
        printf("\t");
        for (j=0; j<block_remainder; j++)
        {
            printf("%02x ", data[j+(num_blocks<<4)]);
        }
        printf("\n");
    }
}

struct reg_table_t {
	const char*	name;
    int val;
    int dval; //default value
};

static struct reg_table_t nfbi_reg_table[]=
{
    // {Name, REGAD}
    {"BMCR",   NFBI_REG_BMCR, 0x3000},
    {"BMSR",   NFBI_REG_BMSR, 0x7849},
    {"PHYID1", NFBI_REG_PHYID1, 0x001c},
    {"PHYID2", NFBI_REG_PHYID2, 0xcb61},
    {"ANAR",   NFBI_REG_ANAR, 0x0de1},
    {"ANLPAR", NFBI_REG_ANLPAR, 0x0001},
    {"CMD",    NFBI_REG_CMD, 0x0000},
    {"ADDH",   NFBI_REG_ADDH,0x1fc0},
    {"ADDL",   NFBI_REG_ADDL, 0x0000},
    {"DH",     NFBI_REG_DH, 0x0000},
    {"DL",     NFBI_REG_DL, 0x0000},
    {"SCR",    NFBI_REG_SCR,0x0000},
    {"RSR",    NFBI_REG_RSR,0x0000},
    {"SYSSR",  NFBI_REG_SYSSR,0x0000},
    {"SYSCR",  NFBI_REG_SYSCR,0x1800},
    {"IMR",    NFBI_REG_IMR,0x0001},
    {"ISR",    NFBI_REG_ISR,0x0000},
    {"DCH",    NFBI_REG_DCH,0x5808},
    {"DCL",    NFBI_REG_DCL,0x0000},
    {"DTH",    NFBI_REG_DTH,0xffff},
    {"DTL",    NFBI_REG_DTL,0x0fc0},
    {"RR",     NFBI_REG_RR, 0x0000},
    {NULL, 0, 0}
};

int cmd_common_command(char type, int index, int argc, char *argv[]);
int cmd_write_memory(char type, int index, int argc, char *argv[]);
int cmd_read_memory(char type, int index, int argc, char *argv[]);
int cmd_mac_address(char type, int index, int argc, char *argv[]);
int cmd_data(char type, int index, int argc, char *argv[]);
int cmd_getwlaninfo(char type, int index, int argc, char *argv[]);
int cmd_get_scan_result(char type, int index, int argc, char *argv[]);
int cmd_cfg(char type, int index, int argc, char *argv[]);

struct cmdframe_table_t {
	const char*	name;
    int val;
    int mode;  //0: R/W, 1: R, 2: W, 
    int dlen;
    int min_val;
    int max_val;
    char *defval;
    int (*func)(char type, int index, int argc, char *argv[]);
	int retransmit_count;
	int response_timeout; // in 10ms
};

static struct cmdframe_table_t nfbi_cmdframe_table[]=
{
    // {name,              val,mode,dlen,min,max,defval, func}
    {"cmd_timeout",         0x00, 0,  1,  1, 255, "1", cmd_common_command, NFBI_RETRANSMIT_COUNT_DEFAULT, NFBI_RESPONSE_TIMEOUT_DEFAULT},
    {"mii_pause_enable",    0x01, 0,  1,  0, 1, "1", cmd_common_command, NFBI_RETRANSMIT_COUNT_DEFAULT, NFBI_RESPONSE_TIMEOUT_DEFAULT},
    {"eth_pause_enable",    0x02, 0,  1,  0, 1, "1", cmd_common_command, NFBI_RETRANSMIT_COUNT_DEFAULT, NFBI_RESPONSE_TIMEOUT_DEFAULT},
    {"cpu_suspend_enable",  0x03, 0,  1,  0, 1, "0", cmd_common_command, NFBI_RETRANSMIT_COUNT_DEFAULT, NFBI_RESPONSE_TIMEOUT_DEFAULT},
    {"phy_reg_poll_time",   0x04, 0,  1,  1, 100, "1", cmd_common_command, NFBI_RETRANSMIT_COUNT_DEFAULT, NFBI_RESPONSE_TIMEOUT_DEFAULT},
    {"write_memory",        0x05, 2,  8,  0, 0, "", cmd_write_memory, NFBI_RETRANSMIT_COUNT_DEFAULT, NFBI_RESPONSE_TIMEOUT_DEFAULT},
    {"read_memory",         0x06, 1,  4,  0, 0, "", cmd_read_memory, NFBI_RETRANSMIT_COUNT_DEFAULT, NFBI_RESPONSE_TIMEOUT_DEFAULT},
    {"fw_version",          0x07, 0, 15, 0, 0, "", cmd_data, NFBI_RETRANSMIT_COUNT_DEFAULT, NFBI_RESPONSE_TIMEOUT_DEFAULT},
    {"wlan_mac_addr",       0x08, 1, 6, 0, 0, "000000000000", cmd_mac_address, NFBI_RETRANSMIT_COUNT_DEFAULT, NFBI_RESPONSE_TIMEOUT_DEFAULT},

    {"wlan_link_down_time", 0x10, 0,  1, 1, 60, "5", cmd_common_command, NFBI_RETRANSMIT_COUNT_DEFAULT, NFBI_RESPONSE_TIMEOUT_DEFAULT},
    {"channel",             0x11, 0,  1, 0, 216, "0", cmd_common_command, NFBI_RETRANSMIT_COUNT_DEFAULT, NFBI_RESPONSE_TIMEOUT_DEFAULT},
    //{"ssid",                0x12, 0, 32, 0, 0, "", cmd_common_command, NFBI_RETRANSMIT_COUNT_DEFAULT, NFBI_RESPONSE_TIMEOUT_DEFAULT},
    {"bssid2join",          0x13, 0, 6, 0, 0, "000000000000", cmd_mac_address, NFBI_RETRANSMIT_COUNT_DEFAULT, NFBI_RESPONSE_TIMEOUT_DEFAULT},
    {"regdomain",           0x14, 0,  1,  1, 11, "1", cmd_common_command, NFBI_RETRANSMIT_COUNT_DEFAULT, NFBI_RESPONSE_TIMEOUT_DEFAULT},
    {"autorate",            0x15, 0,  1,  0, 1,  "1", cmd_common_command, NFBI_RETRANSMIT_COUNT_DEFAULT, NFBI_RESPONSE_TIMEOUT_DEFAULT},
    {"fixrate",             0x16, 0,  4,  0, 255,"0", cmd_common_command, NFBI_RETRANSMIT_COUNT_DEFAULT, NFBI_RESPONSE_TIMEOUT_DEFAULT},
    {"authtype",            0x17, 0,  1,  0, 2,  "2", cmd_common_command, NFBI_RETRANSMIT_COUNT_DEFAULT, NFBI_RESPONSE_TIMEOUT_DEFAULT},
    {"encmode",             0x18, 0,  1,  0, 5,  "0", cmd_common_command, NFBI_RETRANSMIT_COUNT_DEFAULT, NFBI_RESPONSE_TIMEOUT_DEFAULT},
    {"wepdkeyid",           0x19, 0,  1,  0, 3,  "0", cmd_common_command, NFBI_RETRANSMIT_COUNT_DEFAULT, NFBI_RESPONSE_TIMEOUT_DEFAULT},
    {"psk_enable",          0x1a, 0,  1,  0, 2,  "0", cmd_common_command, NFBI_RETRANSMIT_COUNT_DEFAULT, NFBI_RESPONSE_TIMEOUT_DEFAULT},
    {"wpa_cipher",          0x1b, 0,  1,  2, 8,  "0", cmd_common_command, NFBI_RETRANSMIT_COUNT_DEFAULT, NFBI_RESPONSE_TIMEOUT_DEFAULT},
    {"wpa2_cipher",         0x1c, 0,  1,  2, 8,  "0", cmd_common_command, NFBI_RETRANSMIT_COUNT_DEFAULT, NFBI_RESPONSE_TIMEOUT_DEFAULT},
    //{"passphrase",          0x1d, 0, 64, 0, 0,  "", cmd_common_command, NFBI_RETRANSMIT_COUNT_DEFAULT, NFBI_RESPONSE_TIMEOUT_DEFAULT},
    //{"wepkey1",             0x1e, 0, 13, 0, 0,  "", cmd_common_command, NFBI_RETRANSMIT_COUNT_DEFAULT, NFBI_RESPONSE_TIMEOUT_DEFAULT},
    //{"wepkey2",             0x1f, 0, 13, 0, 0,  "", cmd_common_command, NFBI_RETRANSMIT_COUNT_DEFAULT, NFBI_RESPONSE_TIMEOUT_DEFAULT},
    //{"wepkey3",             0x20, 0, 13, 0, 0,  "", cmd_common_command, NFBI_RETRANSMIT_COUNT_DEFAULT, NFBI_RESPONSE_TIMEOUT_DEFAULT},
    //{"wepkey4",             0x21, 0, 13, 0, 0,  "", cmd_common_command, NFBI_RETRANSMIT_COUNT_DEFAULT, NFBI_RESPONSE_TIMEOUT_DEFAULT},
    {"opmode",              0x22, 0,  1,  8, 32,  "8", cmd_common_command, NFBI_RETRANSMIT_COUNT_DEFAULT, NFBI_RESPONSE_TIMEOUT_DEFAULT},
    {"rtsthres",            0x23, 0,  2,  0, 2347,  "2347", cmd_common_command, NFBI_RETRANSMIT_COUNT_DEFAULT, NFBI_RESPONSE_TIMEOUT_DEFAULT},
    {"fragthres",           0x24, 0,  2,256, 2346,  "2346", cmd_common_command, NFBI_RETRANSMIT_COUNT_DEFAULT, NFBI_RESPONSE_TIMEOUT_DEFAULT},
    {"shortretry",          0x25, 0,  1,  0, 255,  "0", cmd_common_command, NFBI_RETRANSMIT_COUNT_DEFAULT, NFBI_RESPONSE_TIMEOUT_DEFAULT},
    {"longretry",           0x26, 0,  1,  0, 255,  "0", cmd_common_command, NFBI_RETRANSMIT_COUNT_DEFAULT, NFBI_RESPONSE_TIMEOUT_DEFAULT},
    {"band",                0x27, 0,  1,  1, 255,  "11", cmd_common_command, NFBI_RETRANSMIT_COUNT_DEFAULT, NFBI_RESPONSE_TIMEOUT_DEFAULT},
    {"macclone_enable",     0x28, 0,  1,  0, 1,  "1", cmd_common_command, NFBI_RETRANSMIT_COUNT_DEFAULT, NFBI_RESPONSE_TIMEOUT_DEFAULT},
    {"clone_mac_addr",      0x29, 0, 6, 0, 0, "000000000000", cmd_mac_address, NFBI_RETRANSMIT_COUNT_DEFAULT, NFBI_RESPONSE_TIMEOUT_DEFAULT},
    {"qos_enable",          0x2a, 0,  1,  0, 1,  "1", cmd_common_command, NFBI_RETRANSMIT_COUNT_DEFAULT, NFBI_RESPONSE_TIMEOUT_DEFAULT},
    {"use40M",              0x2b, 0,  1,  0, 1,  "1", cmd_common_command, NFBI_RETRANSMIT_COUNT_DEFAULT, NFBI_RESPONSE_TIMEOUT_DEFAULT},
    {"shortGI20M",          0x2c, 0,  1,  0, 1,  "1", cmd_common_command, NFBI_RETRANSMIT_COUNT_DEFAULT, NFBI_RESPONSE_TIMEOUT_DEFAULT},
    {"shortGI40M",          0x2d, 0,  1,  0, 1,  "1", cmd_common_command, NFBI_RETRANSMIT_COUNT_DEFAULT, NFBI_RESPONSE_TIMEOUT_DEFAULT},
    {"aggregation",         0x2e, 0,  1,  0, 1,  "1", cmd_common_command, NFBI_RETRANSMIT_COUNT_DEFAULT, NFBI_RESPONSE_TIMEOUT_DEFAULT},
    {"power_save_enable",   0x2f, 0,  1,  0, 3,  "0", cmd_common_command, NFBI_RETRANSMIT_COUNT_DEFAULT, NFBI_RESPONSE_TIMEOUT_DEFAULT},
    {"sens",                0x30, 0,  1,  0, 99,  "1", cmd_common_command, NFBI_RETRANSMIT_COUNT_DEFAULT, NFBI_RESPONSE_TIMEOUT_DEFAULT},
    {"txpower_cck",         0x31, 0,  1,  0, 18,  "0", cmd_common_command, NFBI_RETRANSMIT_COUNT_DEFAULT, NFBI_RESPONSE_TIMEOUT_DEFAULT},
    {"txpower_ofdm",        0x32, 0,  1,  0, 15,  "0", cmd_common_command, NFBI_RETRANSMIT_COUNT_DEFAULT, NFBI_RESPONSE_TIMEOUT_DEFAULT},
    {"txpower_ht",          0x33, 0,  1,  0, 13,  "0", cmd_common_command, NFBI_RETRANSMIT_COUNT_DEFAULT, NFBI_RESPONSE_TIMEOUT_DEFAULT},

    {"get_wlan_info",       0x34, 1, 0,  0, 0,  "", cmd_getwlaninfo, NFBI_RETRANSMIT_COUNT_DEFAULT, NFBI_RESPONSE_TIMEOUT_DEFAULT},
    {"request_scan",        0x35, 2, 0,  0, 0,  "", cmd_common_command, NFBI_RETRANSMIT_COUNT_DEFAULT, NFBI_RESPONSE_TIMEOUT_DEFAULT},
    {"get_scan_result",     0x36, 1, 0,  0, 0,  "", cmd_get_scan_result, NFBI_RETRANSMIT_COUNT_DEFAULT, NFBI_RESPONSE_TIMEOUT_DEFAULT},

    {"cfgwrite",            0x50, 2, 0,  0, 0,  "", cmd_cfg, NFBI_RETRANSMIT_COUNT_DEFAULT, NFBI_RESPONSE_TIMEOUT_DEFAULT},
    {"cfgread",             0x51, 1, 0,  0, 0,  "", cmd_cfg, NFBI_RETRANSMIT_COUNT_DEFAULT, NFBI_RESPONSE_TIMEOUT_DEFAULT},
    {"priv_shortretry",     0x52, 2, 1,  0, 255,  "0", cmd_common_command, NFBI_RETRANSMIT_COUNT_DEFAULT, NFBI_RESPONSE_TIMEOUT_DEFAULT},
    {"priv_longretry",      0x53, 2, 1,  0, 255,  "0", cmd_common_command, NFBI_RETRANSMIT_COUNT_DEFAULT, NFBI_RESPONSE_TIMEOUT_DEFAULT},
    {NULL, 0, 0, 0, 0, 0, 0, NULL, 0, 0},
};

// Command Table
struct nfbi_cmd_table_t {
	const char *cmd;			// Input command string
	int debug;                  //command for debugging
	int (*func)(int argc, char *argv[]);
	const char *msg;			// Help message
};

int cmd_version(int argc, char *argv[]);
int cmd_97fwversion(int argc, char *argv[]);
int cmd_createfile(int argc, char *argv[]);
int cmd_dump_regs(int argc, char *argv[]);
int cmd_dump_eq(int argc, char *argv[]);
int cmd_regread(int argc, char *argv[]);
int cmd_regwrite(int argc, char *argv[]);
int cmd_wait4bit(int argc, char *argv[]);
int cmd_phyreset(int argc, char *argv[]);
int cmd_restart_auto_nego(int argc, char *argv[]);
int cmd_meminit(int argc, char *argv[]);
int cmd_memread(int argc, char *argv[]);
int cmd_memwrite(int argc, char *argv[]);
int cmd_dw(int argc, char *argv[]);
int cmd_ew(int argc, char *argv[]);
int cmd_hwreset(int argc, char *argv[]);
int cmd_eqreset(int argc, char *argv[]);
int cmd_probephyaddr(int argc, char *argv[]);
int cmd_phyaddr(int argc, char *argv[]);
int cmd_testphyaddr(int argc, char *argv[]);
int cmd_dump_priv_data(int argc, char *argv[]);
int cmd_bootdownload(int argc, char *argv[]);
//int cmd_bootdownload2(int argc, char *argv[]);
int cmd_bootdownload3(int argc, char *argv[]);
int cmd_fwdownload_w_boot(int argc, char *argv[]);
int cmd_fwdownload_wo_boot(int argc, char *argv[]);
int cmd_fwreset(int argc, char *argv[]);
int cmd_testChecksumDone(int argc, char *argv[]);
int cmd_testChecksumOK(int argc, char *argv[]);
int cmd_testEthLink(int argc, char *argv[]);
int cmd_testEthPHYStatusChange(int argc, char *argv[]);
int cmd_testAllSwReady(int argc, char *argv[]);
int cmd_testPrevMsgFetch(int argc, char *argv[]);
int cmd_testNewMsgComing(int argc, char *argv[]);
int cmd_testNeedBootCode(int argc, char *argv[]);
int cmd_testPhyRegPollTime(int argc, char *argv[]);
int cmd_checkInterruptEvent(int argc, char *argv[]);
int cmd_start2RecvEvent(int argc, char *argv[]);
int cmd_stop2RecvEvent(int argc, char *argv[]);
int cmd_checkAssociatedInfo(int argc, char *argv[]);

int cmd_bulkmemwrite(int argc, char *argv[]);
int cmd_bulkmemread(int argc, char *argv[]);

int cmd_getcmd(int argc, char *argv[]);
int cmd_setcmd(int argc, char *argv[]);
int cmd_nfbiloop(int argc, char *argv[]);
int cmd_tx_cmdword_interval(int argc, char *argv[]);
int cmd_interrupt_timeout(int argc, char *argv[]);
int cmd_handshake_polling(int argc, char *argv[]);

int cmd_syncmiimac(int argc, char *argv[]);
int cmd_etherloop(int argc, char *argv[]);
int cmd_wlanloop(int argc, char *argv[]);
int cmd_gen_pauseframe(int argc, char *argv[]);
int cmd_gen_packet(int argc, char *argv[]);
int cmd_dot3InPauseFrames(int argc, char *argv[]);
int cmd_resetMibCounters(int argc, char *argv[]);
int cmd_compare(int argc, char *argv[]);
int cmd_testBootcodeReady(int argc, char *argv[]);
int cmd_sitesurvey(int argc, char *argv[]);
int cmd_getScanResult(int argc, char *argv[]);
int cmd_checkScanResult(int argc, char *argv[]);
int cmd_checkAP(int argc, char *argv[]);

int cmd_dram_type(int argc, char *argv[])
{
    if (argc==3) {
    	/*
        if (strcmp(argv[2], "DDR")==0) {
            create_ddr_tmp_file();
            printf("Suppose that DRAM type of device is DDR\n");
        }
        else {
            unlink(DDR_TMP_FILE_NAME);
            printf("Suppose that DRAM type of device is SDR\n");
        }
        */
    }
    else {
	//if (check_ddr_tmp_file())
	if (get_dram_type()==1) //DDR1
		printf("DRAM type of device: DDR1\n");
	else if (get_dram_type()==2) //DDR2
	        printf("DRAM type of device: DDR2\n");
	else
		printf("DRAM type of device: SDR\n");
    }
	return 0;
}

struct nfbi_cmd_table_t nfbi_cmd_table[]=
{
    {"version",         0, cmd_version,        "nfbi version"},
    {"97fwversion",     0, cmd_97fwversion,    "nfbi 97fwversion"},
    {"createfile",      0, cmd_createfile,     "nfbi createfile /tmp/ap1_mac 001122334455"},
    {"hwreset",         0, cmd_hwreset,        "nfbi hwreset"},
    {"eqreset",         0, cmd_eqreset,        "nfbi eqreset"},
    {"probephyaddr",    1, cmd_probephyaddr,   "nfbi probephyaddr"},
    {"phyaddr",         1, cmd_phyaddr,        "nfbi phyaddr 8"},
    {"testphyaddr",     0, cmd_testphyaddr,    "nfbi testphyaddr"},
	{"regread",         0, cmd_regread,        "nfbi regread SCR 0xf0f0 or nfbi regread SCR"},
	{"regwrite",        0, cmd_regwrite,       "nfbi regwrite SCR 0xff00 0x1234 or nfbi regwrite SCR 0x1234"},
	{"wait4bit",        0, cmd_wait4bit,        "nfbi wait4bit BMSR 5 1 10"},
	{"phyreset",        0, cmd_phyreset,        "nfbi phyreset"},
	{"restart_auto_nego", 0, cmd_restart_auto_nego, "nfbi restart_auto_nego"},
	{"getcmd",          0, cmd_getcmd,         "nfbi getcmd cmd_timeout"},
	{"setcmd",          0, cmd_setcmd,         "nfbi setcmd cmd_timeout 1"},
	{"nfbiloop",        0, cmd_nfbiloop,       "nfbi nfbiloop"},
	{"etherloop",       0, cmd_etherloop,       "nfbi etherloop"},
	{"wlanloop",        0, cmd_wlanloop,        "nfbi wlanloop ap1"},
    {"syncmiimac",      0, cmd_syncmiimac,      "nfbi syncmiimac"},
    {"tx_cmdword_interval",  0, cmd_tx_cmdword_interval, "nfbi tx_cmdword_interval 0"},
    {"interrupt_timeout",  1, cmd_interrupt_timeout, "nfbi interrupt_timeout 10"},
    {"cmd_handshake_polling",  1, cmd_handshake_polling, "nfbi cmd_handshake_polling 1"},
    {"bootdownload",    1, cmd_bootdownload,   "nfbi bootdownload 0 /usr/rtl8197b/boot.bin"},
    //{"bootdownload2",   1, cmd_bootdownload2,  "nfbi bootdownload2 0x00500000 /usr/rtl8197b/boot.img"},
    {"bootdownload3",    1, cmd_bootdownload3,   "nfbi bootdownload3 /usr/rtl8197b/boot.bin"},
    {"fwdownload_w_boot",  0, cmd_fwdownload_w_boot,     "nfbi fwdownload_w_boot 0 /usr/rtl8197b/linux.bin /usr/rtl8197b/boot.bin"},
    {"fwdownload_wo_boot", 0, cmd_fwdownload_wo_boot,    "nfbi fwdownload_wo_boot 0 /usr/rtl8197b/linux.bin "},
    {"fwreset",         0, cmd_fwreset,        "nfbi fwreset 0 /usr/rtl8197b/linux.bin /usr/rtl8197b/boot.bin"},
    {"testChecksumDone",   0, cmd_testChecksumDone, "nfbi testChecksumDone 1 0 /usr/rtl8197b/linux.bin /usr/rtl8197b/boot.bin"},
    {"testChecksumOK",   0, cmd_testChecksumOK, "nfbi testChecksumOK 1 0 /usr/rtl8197b/linux.bin /usr/rtl8197b/boot.bin"},
    {"genpause",   0, cmd_gen_pauseframe, "nfbi genpause 5 5"},
    {"genpkt",   0, cmd_gen_packet, "nfbi genpkt 5 000000000011 001e37d2ce13 2 64 10 0"},
    {"dot3InPauseFrames",   0, cmd_dot3InPauseFrames, "nfbi dot3InPauseFrames 5"},
    {"resetMibCounters",   0, cmd_resetMibCounters, "nfbi resetMibCounters"},
    {"compare",     0, cmd_compare,     "nfbi compare /tmp/A1 /tmp/A2"},
    {"testBootcodeReady",   0, cmd_testBootcodeReady, "nfbi testBootcodeReady 1 0 /usr/rtl8197b/boot.bin"},
    {"testEthLink",   0, cmd_testEthLink, "nfbi testEthLink 1"},
    {"testEthPHYStatusChange", 0, cmd_testEthPHYStatusChange, "nfbi testEthPHYStatusChange 1"},
    {"testAllSwReady",   0, cmd_testAllSwReady, "nfbi testAllSwReady 1 0 /usr/rtl8197b/linux.bin /usr/rtl8197b/boot.bin"},
    {"testPrevMsgFetch",   0, cmd_testPrevMsgFetch, "nfbi testPrevMsgFetch 1"},
    {"testNewMsgComing",   0, cmd_testNewMsgComing, "nfbi testNewMsgComing 1"},
    {"testNeedBootCode",   0, cmd_testNeedBootCode, "nfbi testNeedBootCode 1"},
	{"testPhyRegPollTime", 0, cmd_testPhyRegPollTime, "nfbi testPhyRegPollTime"},
	{"checkInterruptEvent", 0, cmd_checkInterruptEvent, "nfbi checkInterruptEvent 12 1"},
	{"start2RecvEvent", 0, cmd_start2RecvEvent, "nfbi start2RecvEvent"},
	{"stop2RecvEvent", 0, cmd_stop2RecvEvent, "nfbi stop2RecvEvent"},
	{"checkAssociatedInfo", 0, cmd_checkAssociatedInfo, "nfbi checkAssociatedInfo 10 TestAP 1"},
	{"getScanResult",     1, cmd_getScanResult, "nfbi getScanResult"},
	{"checkScanResult",   0, cmd_checkScanResult, "nfbi checkScanResult TestAP 1"},
	{"checkAP",     1, cmd_checkAP, "nfbi checkAP /tmp/scanresult TestAP 1"},
	
	{"meminit",         1, cmd_meminit,        "nfbi meminit"},
	{"memread",         1, cmd_memread,        "nfbi memread 0x00008000"},
	{"memwrite",        1, cmd_memwrite,       "nfbi memwrite 0x00008000 0x1234"},
    {"membulkwrite",    1, cmd_bulkmemwrite,   "nfbi membulkwrite 0x00008000 0x1234567890"},
    {"membulkread",     1, cmd_bulkmemread,    "nfbi membulkread 0x00008000 12"},
    {"dumppriv",        1, cmd_dump_priv_data, "nfbi dumppriv"},
    {"dumpregs",        1, cmd_dump_regs,      "nfbi dumpregs"},
    {"dumpeq",          1, cmd_dump_eq,      "nfbi dumpeq"},
 	{"dw",              1, cmd_dw,        "nfbi dw 0x80008000"},
	{"ew",              1, cmd_ew,        "nfbi ew 0x80008000 0x1234"},
	{"sitesurvey",      1, cmd_sitesurvey, "nfbi sitesurvey"},
    //{"dramtype",        1, cmd_dram_type, "nfbi dramtype DDR"},
    {"dramtype",        1, cmd_dram_type, "nfbi dramtype"},
    {NULL, 0, NULL, NULL},
};

void print_command_list(void)
{
    int i;

    printf("\n==========Commands for testing============\n");
    i = 0;
    while (nfbi_cmd_table[i].cmd != NULL) {
         if (!nfbi_cmd_table[i].debug)
            printf("%s\n", nfbi_cmd_table[i].msg);
        i++;
	}
	
    i = 0;
    printf("\n==========Commands for debugging==========\n");
    while (nfbi_cmd_table[i].cmd != NULL) {
         if (nfbi_cmd_table[i].debug)
            printf("%s\n", nfbi_cmd_table[i].msg);
        i++;
	}
}

void print_regwrite_list(void)
{
    int i;
    
    i = 0;
    while (nfbi_reg_table[i].name != NULL) {
        printf("nfbi regwrite %s 0x%04x\n", nfbi_reg_table[i].name, nfbi_reg_table[i].dval);
        i++;
	}
}

void print_regread_list(void)
{
    int i;
    
    i = 0;
    while (nfbi_reg_table[i].name != NULL) {
        printf("nfbi regread %s\n", nfbi_reg_table[i].name);
        i++;
	}
}

void print_setcmd_list(void)
{
    int i;
    
    i = 0;
    while (nfbi_cmdframe_table[i].name != NULL) {
        if ((nfbi_cmdframe_table[i].mode==0) || (nfbi_cmdframe_table[i].mode==2)) {
            if (nfbi_cmdframe_table[i].dlen > 0)
                printf("nfbi setcmd %s %s\n", nfbi_cmdframe_table[i].name, nfbi_cmdframe_table[i].defval);
            else
                printf("nfbi setcmd %s\n", nfbi_cmdframe_table[i].name);
        }
        i++;
	}
}

void print_getcmd_list(void)
{
    int i;
    
    i = 0;
    while (nfbi_cmdframe_table[i].name != NULL) {
        if ((nfbi_cmdframe_table[i].mode==0) || (nfbi_cmdframe_table[i].mode==1))
            printf("nfbi getcmd %s\n", nfbi_cmdframe_table[i].name);
        i++;
	}
}

int regname2val(char *name, struct reg_table_t *table)
{
    int i;
    
    if (0 == strncmp(name, "0x", 2)) {
        i = _atoi(name+2 ,16);
        return i;
    }

    i = 0;
    while (table[i].name != NULL) {
        if (0 == strcmp(name, table[i].name))
            return table[i].val;
        i++;
    }
    return -1;
}

int cmdname2index(char *name, struct cmdframe_table_t *table)
{
    int i, val;
    
    if (0 == strncmp(name, "0x", 2)) {
        val = _atoi(name+2 ,16);
		i = 0;
	    while (table[i].name != NULL) {
	        if (val == table[i].val)
	            return i;
	        i++;
	    }
        return -1;
    }

    i = 0;
    while (table[i].name != NULL) {
        if (0 == strcmp(name, table[i].name))
            return i;
        i++;
    }
    return -1;
}

int cmd_dump_regs(int argc, char *argv[])
{
    int i;
    int param;
    int val;

    nfbi_register_read(NFBI_REG_PHYID2, &val);
    i = 0;
    printf("Name   Val    Default\n");
    printf("---------------------\n");
    while (nfbi_reg_table[i].name != NULL) {
        if (val == NFBI_REG_PHYID2_DEFAULT2) {
            if ((strcmp(nfbi_reg_table[i].name, "DCH")==0) ||
                (strcmp(nfbi_reg_table[i].name, "DCL")==0) ||
                (strcmp(nfbi_reg_table[i].name, "DTH")==0) ||
                (strcmp(nfbi_reg_table[i].name, "DTL")==0)) {
                i++;
                continue;
            }
        }
        param = nfbi_reg_table[i].val << 16; //put register address to high word
    	if (0 == ioctl(nfbi_fd, NFBI_IOCTL_REGREAD, &param))
    	    printf("%6s 0x%04x 0x%04x\n", nfbi_reg_table[i].name, param&0xffff, nfbi_reg_table[i].dval);
    	else
            printf("%6s  Fail  0x%04x\n", nfbi_reg_table[i].name, nfbi_reg_table[i].dval);
        i++;
	}
    dump_misc("cmd_dump_regs");
    return 0;
}

/*
 * Two kinds of command formats supported:
 * 1) nfbi regread <name> <mask>
 * 2) nfbi regread <name> (i.e.  nfbi regread <name> 0xffff)
 *
 * Retrun value:
 * 0   : the result was printed on console by the function itself
 * !=0 : the caller should be in charge of printing the result on console
 *       >0 stands for "OK",  <0 stands for "FAIL".
 */
int cmd_regread(int argc, char *argv[])
{
    int reg;
    int param, mask;

    if (argc < 3) {
        print_regread_list();
        return 0;
    }
    
    if (argc==3)
        //nfbi regread <name>
        mask = 0xffff;
    else {
        //nfbi regread <name> <mask>
        // parsing <mask>
        if (0 == strncmp(argv[3], "0x", 2)) {
            mask = _atoi(argv[3]+2 ,16);
            mask &= 0xffff;
        }
        else
            return -1; //wrong command format 
    }
    // parsing <name>
    reg = regname2val(argv[2], nfbi_reg_table);
    if (reg < 0) {
        return -1; //wrong command format
    }
    
    param = reg << 16; //put register address to high word
	if (0 == ioctl(nfbi_fd, NFBI_IOCTL_REGREAD, &param)) {
	    printf("0x%04x\n", param&mask);
	    return 0;
	}
	else
	    return -1;
}

/*
 * Two kinds of command formats supported:
 * 1) nfbi regwrite <name> <mask> <value>
 * 2) nfbi regwrite <name> <value> (i.e.  nfbi regread <name> 0xffff <value>)
 *
 * Retrun value:
 * 0   : the result was printed on console by the function itself
 * !=0 : the caller should be in charge of printing the result on console
 *       >0 stands for "OK",  <0 stands for "FAIL".
 */
int cmd_regwrite(int argc, char *argv[])
{
    int reg, val, index;
    int param, mask;

    if (argc < 4) {
        print_regwrite_list();
        return 0;
    }
    
    if (argc==4) {
        //nfbi regwrite <name> <value>
        index = 3; //fourth argument is <value>
        mask = 0xffff;
    }
    else {
        //nfbi regwrite <name> <mask> <value>
        index = 4; //fifth argument is <value>
        // parsing <mask>
        if (0 == strncmp(argv[3], "0x", 2)) {
            mask = _atoi(argv[3]+2 ,16);
            mask &= 0xffff;
        }
        else
            return -1; //wrong command format
    }
    // parsing <name>
    reg = regname2val(argv[2], nfbi_reg_table);
    if (reg < 0) {
        return -1; //wrong command format
    }
    // parsing <value>
    if (0 == strncmp(argv[index], "0x", 2)) {
        val = _atoi(argv[index]+2 ,16);
        val &= 0xffff;
    }
    else {
        return -1; //wrong command format
    }

    // read register first for the 1st kind of command
    if (mask != 0xffff) {
        param = reg << 16; //put register address to high word
    	if (0 == ioctl(nfbi_fd, NFBI_IOCTL_REGREAD, &param)) {
    	    val = ((param&0xffff)&(~mask)) | (val&mask);
    	}
    	else
    	    return -1;
    }

    //put register address to high word and the value to low word
    if (reg == NFBI_REG_BMCR)
        val &= 0x7dff; //filter out reset bit and restart auto nego bit
    param = (reg << 16) | (val & 0xffff);
	if (0 == ioctl(nfbi_fd, NFBI_IOCTL_REGWRITE, &param))
	    return 1;
	else
	    return -1;
}

int cmd_wait4bit(int argc, char *argv[])
{
    int reg, wait4val, shift, timeout;
    int param;

    if (argc != 6)
        goto arg_error;
   
    // parsing <name>
    reg = regname2val(argv[2], nfbi_reg_table);
    if (reg < 0) {
        return -1; //wrong command format
    }
    
    shift = atoi(argv[3]);
    
    wait4val = atoi(argv[4]);
    if ((wait4val != 0) && (wait4val != 1))
        goto arg_error;
    
    timeout = atoi(argv[5]);
        
    //check if the value of the bit we are waiting for is equal to the one we expected.
    while (1) {
        param = reg << 16; //put register address to high word
    	if (0 != ioctl(nfbi_fd, NFBI_IOCTL_REGREAD, &param))
    	    return -1;
        if (wait4val == (((param&0xffff) >> shift) & 0x0001))
            return 1;
        if (timeout <= 0)
            break;
        real_sleep(1);
        timeout--;
    }
    printf("%s=0x%04x\n", argv[2], param&0xffff);
    return -1;
    
arg_error:
    printf("nfbi wait4bit <register name> <bit no. 0~15> <0|1> <timeout>\n");
    return 0;    
}

int cmd_phyreset(int argc, char *argv[])
{
    int param, val, count;

    // read BMCR
    param = 0x00 << 16; //put register address to high word
    if (0 != ioctl(nfbi_fd, NFBI_IOCTL_REGREAD, &param))
        return -1;
    val = param&0xffff;

    //do phy reset
    //1. set reset bit to 0
    param = (0x00 << 16) | (val & 0x7fff);
	if (0 != ioctl(nfbi_fd, NFBI_IOCTL_REGWRITE, &param))
	    return -1;
    
    real_sleep(1);
    // read BMCR
    param = 0x00 << 16; //put register address to high word
    if (0 != ioctl(nfbi_fd, NFBI_IOCTL_REGREAD, &param))
        return -1;
    val = param&0xffff;
    
    //2. set reset bit to 1
    param = (0x00 << 16) | (val | 0x8000);
	if (0 != ioctl(nfbi_fd, NFBI_IOCTL_REGWRITE, &param))
	    return -1;
    
    //check if phy reset is self-clearing
    count = 0;
    do {
        real_sleep(1);
        count++;
        // read BMCR
        param = 0x00 << 16; //put register address to high word
        if (0 != ioctl(nfbi_fd, NFBI_IOCTL_REGREAD, &param))
            return -1;
        val = param&0xffff;
    } while ((val&0x8000) && (count<3));
    
    if (count >= 3) {
        printf("can't do phy reset");
        return -1;
    }
    
    //3. write BMCR to default
    param = (0x00 << 16) | 0x3000;
	if (0 != ioctl(nfbi_fd, NFBI_IOCTL_REGWRITE, &param))
	    return -1;
	
    //4. write ANAR to default
    param = (0x04 << 16) | 0x0de1;
	if (0 != ioctl(nfbi_fd, NFBI_IOCTL_REGWRITE, &param))
	    return -1;
    return 1;
}

int cmd_restart_auto_nego(int argc, char *argv[])
{
    int param, val, count;

    // read BMCR
    param = 0x00 << 16; //put register address to high word
    if (0 != ioctl(nfbi_fd, NFBI_IOCTL_REGREAD, &param))
        return -1;
    val = param&0xffff;

    //restart auto_nego
    //1. set restart_auto_nego bit to 0
    param = (0x00 << 16) | (val & 0xfdff);
	if (0 != ioctl(nfbi_fd, NFBI_IOCTL_REGWRITE, &param))
	    return -1;
    
    real_sleep(1);
    // read BMCR
    param = 0x00 << 16; //put register address to high word
    if (0 != ioctl(nfbi_fd, NFBI_IOCTL_REGREAD, &param))
        return -1;
    val = param&0xffff;
    
    //2. set restart_auto_nego bit to 1
    param = (0x00 << 16) | (val | 0x0200);
	if (0 != ioctl(nfbi_fd, NFBI_IOCTL_REGWRITE, &param))
	    return -1;
    
    //check if restart_auto_nego is self-clearing
    count = 0;
    do {
        real_sleep(1);
        count++;
        // read BMCR
        param = 0x00 << 16; //put register address to high word
        if (0 != ioctl(nfbi_fd, NFBI_IOCTL_REGREAD, &param))
            return -1;
        val = param&0xffff;
    } while ((val&0x0200) && (count<3));
    
    if (count >= 3) {
        printf("can't restart auto nego");
        return -1;
    }
    
    //3. write BMCR back
    param = (0x00 << 16) | val;
	if (0 != ioctl(nfbi_fd, NFBI_IOCTL_REGWRITE, &param))
	    return -1;
	
    return 1;
}

/*
 * Retrun value:
 * 0   : the result was printed on console by the function itself
 * !=0 : the caller should be in charge of printing the result on console
 *       >0 stands for "OK",  <0 stands for "FAIL".
 */
int cmd_meminit(int argc, char *argv[])
{
    if (argc != 2)
        return -1;      //wrong command format 

    if (0 == do_dram_settings())
	    return 1;
	else
	    return -1;
}

/*
 * Retrun value:
 * 0   : the result was printed on console by the function itself
 * !=0 : the caller should be in charge of printing the result on console
 *       >0 stands for "OK",  <0 stands for "FAIL".
 */
int cmd_memread(int argc, char *argv[])
{
    int addr, val;

    if (argc != 3)
        return -1;      //wrong command format 

    // parsing <addr>
    if (0 == strncmp(argv[2], "0x", 2))
        addr = _atoi(argv[2]+2 ,16);
    else
        return -1; //wrong command format

    if (0 == nfbi_mem32_read(addr, &val)) {
        printf("0x%08x\n", val);
	    return 0;
	}
	else
	    return -1;
}

/*
 * Retrun value:
 * 0   : the result was printed on console by the function itself
 * !=0 : the caller should be in charge of printing the result on console
 *       >0 stands for "OK",  <0 stands for "FAIL".
 */
int cmd_memwrite(int argc, char *argv[])
{
    int addr, val;

    if (argc != 4)
        return -1;      //wrong command format 

    // parsing <addr>
    if (0 == strncmp(argv[2], "0x", 2))
        addr = _atoi(argv[2]+2 ,16);
    else
        return -1; //wrong command format

    // parsing <value>
    if (0 == strncmp(argv[3], "0x", 2))
        val = _atoi(argv[3]+2 ,16);
    else
        return -1; //wrong command format

    if (0 == nfbi_mem32_write(addr, val))
	    return 1;
	else
	    return -1;
}

static int read_word(int addr, int *pval)
{
    struct nfbi_mem32_param param;
    
    param.addr = addr;
    if (0 == ioctl(nfbi_fd, NFBI_IOCTL_DW, &param)) {
        *pval = param.val;
	    return 0;
	}
	else
	    return -1;
}

static int write_word(int addr, int val)
{
    struct nfbi_mem32_param param;
    
    param.addr = addr;
    param.val = val;
    return ioctl(nfbi_fd, NFBI_IOCTL_EW, &param);
}

int cmd_dw(int argc, char *argv[])
{
    int addr, val;

    if (argc != 3)
        return -1;      //wrong command format 

    // parsing <addr>
    if (0 == strncmp(argv[2], "0x", 2))
        addr = _atoi(argv[2]+2 ,16);
    else
        return -1; //wrong command format

    if (0 == read_word(addr, &val)) {
        printf("0x%08x\n", val);
	    return 0;
	}
	else
	    return -1;
}

int cmd_ew(int argc, char *argv[])
{
    int addr, val;

    if (argc != 4)
        return -1;      //wrong command format 

    // parsing <addr>
    if (0 == strncmp(argv[2], "0x", 2))
        addr = _atoi(argv[2]+2 ,16);
    else
        return -1; //wrong command format

    // parsing <value>
    if (0 == strncmp(argv[3], "0x", 2))
        val = _atoi(argv[3]+2 ,16);
    else
        return -1; //wrong command format

    if (0 == write_word(addr, val))
	    return 1;
	else
	    return -1;
}

int cmd_bulkmemwrite(int argc, char *argv[])
{
    int addr;

    if (argc != 4)
        return -1;      //wrong command format 

    // parsing <addr>
    if (0 == strncmp(argv[2], "0x", 2))
        addr = _atoi(argv[2]+2 ,16);
    else
        return -1; //wrong command format

    if (0 == nfbi_bulk_mem_write(addr, strlen(argv[3]), argv[3]))
	    return 1;
	else
	    return -1;
}

int cmd_bulkmemread(int argc, char *argv[])
{
    int addr, i, len;
    char tmp[128];

    if (argc != 4)
        return -1;      //wrong command format 

    // parsing <addr>
    if (0 == strncmp(argv[2], "0x", 2))
        addr = _atoi(argv[2]+2 ,16);
    else
        return -1; //wrong command format
    
    len = atoi(argv[3]);
    if (0 == nfbi_bulk_mem_read(addr, len, tmp)) {
        printf("=================================\n");
        for (i=0; i< len; i++) {
	        printf("%c", tmp[i]);
	    }
	    printf("\n");
	    printf("=================================\n");
	    return 0;
	}
	else
	    return -1;
}

/*
 * Retrun value:
 * 0   : the result was printed on console by the function itself
 * !=0 : the caller should be in charge of printing the result on console
 *       >0 stands for "OK",  <0 stands for "FAIL".
 */
int cmd_hwreset(int argc, char *argv[])
{
    int param;
    
    if (argc > 2)
        param = atoi(argv[2]);
    else
        param = 2;
    if ((param != 0) && (param != 1))
        param = 2; //hardware reset

	if (0 == ioctl(nfbi_fd, NFBI_IOCTL_PRIV_CMD, &param))
	    return 1;
	else
	    return -1;
}

int cmd_eqreset(int argc, char *argv[])
{
    int param;
    
    param = 5; //event queue reset
	if (0 == ioctl(nfbi_fd, NFBI_IOCTL_PRIV_CMD, &param))
	    return 1;
	else
	    return -1;
}

/*
 * Retrun value:
 * 0   : the result was printed on console by the function itself
 * !=0 : the caller should be in charge of printing the result on console
 *       >0 stands for "OK",  <0 stands for "FAIL".
 */
int cmd_probephyaddr(int argc, char *argv[])
{
    int param;

    param = 3; //probe phyaddr
	ioctl(nfbi_fd, NFBI_IOCTL_PRIV_CMD, &param);
	return 0;
}

int cmd_phyaddr(int argc, char *argv[])
{
    int param;

    param = atoi(argv[2]);
   	if (0 == ioctl(nfbi_fd, NFBI_IOCTL_MDIO_PHYAD, &param))
        return 1;
    else
        return -1;
}

int cmd_testphyaddr(int argc, char *argv[])
{
    int param, i, found, addr[32], verbose;

    verbose = 0;
    if (argc > 2) {
        if (strcmp(argv[2], "-v")==0)
            verbose = 1;
    }
    
    found = 0;
    for (i=0; i<32; i++) {
    	ioctl(nfbi_fd, NFBI_IOCTL_MDIO_PHYAD, &i);
    	addr[i] = 0;
        param = NFBI_REG_PHYID2 << 16; //put register address to high word
    	if (0 == ioctl(nfbi_fd, NFBI_IOCTL_REGREAD, &param)) {
    	    param = param&0xffff;
    	    if (verbose)
    	        printf("Probe PHY ADDR=%d  reg3=0x%04x\n", i, param);
    	    if ((param==NFBI_REG_PHYID2_DEFAULT) ||
    	        (param==NFBI_REG_PHYID2_DEFAULT2)){
    	        found++;
    	        addr[i] = 1;
    	    }
    	}
    }

    //update the phy address to the driver
    i = 0xff;
    if (found == 1) {
        if (addr[8]==1)
            i = 8;
        else if (addr[16]==1)
            i = 16;
    }
    ioctl(nfbi_fd, NFBI_IOCTL_MDIO_PHYAD, &i);
    
    if ((found == 1) && ((addr[8]==1)||(addr[16]==1)))
        return 1;
    else
        return -1;
}

int cmd_dump_priv_data(int argc, char *argv[])
{
    int param;

    param = 4; //dump private data
	ioctl(nfbi_fd, NFBI_IOCTL_PRIV_CMD, &param);
	return 0;
}

int cmd_bootdownload(int argc, char *argv[])
{
    if (argc < 4)
        return -1;
    if (0 == bootcode_download(atoi(argv[2]), argv[3]))
        return 1;
    else
        return -1;
}
/*
int cmd_bootdownload2(int argc, char *argv[])
{
    int offset;
//    int ret, val;

    if (argc < 3)
        return -1;
    
    if (0 == strncmp(argv[2], "0x", 2))
        offset = _atoi(argv[2]+2 ,16);
    else
        return -1; //wrong command format
        
    if (0 == bootcode_download2(argv[3], offset))
        return 1;
    else
        return -1;
}
*/

int cmd_bootdownload3(int argc, char *argv[])
{
    if (argc < 3)
        return -1;
    if (0 == bootcode_download3(argv[2]))
        return 1;
    else
        return -1;
}

/*
 * Retrun value:
 * 0   : the result was printed on console by the function itself
 * !=0 : the caller should be in charge of printing the result on console
 *       >0 stands for "OK",  <0 stands for "FAIL".
 */
int cmd_fwdownload_w_boot(int argc, char *argv[])
{
    int retval;
    
    if (argc < 5)
        return -1;
    retval = firmware_download_w_boot(atoi(argv[2]), argv[3], argv[4]);
    if (0 == retval)
        return 1;
    else {
        printf("retval=%d\n", retval);
        return -1;
    }
}

int cmd_fwdownload_wo_boot(int argc, char *argv[])
{
    int retval;
    
    if (argc < 4)
        return -1;
    retval = firmware_download_wo_boot(atoi(argv[2]), argv[3]);
    if (0 == retval)
        return 1;
    else {
        printf("retval=%d\n", retval);
        return -1;
    }
}

//================================================================================
int nfbi_tx_cmd_frame_integer(char type, char cmd, char dlen, int data, int loopback)
{
	char buf[16];
	int num, count;
	
	if (loopback)
	    buf[0] = type | BIT(5);
	else
	    buf[0] = type;
	buf[1] = cmd;
	buf[2] = 0;
	buf[3] = dlen;
	num = 4;
	switch(dlen) {
	  case 1:
	  	buf[4] = 0;
		buf[5] = data & 0xff;
		num += 2;
	  	break;
	  case 2:
		buf[4] = 0;
		buf[5] = (data >> 8) & 0xff;
	  	buf[6] = 0;
		buf[7] = data & 0xff;
		num += 4;
	  	break;
	  case 4:
		buf[4] = 0;
		buf[5] = (data >> 24) & 0xff;
	  	buf[6] = 0;
		buf[7] = (data >> 16) & 0xff;
		buf[8] = 0;
		buf[9] = (data >> 8) & 0xff;
	  	buf[10] = 0;
		buf[11] = data & 0xff;
		num += 8;
	  	break;
	}

	//transmit the command frame
	count = write(nfbi_fd, buf, num);
	if (count != num) {
	   if (errno == ETIME) {
		    printf("write timeout\n");
		    //printf("errno=%d ETIME=%d\n", errno, ETIME);
	    }
	    else
	        printf("write error\n");
		return -1;
	}
	return 0;
}

int nfbi_rx_status_frame_integer(char type, char cmd, char dlen, int *data, int loopback)
{
	char buf[16];
	int num, count;
	//int i;
	
	if (!loopback)
	    *data = 0xff; //init the return code
	//receive the status frame
	count = read(nfbi_fd, buf, 16);
	if (count <= 0) {
		if (errno == ETIME) {
		    printf("read timeout\n");
		    //printf("errno=%d ETIME=%d\n", errno, ETIME);
	    }
	    else
	        printf("read error\n");
		return -1;
	}
	
	/*
	printf("###count=%d###\n", count);
	for(i=0;i<count;i++) {
	    printf("%02x ", buf[i]&0xff);
    }
    printf("\n##########\n");
    */
    num = -1;
	switch(buf[3]) {
	  case 1:
		num = buf[5];
	  	break;
	  case 2:
	    num = ((buf[5] << 8)&0xff00) | (buf[7]&0xff);
	  	break;
	  case 4:
  	    num = ((buf[5]<<24)&0xff000000) | ((buf[7]<<16)&0xff0000) | ((buf[9]<<8)&0xff00) | (buf[11]&0xff);
	  	break;
	}
    
    if (loopback) {
        if (buf[0]!=(type|BIT(5))) {
            return -1;
        }
        if(buf[1]!=cmd) {
    		//printf("incorrect response(%02x) for the command(%02x)\n", buf[1], cmd);
    		return -1;
    	}
    	if ((buf[2]!=0) || (buf[3]!=dlen)) {
            //printf("incorrect data len\n");
            return -1;
        }
        if ((dlen > 0) && (num != *data)) {
            //printf("Loopback Fail\n");
            return -1;
        }
    }
    else {
        if(buf[1]!=cmd) {
    		//printf("incorrect response(%02x) for the command(%02x)\n", buf[1], cmd);
    		return -1;
    	}
    	if (buf[0] == (char)TAG_GOOD_STATUS) {
        	if ((buf[2]!=0) || (buf[3]!=dlen)) {
                //printf("incorrect data len\n");
                return -1;
            }
            if (dlen > 0)
                *data = num;
        }
        else {
            // the data len of return code is equal to 1
            if ((buf[2]==0) && (buf[3]==1) && (buf[4]==0)) {
                *data = buf[5];
            }
            else
                printf("incorrect data len2\n");
    		return -1;
    	}
    }
	return 0;
}

int nfbi_tx_cmd_frame_memory(char type, char cmd, int addr, int value)
{
	char buf[20];
	int num, count;
	
    buf[0] = type;
	buf[1] = cmd;
	buf[2] = 0;
	if (type == (char)TAG_SET_COMMAND) { //write_memory
        buf[3] = 8;
        buf[4] = 0;
		buf[5] = (addr >> 24) & 0xff;
	  	buf[6] = 0;
		buf[7] = (addr >> 16) & 0xff;
		buf[8] = 0;
		buf[9] = (addr >> 8) & 0xff;
	  	buf[10] = 0;
		buf[11] = addr & 0xff;
		buf[12] = 0;
		buf[13] = (value >> 24) & 0xff;
	  	buf[14] = 0;
		buf[15] = (value >> 16) & 0xff;
		buf[16] = 0;
		buf[17] = (value >> 8) & 0xff;
	  	buf[18] = 0;
		buf[19] = value & 0xff;
		num = 20;
	}
	else { //read_memory
        buf[3] = 4;
        buf[4] = 0;
		buf[5] = (addr >> 24) & 0xff;
	  	buf[6] = 0;
		buf[7] = (addr >> 16) & 0xff;
		buf[8] = 0;
		buf[9] = (addr >> 8) & 0xff;
	  	buf[10] = 0;
		buf[11] = addr & 0xff;
	    num = 12;
	}

	//transmit the command frame
	count = write(nfbi_fd, buf, num);
	if (count != num) {
	   if (errno == ETIME) {
		    printf("write timeout\n");
		    //printf("errno=%d ETIME=%d\n", errno, ETIME);
	    }
	    else
	        printf("write error\n");
		return -1;
	}
	return 0;
}

int nfbi_rx_status_frame_memory(char type, char cmd, int *value)
{
	char buf[16];
	int count;
	//int i;
	
	//receive the status frame
	count = read(nfbi_fd, buf, 16);
	if (count <= 0) {
		if (errno == ETIME) {
		    printf("read timeout\n");
		    //printf("errno=%d ETIME=%d\n", errno, ETIME);
	    }
	    else
	        printf("read error\n");
		return -1;
	}
	
    /*
	printf("###count=%d###\n", count);
	for(i=0;i<count;i++) {
	    printf("%02x ", buf[i]&0xff);
    }
    printf("\n##########\n");
    */
    if(buf[1]!=cmd) {
    	//printf("incorrect response(%02x) for the command(%02x)\n", buf[1], cmd);
    	return -1;
    }
    if (buf[0] == (char)TAG_GOOD_STATUS) {
        if (type == (char)TAG_GET_COMMAND) { //read_memory
            if ((buf[2]!=0) || (buf[3]!=4)) {
                //printf("incorrect data len\n");
                return -1;
            }
            *value = ((buf[5]<<24)&0xff000000) | ((buf[7]<<16)&0xff0000) | ((buf[9]<<8)&0xff00) | (buf[11]&0xff);
        }
        else {
            if (buf[2]!=0)
                return -1;
            if (buf[3]==1)
                *value = buf[5];
        }
    }
    else {
        // the data len of return code is equal to 1
        if ((buf[2]==0) && (buf[3]==1) && (buf[4]==0)) {
            *value = buf[5];
        }
        else
            printf("incorrect data len2\n");
    	return -1;
    }
	return 0;
}

int nfbi_tx_cmd_frame_mac(char type, char cmd, char *mac)
{
	char buf[20];
	int num, count;
	
    buf[0] = type;
	buf[1] = cmd;
	buf[2] = 0;
	if (type == (char)TAG_SET_COMMAND) {
        buf[3] = 6;
        buf[4] = 0;
		buf[5] = mac[0];
	  	buf[6] = 0;
		buf[7] = mac[1];
		buf[8] = 0;
		buf[9] = mac[2];
	  	buf[10] = 0;
		buf[11] = mac[3];
		buf[12] = 0;
		buf[13] = mac[4];
	  	buf[14] = 0;
		buf[15] = mac[5];
		num = 16;
	}
	else {
        buf[3] = 0;
	    num = 4;
	}

	//transmit the command frame
	count = write(nfbi_fd, buf, num);
	if (count != num) {
	   if (errno == ETIME) {
		    printf("write timeout\n");
		    //printf("errno=%d ETIME=%d\n", errno, ETIME);
	    }
	    else
	        printf("write error\n");
		return -1;
	}
	return 0;
}

int nfbi_rx_status_frame_mac(char type, char cmd, char *mac)
{
	char buf[20];
	int count;
	//int i;
	
	//receive the status frame
	count = read(nfbi_fd, buf, 20);
	if (count <= 0) {
		if (errno == ETIME) {
		    printf("read timeout\n");
		    //printf("errno=%d ETIME=%d\n", errno, ETIME);
	    }
	    else
	        printf("read error\n");
		return -1;
	}
	
	/*
	printf("###count=%d###\n", count);
	for(i=0;i<count;i++) {
	    printf("%02x ", buf[i]&0xff);
    }
    printf("\n##########\n");
    */

    if(buf[1]!=cmd) {
    	printf("incorrect response(%02x) for the command(%02x)\n", buf[1], cmd);
    	return -1;
    }
    if (buf[0] == (char)TAG_GOOD_STATUS) {
        if (type == (char)TAG_GET_COMMAND) {
            if ((buf[2]!=0) || (buf[3]!=6)) {
                printf("incorrect data len\n");
                return -1;
            }
            mac[0] = buf[5];
            mac[1] = buf[7];
            mac[2] = buf[9];
            mac[3] = buf[11];
            mac[4] = buf[13];
            mac[5] = buf[15];
        }
        else {
            if (buf[2]!=0) {
                printf("should be zero\n");
                return -1;
            }
            if (buf[3]==1)
                mac[0] = buf[5];
        }
    }
    else {
        // the data len of return code is equal to 1
        if ((buf[2]==0) && (buf[3]==1) && (buf[4]==0)) {
            mac[0] = buf[5];
            printf("return code=%d\n", buf[5]&0xff);
        }
        else
            printf("incorrect data len2\n");
    	return -1;
    }
	return 0;
}

int nfbi_tx_cmd_frame_data(char type, char cmd, char *data, int len)
{
	char *buf;
	int num, count, i;

    if (len > 255) {
        printf("len is too long\n");
		return -1;
	}
        
    num = 4 + 2*len;

    buf = (char *)malloc(num);
    if (buf == NULL) {
        printf("malloc fail\n");
		return -1;
	}

    buf[0] = type;
	buf[1] = cmd;
	buf[2] = 0;
    buf[3] = (char)(len&0xff);
    for (i=0; i<len; i++) {
        buf[4+2*i] = 0;
        buf[5+2*i] = data[i];
    }

	//transmit the command frame
	count = write(nfbi_fd, buf, num);
	if (count != num) {
	   if (errno == ETIME) {
		    printf("write timeout\n");
		    //printf("errno=%d ETIME=%d\n", errno, ETIME);
	    }
	    else
	        printf("write error\n");
		free(buf);
		return -1;
	}
	free(buf);
	return 0;
}

int nfbi_rx_status_frame_data(char type, char cmd, char *data, int *dlen)
{
	char buf[516]; //2 + 2 + 2*255 + 2
	int count, len, i;
	
    len = *dlen;
    *dlen = 0;
	//receive the status frame
	count = read(nfbi_fd, buf, 516);
	if (count <= 0) {
		if (errno == ETIME) {
		    printf("read timeout\n");
		    //printf("errno=%d ETIME=%d\n", errno, ETIME);
	    }
	    else
	        printf("read error\n");
		return -1;
	}
	
    /*
	printf("###count=%d###\n", count);
	for(i=0;i<count;i++) {
	    printf("%02x ", buf[i]&0xff);
    }
    printf("\n##########\n");
    */
    if(buf[1]!=cmd) {
    	printf("incorrect response(%02x) for the command(%02x)\n", buf[1], cmd);
    	return -1;
    }
    
    //check data length
    if (buf[2]!=0) {
        printf("should be zero\n");
        return -1;
    }
    if (((count-4)/2) != (int)(buf[3]&0xff)) {
        //incorrect data len
        printf("count=%d  buf[3]=%d\n", count, (buf[3]&0xff));
        return -1;
    }
    count = (count-4) / 2;
    if (buf[0] == (char)TAG_GOOD_STATUS) {
        if (type == (char)TAG_GET_COMMAND) {
            if (len < count) {
                printf("rx data buf too small\n");
                return -1;
            }
            for (i=0; i<count; i++) {
    		    data[i] = buf[5+2*i];
    	    }
    	    *dlen = count;
        }
        else { //TAG_SET_COMMAND
            // the data len of return code is equal to 1
            if ((buf[3]==1) && (buf[4]==0)) {
                data[0] = buf[5]; //return code
                printf("return code=%d\n", buf[5]&0xff);
            }
            else {
                printf("incorrect data len\n");
                return -1;
            }
        }
    }
    else {
        // the data len of return code is equal to 1
        if ((buf[3]==1) && (buf[4]==0)) {
            data[0] = buf[5];
            printf("return code2=%d\n", buf[5]&0xff);
        }
        else
            printf("incorrect data len2\n");
    	return -1;
    }
	return 0;
}

int cmd_common_command(char type, int index, int argc, char *argv[])
{
    int data, value;
    
    value = 0;
    if (type == (char)TAG_GET_COMMAND) {
        if (0==nfbi_tx_cmd_frame_integer(TAG_GET_COMMAND, nfbi_cmdframe_table[index].val, 0, 0, 0)) {
	        if (0==nfbi_rx_status_frame_integer(TAG_GET_COMMAND, nfbi_cmdframe_table[index].val, 
	                                            nfbi_cmdframe_table[index].dlen, &data, 0)) {
                printf("%d\n", data);
	            return 0;
	        }
	        else {
	            printf("Fail: return code=%d\n", data);
	        }
	    }
        return -1;
	}
	else {
	    if (nfbi_cmdframe_table[index].dlen > 0) {
    	    value = atoi(argv[3]);
    	    if ((value < nfbi_cmdframe_table[index].min_val) ||
    	        (value > nfbi_cmdframe_table[index].max_val))
    	        return -1;
        }
        if (0==nfbi_tx_cmd_frame_integer(TAG_SET_COMMAND, nfbi_cmdframe_table[index].val, 
                                            nfbi_cmdframe_table[index].dlen, value, 0)) {
	        if (0==nfbi_rx_status_frame_integer(TAG_SET_COMMAND, nfbi_cmdframe_table[index].val, 
	                                            1, &data, 0)) {
	            return 0;
	        }
	        else {
	            printf("Fail: return code=%d\n", data);
	        }
	    }
        return -1;
	}
}

int cmd_write_memory(char type, int index, int argc, char *argv[])
{
    int addr, value;
    
    if (argc < 5)
        return -1;
    if (type == (char)TAG_SET_COMMAND) {
        if (0 == strncmp(argv[3], "0x", 2))
            addr = _atoi(argv[3]+2 ,16);
        else
            addr = atoi(argv[3]);
        if (0 == strncmp(argv[4], "0x", 2))
            value = _atoi(argv[4]+2 ,16);
        else
            value = atoi(argv[4]);
        if (0==nfbi_tx_cmd_frame_memory(TAG_SET_COMMAND, nfbi_cmdframe_table[index].val, addr, value)) {
	        if (0==nfbi_rx_status_frame_memory(TAG_SET_COMMAND, nfbi_cmdframe_table[index].val, &value)) {
	            return 0;
	        }
	        else {
	            printf("Fail: return code=%d\n", value);
	        }
	    }
        return -1;
	}
	return -1;
}

int cmd_read_memory(char type, int index, int argc, char *argv[])
{
    int addr, value;
    
    if (argc < 4)
        return -1;
    if (type == (char)TAG_GET_COMMAND) {
        if (0 == strncmp(argv[3], "0x", 2))
            addr = _atoi(argv[3]+2 ,16);
        else
            addr = atoi(argv[3]);
        if (0==nfbi_tx_cmd_frame_memory(TAG_GET_COMMAND, nfbi_cmdframe_table[index].val, addr, value)) {
	        if (0==nfbi_rx_status_frame_memory(TAG_GET_COMMAND, nfbi_cmdframe_table[index].val, &value)) {
                printf("0x%08x\n", value);
	            return 0;
	        }
	        else {
	            printf("Fail: return code=%d\n", value);
	        }
	    }
        return -1;
	}
	return -1;
}

int cmd_mac_address(char type, int index, int argc, char *argv[])
{
    int i;
    char mac[6], tmp[4];
	
    if (type == (char)TAG_GET_COMMAND) {
        if (0==nfbi_tx_cmd_frame_mac(TAG_GET_COMMAND, nfbi_cmdframe_table[index].val, mac)) {
	        if (0==nfbi_rx_status_frame_mac(TAG_GET_COMMAND, nfbi_cmdframe_table[index].val, mac)) {
                printf("%02x%02x%02x%02x%02x%02x\n", mac[0]&0xff, mac[1]&0xff,
                                                     mac[2]&0xff, mac[3]&0xff,
                                                     mac[4]&0xff, mac[5]&0xff);
	            return 0;
	        }
	        else {
	            printf("Fail: return code=%d\n", mac[0]);
	        }
	    }
        return -1;
	}
	else {
	    if (argc < 4)
            return -1;
        for (i=0; i<6; i++) {
            strncpy(tmp, argv[3]+i*2, 2);
            tmp[2] = '\0'; 
            mac[i] = _atoi(tmp ,16);
        }
        if (0==nfbi_tx_cmd_frame_mac(TAG_SET_COMMAND, nfbi_cmdframe_table[index].val, mac)) {
	        if (0==nfbi_rx_status_frame_mac(TAG_SET_COMMAND, nfbi_cmdframe_table[index].val, mac)) {
	            return 0;
	        }
	        else {
	            printf("Fail: return code=%d\n", mac[0]);
	        }
	    }
        return -1;
	}
}

int cmd_data(char type, int index, int argc, char *argv[])
{
    int i, len;
    char data[256], tmp[4];
	
    if (type == (char)TAG_GET_COMMAND) {
        if (0==nfbi_tx_cmd_frame_data(TAG_GET_COMMAND, nfbi_cmdframe_table[index].val, data, 0)) {
            len = 256; //buffer size
	        if (0==nfbi_rx_status_frame_data(TAG_GET_COMMAND, nfbi_cmdframe_table[index].val, data, &len)) {
                printf("%02x%02x%02x%02x%02x%02x\n", data[0]&0xff, data[1]&0xff,
                                                     data[2]&0xff, data[3]&0xff,
                                                     data[4]&0xff, data[5]&0xff);
	            return 0;
	        }
	        else {
	            printf("Fail: return code=%d\n", data[0]);
	        }
	    }
        return -1;
	}
	else {
	    if (argc < 4)
            return -1;
        for (i=0; i<6; i++) {
            strncpy(tmp, argv[3]+i*2, 2);
            tmp[2] = '\0'; 
            data[i] = _atoi(tmp ,16);
        }
        if (0==nfbi_tx_cmd_frame_data(TAG_SET_COMMAND, nfbi_cmdframe_table[index].val, data, 6)) {
            len = 256; //buffer size
	        if (0==nfbi_rx_status_frame_data(TAG_SET_COMMAND, nfbi_cmdframe_table[index].val, data, &len)) {
	            return 0;
	        }
	        else {
	            printf("Fail: return code=%d\n", data[0]);
	        }
	    }
        return -1;
	}
}

int getwlaninfo(rtk_wireless_info *wlaninfo)
{
    int len, index;
    int tmp_endian;
    char *data;
   
    index = cmdname2index("get_wlan_info", nfbi_cmdframe_table);
    if (index < 0) {
        return -1; //unknown command
    }
    
    data = (char *)wlaninfo;
    if (0==nfbi_tx_cmd_frame_data(TAG_GET_COMMAND, nfbi_cmdframe_table[index].val, data, 0)) {
        len = sizeof(rtk_wireless_info); //buffer size
	    if (0==nfbi_rx_status_frame_data(TAG_GET_COMMAND, nfbi_cmdframe_table[index].val, data, &len)) {
	        // Convert big-endian data --> Host byte order
	        tmp_endian = wlaninfo->freq;
	        wlaninfo->freq = ntohl(tmp_endian);

	        tmp_endian = wlaninfo->link_quality;
	        wlaninfo->link_quality = ntohl(tmp_endian);

	        tmp_endian = wlaninfo->signal_level;
	        wlaninfo->signal_level = ntohl(tmp_endian);

	        tmp_endian = wlaninfo->noise_level;
	        wlaninfo->noise_level = ntohl(tmp_endian);

	        tmp_endian = wlaninfo->rx_invalid_cypt;
	        wlaninfo->rx_invalid_cypt = ntohl(tmp_endian);

	        tmp_endian = wlaninfo->rx_invalid_frag;
	        wlaninfo->rx_invalid_frag = ntohl(tmp_endian);

	        tmp_endian = wlaninfo->tx_excessive_retry;
	        wlaninfo->tx_excessive_retry = ntohl(tmp_endian);

	        tmp_endian = wlaninfo->invalid_misc;
	        wlaninfo->invalid_misc = ntohl(tmp_endian);

	        tmp_endian = wlaninfo->missed_beacon;
	        wlaninfo->missed_beacon = ntohl(tmp_endian);

	        return 0;
	    }
    }
    return -1;
}

int cmd_getwlaninfo(char type, int index, int argc, char *argv[])
{
    rtk_wireless_info wlaninfo;

    if (argc != 3)
        return -1;
    if (type == (char)TAG_GET_COMMAND) {
        if (0==getwlaninfo(&wlaninfo)) {
            printf("AP: %02X:%02X:%02X:%02X:%02X:%02X\n", wlaninfo.ap[0], wlaninfo.ap[1],
	                          wlaninfo.ap[2], wlaninfo.ap[3], wlaninfo.ap[4], wlaninfo.ap[5]);
            printf("SSID:\"%s\"\n", wlaninfo.ssid);
            printf("Channel:%d\n", wlaninfo.freq);
            printf("Link Quality:%d\n", wlaninfo.link_quality);
            printf("Signal Level:%d\n", wlaninfo.signal_level);
            printf("Noise Level:%d\n", wlaninfo.noise_level);
            printf("Rx invalid crypt:%d\n", wlaninfo.rx_invalid_cypt);
            printf("Rx invalid frag:%d\n", wlaninfo.rx_invalid_frag);
            printf("Tx excessive retries:%d\n", wlaninfo.tx_excessive_retry);
            printf("Invalid misc:%d\n", wlaninfo.invalid_misc);
            printf("Missed beacon:%d\n", wlaninfo.missed_beacon);
            return 0;
	    }
    }
    return -1;
}

void print_scan_result(scan_result *result)
{
    int i;
    /*
    printf("index=%d ", result->index);
    printf("number=%d ", result->number);
    printf("more=%d ", result->more);
    printf("pad=%d\n", result->pad);
    */
    if (result->number == 0xff) {
        printf("Scanning is in progress\n");
        return;
    }
    for (i=0; i<result->number; i++) {
        printf("%03d %3d %-32s ",result->index+i, result->bssdb[i].channel, result->bssdb[i].ssid);
        printf("%02x:%02x:%02x:%02x:%02x:%02x ", result->bssdb[i].bssid[0], result->bssdb[i].bssid[1],
                                                 result->bssdb[i].bssid[2], result->bssdb[i].bssid[3],
                                                 result->bssdb[i].bssid[4], result->bssdb[i].bssid[5]);
        if (result->bssdb[i].encrypt == 0)
            printf("Open     ");
        else if (result->bssdb[i].encrypt == 1)
            printf("WEP      ");
        else if (result->bssdb[i].encrypt == 2)
            printf("WPA      ");
        else if (result->bssdb[i].encrypt == 3)
            printf("WPA2     ");
        else if (result->bssdb[i].encrypt == 4)
            printf("WPA/WPA2 ");
        else
            printf("UNKNOW   ");
        printf("%3d ", result->bssdb[i].rssi);
    
        if (result->bssdb[i].type == 0)
            printf("Infra ");
        else
            printf("AdHoc ");
    
        if (result->bssdb[i].network&0x01)
            printf("11b");
        else
            printf("11");
        if (result->bssdb[i].network&0x02)
            printf("g");
        if (result->bssdb[i].network&0x04)
            printf("a");
        if (result->bssdb[i].network&0x08)
            printf("n");
        if (result->bssdb[i].network&0x10)
            printf("n");
        printf("\n");
    }
}

int request_scan(void)
{
    int data, index;
    
    index = cmdname2index("request_scan", nfbi_cmdframe_table);
    if (index < 0) {
        return -1; //unknown command
    }
    
    if (0==nfbi_tx_cmd_frame_integer(TAG_SET_COMMAND, nfbi_cmdframe_table[index].val, 
                                     nfbi_cmdframe_table[index].dlen, data, 0)) {
	    if (0==nfbi_rx_status_frame_integer(TAG_SET_COMMAND, nfbi_cmdframe_table[index].val, 
	                                            1, &data, 0)) {
            //printf("site survey start...\n");
            return 0;
        }
	    else {
            printf("return code=%d\n", data);
            if (data == 102) {
                printf("site survey is already in progress...\n");
                return 0;
            }
            else
                return -1;
	    }
	}
	return -1;
}

int get_scan_result(scan_result *result, int num)
{
    int index, len, i;
    char *data;
    unsigned short u16tmp_endian;
    
    index = cmdname2index("get_scan_result", nfbi_cmdframe_table);
    if (index < 0) {
        return -1; //unknown command
    }

    //len = 500; //in 10ms
    //ioctl(nfbi_fd, NFBI_IOCTL_RESPONSE_TIMEOUT, &len);

    data = (char *)result;
    data[0] = (char)(num & 0xff);
    if (0==nfbi_tx_cmd_frame_data(TAG_GET_COMMAND, nfbi_cmdframe_table[index].val, data, 1)) {
        len = sizeof(scan_result); //buffer size
        if (0==nfbi_rx_status_frame_data(TAG_GET_COMMAND, nfbi_cmdframe_table[index].val, data, &len)) {
	        // Convert big-endian data --> Host byte order
	        // There is only channel member in the scan_result structure;
	        for (i=0; i<( sizeof(result->bssdb)/sizeof(bss_info) ); i++) {
	            u16tmp_endian = result->bssdb[i].channel;
	            result->bssdb[i].channel = ntohs(u16tmp_endian);
	        }
	        return 0;
	    }
    }
    return -1;
}

int cmd_get_scan_result(char type, int index, int argc, char *argv[])
{
    int num;
    scan_result result;

    if (argc != 4) {
        printf("nfbi getcmd get_scan_result <0~255>\n");
        return -1;
    }
    num = atoi(argv[3]);
    if ((num < 0) || (num > 255)) {
        printf("nfbi getcmd get_scan_result <0~255>\n");
        return -1;
    }
    if (type == (char)TAG_GET_COMMAND) {
        if (0 == get_scan_result(&result, num)) {
            print_scan_result(&result);
	        return 0;
	    }
    }
    return -1;
}

int cmd_cfg(char type, int index, int argc, char *argv[])
{
    int len;
    char data[256];

    if (argc < 4)
        return -1;
    len = strlen(argv[3]);
    if (len > 255)
        return -1;	
    if (type == (char)TAG_GET_COMMAND) {
        if (0==nfbi_tx_cmd_frame_data(TAG_GET_COMMAND, nfbi_cmdframe_table[index].val, argv[3], len)) {
            len = 256; //buffer size
	        if (0==nfbi_rx_status_frame_data(TAG_GET_COMMAND, nfbi_cmdframe_table[index].val, data, &len)) {
	            data[len] = '\0';
                printf("%s\n", data);
	            return 0;
	        }
	        /*else {
	            printf("Fail: return code=%d\n", data[0]);
	        }*/
	    }
        return -1;
	}
	else {
        if (0==nfbi_tx_cmd_frame_data(TAG_SET_COMMAND, nfbi_cmdframe_table[index].val, argv[3], len)) {
            len = 256; //buffer size
	        if (0==nfbi_rx_status_frame_data(TAG_SET_COMMAND, nfbi_cmdframe_table[index].val, data, &len)) {
	            return 0;
	        }
	        /*else {
	            printf("Fail: return code=%d\n", data[0]);
	        }*/
	    }
        return -1;
	}
}

int cmd_sitesurvey(int argc, char *argv[])
{
    int num;
    scan_result result;
    
    if (0==request_scan()) {
        real_sleep(5);
        num = 0;
        while(0 == get_scan_result(&result, num)) {
            if (result.number == 0xff) {
                printf("site survey is still in progress...\n");
                real_sleep(3);
                num = 0;
            }
            else {
                print_scan_result(&result);
                if (result.more == 0)
                    break;
                num =  result.index + result.number;
            }
        }
        return 0;
	}
	return -1;
}

int cmd_getScanResult(int argc, char *argv[])
{
    int num;
    scan_result result;
    
    num = 0;
    while(0 == get_scan_result(&result, num)) {
        if (result.number == 0xff) {
            //printf("site survey is still in progress...\n");
            real_sleep(3);
            num = 0;
        }
        else {
            print_scan_result(&result);
            if (result.more == 0)
                break;
            num =  result.index + result.number;
        }
    }
    return 0;
}

int cmd_checkScanResult(int argc, char *argv[])
{
    int num, i;
    scan_result result;
    unsigned short channel;
    
    if (argc < 4)
        return -1;
    
    channel = (unsigned short)(atoi(argv[3]) & 0xffff);
    num = 0;
    while(0 == get_scan_result(&result, num)) {
        if (result.number == 0xff) {
            printf("site survey is still in progress...\n");
            real_sleep(3);
            num = 0;
        }
        else {
            //print_scan_result(&result);
            for (i=0; i<result.number; i++) {
                if ((channel == result.bssdb[i].channel) &&
                    (strcmp(result.bssdb[i].ssid, argv[2])==0))
                    return 1;
            }

            if (result.more == 0)
                break;
            num =  result.index + result.number;
        }
        printf("num=%d\n", num);
    }
    return -1;
}

int cmd_checkAP(int argc, char *argv[])
{
    FILE *fp = NULL;
    char s1[128], s2[128];
    int found;

    if (argc < 5)
        return -1;
        
    if (!(fp = fopen(argv[2], "r"))) {
        perror(argv[2]);
        return -1;
    }
    found = 0;
    sprintf(s2, "%3d %-32s", atoi(argv[4]), argv[3]);
    while (fgets(s1, sizeof(s1), fp)) {
        if (strstr(s1, s2)) {
            found = 1;
            break;
        }
    }
    fclose(fp);
    if (found)
        return 1;
    else
        return -1;
}

//nfbi setcmd <name> <value>
int cmd_setcmd(int argc, char *argv[])
{
    int index, ret;
    int data;

    if (argc < 3) {
        print_setcmd_list();
        return 0;
    }
    // parsing <name>
    index = cmdname2index(argv[2], nfbi_cmdframe_table);
    if (index < 0) {
        return -1; //unknown command
    }

    data = nfbi_cmdframe_table[index].retransmit_count;
    ioctl(nfbi_fd, NFBI_IOCTL_RETRANSMIT_COUNT, &data);
    data = nfbi_cmdframe_table[index].response_timeout; //in 10ms
    ioctl(nfbi_fd, NFBI_IOCTL_RESPONSE_TIMEOUT, &data);

    ret = -1;
    if ((nfbi_cmdframe_table[index].mode==0) || (nfbi_cmdframe_table[index].mode==2))
        ret = nfbi_cmdframe_table[index].func(TAG_SET_COMMAND, index, argc, argv);
	if (0 == ret) {
	    return 1;
	}
	else
	    return -1;
}

int cmd_getcmd(int argc, char *argv[])
{
    int index, ret;
    int data;

    if (argc < 3) {
        print_getcmd_list();
        return 0;
    }
    // parsing <name>
    index = cmdname2index(argv[2], nfbi_cmdframe_table);
    if (index < 0) {
        return -1; //unknown command
    }

    data = nfbi_cmdframe_table[index].retransmit_count;
    ioctl(nfbi_fd, NFBI_IOCTL_RETRANSMIT_COUNT, &data);
    data = nfbi_cmdframe_table[index].response_timeout; //in 10ms
    ioctl(nfbi_fd, NFBI_IOCTL_RESPONSE_TIMEOUT, &data);
    
    ret = -1;
    if ((nfbi_cmdframe_table[index].mode==0) || (nfbi_cmdframe_table[index].mode==1))
        ret = nfbi_cmdframe_table[index].func(TAG_GET_COMMAND, index, argc, argv);
	if (0 == ret) {
	    return 0;
	}
	else
	    return -1;
}

int nfbiloop(void)
{
    int data;
    /*
    data = 0;
    ioctl(nfbi_fd, NFBI_IOCTL_RETRANSMIT_COUNT, &data);
    data = NFBI_RESPONSE_TIMEOUT_DEFAULT;
    ioctl(nfbi_fd, NFBI_IOCTL_RESPONSE_TIMEOUT, &data);
    */
    data = 1;
    if (0==nfbi_tx_cmd_frame_integer(TAG_SET_COMMAND, nfbi_cmdframe_table[0].val, 
                                        nfbi_cmdframe_table[0].dlen, data, 1)) {
	    if (0==nfbi_rx_status_frame_integer(TAG_SET_COMMAND, nfbi_cmdframe_table[0].val, 
	                                    nfbi_cmdframe_table[0].dlen, &data, 1))
	        return 0;
	    else
	        return -1;
	}
	else
	    return -1;
}

int cmd_nfbiloop(int argc, char *argv[])
{
    if (argc != 2)
        printf("usage: nfbi nfbiloop\n");

    if (0==nfbiloop())
	    return 1;
	else
	    return -1;
}

int get_netif_mac_addr(char *ifname, char *macaddr)
{
    struct ifreq ifr;
    int sockfd = 0;
    int i;
    
    memset(macaddr, '\0', 6);
    /*open socket*/
	sockfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
	if (sockfd == -1) {
		perror("socket():");
        return -1;
	}
	
    bzero(&ifr, sizeof(ifr));
		
	/*retrieve interface index*/
	strncpy(ifr.ifr_name, ifname, IFNAMSIZ);
	if (ioctl(sockfd, SIOCGIFINDEX, &ifr) == -1) {
		//perror("SIOCGIFINDEX");
		close(sockfd);
		return -1;
	}
    //printf("Successfully got interface index: %i\n", ifr.ifr_ifindex);

	/*retrieve corresponding MAC*/
	if (ioctl(sockfd, SIOCGIFHWADDR, &ifr) == -1) {
		perror("SIOCGIFINDEX");
		close(sockfd);
		return -1;
	}
    for (i = 0; i < 6; i++) {
		macaddr[i] = ifr.ifr_hwaddr.sa_data[i];
	}
    //printf("Successfully got our MAC address: %02X:%02X:%02X:%02X:%02X:%02X\n", 
	//		macaddr[0],macaddr[1],macaddr[2],macaddr[3],macaddr[4],macaddr[5]);
	close(sockfd);
	return 0;
}

int cmd_syncmiimac(int argc, char *argv[])
{
    char mac[6], tmp[64], mac_org[6];
    int index;
    /*
    int data;
	struct timeval begin;
    struct timeval end;
    struct timeval result;
    gettimeofday(&begin,NULL);
    */
    index = cmdname2index("wlan_mac_addr", nfbi_cmdframe_table);
    if (index < 0) {
        return -1; //unknown command
    }

    /*
    data = nfbi_cmdframe_table[index].retransmit_count;
    ioctl(nfbi_fd, NFBI_IOCTL_RETRANSMIT_COUNT, &data);
    data = nfbi_cmdframe_table[index].response_timeout; //in 10ms
    ioctl(nfbi_fd, NFBI_IOCTL_RESPONSE_TIMEOUT, &data);
    */
    
    if (0==nfbi_tx_cmd_frame_mac(TAG_GET_COMMAND, nfbi_cmdframe_table[index].val, mac)) {
	    if (0==nfbi_rx_status_frame_mac(TAG_GET_COMMAND, nfbi_cmdframe_table[index].val, mac)) {
	        /*
	        gettimeofday(&end,NULL);
	        timersub(&end,&begin,&result);
	        printf("spent %ld seconds %ld microseconds\n",result.tv_sec, result.tv_usec);
	        */
	        //check if mac is all zero
	        if ((mac[0]|mac[1]|mac[2]|mac[3]|mac[4]|mac[5])&0xff) {
	            /*================================================================*/
	            //@Pana_TBD, interface name might be customized for different platform
#ifndef NFBI_HOST_IS_PANABOARD
	            get_netif_mac_addr("eth0", mac_org);
	            if (memcmp(mac_org, mac, 6)) {
        	        system("ifconfig eth0 down");
                    sprintf(tmp, "ifconfig eth0 hw ether %02x%02x%02x%02x%02x%02x",
                                    mac[0]&0xff, mac[1]&0xff, mac[2]&0xff, 
                                    mac[3]&0xff, mac[4]&0xff, mac[5]&0xff);
        	        system(tmp);
        	        system("ifconfig eth0 up");
    	        }
#else
	            get_netif_mac_addr("eth1", mac_org);
	            if (memcmp(mac_org, mac, 6)) {
        	        system("ifconfig eth1 down");
                    sprintf(tmp, "ifconfig eth1 hw ether %02x%02x%02x%02x%02x%02x",
                                    mac[0]&0xff, mac[1]&0xff, mac[2]&0xff, 
                                    mac[3]&0xff, mac[4]&0xff, mac[5]&0xff);
        	        system(tmp);
        	        system("ifconfig eth1 up");
    	        }
#endif
    	        /*================================================================*/
    	        return 1;
    	    }
    	    printf("%02x%02x%02x%02x%02x%02x\n",
                                    mac[0]&0xff, mac[1]&0xff, mac[2]&0xff, 
                                    mac[3]&0xff, mac[4]&0xff, mac[5]&0xff);
    	    printf("mac is all zero\n");
    	    return -1;
        }
        else {
            printf("fail on nfbi_rx_status_frame_mac\n");
            return -1;
        }
    }
	/*
	gettimeofday(&end,NULL);
	timersub(&end,&begin,&result);
	printf("spent %ld seconds %ld microseconds\n",result.tv_sec, result.tv_usec);
	*/        	        
    printf("fail on nfbi_tx_cmd_frame_mac\n");
    return -1;
}

int cmd_etherloop(int argc, char *argv[])
{
#ifndef NFBI_HOST_IS_PANABOARD
    system("ifconfig eth0 down");
    real_sleep(1);
    system("ifconfig eth0 up");
    real_sleep(1);
#else
    system("ifconfig eth1 down");
    real_sleep(1);
    system("ifconfig eth1 up");
    real_sleep(1);
#endif
    system("pktgen 1 1 ffffffffffff 50 10 10");
    
    return 0;
}

int cmd_wlanloop(int argc, char *argv[])
{
    char filename[16], buf[16], tmp[64];
    FILE *fp = NULL;
    int len;
    
    if ((argc != 3) && (argc != 4)){
        printf("nfbi wlanloop <ap1|ap2>\n");
        return 0;
    }

    if (strcmp(argv[2], "ap1")==0)
        strcpy(filename, "/tmp/ap1_mac");
    else if (strcmp(argv[2], "ap2")==0)
        strcpy(filename, "/tmp/ap2_mac");
    else {
        printf("nfbi wlanloop <ap1|ap2>\n");
        return 0;
    }
    
    len = 50;
    if (argc == 4) {
        len = atoi(argv[3]);
        if (len < 50)
            len = 50;
    }
        
    /* Read AP's MAC */
    if (!(fp = fopen(filename, "r"))) {
        perror(filename);
        return -1;
    }

    fgets(buf, sizeof(buf), fp);
    fclose(fp);
    
    //check if mac is all zero
    buf[12]='\0';
	if (strcmp(buf, "000000000000")!= 0) {
#ifndef NFBI_HOST_IS_PANABOARD
        system("ifconfig eth0 down");
        real_sleep(1);
        system("ifconfig eth0 up");
        real_sleep(1);
#else
        system("ifconfig eth1 down");
        real_sleep(1);
        system("ifconfig eth1 up");
        real_sleep(1);
#endif
        sprintf(tmp, "pktgen 1 2 %s %d 10 1", buf, len);
    	system(tmp);
        return 0;
    }
    return -1;
}

int cmd_tx_cmdword_interval(int argc, char *argv[])
{
    int param;
    
    if (argc > 2) {
        param = atoi(argv[2]);
        if ((param<0) || (param>1000))
            return -1;
        param |= 0x10000; //set
        if (0 == ioctl(nfbi_fd, NFBI_IOCTL_TX_CMDWORD_INTERVAL, &param))
	        return 1;
	}
	else {
	    param = 0x0; //get
	    if (0 == ioctl(nfbi_fd, NFBI_IOCTL_TX_CMDWORD_INTERVAL, &param)) {
	        printf("%d\n", param);
	        return 0;
	    }
	}
	return -1;
}

int cmd_interrupt_timeout(int argc, char *argv[])
{
    int param;
    
    if (argc > 2) {
        param = atoi(argv[2]);
        if ((param<0) || (param>1000))
            return -1;
        param |= 0x10000; //set
        if (0 == ioctl(nfbi_fd, NFBI_IOCTL_INTERRUPT_TIMEOUT, &param))
	        return 1;
	}
	else {
	    param = 0x0; //get
	    if (0 == ioctl(nfbi_fd, NFBI_IOCTL_INTERRUPT_TIMEOUT, &param)) {
	        printf("%d\n", param);
	        return 0;
	    }
	}
	return -1;
}

int cmd_handshake_polling(int argc, char *argv[])
{
    int param;
    
    if (argc > 2) {
        param = atoi(argv[2]);
        if (param != 0)
            param = 1;
        param |= 0x10000; //set
        if (0 == ioctl(nfbi_fd, NFBI_IOCTL_CMD_HANDSHAKE_POLLING, &param))
	        return 1;
	}
	else {
	    param = 0x0; //get
	    if (0 == ioctl(nfbi_fd, NFBI_IOCTL_CMD_HANDSHAKE_POLLING, &param)) {
	        printf("%d\n", param);
	        return 0;
	    }
	}
	return -1;
}

static void event_handler(int sig_no)
{
    got_sigusr1 = 1;
}

int cmd_dump_eq(int argc, char *argv[])
{
    int ret;
    struct evt_msg evt;
    
    ret = nfbi_get_event_msg(&evt);
    while ((ret==0) && (evt.id==1)) {
        printf("evt.id=%d evt.value=%x evt.status=%x\n", evt.id, evt.value, evt.status);
        if (evt.status & IP_CHECKSUM_DONE) {
            if (evt.value & BM_CHECKSUM_DONE)
                printf("Checksum Done: 1\n");
            else
                printf("Checksum Done: 0\n");
        }
        if (evt.status & IP_CHECKSUM_OK) {
            if (evt.value & BM_CHECKSUM_OK)
                printf("Checksum OK: 1\n");
            else
                printf("Checksum OK: 0\n");
        }
        if (evt.status & IP_WLANLINK) {
            if (evt.value & BM_WLANLINK)
                printf("WLAN link: up\n");
            else
                printf("WLAN link: down\n");
        }
        if (evt.status & IP_ETHLINK) {
            if (evt.value & BM_ETHLINK)
                printf("Eth link: up\n");
            else
                printf("Eth link: down\n");
        }
        if (evt.status & IP_ETHPHY_STATUS_CHANGE) {
            if (evt.value & BM_ETHPHY_STATUS_CHANGE)
                printf("EthPhy status: change\n");
            else
                printf("EthPhy status: unchange\n");
        }
        if (evt.status & IP_ALLSOFTWARE_READY) {
            if (evt.value & BM_ALLSOFTWARE_READY)
                printf("All Software: ready\n");
            else
                printf("All Software: not ready\n");
        }
        if (evt.status & IP_NEED_BOOTCODE) {
            printf("Need Bootcode\n");
        }
        ret = nfbi_get_event_msg(&evt);
    }
    return 0;
}

int testChecksumDone(int mask_enable, int verify, char *fw_filename, char *boot_filename)
{
    int ret, val, ret2, timeout;
    char temp[256];
    struct evt_msg evt;

    //download bootcode to memory
    ret = bootcode_download(verify, boot_filename);
    if (ret != 0) {
        ret2 = -1;
        goto err;
    }
    
    //enable ChecksumDone
    if (mask_enable) {
        ret = nfbi_register_mask_write(NFBI_REG_IMR, IM_CHECKSUM_DONE, IM_CHECKSUM_DONE);
        if (ret != 0) {
            ret2 = -2;
            goto err;
        }
    }

    //register signal event handler to receive interrupt notification
    got_sigusr1 = 0;
    signal(SIGUSR1, event_handler);
    //register pid to driver for interrupt notification
    nfbi_set_hcd_pid(getpid());
    eqreset();
    	
	//upload the firmware to 8197B by TFTP
	sprintf(temp, "tftp -p 192.168.1.97 -l %s", fw_filename);
	system(temp);
	
	//wait for bootcode to calculate the checksum of firmware
	//real_sleep(5);

    //check if ChecksumDone bit of SYSSR is equal to 1
    /*if (0 != nfbi_register_read(NFBI_REG_SYSSR, &val)) {
        ret2 = -3;
        goto err;
    }*/

	timeout = 10;
	while (timeout > 0) {
	    real_sleep(1);
        if (0 != nfbi_register_read(NFBI_REG_SYSSR, &val)) {
            printf("fail to read SYSSR\n");
            timeout--;
            continue;
        }
        if (val&BM_CHECKSUM_DONE)
            break;
        timeout--;
    }
    printf("SYSSR=0x%04x timeout=%d\n", val, timeout);
    if (timeout <= 0) {
        printf("timeout\n");
        ret2 = -3;
        goto err;
    }
    if (!(val&BM_CHECKSUM_DONE)) {
        ret2 = -4;
        goto err;
    }
    real_sleep(1);
    if (got_sigusr1) {
        ret = nfbi_get_event_msg(&evt);
        printf("%s line %d: evt.id=%d evt.value=%x evt.status=%x ret=%d\n", __FUNCTION__, __LINE__, evt.id, evt.value, evt.status, ret);
        if ((ret==0) && (evt.id==1)) {
            if (evt.status & IP_CHECKSUM_DONE) {
                /*
                if (evt.value & BM_CHECKSUM_DONE)
                    printf("Checksum Done: 1\n");
                else
                    printf("Checksum Done: 0\n");
                */
                if (!mask_enable) {
                    ret2 = -5;
                    goto err;
                }
            }
            else {
                ret2 = -6;
                printf("interrupt, but not ChecksumDone\n");
                goto err;
            }
        }
        else {
            if (mask_enable) {
                ret2 = -7;
                goto err;
            }
        }
    }
    else {
        if (mask_enable) {
            ret2 = -8;
            goto err;
        }
    }
    
    if (0 != nfbi_register_read(NFBI_REG_ISR, &val)) {
        ret2 = -9;
        goto err;
    }
    printf("ISR=0x%04x\n", val);
    if (val&IP_CHECKSUM_DONE) {
        if (mask_enable) {
            //we had written 1 to the CheckSumDoneIP bit of ISR at the interrupt service routine
            //Just check again if the bit is still equal to 1.  If yes, it is 'Fail'
            ret2 = -10;
            goto err;
        }
    }
    else {
        if (!mask_enable) {
            //CheckSumDoneIP bit of ISR should be equal to 1
            ret2 = -11;
            goto err;
        }
    }

    //Write 1 to clear the CheckSumDoneIP bit of ISR 
    //and then check again if the bit is equal to 1.  If yes, it is 'Fail'
    ret = nfbi_register_write(NFBI_REG_ISR, IP_CHECKSUM_DONE);
    if (ret != 0) {
        ret2 = -12;
        goto err;
    }
    real_sleep(1);
    if (0 != nfbi_register_read(NFBI_REG_ISR, &val)) {
        ret2 = -13;
        goto err;
    }
    printf("ISR=0x%04x\n", val);
    if (val&IP_CHECKSUM_DONE) {
        ret2 = -14;
        goto err;
    }
    
    //Check again if CheckSumDone interrupt happens or not
    //If the interrupt continue, it is 'Fail'. (interrupt clear check)
    if (got_sigusr1) {
        ret = nfbi_get_event_msg(&evt);
        printf("%s line %d: evt.id=%d evt.value=%x evt.status=%x ret=%d\n", __FUNCTION__, __LINE__, evt.id, evt.value, evt.status, ret);
        if ((ret==0) && (evt.id==1)) {
            if (evt.status & IP_CHECKSUM_DONE) {
                /*
                if (evt.value & BM_CHECKSUM_DONE)
                    printf("Checksum Done: 1\n");
                else
                    printf("Checksum Done: 0\n");
                */
                ret2 = -15;
                goto err;
            }
        }
    }
    ret2 = 0;

err:
    nfbi_register_mask_write(NFBI_REG_IMR, IM_CHECKSUM_DONE, 0x0);
    got_sigusr1 = 0;
    nfbi_set_hcd_pid(-1); //unregister
    eqreset();

    return ret2;
}

int cmd_testChecksumDone(int argc, char *argv[])
{
    int retval;

    if (argc < 6)
        return -1;
    retval = testChecksumDone(atoi(argv[2]), atoi(argv[3]), argv[4], argv[5]);
    if (0 == retval)
        return 1;
    else {
        printf("retval=%d\n", retval);
        return -1;
    }
}

int testChecksumOK(int mask_enable, int verify, char *fw_filename, char *boot_filename)
{
    int ret, val, ret2, timeout;
    char temp[256];
    struct evt_msg evt;

    //download bootcode to memory
    ret = bootcode_download(verify, boot_filename);
    if (ret != 0) {
        ret2 = -1;
        goto err;
    }
        
    //enable ChecksumOK
    if (mask_enable) {
        ret = nfbi_register_mask_write(NFBI_REG_IMR, IM_CHECKSUM_OK, IM_CHECKSUM_OK);
        if (ret != 0) {
            ret2 = -2;
            goto err;
        }
    }

    //register signal event handler to receive interrupt notification
    got_sigusr1 = 0;
    signal(SIGUSR1, event_handler);
    //register pid to driver for interrupt notification
    nfbi_set_hcd_pid(getpid());
    eqreset();
    
	//upload the firmware to 8197B by TFTP
	sprintf(temp, "tftp -p 192.168.1.97 -l %s", fw_filename);
	system(temp);
	
	//wait for bootcode to calculate the checksum of firmware
	//real_sleep(5);
	
    //check if ChecksumOK bit of SYSSR is equal to 1
    /*if (0 != nfbi_register_read(NFBI_REG_SYSSR, &val)) {
        ret2 = -3;
        goto err;
    }*/
    
    timeout = 10;
	while (timeout > 0) {
	    real_sleep(1);
        if (0 != nfbi_register_read(NFBI_REG_SYSSR, &val)) {
            printf("fail to read SYSSR\n");
            timeout--;
            continue;
        }
        if (val&BM_CHECKSUM_OK)
            break;
        timeout--;
    }
    printf("SYSSR=0x%04x timeout=%d\n", val, timeout);
    if (timeout <= 0) {
        printf("timeout\n");
        ret2 = -3;
        goto err;
    }
    if (!(val&BM_CHECKSUM_OK)) {
        ret2 = -4;
        goto err;
    }
    real_sleep(1);
    if (got_sigusr1) {
        ret = nfbi_get_event_msg(&evt);
        printf("%s line %d: evt.id=%d evt.value=%x evt.status=%x ret=%d\n", __FUNCTION__, __LINE__, evt.id, evt.value, evt.status, ret);
        if ((ret==0) && (evt.id==1)) {
            if (evt.status & IP_CHECKSUM_OK) {
                /*
                if (evt.value & BM_CHECKSUM_OK)
                    printf("ChecksumOK: 1\n");
                else
                    printf("ChecksumOK: 0\n");
                */
                if (!mask_enable) {
                    ret2 = -5;
                    goto err;
                }
            }
            else {
                ret2 = -6;
                printf("interrupt, but not ChecksumOK\n");
                goto err;
            }
        }
        else {
            if (mask_enable) {
                ret2 = -7;
                goto err;
            }
        }
    }
    else {
        if (mask_enable) {
            ret2 = -8;
            goto err;
        }
    }
    
    if (0 != nfbi_register_read(NFBI_REG_ISR, &val)) {
        ret2 = -9;
        goto err;
    }
    printf("ISR=0x%04x\n", val);
    if (val&IP_CHECKSUM_OK) {
        if (mask_enable) {
            //we had written 1 to the CheckSumOKIP bit of ISR at the interrupt service routine
            //Just check again if the bit is still equal to 1.  If yes, it is 'Fail'
            ret2 = -10;
            goto err;
        }
    }
    else {
        if (!mask_enable) {
            //CheckSumOKIP bit of ISR should be equal to 1
            ret2 = -11;
            goto err;
        }
    }

    //Write 1 to clear the CheckSumOKIP bit of ISR 
    //and then check again if the bit is equal to 1.  If yes, it is 'Fail'
    ret = nfbi_register_write(NFBI_REG_ISR, IP_CHECKSUM_OK);
    if (ret != 0) {
        ret2 = -12;
        goto err;
    }
    real_sleep(1);
    if (0 != nfbi_register_read(NFBI_REG_ISR, &val)) {
        ret2 = -13;
        goto err;
    }
    printf("ISR=0x%04x\n", val);
    if (val&IP_CHECKSUM_OK) {
        ret2 = -14;
        goto err;
    }
    
    //Check again if CheckSumOK interrupt happens or not
    //If the interrupt continue, it is 'Fail'. (interrupt clear check)
    if (got_sigusr1) {
        ret = nfbi_get_event_msg(&evt);
        printf("%s line %d: evt.id=%d evt.value=%x evt.status=%x ret=%d\n", __FUNCTION__, __LINE__, evt.id, evt.value, evt.status, ret);
        if ((ret==0) && (evt.id==1)) {
            if (evt.status & IP_CHECKSUM_OK) {
                /*
                if (evt.value & BM_CHECKSUM_OK)
                    printf("ChecksumOK: 1\n");
                else
                    printf("ChecksumOK: 0\n");
                */
                ret2 = -15;
                goto err;
            }
        }
    }
    ret2 = 0;

err:
    nfbi_register_mask_write(NFBI_REG_IMR, IM_CHECKSUM_OK, 0x0);
    got_sigusr1 = 0;
    nfbi_set_hcd_pid(-1); //unregister
    eqreset();

    return ret2;
}

int cmd_testChecksumOK(int argc, char *argv[])
{
    int retval;

    if (argc < 6)
        return -1;
    retval = testChecksumOK(atoi(argv[2]), atoi(argv[3]), argv[4], argv[5]);
    if (0 == retval)
        return 1;
    else {
        printf("retval=%d\n", retval);
        return -1;
    }
}

int testAllSwReady(int mask_enable, int verify, char *fw_filename, char *boot_filename)
{
    int ret, val, ret2, timeout;
    char temp[256];
    struct evt_msg evt;

    //download bootcode to memory
    ret = bootcode_download(verify, boot_filename);
    if (ret != 0) {
        ret2 = -1;
        goto err;
    }
        
    //enable AllSwReady
    if (mask_enable) {
        ret = nfbi_register_mask_write(NFBI_REG_IMR, IM_ALLSOFTWARE_READY, IM_ALLSOFTWARE_READY);
        if (ret != 0) {
            ret2 = -2;
            goto err;
        }
    }

    //register signal event handler to receive interrupt notification
    got_sigusr1 = 0;
    signal(SIGUSR1, event_handler);
    //register pid to driver for interrupt notification
    nfbi_set_hcd_pid(getpid());
    eqreset();
            	
	//upload the firmware to 8197B by TFTP
	sprintf(temp, "tftp -p 192.168.1.97 -l %s", fw_filename);
	system(temp);
	
	//wait for bootcode to calculate the checksum of firmware
	//real_sleep(5);
	
    //check if ChecksumOK & ChecksumDone bit of SYSSR is equal to 1
    /*if (0 != nfbi_register_read(NFBI_REG_SYSSR, &val)) {
        ret2 = -3;
        goto err;
    }*/
    
    timeout = 10;
	while (timeout > 0) {
	    real_sleep(1);
        if (0 != nfbi_register_read(NFBI_REG_SYSSR, &val)) {
            printf("fail to read SYSSR\n");
            timeout--;
            continue;
        }
        if (val&BM_ALLSOFTWARE_READY)
            break;
        timeout--;
    }
    printf("SYSSR=0x%04x timeout=%d\n", val, timeout);
    if (timeout <= 0) {
        printf("timeout\n");
        ret2 = -3;
        goto err;
    }
    if (!(val&BM_ALLSOFTWARE_READY)) {
        ret2 = -4;
        goto err;
    }
	real_sleep(1);
    if (got_sigusr1) {
        ret = nfbi_get_event_msg(&evt);
        printf("%s line %d: evt.id=%d evt.value=%x evt.status=%x ret=%d\n", __FUNCTION__, __LINE__, evt.id, evt.value, evt.status, ret);
        if ((ret==0) && (evt.id==1)) {
            if (evt.status & IP_ALLSOFTWARE_READY) {
                /*
                if (evt.value & BM_ALLSOFTWARE_READY)
                    printf("AllSwReady: 1\n");
                else
                    printf("AllSwReady: 0\n");
                */
                if (!mask_enable) {
                    ret2 = -5;
                    goto err;
                }
            }
            else {
                ret2 = -6;
                printf("interrupt, but not AllSwReady\n");
                goto err;
            }
        }
        else {
            if (mask_enable) {
                ret2 = -7;
                goto err;
            }
        }
    }
    else {
        if (mask_enable) {
            ret2 = -8;
            goto err;
        }
    }

    if (0 != nfbi_register_read(NFBI_REG_ISR, &val)) {
        ret2 = -10;
        goto err;
    }
    printf("ISR=0x%04x\n", val);
    if (val&IP_ALLSOFTWARE_READY) {
        if (mask_enable) {
            //we had written 1 to the AllSwReady bit of ISR at the interrupt service routine
            //Just check again if the bit is still equal to 1.  If yes, it is 'Fail'
            ret2 = -11;
            goto err;
        }
    }
    else {
        if (!mask_enable) {
            //AllSwReady bit of ISR should be equal to 1
            ret2 = -12;
            goto err;
        }
    }

    //Write 1 to clear the AllSwReady bit of ISR 
    //and then check again if the bit is equal to 1.  If yes, it is 'Fail'
    ret = nfbi_register_write(NFBI_REG_ISR, IP_ALLSOFTWARE_READY);
    if (ret != 0) {
        ret2 = -13;
        goto err;
    }
    real_sleep(1);
    if (0 != nfbi_register_read(NFBI_REG_ISR, &val)) {
        ret2 = -14;
        goto err;
    }
    printf("ISR=0x%04x\n", val);
    if (val&IP_ALLSOFTWARE_READY) {
        ret2 = -15;
        goto err;
    }
    
    //Check again if AllSwReady interrupt happens or not
    //If the interrupt continue, it is 'Fail'. (interrupt clear check)
    if (got_sigusr1) {
        ret = nfbi_get_event_msg(&evt);
        printf("%s line %d: evt.id=%d evt.value=%x evt.status=%x ret=%d\n", __FUNCTION__, __LINE__, evt.id, evt.value, evt.status, ret);
        if ((ret==0) && (evt.id==1)) {
            if (evt.status & IP_ALLSOFTWARE_READY) {
                /*
                if (evt.value & BM_ALLSOFTWARE_READY)
                    printf("AllSwReady: 1\n");
                else
                    printf("AllSwReady: 0\n");
                */
                ret2 = -16;
                goto err;
            }
        }
    }
    ret2 = 0;

err:
    nfbi_register_mask_write(NFBI_REG_IMR, IM_ALLSOFTWARE_READY, 0x0);
    got_sigusr1 = 0;
    nfbi_set_hcd_pid(-1); //unregister
    eqreset();

    return ret2;
}

int cmd_testAllSwReady(int argc, char *argv[])
{
    int retval;
    
    if (argc < 6)
        return -1;
    retval = testAllSwReady(atoi(argv[2]), atoi(argv[3]), argv[4], argv[5]);
    if (0 == retval)
        return 1;
    else {
        printf("retval=%d\n", retval);
        return -1;
    }
}

int testNeedBootCode(int mask_enable)
{
    int ret, val, ret2;
    struct evt_msg evt;
    
    ret2 = -1;

    ret = nfbi_register_write(NFBI_REG_IMR, 0x0);
    if (ret != 0)
        return -1;

    //clear all bits of ISR
    ret = nfbi_register_write(NFBI_REG_ISR, 0xffff);
    if (ret != 0)
        return -2;
   
    //enable NeedBootCode
    if (mask_enable) {
        ret = nfbi_register_mask_write(NFBI_REG_IMR, IM_NEED_BOOTCODE, IM_NEED_BOOTCODE);
        if (ret != 0) {
            ret2 = -3;
            goto err;
        }
    }

    //register signal event handler to receive interrupt notification
    got_sigusr1 = 0;
    signal(SIGUSR1, event_handler);
    //register pid to driver for interrupt notification
    nfbi_set_hcd_pid(getpid());
    eqreset();
    
    //write 1 to SystemRst bit to set the 8197B
    ret = nfbi_register_write(NFBI_REG_CMD, BM_SYSTEMRST);
    if (ret != 0) {
        ret2 = -4;
        goto err;
    }

    /* To make sure hardware reset finish,
     * check if BM_SYSTEMRST bit automatically retun to zero
     */
    do {
        ret = nfbi_register_read(NFBI_REG_CMD, &val);
        if (ret != 0) {
            ret2 = -5;
            goto err;
        }
    } while( (val & BM_SYSTEMRST) == BM_SYSTEMRST );
    
	real_sleep(1);

    //Check if NeedBootCode interrupt happens or not
    if (got_sigusr1) {
        ret = nfbi_get_event_msg(&evt);
        printf("%s line %d: evt.id=%d evt.value=%x evt.status=%x ret=%d\n", __FUNCTION__, __LINE__, evt.id, evt.value, evt.status, ret);
        if ((ret==0) && (evt.id==1)) {
            if (evt.status & IP_NEED_BOOTCODE) {
                if (!mask_enable) {
                    ret2 = -6;
                    goto err;
                }
            }
            else {
                ret2 = -7;
                printf("interrupt, but not ChecksumDone\n");
                goto err;
            }
        }
        else {
            if (mask_enable) {
                ret2 = -8;
                goto err;
            }
        }
    }
    else {
        if (mask_enable) {
            ret2 = -9;
            goto err;
        }
    }

    if (0 != nfbi_register_read(NFBI_REG_ISR, &val)) {
        ret2 = -10;
        goto err;
    }
    if (val&IP_NEED_BOOTCODE) {
        if (mask_enable) {
            //we had written 1 to the CheckSumDoneIP bit of ISR at the interrupt service routine
            //Just check again if the bit is still equal to 1.  If yes, it is 'Fail'
            ret2 = -11;
            goto err;
        }
    }
    else {
        if (!mask_enable) {
            //CheckSumDoneIP bit of ISR should be equal to 1
            ret2 = -12;
            goto err;
        }
    }

    //Write 1 to clear the NeedBootCode bit of ISR 
    //and then check again if the bit is equal to 1.  If yes, it is 'Fail'
    ret = nfbi_register_write(NFBI_REG_ISR, IP_NEED_BOOTCODE);
    if (ret != 0) {
        ret2 = -13;
        goto err;
    }
    real_sleep(1);
    if (0 != nfbi_register_read(NFBI_REG_ISR, &val)) {
        ret2 = -14;
        goto err;
    }
    if (val&IP_NEED_BOOTCODE) {
        ret2 = -15;
        goto err;
    }

    //Check again if IP_NEED_BOOTCODE interrupt happens or not
    //If the interrupt continue, it is 'Fail'. (interrupt clear check)
    if (got_sigusr1) {
        ret = nfbi_get_event_msg(&evt);
        printf("%s line %d: evt.id=%d evt.value=%x evt.status=%x ret=%d\n", __FUNCTION__, __LINE__, evt.id, evt.value, evt.status, ret);
        if ((ret==0) && (evt.id==1)) {
            if (evt.status & IP_NEED_BOOTCODE) {
                ret2 = -16;
                goto err;
            }
        }
    }
    ret2 = 0;

err:
    nfbi_register_mask_write(NFBI_REG_IMR, IM_CHECKSUM_DONE, 0x0);
    got_sigusr1 = 0;
    nfbi_set_hcd_pid(-1); //unregister
    eqreset();

    return ret2;
}

int cmd_testNeedBootCode(int argc, char *argv[])
{
    int retval;
    
    if (argc < 3)
        return -1;
    retval = testNeedBootCode(atoi(argv[2]));
    if (0 == retval)
        return 1;
    else {
        printf("retval=%d\n", retval);        
        return -1;
    }
}

int testPrevMsgFetch(int mask_enable)
{
    int ret, val, ret2;
    struct evt_msg evt;
    
    ret2 = -1;

    ret = nfbi_register_write(NFBI_REG_IMR, 0x0);
    if (ret != 0)
        return -1;

    //clear all bits of ISR
    ret = nfbi_register_write(NFBI_REG_ISR, 0xffff);
    if (ret != 0)
        return -2;
   
    //enable PrevMsgFetch
    if (mask_enable) {
        ret = nfbi_register_mask_write(NFBI_REG_IMR, IM_PREVMSG_FETCH, IM_PREVMSG_FETCH);
        if (ret != 0) {
            ret2 = -3;
            goto err;
        }
    }

    //register signal event handler to receive interrupt notification
    got_sigusr1 = 0;
    signal(SIGUSR1, event_handler);
    //register pid to driver for interrupt notification
    nfbi_set_hcd_pid(getpid());
    eqreset();
    
    //writes 0x0 to SCR register
    ret = nfbi_register_write(NFBI_REG_SCR, 0x0);
    if (ret != 0) {
        ret2 = -4;
        goto err;
    }
        
	real_sleep(1);
	    
    //Check if PrevMsgFetch interrupt happens or not
    if (got_sigusr1) {
        ret = nfbi_get_event_msg(&evt);
        printf("%s line %d: evt.id=%d evt.value=%x evt.status=%x ret=%d\n", __FUNCTION__, __LINE__, evt.id, evt.value, evt.status, ret);
        if ((ret==0) && (evt.id==1)) {
            if (evt.status & IP_PREVMSG_FETCH) {
                if (!mask_enable) {
                    ret2 = -5;
                    goto err;
                }
            }
            else {
                printf("interrupt, but not PrevMsgFetch\n");
                ret2 = -6;
                goto err;
            }
        }
        else {
            if (mask_enable) {
                ret2 = -7;
                goto err;
            }
        }
    }
    else {
        if (mask_enable) {
            ret2 = -8;
            goto err;
        }
    }

    if (0 != nfbi_register_read(NFBI_REG_ISR, &val)) {
        ret2 = -9;
        goto err;
    }
    printf("ISR=0x%04x\n", val);
    if (val&IP_PREVMSG_FETCH) {
        if (mask_enable) {
            //we had written 1 to the PrevMsgFetch bit of ISR at the interrupt service routine
            //Just check again if the bit is still equal to 1.  If yes, it is 'Fail'
            ret2 = -10;
            goto err;
        }
    }
    else {
        if (!mask_enable) {
            //PrevMsgFetch bit of ISR should be equal to 1
            ret2 = -11;
            goto err;
        }
    }

    //Write 1 to clear the PrevMsgFetch bit of ISR 
    //and then check again if the bit is equal to 1.  If yes, it is 'Fail'
    ret = nfbi_register_write(NFBI_REG_ISR, IP_PREVMSG_FETCH);
    if (ret != 0) {
        ret2 = -12;
        goto err;
    }
    real_sleep(1);
    if (0 != nfbi_register_read(NFBI_REG_ISR, &val)) {
        ret2 = -13;
        goto err;
    }
    printf("ISR=0x%04x\n", val);
    if (val&IP_PREVMSG_FETCH) {
        ret2 = -14;
        goto err;
    }

    //Check again if IP_PREVMSG_FETCH interrupt happens or not
    //If the interrupt continue, it is 'Fail'. (interrupt clear check)
    if (got_sigusr1) {
        ret = nfbi_get_event_msg(&evt);
        printf("%s line %d: evt.id=%d evt.value=%x evt.status=%x ret=%d\n", __FUNCTION__, __LINE__, evt.id, evt.value, evt.status, ret);
        if ((ret==0) && (evt.id==1)) {
            if (evt.status & IP_PREVMSG_FETCH) {
                ret2 = -15;
                goto err;
            }
        }
    }
    ret2 = 0;

err:
    system("nfbi regread IMR");
    system("nfbi regread ISR");
    nfbi_register_mask_write(NFBI_REG_IMR, IM_PREVMSG_FETCH, 0x0);
    got_sigusr1 = 0;
    nfbi_set_hcd_pid(-1); //unregister
    eqreset();

    return ret2;
}

int cmd_testPrevMsgFetch(int argc, char *argv[])
{
    int retval;
    
    if (argc < 3)
        return -1;
    retval = testPrevMsgFetch(atoi(argv[2]));
    if (0 == retval)
        return 1;
    else {
        printf("retval=%d\n", retval);
        return -1;
    }
}

int testNewMsgComing(int mask_enable)
{
    int ret, val, ret2, count;
    struct evt_msg evt;
    
    ret2 = 0;
    count = 0;
begin:
    if (ret2)
        printf("count=%d ret2=%d\n", count, ret2);
    //loopback frame may be corrupted during transmision, re-transmision is necessary
    if (count < 3)
        count++;
    else
        goto err;

    nfbi_register_mask_write(NFBI_REG_IMR, IM_NEWMSG_COMING, 0x0);
    got_sigusr1 = 0;
    nfbi_set_hcd_pid(-1); //unregister
    eqreset();
    
    ret = nfbi_register_write(NFBI_REG_IMR, 0x0);
    if (ret != 0) {
        ret2 = -1;
        goto begin;
    }

    //clear all bits of ISR
    ret = nfbi_register_write(NFBI_REG_ISR, 0xffff);
    if (ret != 0) {
        ret2 = -2;
        goto begin;
    }
   
    //enable NewMsgComing
    if (mask_enable) {
        ret = nfbi_register_mask_write(NFBI_REG_IMR, IM_NEWMSG_COMING, IM_NEWMSG_COMING);
        if (ret != 0) {
            ret2 = -3;
            goto begin;
        }
    }

    //send loopback command frame - e000 0000 0020
    if (0 != nfbi_register_write(NFBI_REG_SCR, 0xe000)) {
        ret2 = -4;
        goto begin;
    }
    real_sleep(1);
    if (0 != nfbi_register_write(NFBI_REG_ISR, IP_PREVMSG_FETCH)) {
        ret2 = -5;
        goto begin;
    }
    
    if (0 != nfbi_register_write(NFBI_REG_SCR, 0x0000)) {
        ret2 = -6;
        goto begin;
    }
    real_sleep(1);
    if (0 != nfbi_register_write(NFBI_REG_ISR, IP_PREVMSG_FETCH)) {
        ret2 = -7;
        goto begin;
    }

    //register signal event handler to receive interrupt notification
    got_sigusr1 = 0;
    signal(SIGUSR1, event_handler);
    //register pid to driver for interrupt notification
    nfbi_set_hcd_pid(getpid());
    eqreset();

    if (0 != nfbi_register_write(NFBI_REG_SCR, 0x0020)) {
        ret2 = -8;
        goto begin;
    }
    real_sleep(1);
    if (0 != nfbi_register_write(NFBI_REG_ISR, IP_PREVMSG_FETCH)) {
        ret2 = -9;
        goto begin;
    }
        
	real_sleep(3);
	    
    //Check if NewMsgComing interrupt happens or not
    if (got_sigusr1) {
        ret = nfbi_get_event_msg(&evt);
        printf("%s line %d: evt.id=%d evt.value=%x evt.status=%x ret=%d\n", __FUNCTION__, __LINE__, evt.id, evt.value, evt.status, ret);
        if ((ret==0) && (evt.id==1)) {
            if (evt.status & IP_NEWMSG_COMING) {
                if (!mask_enable) {
                    ret2 = -10;
                    goto begin;
                }
            }
            else {
                ret2 = -11;
                goto begin;
            }
        }
        else {
            if (mask_enable) {
                ret2 = -12;
                goto begin;
            }
        }
    }
    else {
        if (mask_enable) {
            ret2 = -13;
            goto begin;
        }
    }

    if (0 != nfbi_register_read(NFBI_REG_ISR, &val)) {
        ret2 = -14;
        goto begin;
    }
    printf("ISR=0x%04x\n", val);
    if (val&IP_NEWMSG_COMING) {
        if (mask_enable) {
            //we had written 1 to the NewMsgComing bit of ISR at the interrupt service routine
            //Just check again if the bit is still equal to 1.  If yes, it is 'Fail'
            ret2 = -15;
            goto begin;
        }
    }
    else {
        if (!mask_enable) {
            //NewMsgComing bit of ISR should be equal to 1
            ret2 = -16;
            goto begin;
        }
    }

    //Write 1 to clear the NewMsgComing bit of ISR 
    //and then check again if the bit is equal to 1.  If yes, it is 'Fail'
    ret = nfbi_register_write(NFBI_REG_ISR, IP_NEWMSG_COMING);
    if (ret != 0) {
        ret2 = -17;
        goto err;
    }
    real_sleep(1);
    if (0 != nfbi_register_read(NFBI_REG_ISR, &val)) {
        ret2 = -18;
        goto err;
    }
    printf("ISR=0x%04x\n", val);
    if (val&IP_NEWMSG_COMING) {
        ret2 = -19;
        goto err;
    }

    //Check again if IP_NEWMSG_COMING interrupt happens or not
    //If the interrupt continue, it is 'Fail'. (interrupt clear check)
    if (got_sigusr1) {
        ret = nfbi_get_event_msg(&evt);
        printf("%s line %d: evt.id=%d evt.value=%x evt.status=%x ret=%d\n", __FUNCTION__, __LINE__, evt.id, evt.value, evt.status, ret);
        if ((ret==0) && (evt.id==1)) {
            if (evt.status & IP_NEWMSG_COMING) {
                ret2 = -20;
                goto err;
            }
        }
    }
    ret2 = 0;

err:
    nfbi_register_mask_write(NFBI_REG_IMR, IM_NEWMSG_COMING, 0x0);
    got_sigusr1 = 0;
    nfbi_set_hcd_pid(-1); //unregister
    eqreset();

    return ret2;
}

int cmd_testNewMsgComing(int argc, char *argv[])
{
    int retval;
        
    if (argc < 3)
        return -1;
    retval = testNewMsgComing(atoi(argv[2]));
    if (0 == retval)
        return 1;
    else {
        printf("retval=%d\n", retval);
        return -1;
    }
}

int testEthLink(int mask_enable)
{
    int ret, val, ret2;
    struct evt_msg evt;
    
    ret2 = -1;

    ret = nfbi_register_write(NFBI_REG_IMR, 0x0);
    if (ret != 0)
        return -1;

    //clear all bits of ISR
    ret = nfbi_register_write(NFBI_REG_ISR, 0xffff);
    if (ret != 0)
        return -2;

    //Write 1 to EnEthPHY bit of SYSCR
    ret = nfbi_register_mask_write(NFBI_REG_SYSCR, 0x1000, 0x1000);
    if (ret != 0)
        return -3;

    //Write 0 to Power down bit of BMCR, normal operation
    ret = nfbi_register_mask_write(NFBI_REG_BMCR, 0x0800, 0x0000);
    if (ret != 0)
        return -4;
	real_sleep(1);
  
    //clear all bits of ISR
    ret = nfbi_register_write(NFBI_REG_ISR, 0xffff);
    if (ret != 0) {
        ret2 = -5;
        goto err;
    }
        
    //enable ETHLINK
    if (mask_enable) {
        ret = nfbi_register_mask_write(NFBI_REG_IMR, IM_ETHLINK, IM_ETHLINK);
        if (ret != 0) {
            ret2 = -6;
            goto err;
        }
    }

    //register signal event handler to receive interrupt notification
    got_sigusr1 = 0;
    signal(SIGUSR1, event_handler);
    //register pid to driver for interrupt notification
    nfbi_set_hcd_pid(getpid());
    eqreset();
    
    //Write 1 to Power down bit of BMCR, power down
    ret = nfbi_register_mask_write(NFBI_REG_BMCR, 0x0800, 0x0800);
    if (ret != 0) {
        ret2 = -7;
        goto err;
    }

	real_sleep(1);
	    
    //Check if ETHLINK interrupt happens or not
    if (got_sigusr1) {
        ret = nfbi_get_event_msg(&evt);
        printf("%s line %d: evt.id=%d evt.value=%x evt.status=%x ret=%d\n", __FUNCTION__, __LINE__, evt.id, evt.value, evt.status, ret);
        if ((ret==0) && (evt.id==1)) {
            if (evt.status & IP_ETHLINK) {
                if (!mask_enable) {
                    ret2 = -8;
                    goto err;
                }
            }
            else {
                ret2 = -9;
                printf("interrupt, but not ETHLINK\n");
                goto err;
            }
        }
        else {
            if (mask_enable) {
                ret2 = -10;
                goto err;
            }
        }
    }
    else {
        if (mask_enable) {
            ret2 = -11;
            goto err;
        }
    }

    if (0 != nfbi_register_read(NFBI_REG_ISR, &val)) {
        ret2 = -12;
        goto err;
    }
    if (val&IP_ETHLINK) {
        if (mask_enable) {
            //we had written 1 to the ETHLINK bit of ISR at the interrupt service routine
            //Just check again if the bit is still equal to 1.  If yes, it is 'Fail'
            ret2 = -13;
            goto err;
        }
    }
    else {
        if (!mask_enable) {
            //ETHLINK bit of ISR should be equal to 1
            ret2 = -14;
            goto err;
        }
    }

    //Write 1 to clear the ETHLINK bit of ISR 
    //and then check again if the bit is equal to 1.  If yes, it is 'Fail'
    ret = nfbi_register_write(NFBI_REG_ISR, IP_ETHLINK);
    if (ret != 0) {
        ret2 = -15;
        goto err;
    }
    real_sleep(1);
    if (0 != nfbi_register_read(NFBI_REG_ISR, &val)) {
        ret2 = -16;
        goto err;
    }
    if (val&IP_ETHLINK) {
        ret2 = -17;
        goto err;
    }

    //Check again if ETHLINK interrupt happens or not
    //If the interrupt continue, it is 'Fail'. (interrupt clear check)
    if (got_sigusr1) {
        ret = nfbi_get_event_msg(&evt);
        printf("%s line %d: evt.id=%d evt.value=%x evt.status=%x ret=%d\n", __FUNCTION__, __LINE__, evt.id, evt.value, evt.status, ret);
        if ((ret==0) && (evt.id==1)) {
            if (evt.status & IP_ETHLINK) {
                ret2 = -18;
                goto err;
            }
        }
    }
    ret2 = 0;

err:
    nfbi_register_mask_write(NFBI_REG_IMR, IM_ETHLINK, 0x0);
    got_sigusr1 = 0;
    nfbi_set_hcd_pid(-1); //unregister
    eqreset();

    return ret2;
}

int cmd_testEthLink(int argc, char *argv[])
{
    int retval;

    if (argc < 3)
        return -1;
    retval = testEthLink(atoi(argv[2]));
    if (0 == retval)
        return 1;
    else {
        printf("retval=%d\n", retval);
        return -1;
    }
}


int testEthPHYStatusChange(int mask_enable)
{
    int ret, val, ret2;
    struct evt_msg evt;
    
    ret2 = -1;

    ret = nfbi_register_write(NFBI_REG_IMR, 0x0);
    if (ret != 0)
        return -1;

    //clear all bits of ISR
    ret = nfbi_register_write(NFBI_REG_ISR, 0xffff);
    if (ret != 0)
        return -2;

    //Write 1 to EnEthPHY bit of SYSCR
    ret = nfbi_register_mask_write(NFBI_REG_SYSCR, 0x1000, 0x1000);
    if (ret != 0)
        return -3;

    //Write 1 to auto nego enable bit of BMCR
    ret = nfbi_register_mask_write(NFBI_REG_BMCR, 0x1000, 0x1000);
    if (ret != 0)
        return -4;
	real_sleep(2);

    //clear all bits of ISR
    ret = nfbi_register_write(NFBI_REG_ISR, 0xffff);
    if (ret != 0)
        return -5;
       
    //enable ETHPHY_STATUS_CHANGE
    if (mask_enable) {
        ret = nfbi_register_mask_write(NFBI_REG_IMR, IM_ETHPHY_STATUS_CHANGE, IM_ETHPHY_STATUS_CHANGE);
        if (ret != 0) {
            ret2 = -6;
            goto err;
        }
    }

    //register signal event handler to receive interrupt notification
    got_sigusr1 = 0;
    signal(SIGUSR1, event_handler);
    //register pid to driver for interrupt notification
    nfbi_set_hcd_pid(getpid());
    eqreset();
    
    //Write 0 to auto nego enable bit of BMCR
    ret = nfbi_register_mask_write(NFBI_REG_BMCR, 0x1000, 0x0000);
    if (ret != 0) {
        ret2 = -7;
        goto err;
    }
    
    real_sleep(3);
	
    //Check if ETHPHY_STATUS_CHANGE interrupt happens or not
    if (got_sigusr1) {
        ret = nfbi_get_event_msg(&evt);
        printf("%s line %d: evt.id=%d evt.value=%x evt.status=%x ret=%d\n", __FUNCTION__, __LINE__, evt.id, evt.value, evt.status, ret);
        if ((ret==0) && (evt.id==1)) {
            if (evt.status & IP_ETHPHY_STATUS_CHANGE) {
                if (!mask_enable) {
                    ret2 = -8;
                    goto err;
                }
            }
            else {
                ret2 = -9;
                printf("interrupt, but not ETHPHY_STATUS_CHANGE\n");
                goto err;
            }
        }
        else {
            if (mask_enable) {
                ret2 = -10;
                goto err;
            }
        }
    }
    else {
        if (mask_enable) {
            ret2 = -11;
            goto err;
        }
    }
    eqreset(); //flush the second event ethphy status un-change

    if (0 != nfbi_register_read(NFBI_REG_ISR, &val)) {
        ret2 = -12;
        goto err;
    }
    if (val&IP_ETHPHY_STATUS_CHANGE) {
        if (mask_enable) {
            //we had written 1 to the ETHPHY_STATUS_CHANGE bit of ISR at the interrupt service routine
            //Just check again if the bit is still equal to 1.  If yes, it is 'Fail'
            ret2 = -13;
            goto err;
        }
    }
    else {
        if (!mask_enable) {
            //ETHPHY_STATUS_CHANGE bit of ISR should be equal to 1
            ret2 = -14;
            goto err;
        }
    }
    //Write 1 to clear the ETHPHY_STATUS_CHANGE bit of ISR 
    //and then check again if the bit is equal to 1.  If yes, it is 'Fail'
    ret = nfbi_register_write(NFBI_REG_ISR, IP_ETHPHY_STATUS_CHANGE);
    if (ret != 0) {
        ret2 = -15;
        goto err;
    }
    real_sleep(1);
    if (0 != nfbi_register_read(NFBI_REG_ISR, &val)) {
        ret2 = -16;
        goto err;
    }
    if (val&IP_ETHPHY_STATUS_CHANGE) {
        ret2 = -17;
        goto err;
    }
    //Check again if ETHPHY_STATUS_CHANGE interrupt happens or not
    //If the interrupt continue, it is 'Fail'. (interrupt clear check)
    if (got_sigusr1) {
        ret = nfbi_get_event_msg(&evt);
        printf("%s line %d: evt.id=%d evt.value=%x evt.status=%x ret=%d\n", __FUNCTION__, __LINE__, evt.id, evt.value, evt.status, ret);
        if ((ret==0) && (evt.id==1)) {
            if (evt.status & IP_ETHPHY_STATUS_CHANGE) {
                ret2 = -18;
                goto err;
            }
        }
    }
    ret2 = 0;

err:
    nfbi_register_mask_write(NFBI_REG_IMR, IM_ETHPHY_STATUS_CHANGE, 0x0);
    got_sigusr1 = 0;
    nfbi_set_hcd_pid(-1); //unregister
    eqreset();

    return ret2;
}

int cmd_testEthPHYStatusChange(int argc, char *argv[])
{
    int retval;

    if (argc < 3)
        return -1;
    retval = testEthPHYStatusChange(atoi(argv[2]));
    if (0 == retval)
        return 1;
    else {
        printf("retval=%d\n", retval);
        return -1;
    }
}

int reset_regs_to_default(void)
{
    int i;
    int param;
    
    i = 0;
    while (nfbi_reg_table[i].name != NULL) {
        if ((strcmp(nfbi_reg_table[i].name, "BMCR")==0) ||
            (strcmp(nfbi_reg_table[i].name, "ANAR")==0) ||
            (strcmp(nfbi_reg_table[i].name, "SYSCR")==0) ||
            (strcmp(nfbi_reg_table[i].name, "IMR")==0) ||
            (strcmp(nfbi_reg_table[i].name, "ISR")==0)
            ) {
            //put register address to high word and the value to low word
            param = (nfbi_reg_table[i].val << 16) | (nfbi_reg_table[i].dval & 0xffff);
    	    ioctl(nfbi_fd, NFBI_IOCTL_REGWRITE, &param);
	    }
        i++;
	}
    return 0;
}

int firmware_reset(int verify, char *fw_filename, char *boot_filename)
{
    char buf[8];
    int rc, count, val;
    char header[8]={0x63, 0x73, 0x79, 0x73, 0x80, 0x70, 0x00, 0x00};
    int ret;
    
    rc = nfbi_bulk_mem_read((NFBI_KERNADDR-0x10), 8, buf);
    if (rc != 0) {
        printf("error: %s %d\n", __FUNCTION__, __LINE__);
  	    return -101;
    }

    reset_regs_to_default();

    if (memcmp(header, buf, 8)==0) {
        //printf("firmware exist\n");
        ret = firmware_download_wo_boot(verify, NULL);
        if (0 == ret) {
           	//wait for firmware running to be ready
            real_sleep(5);
        	count = 0;
            while (1) {
                //check if AllSoftwareReadyIP of SYSSR is equal to 1
                if (0 != nfbi_register_read(NFBI_REG_SYSSR, &val))
                    return -102;
                if (val&BM_ALLSOFTWARE_READY) {
                    if (0 == nfbiloop())
                        return 0;
                }
                if (count >= 5)
                    break;
                real_sleep(1);
                count++;
            }
        }
        else {
            printf("ret=%d\n", ret);
        }
    }
    //printf("firmware not exist\n");
    return firmware_download_w_boot(verify, fw_filename, boot_filename);
}

int cmd_fwreset(int argc, char *argv[])
{
    int retval;
    
    if (argc < 5)
        return -1;
    retval = firmware_reset(atoi(argv[2]), argv[3], argv[4]);
    if (0 == retval)
        return 1;
    else {
        printf("retval=%d\n", retval);
        return -1;
    }
}

int cmd_version(int argc, char *argv[])
{
    printf("version=%s\n", NFBI_VERSION);
    return 0;
}

int cmd_97fwversion(int argc, char *argv[])
{
    char version[16];
    int index, len;
    //int data;

    index = cmdname2index("fw_version", nfbi_cmdframe_table);
    if (index < 0) {
        return -1; //unknown command
    }

    /*
    data = nfbi_cmdframe_table[index].retransmit_count;
    ioctl(nfbi_fd, NFBI_IOCTL_RETRANSMIT_COUNT, &data);
    data = nfbi_cmdframe_table[index].response_timeout; //in 10ms
    ioctl(nfbi_fd, NFBI_IOCTL_RESPONSE_TIMEOUT, &data);
    */
    
    if (0==nfbi_tx_cmd_frame_data(TAG_GET_COMMAND, nfbi_cmdframe_table[index].val, version, 0)) {
        len = 16;
	    if (0==nfbi_rx_status_frame_data(TAG_GET_COMMAND, nfbi_cmdframe_table[index].val, version, &len)) {
	        if (len<16)
	            version[len]='\0';
	        else
	            version[15]='\0';    
	        //printf("len=%d\n", len);
            printf("version=%s\n", version);
    	    return 0;
        }
        else {
            printf("%s %d\n", __FUNCTION__, __LINE__);
            return -1;
        }
    }
    printf("%s %d\n", __FUNCTION__, __LINE__);
    return -1;
}

int cmd_createfile(int argc, char *argv[])
{
    FILE *fp = NULL;

    if (argc < 4)
        return -1;
        
    if (!(fp = fopen(argv[2], "w"))) {
        perror(argv[2]);
        return -1;
    }
    //printf("argv[3]=%s, strlen(argv[3])=%d\n", argv[3], strlen(argv[3]));
    fputs(argv[3], fp);
    fputs("\n", fp);
    fclose(fp);

    return 0;
}


int testPhyRegPollTime(void)
{
    int ret, ret2, i;
    struct evt_msg evt;
    int count1, count2;

    ret2 = -1;
    ret = nfbi_register_write(NFBI_REG_IMR, 0x0);
    if (ret != 0)
        return -1;

    //clear all bits of ISR
    ret = nfbi_register_write(NFBI_REG_ISR, 0xffff);
    if (ret != 0)
        return -2;

    //Write 1 to EnEthPHY bit of SYSCR
    ret = nfbi_register_mask_write(NFBI_REG_SYSCR, 0x1000, 0x1000);
    if (ret != 0)
        return -3;

    //clear all bits of ISR
    ret = nfbi_register_write(NFBI_REG_ISR, 0xffff);
    if (ret != 0)
        return -4;
       
    //enable ETHLINK
    ret = nfbi_register_mask_write(NFBI_REG_IMR, IM_ETHLINK, IM_ETHLINK);
    if (ret != 0) {
        ret2 = -5;
        goto err;
    }

    //register signal event handler to receive interrupt notification
    got_sigusr1 = 0;
    signal(SIGUSR1, event_handler);
    //register pid to driver for interrupt notification
    nfbi_set_hcd_pid(getpid());
    eqreset();
    
    //Toggle Power down bit of BMCR every 300ms in Host, the duration is 10 sec
    for (i=0; i<16; i++) {
        //Write 1 to Power down bit of BMCR, power down
        ret = nfbi_register_mask_write(NFBI_REG_BMCR, 0x0800, 0x0800);
        if (ret != 0) {
            ret2 = -6;
            goto err;
        }
    	usleep(300000);  //300 ms
        //Write 0 to Power down bit of BMCR, normal operation
        ret = nfbi_register_mask_write(NFBI_REG_BMCR, 0x0800, 0x0000);
        if (ret != 0) {
            ret2 = -7;
            goto err;
        }
        usleep(300000);  //300 ms
    }
    
    //Count the number of ISR IP_ETHLINK interrupt happened
    count1 = 0;
    while (1) {
        ret = nfbi_get_event_msg(&evt);
        //printf("%s line %d: evt.id=%d evt.value=%x evt.status=%x ret=%d\n", __FUNCTION__, __LINE__, evt.id, evt.value, evt.status, ret);
        if ((ret==0) && (evt.id==1)) {
            if (evt.status & IP_ETHLINK) {
                count1++;
            }
        }
        else
            break;
    }

    eqreset(); //flush the second event ethphy status un-change

    system("nfbi setcmd phy_reg_poll_time 100");
    
    //Toggle Power down bit of BMCR every 300ms in Host, the duration is 10 sec
    for (i=0; i<16; i++) {
        //Write 1 to Power down bit of BMCR, power down
        ret = nfbi_register_mask_write(NFBI_REG_BMCR, 0x0800, 0x0800);
        if (ret != 0) {
            ret2 = -8;
            goto err;
        }
    	usleep(300000);  //300 ms
        //Write 0 to Power down bit of BMCR, normal operation
        ret = nfbi_register_mask_write(NFBI_REG_BMCR, 0x0800, 0x0000);
        if (ret != 0) {
            ret2 = -9;
            goto err;
        }
        usleep(300000);  //300 ms
    }
    
    //Count the number of ISR IP_ETHLINK interrupt happened
    count2 = 0;
    while (1) {
        ret = nfbi_get_event_msg(&evt);
        //printf("%s line %d: evt.id=%d evt.value=%x evt.status=%x ret=%d\n", __FUNCTION__, __LINE__, evt.id, evt.value, evt.status, ret);
        if ((ret==0) && (evt.id==1)) {
            if (evt.status & IP_ETHLINK) {
                count2++;
            }
        }
        else
            break;
    }
    //printf("count1=%d count2=%d\n", count1, count2);
    if (count1 > count2)
        ret2 = 0;
    else
        ret2 = -10;
    
err:    
    nfbi_register_write(NFBI_REG_ISR, IP_ETHLINK);
    nfbi_register_mask_write(NFBI_REG_IMR, IM_ETHLINK, 0x0);
    got_sigusr1 = 0;
    nfbi_set_hcd_pid(-1); //unregister
    eqreset();

    return ret2;
}

int cmd_testPhyRegPollTime(int argc, char *argv[])
{
    int retval;
    
    retval = testPhyRegPollTime();
    if (0 == retval)
        return 1;
    else {
        printf("retval=%d\n", retval);        
        return -1;
    }
}

int checkInterruptEvent(int mask, int expected)
{
    int ret, ret2;
    struct evt_msg evt;

    //printf("mask=0x%04x expected=%d\n", mask, expected);
    ret2 = -1;
    while (1) {
        ret = nfbi_get_event_msg(&evt);
        //printf("%s line %d: evt.id=%d evt.value=%x evt.status=%x ret=%d\n", __FUNCTION__, __LINE__, evt.id, evt.value, evt.status, ret);
        if ((ret==0) && (evt.id==1)) {
            if (evt.status & mask) {
                switch(expected) {
                  case 0:
                    if (evt.value & mask)
                        ret2 = -2;
                    else
                        ret2 = 0;
                    break;
                  case 1:
                    if (evt.value & mask)
                        ret2 = 0;
                    else
                        ret2 = -3;
                    break;
                  default:
                    ret2 = 0; //don't care the value of SYSSR
                    break;
                }
            }
        }
        else
            break;
    }
    return ret2;
}

int cmd_checkInterruptEvent(int argc, char *argv[])
{
    int retval;
    
    if (argc < 4)
        return -1;
    retval = checkInterruptEvent((1<<atoi(argv[2])), atoi(argv[3]));
    if (0 == retval)
        return 1;
    else {
        printf("retval=%d\n", retval);        
        return -1;
    }
}

int cmd_start2RecvEvent(int argc, char *argv[])
{
    //register signal event handler to receive interrupt notification
    got_sigusr1 = 0;
    signal(SIGUSR1, event_handler);
    //register pid to driver for interrupt notification
    nfbi_set_hcd_pid(getpid());
    eqreset();
    return 1;
}

int cmd_stop2RecvEvent(int argc, char *argv[])
{
    got_sigusr1 = 0;
    nfbi_set_hcd_pid(-1); //unregister
    eqreset();
    return 1;
}

int cmd_checkAssociatedInfo(int argc, char *argv[])
{
    int channel, timeout, retry;
    rtk_wireless_info wlaninfo;

    if (argc < 3)
        return -1;
    timeout = atoi(argv[2]);
    channel = 0;
    if (argc >= 5)
        channel = atoi(argv[4]);

    retry = 0;
    while (1) {
        if (0==getwlaninfo(&wlaninfo)) {
            if (memcmp(wlaninfo.ap, ALL_ZERO_MAC_ADDR, 6)) {
                if (argc == 3)
                    return 1;
                else {
                    if (strcmp(argv[3], wlaninfo.ssid)==0) {
                        if (argc == 4) //don't care channel number
                            return 1;
                        else if (channel == wlaninfo.freq)
                            return 1;
                    }
                    else {
                        if ((argc == 5) && (channel == wlaninfo.freq)) {
                            //sometimes SSID of AP we got is empty.
                            //If channel number is correct, just try again
                            if (wlaninfo.ssid[0]=='\0') {
                                if (retry==0) {
                                    retry++;
                                    printf("getwlaninfo again\n");
                                    real_sleep(1);
                                    continue;
                                }
                            }
                        }
                    }
                    printf("AP: %02X:%02X:%02X:%02X:%02X:%02X\n", wlaninfo.ap[0], wlaninfo.ap[1],
                              wlaninfo.ap[2], wlaninfo.ap[3], wlaninfo.ap[4], wlaninfo.ap[5]);
                    printf("SSID:\"%s\"\n", wlaninfo.ssid);
                    printf("Channel:%d\n", wlaninfo.freq);
                    return -1;
                }
            }
            
            if (timeout <= 0)
                break;
            printf("%d\n", timeout);
            real_sleep(1);
            timeout--;
        }
        else {
            printf("fail to get wlan info\n");
            break;
        }
    }
    return -1;
}

int cmd_gen_pauseframe(int argc, char *argv[])
{
    int portnum, second, i;
    int val;

    portnum = atoi(argv[2]);
    second = atoi(argv[3]);
#ifndef NFBI_HOST_IS_PANABOARD
    //use Embedded SmartBit-like function to generate Pause frame
    if (portnum == 0) {
        read_word(0xbb804104, &val);
        write_word(0xbb804104, (val|0x00000300)); //enable P0 pause flow control
        write_word(0xbb806900, 0x00000100); //"force send Pause frame" enable for P0 
        for (i=0; i<second; i++) {
            write_word(0xbb806904, 0x01000001); //SMB mode enable, start Tx
            usleep(100);
            write_word(0xbb806904, 0x01000002); //SMB mode enable, stop Tx
            usleep(100);  //0.1ms, the delay is necessary
            write_word(0xbb806904, 0x00000000); //SMB mode disable
            real_sleep(1);
        }
    }
    else if (portnum == 3) {
        read_word(0xbb804110, &val);
        write_word(0xbb804110, (val|0x00000300)); //enable P3 pause flow control
        write_word(0xbb806900, 0x00000800); //"force send Pause frame" enable for P3 
        for (i=0; i<second; i++) {
            write_word(0xbb806904, 0x08001000); //SMB mode enable, start Tx
            usleep(100);
            write_word(0xbb806904, 0x08002000); //SMB mode enable, stop Tx
            usleep(100);  //0.1ms, the delay is necessary
            write_word(0xbb806904, 0x00000000); //SMB mode disable
            real_sleep(1);
        }
    }
    else if (portnum == 5) {
        read_word(0xbb804118, &val);
        write_word(0xbb804118, (val|0x00000300)); //enable P5 pause flow control
        write_word(0xbb806900, 0x00002000); //"force send Pause frame" enable for P5 
        for (i=0; i<second; i++) {
            write_word(0xbb806904, 0x20100000); //SMB mode enable, start Tx
            usleep(100);
            write_word(0xbb806904, 0x20200000); //SMB mode enable, stop Tx
            usleep(100);  //0.1ms, the delay is necessary
            write_word(0xbb806904, 0x00000000); //SMB mode disable
            real_sleep(1);
        }
    }
    else {
        return -1;
    }
    write_word(0xbb806900, 0x00000000); //"force send Pause frame" disable
#else
    //TBD
#endif
    return 0;
}

int cmd_gen_packet(int argc, char *argv[])
{
    int portnum, patterntype, seconds, length, pktcnt;
    char saddr[6], daddr[6], tmp[4];
    int i, val;

    if (argc < 9) {
        printf("nfbi genpkt <port#> <SA> <DA> <data pattern type> <length> <seconds> <pktcnt>\n");
        printf("<port#>: 3, 5\n<data pattern type>:0-random, 1-repeating pattern, 2-INC byte\n");
        printf("<length>: 0-random, 64-1518\n");
        return 0;
    }
        
    portnum = atoi(argv[2]);
    //SA
    for (i=0; i<6; i++) {
        strncpy(tmp, argv[3]+i*2, 2);
        tmp[2] = '\0'; 
        saddr[i] = _atoi(tmp ,16);
    }
    //DA
    for (i=0; i<6; i++) {
        strncpy(tmp, argv[4]+i*2, 2);
        tmp[2] = '\0'; 
        daddr[i] = _atoi(tmp ,16);
    }
    patterntype = atoi(argv[5]);
    length = atoi(argv[6]);
    seconds = atoi(argv[7]);
    pktcnt = atoi(argv[8]);
    if ((seconds==0)&&(pktcnt==0))
        return -1;
#ifndef NFBI_HOST_IS_PANABOARD
    //use Embedded SmartBit-like function to generate frames
    if (portnum == 3) {
        read_word(0xbb804110, &val);
        write_word(0xbb804110, (val|0x00000300)); //enable P3 pause flow control
        write_word(0xbb806900, 0x00000000); //"force send Pause frame" disable
        //append ethertype header
        if (patterntype==1) //repeating pattern
            val = 0x00110000;
        else if (patterntype==2) //INC byte
            val = 0x00120000;
        else
            val = 0x00100000; //random
        val = val | (length & 0xffff); //64~1518, 0-random
        write_word(0xbb806b14, val);
        write_word(0xbb806b30, 0x00000800); //0x0800 IP proto
        val = ((daddr[0]<<8)&0xff00) | (daddr[1]&0xff);
        write_word(0xbb806b18, val); //DA:00-00-00-00-00-98
        val = ((daddr[2]<<24)&0xff000000) | ((daddr[3]<<16)&0xff0000) |((daddr[4]<<8)&0xff00) | (daddr[5]&0xff);
        write_word(0xbb806b1c, val);
        val = ((saddr[0]<<8)&0xff00) | (saddr[1]&0xff);
        write_word(0xbb806b20, val); //SA:00-00-00-00-00-11
        val = ((saddr[2]<<24)&0xff000000) | ((saddr[3]<<16)&0xff0000) |((saddr[4]<<8)&0xff00) | (saddr[5]&0xff);
        write_word(0xbb806b24, val);
        if (seconds==0)
            write_word(0xbb806b5c, pktcnt);
        else
            write_word(0xbb806b5c, 0x00000000); //continuously transmit packets
        write_word(0xbb806908, 0x00000008); //write 1 to clear Tx Done flag
        write_word(0xbb806b60, 0xffffffff); //clear current transmitted packet counter
        write_word(0xbb806904, 0x08001000); //SMB mode enable, start Tx
        if (seconds==0) {
            read_word(0xbb806908, &val);
            //wait for Tx done
            while (!(val&0x00000008)) {
                real_sleep(1);
                read_word(0xbb806908, &val);
            }
            read_word(0xbb806b60, &val);
            if (val != pktcnt)
                printf("Transmitted packet count != Expected packet count\n");
        }
        else
            real_sleep(seconds);
        write_word(0xbb806904, 0x08002000); //SMB mode enable, stop Tx
        usleep(100);  //0.1ms, the delay is necessary
        write_word(0xbb806904, 0x00000000); //SMB mode disable
    }
    else if (portnum == 5) {
        read_word(0xbb804118, &val);
        write_word(0xbb804118, (val|0x00000300)); //enable P5 pause flow control
        write_word(0xbb806900, 0x00000000); //"force send Pause frame" disable
        //append ethertype header
        if (patterntype==1) //repeating pattern
            val = 0x00110000;
        else if (patterntype==2) //INC byte
            val = 0x00120000;
        else
            val = 0x00100000; //random
        val = val | (length & 0xffff); //64~1518, 0-random
        write_word(0xbb806c14, val);
        write_word(0xbb806c30, 0x00000800); //0x0800 IP proto
        val = ((daddr[0]<<8)&0xff00) | (daddr[1]&0xff);
        write_word(0xbb806c18, val); //DA:00-00-00-00-00-97
        val = ((daddr[2]<<24)&0xff000000) | ((daddr[3]<<16)&0xff0000) |((daddr[4]<<8)&0xff00) | (daddr[5]&0xff);
        write_word(0xbb806c1c, val);
        val = ((saddr[0]<<8)&0xff00) | (saddr[1]&0xff);
        write_word(0xbb806c20, val); //SA:00-00-00-00-00-11
        val = ((saddr[2]<<24)&0xff000000) | ((saddr[3]<<16)&0xff0000) |((saddr[4]<<8)&0xff00) | (saddr[5]&0xff);
        write_word(0xbb806c24, val);
        if (seconds==0)
            write_word(0xbb806c5c, pktcnt);
        else
            write_word(0xbb806c5c, 0x00000000); //continuously transmit packets
        write_word(0xbb806908, 0x00000020); //write 1 to clear Tx Done flag
        write_word(0xbb806c60, 0xffffffff); //clear current transmitted packet counter
        write_word(0xbb806904, 0x20100000); //SMB mode enable, start Tx
        if (seconds==0) {
            read_word(0xbb806908, &val);
            //wait for Tx done
            while (!(val&0x00000020)) {
                real_sleep(1);
                read_word(0xbb806908, &val);
            }
            read_word(0xbb806c60, &val);
            if (val != pktcnt)
                printf("Transmitted packet count != Expected packet count\n");
        }
        else
            real_sleep(seconds);
        write_word(0xbb806904, 0x20200000); //SMB mode enable, stop Tx
        usleep(100);  //0.1ms, the delay is necessary
        write_word(0xbb806904, 0x00000000); //SMB mode disable
    }
    else {
        return -1;
    }
#else
    //TBD
#endif
    return 0;
}

int cmd_dot3InPauseFrames(int argc, char *argv[])
{
    int portnum;
    int val;

    portnum = atoi(argv[2]);
#ifndef NFBI_HOST_IS_PANABOARD
    //use Embedded SmartBit-like function to generate frames
    if (portnum == 3) {
        read_word(0xbb8012d8, &val);
        printf("%d\n", val);
    }
    else if (portnum == 5) {
        read_word(0xbb8013d8, &val);
        printf("%d\n", val);
    }
    else {
        return -1;
    }
#else
    //TBD
#endif
    return 0;
}

int cmd_resetMibCounters(int argc, char *argv[])
{
#ifndef NFBI_HOST_IS_PANABOARD
    write_word(0xbb801000, 0xffffffff);
#else
    //TBD
#endif
    return 1;
}

int cmd_compare(int argc, char *argv[])
{
    FILE *fp1 = NULL;
    FILE *fp2 = NULL;
    char s1[128], s2[128];
    int v1, v2;

    if (argc < 4)
        return -1;
        
    if (!(fp1 = fopen(argv[2], "r"))) {
        perror(argv[2]);
        return -1;
    }
    if (!(fp2 = fopen(argv[3], "r"))) {
        perror(argv[3]);
        return -1;
    }
    fgets(s1, 128, fp1);
    fgets(s2, 128, fp2);
    fclose(fp1);
    fclose(fp2);
    v1 = atoi(s1);
    v2 = atoi(s2);
    if (v1 == v2)
        printf("Equal\n");
    else if (v1 > v2)
        printf("More\n");
    else
        printf("Less\n");
    return 0;
}

int testBootcodeReady(int mask_enable, int verify, char *boot_filename)
{
    int ret, val, ret2;
    struct evt_msg evt;

    //download bootcode to memory
    ret = bootcode_download(verify, boot_filename);
    if (ret != 0) {
        ret2 = -1;
        goto err;
    }
    
    //enable ChecksumDone
    if (mask_enable) {
        ret = nfbi_register_mask_write(NFBI_REG_IMR, IM_BOOTCODE_READY, IM_BOOTCODE_READY);
        if (ret != 0) {
            ret2 = -2;
            goto err;
        }
    }

    //clear all bits of ISR
    ret = nfbi_register_write(NFBI_REG_ISR, 0xffff);
    if (ret != 0) {
        ret2 = -3;
        goto err;
    }

    //register signal event handler to receive interrupt notification
    got_sigusr1 = 0;
    signal(SIGUSR1, event_handler);
    //register pid to driver for interrupt notification
    nfbi_set_hcd_pid(getpid());
    eqreset();

    //write 1 to SystemRst bit to reset the 8197B
    ret = nfbi_register_write(NFBI_REG_CMD, BM_SYSTEMRST);
    if (ret != 0) {
        ret2 = -4;
        goto err;
    }
        
    /* When mdio access is high speed, it's possible that
     * it begin to send bootcode before reset complete.
     * To make sure hardware reset finish,
     * check if BM_SYSTEMRST bit automatically retun to zero.
     */
    do {
        ret = nfbi_register_read(NFBI_REG_CMD, &val);
        if (ret != 0) {
            ret2 = -5;
            goto err;
        }
    } while( (val & BM_SYSTEMRST) == BM_SYSTEMRST );

	// need to do_dram_settings again for DDR after system reset
	ret = nfbi_register_read(NFBI_REG_PHYID2, &val);
	if (ret != 0)
		return -31;
	if (val == NFBI_REG_PHYID2_DEFAULT2) {
		//8198
		//Due to the default dram setting is unable to boot 8198 up,
		//it's necessary to set proper DRAM config and timing registers here.
		if (0 != do_dram_settings())
			return -32;
	}

    //run the bootcode, write 1 to StartRunBootCode bit
    ret = nfbi_register_mask_write(NFBI_REG_CMD, BM_START_RUN_BOOTCODE, BM_START_RUN_BOOTCODE);
    if (ret != 0) {
        ret2 = -6;
        goto err;
    }

    real_sleep(3);
    if (got_sigusr1) {
        ret = nfbi_get_event_msg(&evt);
        printf("%s line %d: evt.id=%d evt.value=%x evt.status=%x ret=%d\n", __FUNCTION__, __LINE__, evt.id, evt.value, evt.status, ret);
        if ((ret==0) && (evt.id==1)) {
            if (evt.status & IP_BOOTCODE_READY) {
                if (!mask_enable) {
                    ret2 = -7;
                    goto err;
                }
            }
            else {
                ret2 = -8;
                printf("interrupt, but not BOOTCODE_READY\n");
                goto err;
            }
        }
        else {
            if (mask_enable) {
                ret2 = -9;
                goto err;
            }
        }
    }
    else {
        if (mask_enable) {
            ret2 = -10;
            goto err;
        }
    }
    
    if (0 != nfbi_register_read(NFBI_REG_ISR, &val)) {
        ret2 = -11;
        goto err;
    }
    printf("ISR=0x%04x\n", val);
    if (val&IP_BOOTCODE_READY) {
        if (mask_enable) {
            //we had written 1 to the IP_BOOTCODE_READY bit of ISR at the interrupt service routine
            //Just check again if the bit is still equal to 1.  If yes, it is 'Fail'
            ret2 = -12;
            goto err;
        }
    }
    else {
        if (!mask_enable) {
            //IP_BOOTCODE_READY bit of ISR should be equal to 1
            ret2 = -13;
            goto err;
        }
    }

    //Write 1 to clear the IP_BOOTCODE_READY bit of ISR 
    //and then check again if the bit is equal to 1.  If yes, it is 'Fail'
    ret = nfbi_register_write(NFBI_REG_ISR, IP_BOOTCODE_READY);
    if (ret != 0) {
        ret2 = -14;
        goto err;
    }
    real_sleep(1);
    if (0 != nfbi_register_read(NFBI_REG_ISR, &val)) {
        ret2 = -15;
        goto err;
    }
    printf("ISR=0x%04x\n", val);
    if (val&IP_BOOTCODE_READY) {
        ret2 = -16;
        goto err;
    }
    
    //Check again if IP_BOOTCODE_READY interrupt happens or not
    //If the interrupt continue, it is 'Fail'. (interrupt clear check)
    if (got_sigusr1) {
        ret = nfbi_get_event_msg(&evt);
        printf("%s line %d: evt.id=%d evt.value=%x evt.status=%x ret=%d\n", __FUNCTION__, __LINE__, evt.id, evt.value, evt.status, ret);
        if ((ret==0) && (evt.id==1)) {
            if (evt.status & IP_BOOTCODE_READY) {
                ret2 = -17;
                goto err;
            }
        }
    }
    ret2 = 0;

err:
    nfbi_register_mask_write(NFBI_REG_IMR, IM_BOOTCODE_READY, 0x0);
    got_sigusr1 = 0;
    nfbi_set_hcd_pid(-1); //unregister
    eqreset();

    return ret2;
}

int cmd_testBootcodeReady(int argc, char *argv[])
{
    int retval;

    if (argc < 5)
        return -1;
    retval = testBootcodeReady(atoi(argv[2]), atoi(argv[3]), argv[4]);
    if (0 == retval)
        return 1;
    else {
        printf("retval=%d\n", retval);
        return -1;
    }
}

int main(int argc, char *argv[])
{
	//int fdflags;
	//unsigned int arg;
	int i, ret;

    if (argc < 2) {
        print_command_list();
        return 0;
    }
    
    //open device file
    nfbi_fd = open(DEV_NAME, O_RDWR); 
	if (-1 == nfbi_fd) {
		printf("open driver failed!\n");
		return -1;
	}
    /*
	signal(SIGIO, event_handler);
	fcntl(nfbi_fd, F_SETOWN, getpid()); //specify myself as the ¡§owner¡¨ of the file
	fdflags = fcntl(nfbi_fd, F_GETFL);
	fcntl(nfbi_fd, F_SETFL, fdflags | FASYNC); //enable asynchronous notification
    */
    i = 0;
    while (nfbi_cmd_table[i].cmd != NULL) {
        if (0 == strcmp(argv[1], nfbi_cmd_table[i].cmd)) {
			if (nfbi_cmd_table[i].func) {
			    ret = nfbi_cmd_table[i].func(argc, argv);
		        if (ret > 0)
		            printf("OK\n");
		        else if (ret < 0)
		            printf("FAIL\n");
		    }
			break;
        }
        i++;
	}
	
	close(nfbi_fd);
    return 0;
}
