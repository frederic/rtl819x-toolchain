#ifndef INCLUDE_INBAND_IF_H
#define INCLUDE_INBAND_IF_H

#define MAX_APP_DATA_SIZE 65535
#define CMD_ERROR_REPLY_BIT 0x80
#define MAX_INBAND_PAYLOAD_LEN 1480
#define MAX_FRAG_ID (0x8000 -1)
#define FRAG_ID_MASK (~(0x8000))

#define EOF_BIT 0x8000
#define SINGLE_FRAME 0x8000
#define FIRST_FRAG_ID 0x0000
#define PER_FRAME_TIMEOUT 2000 //ms

#define ERROR_TIMOUT -1
#define ERROR_DEFRAGMENT -2

#define RRCP_P_IOH		0x38	// RRCP IOH

//mark_fm
#define id_firm_upgrade						0x03

int inband_open(char *netif_name,char *slave_mac,unsigned short eth_type,int debug);
void inband_close(int chan);
void inband_free_buf(char *data_buf,int data_len);
int inband_rcv_data(int chan,char *cmd_type,char **data,int timout_ms); //return data length
int inband_rcv_data_and_seq(int chan,unsigned int *seq,char *cmd_type,char **data,int timout_ms); //return data length
int inband_write(int chan,unsigned int seq,char cmd,char *data,int data_len,int reply);

#endif
