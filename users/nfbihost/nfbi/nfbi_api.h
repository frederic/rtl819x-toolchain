#ifndef _NFBI_API_H_
#define _NFBI_API_H_

#include "../../../linux-2.6.30/drivers/char/rtl_nfbi/rtl_nfbi.h"

#define TAG_SET_COMMAND	0x80
#define TAG_GET_COMMAND	0xC0
#define TAG_GOOD_STATUS	0x80
#define TAG_BAD_STATUS	0xC0

//#define BOOTCODE_DOWNLOAD_RETRY_MAX 3
#define BOOTCODE_DOWNLOAD_RETRY_MAX 1

#define ALL_ZERO_MAC_ADDR	"\x0\x0\x0\x0\x0\x0"
#define DDR_TMP_FILE_NAME   "/tmp/ddr"

extern void real_sleep(unsigned int sec);
extern int _atoi(char *s, int base);

extern int nfbi_set_hcd_pid(int pid);
extern int nfbi_get_event_msg(struct evt_msg *evt);
extern int nfbi_register_read(int reg, int *pval);
extern int nfbi_register_write(int reg, int val);
extern int nfbi_register_mask_read(int reg, int mask, int *pval);
extern int nfbi_register_mask_write(int reg, int mask, int val);
extern int nfbi_mem32_write(int addr, int val);
extern int nfbi_mem32_read(int addr, int *pval);
extern int nfbi_bulk_mem_write(int addr, int len, char *buf);
extern int nfbi_bulk_mem_read(int addr, int len, char *buf);

extern int do_dram_settings(void);
extern void send_jumpcmmand_to_ram(int ram_addr, int jump_addr);
extern int send_file_to_ram(char *filename, unsigned int ram_addr, int verify);
extern int bootcode_download3(char *filename);
extern int bootcode_download2(char *filename, int offset);
extern int bootcode_download(int verify, char *filename);
extern int firmware_download_w_boot(int verify, char *fw_filename, char *boot_filename);
extern int firmware_download_wo_boot(int verify, char *fw_filename);
extern int hwreset(void);
extern int eqreset(void);

extern void dump_misc(char *msg);
//extern int create_ddr_tmp_file(void);
//extern int check_ddr_tmp_file(void);
extern int get_dram_type(void);
#endif //_NFBI_API_H_
