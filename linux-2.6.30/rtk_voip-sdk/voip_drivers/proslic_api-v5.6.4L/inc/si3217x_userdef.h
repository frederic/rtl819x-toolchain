#ifndef SI3217x_USERDEF_H
#define SI3217x_USERDEF_H

void SI3217X_Set_SO_DTx_Loopback(proslicChanType *pProslic, unsigned int enable);
int Si3217x_GetLinefeedStatus (proslicChanType *pProslic,uInt8 *newLinefeed);
void Si3217x_Set_Ring_Cadence_ON(proslicChanType *pProslic, unsigned short msec);
void Si3217x_Set_Ring_Cadence_OFF(proslicChanType *pProslic, unsigned short msec);
void Si3217x_Set_Impendance_Silicon(proslicChanType *pProslic, unsigned short country, unsigned short impd);
void Si3217x_Set_Impendance(proslicChanType *pProslic, unsigned short preset);
void Si3217x_SendNTTCAR(proslicChanType *pProslic);
unsigned int Si3217x_SendNTTCAR_check(unsigned int chid, proslicChanType *pProslic, unsigned long time_out);
int SI3217x_SetUserMode(proslicChanType *pProslic, int on);

#endif
