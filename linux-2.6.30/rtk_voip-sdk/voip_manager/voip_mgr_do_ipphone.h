#ifndef __VOIP_MGR_DO_IPPHONE_H__
#define __VOIP_MGR_DO_IPPHONE_H__

#define M_EXTERN_IPPHONE( x )	extern int do_mgr_ ## x ( int cmd, void *user, unsigned int len, unsigned short seq_no );
		
// IP Phone - keypad, LCM, Codec, LED and etc  
//! @addtogroup VOIP_IPPHONE
//! @ingroup VOIP_CONTROL
M_EXTERN_IPPHONE( VOIP_MGR_CTL_KEYPAD );
M_EXTERN_IPPHONE( VOIP_MGR_CTL_LCM ); 
M_EXTERN_IPPHONE( VOIP_MGR_CTL_VOICE_PATH );
M_EXTERN_IPPHONE( VOIP_MGR_CTL_LED ); 
M_EXTERN_IPPHONE( VOIP_MGR_CTL_MISC );

#undef M_EXTERN_IPPHONE

#endif /* __VOIP_MGR_DO_IPPHONE_H__ */

