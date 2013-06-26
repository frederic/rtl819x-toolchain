/*
 * In our coding, 'DD' is short for data dump. 
 * This kind of device is useful for dump arbitrary data especially 
 * PCM data, and it only needs very little cost. 
 * In kernel part, add a *SINGLE* function to tell this module the 
 * data you want to dump. 
 * In application, just use 'cat' command to read data from device. 
 *
 * Note: 
 *  1. This module doesn't know data structure and all are seen as 
 *     byte stream. 
 *  2. Data in single writing operation is seens as an element, so
 *     we write nothing if free buffer size is not enough. 
 */

#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/wait.h>
#include <linux/slab.h>
#include <linux/sched.h>

#include "rtk_voip.h"
#include "voip_init.h"
#include "voip_dev.h"

#define VOIP_DD_DEV_NAME	"DD"

#define NUM_OF_DD_DEV	( VOIP_DD_DEV_END - VOIP_DD_DEV_START + 1 )
#define DD_BUFF_SIZE	( 80 * 2 * 100 )	// 1 sec in narrow band mode 

// ==================================================================
// Data dump fifo 
// ==================================================================

enum {
	DDMODE_U2K 	= 0x0001,	// bit 0
	DDMODE_K2U	= 0x0002,	// bit 1
};

typedef struct {
	// sync. 
	wait_queue_head_t wq;
	// mode 
	unsigned int mode;
	// fifo
	int ri, wi;
	unsigned char data[ DD_BUFF_SIZE ];
} ddinst_t;

static ddinst_t *ddinst[ NUM_OF_DD_DEV ];

static int check_and_get_ddinst_idx( unsigned int minor )
{
	unsigned int n = minor - VOIP_DD_DEV_START;
	
	if( n < NUM_OF_DD_DEV )
		return n;
	
	return -1;
}

static int ddinst_write_internal( ddinst_t *pddinst, const char *buff, size_t count, struct file *filp )
{
	// If filp == NULL, write from kernel 
	size_t space, stage1_len, stage2_len;
	const int is_user = ( int )filp;	// if filp != NULL, it is user's command 
	int old_ri;

	// check mode 
	if( is_user && !( pddinst ->mode & DDMODE_U2K ) )
		return -EPERM;
			
	// wait until enough space for atomic writing 
	while( 1 ) {
		// calculate space 
		if( pddinst ->wi >= ( old_ri = pddinst ->ri ) ) {
			space = DD_BUFF_SIZE - pddinst ->wi + pddinst ->ri;
		} else {
			space = pddinst ->ri - pddinst ->wi;
		}
		
		// check whether space is enough 
		if( is_user ) {
			if( space > 1 ) {		// circular buffer must reserve 1 bytes 
				if( space <= count )
					count = space - 1;	// truncate writing data 
				
				break;	// do writing!! 
			}
		} else {
			if( space > count ) { 	// '=' because (wi==ri) indicates empty 
				break;	// do writing!! 
			} else {
				return -1;	// kernel return immediately 
			}
		}
		
		// here is user's process 
		if( filp ->f_flags & O_NONBLOCK )
			return -EAGAIN;
		
		// wait for ri becomes different, I will check again 
		if( wait_event_interruptible( pddinst ->wq, pddinst ->ri != old_ri ) )
			return -ERESTARTSYS;	/* signal: tell the fs layer to handle it */
	}
	
	// write it! (stage 1, wrap case)
	if( pddinst ->wi + count >= DD_BUFF_SIZE ) {
		stage1_len = DD_BUFF_SIZE - pddinst ->wi;
		if( is_user )
			copy_from_user( pddinst ->data + pddinst ->wi, buff, stage1_len );
		else
			memcpy		  ( pddinst ->data + pddinst ->wi, buff, stage1_len );
		
		pddinst ->wi = 0;
		buff += stage1_len;
	} else
		stage1_len = 0;
	
	// copy it! (stage 2)
	stage2_len = count - stage1_len;
	if( is_user )
		copy_from_user( pddinst ->data + pddinst ->wi, buff, stage2_len );
	else
		memcpy		  ( pddinst ->data + pddinst ->wi, buff, stage2_len );
	
	pddinst ->wi += stage2_len;
	
	// wake up reading process 
	if( !is_user )
		wake_up_interruptible( &pddinst ->wq );
	
	return count;
}

int ddinst_write( unsigned int minor, const void *buff, size_t count )
{
	int idx;
	int ret;
	ddinst_t *pddinst;
	
	// get instance  
	idx = check_and_get_ddinst_idx( minor );
	
	if( idx < 0 || ddinst[ idx ] == NULL )
		return -1;

	pddinst = ddinst[ idx ];
	
	// check mode 
	if( !( pddinst ->mode & DDMODE_K2U ) )
		return -1;
	
	if( ( ret = ddinst_write_internal( pddinst, buff, count, NULL ) ) < 0 )
		printk( "DWF%d ", minor );
	
	return ret;
}

static ssize_t ddinst_read_internal( ddinst_t *pddinst, char *buff, size_t count, struct file *filp )
{
	// If filp == NULL, read from kernel 
	size_t data_len, stage1_len, stage2_len;
	const int is_user = ( int )filp;	// if filp != NULL, it is user's command  
	
	// check mode 
	if( is_user && !( pddinst ->mode & DDMODE_K2U ) )
		return -EPERM;
	
	// kernel don't wait for data 
	if( !is_user && ( pddinst ->ri == pddinst ->wi ) )
		return -EAGAIN;	// no data, try again 
	
	// wait for some data (kernel must not enter this loop)
	while( pddinst ->ri == pddinst ->wi ) {
		
		if( filp ->f_flags & O_NONBLOCK )
			return -EAGAIN;
		
		if( wait_event_interruptible( pddinst ->wq, pddinst ->ri != pddinst ->wi ) )
			return -ERESTARTSYS;	/* signal: tell the fs layer to handle it */
	}
	
	// calculate data length 
	if( pddinst ->ri > pddinst ->wi )
		data_len = DD_BUFF_SIZE - pddinst ->ri + pddinst ->wi;
	else
		data_len = pddinst ->wi - pddinst ->ri;
	
	// kernel should atmoic reading 
	if( !is_user && data_len < count )
		return -EAGAIN;	// not enough data, try again 
	
	// truncate copy size  
	if( data_len > count )
		data_len = count;
	
	// copy it! (stage 1, wrap case)
	if( pddinst ->ri + data_len >= DD_BUFF_SIZE ) {
		stage1_len = DD_BUFF_SIZE - pddinst ->ri;
		if( is_user )
			copy_to_user( buff, pddinst ->data + pddinst ->ri, stage1_len );
		else
			memcpy		( buff, pddinst ->data + pddinst ->ri, stage1_len );
		
		pddinst ->ri = 0;
		buff += stage1_len;
	} else
		stage1_len = 0;
	
	// copy it! (stage 2)
	stage2_len = data_len - stage1_len;
	if( is_user )
		copy_to_user( buff, pddinst ->data + pddinst ->ri, stage2_len );
	else
		memcpy		( buff, pddinst ->data + pddinst ->ri, stage2_len );
	
	pddinst ->ri += stage2_len;

	// wake up writing process 
	if( !is_user )
		wake_up_interruptible( &pddinst ->wq );
	
	return data_len;
}

int ddinst_read( unsigned int minor, void *buff, size_t count )
{
	int idx;
	int ret;
	ddinst_t *pddinst;
	
	// get instance  
	idx = check_and_get_ddinst_idx( minor );
	
	if( idx < 0 || ddinst[ idx ] == NULL )
		return -1;
	
	pddinst = ddinst[ idx ];
	
	// check mode 
	if( !( pddinst ->mode & DDMODE_U2K ) )
		return -1;
	
	if( ( ret = ddinst_read_internal( pddinst, buff, count, NULL ) ) < 0 )
		;//printk( "DRE%d ", minor );
	
	return ret;
}

int ddinst_rw_auto( unsigned int minor, const void *buff, size_t count )
{
	int idx;
	int ret = -1;
	ddinst_t *pddinst;
	
	// get instance  
	idx = check_and_get_ddinst_idx( minor );
	
	if( idx < 0 || ddinst[ idx ] == NULL )
		return -1;
	
	pddinst = ddinst[ idx ];
	
	// check mode 
	if( pddinst ->mode & DDMODE_U2K ) {
		// kernel read 
		if( ( ret = ddinst_read_internal( pddinst, buff, count, NULL ) ) < 0 )
			;
	} else if( pddinst ->mode & DDMODE_K2U ) {
		// kernel write 
		if( ( ret = ddinst_write_internal( pddinst, buff, count, NULL ) ) < 0 )
			printk( "DWf%d ", minor );
	} 
	
	return ret;
}

static int ddinst_open( unsigned int minor, void **pp_pd, unsigned int mode )
{
	int idx;
	ddinst_t *pddinst;
	int ret;
	
	idx = check_and_get_ddinst_idx( minor );
	
	if( idx < 0 )
		return -ENODEV;
	
	// check inst exist? 
	if( ( pddinst = ddinst[ idx ] ) )
		return -EBUSY;
	
	ddinst[ idx ] = ( ddinst_t * )1;		// make it non-zero to avoid race condition 
	
	// allocate space 
	pddinst = kmalloc( sizeof( ddinst_t ), GFP_KERNEL );
	
	if( pddinst == NULL ) {
		ret = -ENOMEM;
		goto label_err;
	}
	
	// init wait queue
	init_waitqueue_head( &pddinst ->wq );
	
	// init fifo 
	pddinst ->ri = pddinst ->wi = 0;
	
	// set mode 
	pddinst ->mode = mode;
	
	// save pointer for reading and writing function 
	ddinst[ idx ] = *pp_pd = pddinst;
	
	return 0;

label_err:
	ddinst[ idx ] = 0;

	return ret;
}

static int ddinst_close( unsigned int minor, ddinst_t *pddinst )
{
	int idx;
		
	idx = check_and_get_ddinst_idx( minor );
	
	if( idx < 0 )
		return -ENODEV;
	
	// clean pointer 
	ddinst[ idx ] = NULL;
	
	// free memory 
	kfree( pddinst );
	
	return 0;
}

// ==================================================================
// dev f_op 
// ==================================================================

static ssize_t dd_read( struct file *filp, char *buff, size_t count, loff_t *offp )
{
	return ddinst_read_internal( ( ddinst_t * )filp ->private_data, buff, count, filp );
}

static ssize_t dd_write( struct file *filp, const char *buff, size_t count, loff_t *offp )
{
	return ddinst_write_internal( ( ddinst_t * )filp ->private_data, buff, count, filp );
}

static int dd_open( struct inode *node, struct file *filp )
{
	int ret;
	void *p_pd;	// private data 
	unsigned int minor;
	unsigned int mode;
	
	if( ( filp ->f_flags & O_ACCMODE ) == O_WRONLY )
		mode = DDMODE_U2K;		// support write only
	else if( ( filp ->f_flags & O_ACCMODE ) == O_RDONLY )
		mode = DDMODE_K2U;		// support read only
	else
		return -EINVAL;
	
	minor = MINOR( node ->i_rdev );
	
	ret = ddinst_open( minor, &p_pd, mode );
	
	if( ret < 0 )
		return ret;
	
	filp ->private_data = p_pd;
	
	return 0;
}

static int dd_close( struct inode *node, struct file *filp )
{
	int ret;
	unsigned int minor;
	
	minor = MINOR( node ->i_rdev );
	
	ret = ddinst_close( minor, ( ddinst_t * )filp ->private_data );
	
	if( ret < 0 )
		return ret;
	
	return 0;
}

static struct file_operations dd_fops = {
	read:		dd_read,
	write:		dd_write,
	open:		dd_open,
	release:	dd_close,
};

static int __init voip_dd_dev_init( void )
{
	int ret;
	
	ret = register_voip_chrdev( VOIP_DD_DEV_START, NUM_OF_DD_DEV, VOIP_DD_DEV_NAME, &dd_fops );
	
	if( ret < 0 )
		printk( "register data dump dev fail\n" );
	
	return 0;
}

static void __exit voip_dd_dev_exit( void )
{
	unregister_voip_chrdev( VOIP_DD_DEV_START, NUM_OF_DD_DEV );
}

voip_initcall( voip_dd_dev_init );
voip_exitcall( voip_dd_dev_exit );

