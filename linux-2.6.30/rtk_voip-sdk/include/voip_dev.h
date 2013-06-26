#ifndef __VOIP_DEV_H__
#define __VOIP_DEV_H__

#include <linux/fs.h>

// ========================================================
// MAJOR and MINOR of dev 
// ========================================================
#define VOIP_DEV_MAJOR	243
#define VOIP_DEV_NAME	"voip"

enum {		/* This is minor number */
	VOIP_DEV_LCM0		= 0,
	VOIP_DEV_MGR_IOCTL	= 1,	// MGR IOCTL 
	VOIP_DEV_DECT_SITEL	= 8,	// SPI 
	VOIP_DEV_DECT_DSPG	= 9,	// SPI 
	VOIP_DEV_IVR8K		= 16,	
	
	// -------------------------- 
	VOIP_DD_DEV_START	= 60,	/* data dump dev - start */
	VOIP_DEV_JBC0		= VOIP_DD_DEV_START,
	VOIP_DEV_JBC1,		// 61
	VOIP_DEV_JBC2,		// 62
	VOIP_DEV_JBC3,		// 63
	VOIP_DEV_PCM0_TX,	// 64
	VOIP_DEV_PCM0_RX,	// 65
	VOIP_DEV_IPC,		// 66
	VOIP_DEV_WSOLA_IN,	// 67
	VOIP_DEV_WSOLA_OUT,	// 68
	VOIP_DEV_ENC_IN,	// 69
	VOIP_DEV_ENC_OUT,	// 70
	VOIP_DEV_DEC_IN,	// 71
	VOIP_DEV_DEC_OUT,	// 72
	VOIP_DEV_PCM1_TX,	// 73
	VOIP_DEV_PCM1_RX,	// 74
	VOIP_DEV_RTCPTRACE,	// 75
	VOIP_DD_DEV_END_GUARD,	/* data dump dev - end */
	VOIP_DD_DEV_END		= VOIP_DD_DEV_END_GUARD - 1,	
	// -------------------------- 
};

// ========================================================
// register / unregister functions 
// ========================================================
/* register a voip dev */
extern int register_voip_chrdev( unsigned int minor, unsigned int num, const char *name, const struct file_operations *fop );

/* unregister a voip dev */
extern void unregister_voip_chrdev( unsigned int minor, unsigned int num );

// ========================================================
// data dump (DD) dev functions 
// (minor = VOIP_DD_DEV_START ~ VOIP_DD_DEV_END)
// ========================================================
/* writing function for data dump dev (write fail: return < 0) */
/* NOTE: kernel is atomic writing */
extern int ddinst_write( unsigned int minor, const void *buff, size_t count );

/* reading function for data dump dev (read fail: return < 0) */
/* NOTE: kernel is atomic reading */
extern int ddinst_read( unsigned int minor, void *buff, size_t count );

/* read or write data to data dump dev (fail: return < 0) */
/* This function switch to ddinst_write() or ddinst_read() automatically. */
extern int ddinst_rw_auto( unsigned int minor, const void *buff, size_t count );

#endif /* __VOIP_DEV_H__ */

