#define GPAB_DIR *((volatile unsigned int *)0xbd010124)
#define GPAB_DATA *((volatile unsigned int *)0xbd010120)

#define GPCD_DIR *((volatile unsigned int *)0xbd010134)
#define GPCD_DATA *((volatile unsigned int *)0xbd010130)

#define GPEF_DIR  *((volatile unsigned int *)0xbd010144)
#define GPEF_DATA  *((volatile unsigned int *)0xbd010140)

#define RELAY_PSTN	0
#define RELAY_SLIC	1
#define ON_HOOK		0
#define OFF_HOOK	1
#define NO_RING		0
#define RING_INCOMING	1
#define RELAY_SUCCESS	1
#define RELAY_FAIL	0

extern char relay_2_PSTN_flag[];

/* Virtual DAA function call prototype */
//unsigned char virtual_daa_hook_detect(unsigned char relay_id);
//unsigned char virtual_daa_ring_det(void);
//char virtual_daa_relay_switch(unsigned char relay_id, unsigned char state);
//unsigned char virtual_daa_ring_incoming_detect();

