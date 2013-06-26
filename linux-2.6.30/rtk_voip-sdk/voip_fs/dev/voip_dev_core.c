#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/slab.h>

#include "rtk_voip.h"
#include "voip_dev.h"

// ========================================================
// minor dev link list structure  
// ========================================================

#define MAX_DEV_NAME_LEN	8

typedef struct voip_dev_list_s {
	unsigned int minor;
	unsigned int num;
	char name[ MAX_DEV_NAME_LEN + 1 ];
	const struct file_operations *fop;
	struct voip_dev_list_s *next;
	struct voip_dev_list_s *prev;
} voip_dev_list_t;

static voip_dev_list_t * voip_dev_list_head = NULL;

int sprintf_voip_dev_entry( char *buff, const char *format )
{
	int len = 0;
	voip_dev_list_t *list = voip_dev_list_head;
	
	while( list ) {
		
		len += sprintf( buff + len, format, 
						list ->minor, list ->num, list ->name );
			
		list = list ->next;
	}
	
	return len;
}

static voip_dev_list_t *search_voip_dev_entry( unsigned int minor, unsigned int num )
{
	voip_dev_list_t *list = voip_dev_list_head;
	
	while( list ) {
		
		if( ( minor >= list ->minor + list ->num ) ||
			( minor + num <= list ->minor ) )
		{
		} else
			return list;
			
		list = list ->next;
	}
	
	return NULL;
}

static voip_dev_list_t *get_voip_dev_entry( unsigned int minor )
{
	return search_voip_dev_entry( minor, 1 );
}

static voip_dev_list_t *add_voip_dev_entry( unsigned int minor, unsigned int num, const char *name, const struct file_operations *fop )
{
	voip_dev_list_t *entry;
	
	entry = kmalloc( sizeof( voip_dev_list_t ), GFP_KERNEL );
	
	if( entry == NULL )
		return NULL;
	
	if( voip_dev_list_head )
		voip_dev_list_head ->prev = entry;
	
	entry ->minor = minor;
	entry ->num = num;
	strncpy( entry ->name, name, MAX_DEV_NAME_LEN );
	entry ->name[ MAX_DEV_NAME_LEN ] = '\x0';
	entry ->fop = fop;
	entry ->next = voip_dev_list_head;
	entry ->prev = NULL;
	
	voip_dev_list_head = entry;
	
	return entry;
}

static int del_voip_dev_entry( unsigned int minor, unsigned int num )
{
	voip_dev_list_t *entry, *entry_prev, *entry_next;
	
	entry = search_voip_dev_entry( minor, num );
	
	if( entry == NULL )
		return -ENODEV;
	
	if( entry ->minor != minor || entry ->num != num )
		return -ENODEV;
	
	entry_prev = entry ->prev;
	entry_next = entry ->next;
	
	if( entry_prev == NULL ) {	// first one 
		voip_dev_list_head = entry_next;
		entry_next ->prev = NULL;
	} else {
		entry_prev ->next = entry_next;
		entry_next ->prev = entry_prev;
	}
	
	kfree( entry );
	
	return 0;
}

// ========================================================
// register / unregister function 
// ========================================================
int voip_dev_open( struct inode *node, struct file *filp )
{
	unsigned int minor = MINOR( node ->i_rdev );
	voip_dev_list_t *entry;
	
	entry = get_voip_dev_entry( minor );
	
	// no found
	if( entry == NULL )
		return -ENODEV;
	
	// set f_op to registered fop 
	filp ->f_op = entry ->fop;
	
	return filp ->f_op ->open( node, filp );
}

struct file_operations voip_dev_fops = {
	open:		voip_dev_open,
};

int register_voip_chrdev( unsigned int minor, unsigned int num, const char *name, const struct file_operations *fop )
{
	int do_register = ( voip_dev_list_head == NULL ? 1 : 0 );
	int ret = 0;
	
	if( num == 0 || fop == NULL )
		return -EINVAL;
	
	// check entry exist 
	if( search_voip_dev_entry( minor, num ) )
		return -EFAULT;
	
	// add entry 
	if( add_voip_dev_entry( minor, num, name, fop ) == NULL )
		return -ENOMEM;
	
	// if first entry, do chrdev register 
	if( do_register )
		ret = register_chrdev( VOIP_DEV_MAJOR, VOIP_DEV_NAME, &voip_dev_fops );
	
	if( ret < 0 ) {
		// if chrdev register fail, remove this entry 
		del_voip_dev_entry( minor, num );
		return ret;
	}
	
	return minor;	
}

void unregister_voip_chrdev( unsigned int minor, unsigned int num )
{
	// NOTE: (minor, num) should absolutely match!! 
	int may_unregister = ( voip_dev_list_head ? 1 : 0 );
	
	if( num == 0 )
		return /*-EINVAL*/;
	
	// remove entry 
	if( del_voip_dev_entry( minor, num ) < 0 )
		return;
	
	// if there is no entry, unregister chrdev 
	if( may_unregister && voip_dev_list_head == NULL )
		unregister_chrdev( VOIP_DEV_MAJOR, VOIP_DEV_NAME );
}

