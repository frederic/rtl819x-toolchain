#ifndef	_TONE_DET_I_H_
#define _TONE_DET_I_H_


#define FREQ_NA	       -1
#define FREQ_400HZ	0
#define FREQ_425HZ	1
#define FREQ_440HZ	2
#define FREQ_450HZ	3
#define FREQ_480HZ	4
#define FREQ_620HZ	5


typedef struct
{
	unsigned int frequency1; /* the first busy tone detect frequency unit: Hz. samling rate = 8000hz, allow 1~ 3999*/
	unsigned int frequency2; /* the second busy tone detect frequency unit: Hz. samling rate = 8000hz, allow 0~ 3999 */
	                         /*  0 for only one frequency. */
	int busytone_on_low_limit; /* the lower limit of busytone ON time unit:10ms, ex 0.5s=500ms = 50unit  */
	int busytone_on_up_limit;  /* the upper limit of busytone ON time unit:10ms */
	int busytone_off_low_limit; /* the lower limit of busytone OFF time unit:10ms */
	int busytone_off_up_limit;  /* the upper limit of busytone OFF time unit:10ms */
}
TstVoiptonedet_parm;

extern TstVoiptonedet_parm stVoiptonedet_parm;

extern TstVoipdistonedet_parm stVoipdistonedet_parm;

/* tone_det.c Variables */
extern int det_freq[];
extern unsigned int fsk_cid_mark_pw_lvl;
/* the original fsk_cid_mark_pw_lvl is 0x8000, new value eq 0x450, but user can config the thershold  */
/* fsk_cid_mark_pw_lvl reference in Si3050  mark frequency in - ? dBm   */
/*    (-dBm)  	|   approximaty fsk_cid_mark_pw_lvl  ETSI 	|	approximaty fsk_cid_mark_pw_lvl  BELLCORE
	-28	|		0x550				|			0x4fe
	-29	|		0x430				|			0x400
	-30	|		0x360				|			0x320
	-35	|		0x110				|			0x100
	-36	|		0x d0				|			0x c0
	-39	|		0x 70				|			0x 60
 */
void busy_tone_det_cfg_apply( void );


/* busytone_det.c Function Prototype */
void busy_tone_det(int chid, unsigned char *adr);
void busy_tone_flag_set(int chid, int cfg);
int  busy_tone_flag_get(int chid);
void busy_tone_det_init(int chid);
int  busy_tone_size(void);

/* the higher level vaule is less chance to mis-detect. */
/* but may strict than country spec */
/* user can try 20 ~ 10  */
void set_ring_tone_level(int level);
int ring_tone_flag_get(int chid);


void dis_tone_det_cfg_apply( void );
int  dis_tone_flag_get(int chid);
void dis_tone_det_init(int chid);
void dis_tone_det(int chid, unsigned char *adr);


#endif

