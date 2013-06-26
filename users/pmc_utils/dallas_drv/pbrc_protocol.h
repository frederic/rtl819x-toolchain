/*
#*******************************************************************************
# Copyright (C) 2006 PMC-Sierra Inc.  All Rights Reserved.
#-------------------------------------------------------------------------------
# This software embodies materials and concepts which are proprietary and
# confidential to PMC-Sierra, Inc.  PMC-Sierra distributes this software to
# its customers pursuant to the terms and conditions of the Software License
# Agreement contained in the text file software.lic that is distributed along
# with the software.  This software can only be utilized if all terms and
# conditions of the Software License Agreement are accepted.  If there are
# any questions, concerns, or if the Software License Agreement text file
# software.lic is missing, please contact PMC-Sierra for assistance.
#-------------------------------------------------------------------------------
# $RCSfile: pbrc_protocol.h,v $
#
# $Date: 2009-01-14 06:02:04 $
#
# $Revision: 1.4 $
#-------------------------------------------------------------------------------
# 
#-------------------------------------------------------------------------------
*/


#ifndef PBRC_PROTOCOL
#define PBRC_PROTOCOL

/*Includings*/
#if 0
#include <asm/semaphore.h>
#include <asm/spinlock.h>
#endif
#include <linux/semaphore.h>
#include <linux/spinlock.h>
#include <linux/syscalls.h>
#include <linux/timer.h>

#include "ksocket.h"
#include "pmc_typs.h"

/*Structure definitions*/

struct pbrc_struct_t {
	bool init_flag;     
	unsigned int cur_frameid;	/*current available frame ID to be assigned to new command frame*/
	spinlock_t spin_pbrc_rsrc;   /*spinlock used to protect the critical resource*/
	spinlock_t spin_pbrc_indirect_access;   /* spinlock for read/write UNI MAC and LAG */
	long msg_qid;   /*ID for message queue that buffers the waiting commands*/
	ksocket_t sock;
	struct net_device *dev; /*network device structure*/
	struct timer_list timer;    /*Linux timer for command timeout*/
	struct semaphore mutex_send;    /* mutex for sending packets */
	struct semaphore mutex_exit;    /*mutex for reply frame waiting*/
	struct semaphore mutex_send_exit;    /*mutex for sending task*/
};

struct pbrc_initparam_t {
	unsigned char dev_string[10];    /*name of the network device used to connect to Dallas*/
	unsigned char local_mac[30];   /*MAC address of Local network device*/
	unsigned char dallas_mac[30];  /*MAC address assigned for Dallas*/    
};

/*Function declarations*/
unsigned long pbrc_init(struct pbrc_initparam_t *p_initparam);

unsigned long pbrc_set(unsigned long dst_addr, unsigned long *p_value, unsigned long length, unsigned long timeout, unsigned long user_id);

unsigned long pbrc_set_field(unsigned long dst_addr, unsigned long value, unsigned long mask, unsigned long timeout, unsigned long user_id);

unsigned long pbrc_get(unsigned long src_addr, unsigned long *p_value, unsigned long length, unsigned long timeout, unsigned long user_id);

unsigned long pbrc_branch(unsigned long dst_addr, unsigned long timeout, unsigned long user_id);

unsigned long pbrc_read_lag(unsigned long src_addr, unsigned long *p_value, unsigned long timeout, unsigned long user_id);
unsigned long pbrc_write_lag(unsigned long dst_addr, unsigned long p_value, unsigned long timeout, unsigned long user_id);
unsigned long pbrc_read_uni_mac(unsigned long src_addr, unsigned long *p_value, unsigned long timeout, unsigned long user_id);
unsigned long pbrc_write_uni_mac(unsigned long dst_addr, unsigned long p_value, unsigned long timeout, unsigned long user_id);

unsigned long pbrc_dallas_rpc(unsigned long ioctl_number, unsigned long *legal_size, long *status, unsigned long * data, unsigned long timeout, unsigned long user_id);

unsigned long pbrc_uninit();

void dallas_daemonize(const char *name, ...);

#endif
