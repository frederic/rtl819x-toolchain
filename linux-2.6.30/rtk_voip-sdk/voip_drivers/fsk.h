#ifndef __FSK__
#define __FSK__

// Message Type
#define FSK_MSG_CALLSETUP			0x80		// Call Set-up
#define FSK_MSG_MWSETUP				0x82		// Message Waiting (VMWI)
#define FSK_MSG_ADVICECHARGE		0x86		// Advice of Charge
#define FSK_MSG_SMS					0x89		// Short Message Service

void sendProSLICID (unsigned int chid, char mode, unsigned char msg_type, char *str, char *str2, char *cid_name);

extern char SiLabsID2[];
extern int fsk_spec_areas[];

#endif

