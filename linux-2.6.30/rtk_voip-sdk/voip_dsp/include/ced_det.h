#ifndef CED_DET_H
#define CED_DET_H
/* ced.c function prototype */

char CED_routine(int chid); /* return value: 1: fax, 2: local-modem, 3: remote-modem */
void ced_time_power_ths_set(int power);	/* set the power threshold for ced tone detection */
void modem_time_power_local_ths_set(int power); /* set the power threshold for local modem tone detection */
void modem_time_power_remote_ths_set(int power); /* set the power threshold for remote modeom tone detection */

void Init_CED_Det(unsigned char CH);

/* answer tone   */
typedef struct
{
	unsigned cng_det_local_on:1  /*__attribute__((aligned(16)))*/;
	unsigned ans_det_local_on:1;
	unsigned ansam_det_local_on:1;
	unsigned ansbar_det_local_on:1;
	unsigned ansambar_det_local_on:1;
	unsigned bellans_2225_det_local_on:1;
	unsigned vdot22_2250_det_local_on:1;
	unsigned v8bis_cre_det_local_on:1;
	unsigned v21flag_det_local_on:1;
	unsigned v21dis_det_local_on:1;
	unsigned v21dcn_det_local_on:1;


	unsigned cng_det_remote_on:1;
	unsigned ans_det_remote_on:1;
	unsigned ansam_det_remote_on:1;
	unsigned ansbar_det_remote_on:1;
	unsigned ansambar_det_remote_on:1;
	unsigned bellans_2225_det_remote_on:1;
	unsigned vdot22_2250_det_remote_on:1;
	unsigned v8bis_cre_det_remote_on:1;
	unsigned v21flag_det_remote_on:1;
	unsigned v21dis_det_remote_on:1;
	unsigned v21dcn_det_remote_on:1;

	//TstVoIPAnsDetStat stVoIPAnsDetStat[2];
}TstVoipAnsDetCtrl;//answer tone detection control

extern TstVoipAnsDetCtrl stVoipAnsDetCtrl[];

#define ANSWER_TONE_LOCAL_CNG 1
#define ANSWER_TONE_LOCAL_ANS 2
#define ANSWER_TONE_LOCAL_ANSAM 4
#define ANSWER_TONE_LOCAL_ANSBAR 8
#define ANSWER_TONE_LOCAL_ANSAMBAR 16
#define ANSWER_TONE_LOCAL_BELLANS 32
#define ANSWER_TONE_LOCAL_V22 64
#define ANSWER_TONE_LOCAL_V8BIS 128


#define ANSWER_TONE_REMOTE_CNG 256
#define ANSWER_TONE_REMOTE_ANS 512
#define ANSWER_TONE_REMOTE_ANSAM 1024
#define ANSWER_TONE_REMOTE_ANSBAR 2048
#define ANSWER_TONE_REMOTE_ANSAMBAR 4096
#define ANSWER_TONE_REMOTE_BELLANS 8192
#define ANSWER_TONE_REMOTE_V22 16384
#define ANSWER_TONE_REMOTE_V8BIS 32768

#define ANSWER_TONE_LOCAL_V21FLAG 0x10000
#define ANSWER_TONE_REMOTE_V21FLAG 0x20000

#define ANSWER_TONE_LOCAL_V21DIS 0x40000
#define ANSWER_TONE_REMOTE_V21DIS 0x80000

#define ANSWER_TONE_LOCAL_V21DCN 0x100000
#define ANSWER_TONE_REMOTE_V21DCN 0x200000

int set_answer_tone_det(unsigned int chid, unsigned int config);
void answer_tone_det(unsigned int chid, unsigned short* page_addr, int dir);
int reinit_answer_tone_det(unsigned int chid);

#endif

