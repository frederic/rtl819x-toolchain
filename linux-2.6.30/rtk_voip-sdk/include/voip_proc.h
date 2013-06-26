#ifndef __VOIP_PROC_H__
#define __VOIP_PROC_H__
#include <linux/proc_fs.h>

#define PROC_VOIP_DIR			"voip"
#define PROC_VOIP_CH_FORMAT		"ch%u"
#define PROC_VOIP_SS_FORMAT		"ss%u"
#define PROC_VOIP_DECT_DIR		"dect"
#define PROC_VOIP_CH_MAPS_DIR	"maps"
#define PROC_VOIP_PCM_DIR		"pcm"

/* 
 * We use the last parameter of create_proc_read_entry() to encode 
 * proc path. 
 */
/*
 * bit0-6: channel ID or session ID 
 * bit7: channel(1) or session(0)
 */
#define PROC_DATA_BIT_CH			0x80
#define PROC_DATA_FROM_CH( ch )		( PROC_DATA_BIT_CH | ( ch & 0x7F ) )
#define PROC_DATA_FROM_SS( ss )		( 0                | ( ss & 0x7F ) )

#define CH_FROM_PROC_DATA( data )	( ( ( unsigned long )data & PROC_DATA_BIT_CH ) ? \
									  ( ( unsigned long )data & 0x7F ) : 0xFF )
#define SS_FROM_PROC_DATA( data )	( !( ( unsigned long )data & PROC_DATA_BIT_CH ) ? \
									   ( ( unsigned long )data & 0x7F ) : 0xFF )
#define IS_CH_PROC_DATA( data )		( ( unsigned long )data & PROC_DATA_BIT_CH )

/*
 * use which number as channel and session ? 
 */
#if 1
extern int dsp_ch_num;
extern int dsp_ss_num;
#define PROC_VOIP_CH_NUM			( dsp_ch_num )
#define PROC_VOIP_SS_NUM			( dsp_ss_num )
#endif

/*
 * Create entries for voip (indirect to be cross platform)
 */
extern struct proc_dir_entry *create_voip_proc_read_entry(const char *name,
	mode_t mode, struct proc_dir_entry *base,
	read_proc_t *read_proc, void * data);
	
extern struct proc_dir_entry *create_voip_proc_rw_entry(const char *name, mode_t mode,
						struct proc_dir_entry *parent,
						read_proc_t * read_proc, write_proc_t * write_proc);

/*
 * Create entries for channel or session 
 */
extern void create_voip_channel_proc_read_entry(
	const char * name,
	read_proc_t * read_proc );

extern void create_voip_channel_proc_rw_entry(
	const char * name,
	read_proc_t * read_proc,
	write_proc_t * write_proc );

extern void create_voip_session_proc_read_entry(
	const char * name,
	read_proc_t * read_proc );

/*
 * Remove entries from channel or session 
 */
extern void remove_voip_proc_entry(
	const char * name , struct proc_dir_entry *parent);
	
extern void remove_voip_channel_proc_entry(
	const char * name );

extern void remove_voip_session_proc_entry(
	const char * name );

#endif /* __VOIP_PROC_H__ */

