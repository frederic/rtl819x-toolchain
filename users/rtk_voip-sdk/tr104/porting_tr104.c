#include "prmt_limit.h"
#include "porting_tr104.h"

int tr104_port_before_download( int file_type, char *target )
{
#if 0
	/* Now, CWMP library doesn't support these types yet. */
	switch case( file_type ) {
	case DLTYPE_TONE:	/* ¡§4 Tone File¡¨ */
		break;
	case DLTYPE_RINGER:	/* ¡§5 Ringer File¡¨ */
		break;
	}
#endif
	return 0;
}

int tr104_port_after_download( int file_type, char *target )
{
#if 0
	/* Now, CWMP library doesn't support these types yet. */
	switch case( file_type ) {
	case DLTYPE_TONE:	/* ¡§4 Tone File¡¨ */
		break;
	case DLTYPE_RINGER:	/* ¡§5 Ringer File¡¨ */
		break;
	}
#endif
	return 0;
}

