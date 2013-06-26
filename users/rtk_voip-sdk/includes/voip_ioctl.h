#ifndef __VOIP_IOCTL__
#define __VOIP_IOCTL__

#ifdef __ECOS
extern int nf_setsockopt(void *sk, u_int8_t pf, int val, char *opt, int len);
#define SETSOCKOPT(optid, varptr, vartype, qty) \
        { \
				if( nf_setsockopt( NULL, AF_INET, optid, (void *)varptr, sizeof(vartype)*qty ) != 0 )	\
					return -2;	\
        }
#else

#define VOIP_MGR_IOCTL_DEV_NAME		"/dev/voip/mgr"

extern int g_VoIP_Mgr_FD;

#define SETSOCKOPT(optid, varptr, vartype, qty) \
		{	\
			int ret;	\
						\
			if( g_VoIP_Mgr_FD < 0 )	\
				return -1;	\
			if( ( ret = ioctl( g_VoIP_Mgr_FD, 	\
						VOIP_MGR_IOC_CMD( optid, sizeof(vartype)*qty ), 	\
						varptr ) ) < 0 ) 	\
			{							\
				return ret;				\
			}							\
		}

#endif

#endif /* __VOIP_IOCTL__ */

