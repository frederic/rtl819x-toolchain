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
# $RCSfile: pbrc_protocol.c,v $
#
# $Date: 2009-01-14 06:02:04 $
#
# $Revision: 1.4 $
#-------------------------------------------------------------------------------
# PBRC protocol for Dallas Controller
#-------------------------------------------------------------------------------
*/


/*Includings*/
#include "pbrc_protocol.h"
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/socket.h>
#include <linux/net.h>
#include <linux/in.h>
#include <net/sock.h>

#include <linux/types.h>
#include <linux/sched.h>
#include <linux/sockios.h>
#include <linux/errno.h>
#include <linux/time.h>
#include <linux/jiffies.h>

#include "ksocket.h"
#include "pmc_typs.h"
#include "dallas_util.h"

#if 0
#define PBRC_DEBUG 1
#endif

/* debug macro*/
#ifdef  PBRC_DEBUG
#	define pbrc_debug(fmt, args...)	printk(fmt, ##args)
#else
#	define pbrc_debug(fmt, args...)
#endif

struct cmd_msg_t {
	unsigned long command;
	unsigned long address;
	unsigned long length;
	unsigned long mask;    /* mask for set field command */
	unsigned long value;   /* value for set field command */
	unsigned long timeout;
	unsigned long enqueue_time;    /* the jitties that the command enqueue */
	unsigned long check_time;	/*the jitties that the command sent out next time */
	unsigned long *p_data;	/* start address to place the reply data */
	unsigned long ioctl_number; /* ioctl number for RPC operation */
	unsigned long * legal_size; /* legal size for RPC operation */
	long * status;		/* status for RPC operation */
};

struct msgbuf_pbrc_t {
	unsigned int errorno;
	unsigned long snd_count;   /* counter of sending */
	unsigned int waiting_frameid;	/*frame ID that waiting for the reply*/
	struct cmd_msg_t  *p_cmd_msg;	/*pointer to command message structure*/
	struct semaphore mutex_rply;    /*mutex for reply frame waiting*/
	struct msgbuf_pbrc_t *p_prev;
	struct msgbuf_pbrc_t *p_next;
}; /*this structure is used to transmit messages between command tasks and frame dispatching task*/

struct pbrc_queue {
	/*struct semaphore mutex;*/  /*mutex used to protect the pbrc_queue structure*/
	spinlock_t spinlock;
	int    msg_count;
	struct msgbuf_pbrc_t *p_head;
	struct msgbuf_pbrc_t *p_tail;    
};

/*Global variables*/
/* 
 * Bootrom cmd frame structure
 * 0xAA, 0xAA, 0xAA, 0xAA,  0xAA..AA, destination MAC address
 * 0xAA, 0xAA, 0xBB, 0xBB,  0xBB..BB, source MAC address
 * 0xBB, 0xBB, 0xBB, 0xBB, 
 * 0xCC, 0x00, 0x00, 0x00,  ethernet type, cmd:0x1200, reply:0x4000
 * 0x43, 0x52, 0x42, 0x50,  magic number, fixed value 
 * 0xDD, 0xDD, 0xDD, 0xDD,  opcode
 * ...
 */

/* 
 * RPC cmd frame structure
 * 0xAA, 0xAA, 0xAA, 0xAA,  0xAA..AA, destination MAC address
 * 0xAA, 0xAA, 0xBB, 0xBB,  0xBB..BB, source MAC address
 * 0xBB, 0xBB, 0xBB, 0xBB, 
 * 0xCC, 0x00,              ethernet type, cmd:0x1200, reply:0x4000
 * 0x00, 0x43, 0x50, 0x52,  magic number, fixed value 
 * 0xDD, 0xDD, 0xDD, 0xDD,  Frame ID
 * 0xEE, 0xEE, 0xEE, 0xEE, IOCTL number or STATUS
 * 0xFF, 0xFF, 0xFF, 0xFF, Legal size 
 * DATA...
 */
 
static unsigned char cmd_frame_buf[1518]={0};
static unsigned char rpl_frame_buf[1518]={0};
static long flag_sending_task = 1;

static unsigned char public_cmd_head[4] = {
0x12, 0x00, 0x00, 0x00,	//ethernet type, command
};

static unsigned char rpc_cmd_head[2] = {
0x12, 0x00,	//ethernet type, command
};

static unsigned char pbrc_magic_number[4] = {
0x43, 0x52, 0x42, 0x50,
};

static unsigned char rpc_magic_number[4] = {
0x52, 0x50, 0x43, 0x00,
};

struct pbrc_struct_t s_pbrc_struct={0};
struct pbrc_queue s_pbrc_queue;

static void pbrc_frame_sending_task(void *arg);
static void pbrc_frame_dispatching_task(void *arg);

static unsigned long convert_endian(unsigned long value)
{
	return (((value & 0xFF000000) >> 24) + ((value & 0x00FF0000) >> 8) + ((value & 0x0000FF00) << 8) + ((value & 0x000000FF) << 24));
}

/*PBRC queue functions*/
static long pbrcq_init()
{
    struct pbrc_queue *new_queue;
    new_queue = kmalloc(sizeof(struct pbrc_queue), GFP_KERNEL);
    
    if(new_queue==NULL) {
        /*memory allocation error*/
        return 0;
    } else {
        new_queue->msg_count=0;
        /*init_MUTEX (&new_queue->mutex);*/
        spin_lock_init(&new_queue->spinlock);
        new_queue->p_head = NULL;
        new_queue->p_tail = NULL;
    }

    return ((long)new_queue);
}

static long pbrcq_uninit(long queue_id)
{
	struct pbrc_queue *p_queue = (struct pbrc_queue *)queue_id;
	
	if(p_queue == NULL) {
		/*input queue not exist*/
		return -1;
	} else {
		spin_lock(&p_queue->spinlock);
		p_queue->msg_count = 0;
		spin_unlock(&p_queue->spinlock);

		kfree(p_queue);
	}
	        
	return 0;
}

static long pbrcq_enqueue(long queue_id, struct msgbuf_pbrc_t *p_cmd)
{
	struct pbrc_struct_t *p_pbrc = (struct pbrc_struct_t *)&s_pbrc_struct;
	struct pbrc_queue *p_queue = (struct pbrc_queue *)queue_id;
	
	if(p_queue == NULL) {
		/*input queue not exist*/
		return -1;
	}
		
	if(p_queue->msg_count == PBRC_QUEUE_MAX_LENGTH) {
		/*queue max length reached, return error*/
		return -1;
	}
	/*lock the mutex*/
	/*down(&p_queue->mutex);*/
	spin_lock(&p_queue->spinlock);

	p_cmd->snd_count = 0;
	p_cmd->p_cmd_msg->enqueue_time = jiffies;
	p_cmd->p_cmd_msg->check_time = jiffies;
	p_cmd->waiting_frameid = p_pbrc->cur_frameid;
	/*Update the current available frame ID*/
	p_pbrc->cur_frameid = (p_pbrc->cur_frameid == PBRC_MAX_FRAME_ID)?
		0: (p_pbrc->cur_frameid+1);
	/*attach the command to the queue*/
	if(p_queue->msg_count==0) {
		p_queue->p_head = p_cmd;
		p_queue->p_tail = p_cmd;
	} else {
		p_cmd->p_prev = p_queue->p_tail;
		p_queue->p_tail->p_next = p_cmd;
		p_queue->p_tail = p_cmd;        
	}
	
	p_queue->p_head->p_prev = NULL;
	p_queue->p_tail->p_next = NULL;
	p_queue->msg_count+=1;
	
	pbrc_debug("PBRC_queue: frame [%d] enqueued!\n", p_queue->p_head->waiting_frameid);
	
	spin_unlock(&p_queue->spinlock);

	p_pbrc->timer.expires = jiffies + (unsigned long)(PBRC_TIMEOUT_UNIT/10);
	if(p_queue->msg_count == 1 && p_pbrc->init_flag == true)
		add_timer(&p_pbrc->timer);

	/* send signal to sending kernel thread */
	up(&p_pbrc->mutex_send);
	
	return 0;
}

static long pbrcq_dequeue(long queue_id, struct msgbuf_pbrc_t **p_cmd, unsigned int frame_id)
{
	struct pbrc_struct_t *p_pbrc = (struct pbrc_struct_t *)&s_pbrc_struct;
	struct pbrc_queue *p_queue = (struct pbrc_queue *)queue_id;
	int rev = 0;

	if(p_queue == NULL) {
		/*input queue not exist*/
		return -1;
	}
	
	if(p_queue->msg_count == 0) {
		/*queue empty, return error*/
		return -1;
	}
	
	/*lock the mutex*/
	spin_lock(&p_queue->spinlock);


	if(p_queue->msg_count==0) {
		pbrc_debug("PBRC_queue: no command in queue!\n");
		spin_unlock(&p_queue->spinlock);
		return -1;
	} else {
		*p_cmd = p_queue->p_head;
		do {
			if((*p_cmd)->waiting_frameid == frame_id) {
				pbrc_debug("PBRC_dequeue: found the cmd!\n");
				break;
			}
			*p_cmd = (*p_cmd)->p_next;
		} while(*p_cmd != NULL);
	}

	if (*p_cmd != NULL) {
		if (p_queue->msg_count == 1) {
			p_queue->p_head=NULL;
			p_queue->p_tail=NULL;
		} else if ((*p_cmd)->p_next == NULL && (*p_cmd)->p_prev != NULL) {	/* tail of the queue */
			p_queue->p_tail = (*p_cmd)->p_prev;
			p_queue->p_tail->p_next = NULL;
		} else if ((*p_cmd)->p_prev == NULL && (*p_cmd)->p_next != NULL) {/* head of the queue */
			p_queue->p_head = (*p_cmd)->p_next;
			p_queue->p_head->p_prev = NULL;
		} else {
			(*p_cmd)->p_prev->p_next = (*p_cmd)->p_next;
			(*p_cmd)->p_next->p_prev = (*p_cmd)->p_prev;
		}
		p_queue->msg_count -= 1;
	} else {
		pbrc_debug("PBRC_queue: command not found!\n");
		rev = -1;
	}

	/*up(&p_queue->mutex);*/
	spin_unlock(&p_queue->spinlock);

	if(p_queue->msg_count == 0)
		del_timer(&p_pbrc->timer);

	return rev;
}

static inline long pbrcq_cmd_count(long queue_id)
{
	struct pbrc_queue *p_queue = (struct pbrc_queue *)queue_id;
	return p_queue->msg_count;
}

static ksocket_t pbrc_create_sock(char *str_var,struct net_device **dev)
{
	struct timeval tv;
	int rev = 0;
	ksocket_t sock;
	struct sockaddr_ll sll;
	
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,24))
{
	extern struct net init_net; //linux global variable
	*dev = (struct net_device *) dev_get_by_name(&init_net, str_var);
}
#else
	*dev = dev_get_by_name(str_var);
#endif
	if(*dev==NULL) {
		pbrc_debug("Invalid device [%s]!\n", str_var);
		return NULL;
	}  
	
	sock = ksocket(PF_PACKET, SOCK_RAW, htons(PBRC_ETHER_TYPE_RX));
	if(NULL == sock) {
		pbrc_debug("Error: create socket failed\n");
		return NULL;
	}
	pbrc_debug("sock=0x%x.\n", (unsigned long)sock);
	
	memset(&sll, 0, sizeof(sll)); 
	sll.sll_family = PF_PACKET;
	sll.sll_ifindex = (*dev)->ifindex;
	sll.sll_protocol = htons(PBRC_ETHER_TYPE_RX);
	
	if ((rev = kbind(sock, (struct sockaddr*)&sll, sizeof(struct sockaddr_ll))) < 0)
		printk("bind socket failed.\n");
	
	tv.tv_sec = 0;
	tv.tv_usec= PBRC_SOCKET_TIME;
	if ( 0 == ksetsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) ) {
		pbrc_debug("Set timeout of socket receiving successfully\n");
		return sock;
	} else {
		pbrc_debug("Warning: failed to set reveive timeout\n");    
		kclose(sock);
		return NULL;
	}
}

/*command timeout handler*/
void cmd_timeout_handler()
{
	struct pbrc_struct_t *p_pbrc = (struct pbrc_struct_t *)&s_pbrc_struct;
	struct pbrc_queue *p_queue = (struct pbrc_queue *)p_pbrc->msg_qid;
	struct msgbuf_pbrc_t *p_cmd;

	if(p_queue == NULL) {
		/*input queue not exist*/
		return;
	}
		
	if(p_queue->msg_count == 0) {
		/*queue empty, return */
		return;
	}
	/*lock the mutex*/
	spin_lock(&p_queue->spinlock);
	p_cmd = p_queue->p_head;
	do{
		if(time_after(jiffies, p_cmd->p_cmd_msg->enqueue_time + p_cmd->p_cmd_msg->timeout)) {
			if (p_queue->msg_count == 1) {
				p_queue->p_head=NULL;
				p_queue->p_tail=NULL;
			}
			else if(p_cmd->p_next == NULL && p_cmd->p_prev != NULL) {	/* tail of the queue */
				p_queue->p_tail = p_cmd->p_prev;
				p_queue->p_tail->p_next = NULL;
			}
			else if (p_cmd->p_prev == NULL && p_cmd->p_next != NULL) {	/* head of the queue */
				p_queue->p_head = p_cmd->p_next;
				p_queue->p_head->p_prev = NULL;
			} else {
				p_cmd->p_prev->p_next = p_cmd->p_next;
				p_cmd->p_next->p_prev = p_cmd->p_prev;
			}
			p_queue->msg_count -= 1;
			
			pbrc_debug("PBRC_dequeue: command timeout!\n");
			if (p_cmd->p_cmd_msg->command == PBRC_CMD_RPC) {
				p_cmd->errorno = RPC_S_TIMEOUT;
			} else {
				p_cmd->errorno = PBRC_E_TIMEOUT;
			}
			up(&p_cmd->mutex_rply);
		}
		p_cmd = p_cmd->p_next;
	}while(p_cmd != NULL);
	
	
	/*up(&p_queue->mutex);*/
	spin_unlock(&p_queue->spinlock);
	/* send signal to sending task */
	p_pbrc->timer.expires = jiffies + (unsigned long)(PBRC_TIMEOUT_UNIT/10);
	if (p_queue->msg_count > 0 && p_pbrc->init_flag == true) {
		add_timer(&p_pbrc->timer);
		up(&p_pbrc->mutex_send);
	} else {
		del_timer(&p_pbrc->timer);
	}
}

/*PBRC protocol functions*/
unsigned long pbrc_init(struct pbrc_initparam_t *p_initparam)
{
	int tid;
	int rev = 0;
	
	/*store Dallas parameters into PBRC structure*/
	
	/*Initialize the semaphores*/
	spin_lock_init(&s_pbrc_struct.spin_pbrc_rsrc);
	spin_lock_init(&s_pbrc_struct.spin_pbrc_indirect_access);
	init_MUTEX_LOCKED(&s_pbrc_struct.mutex_exit);
	init_MUTEX_LOCKED(&s_pbrc_struct.mutex_send_exit);
	init_MUTEX_LOCKED(&s_pbrc_struct.mutex_send);
	
	/*Initialize the PBRC message queue*/
	if( (s_pbrc_struct.msg_qid = pbrcq_init()) ==0) {
		pbrc_debug("Angela: message queue create fail!\n");
		return PBRC_E_INIT;
	}
	
	/*Update the command_frame_buf with MAC addresses*/
	memcpy(cmd_frame_buf, p_initparam->dallas_mac, 6);
	memcpy(&cmd_frame_buf[6], p_initparam->local_mac, 6);
	
	pbrc_debug("PBRC: Local MAC is %02x:%02x:%02x:%02x:%02x:%02x!\n", cmd_frame_buf[0+6],cmd_frame_buf[1+6],
	                            cmd_frame_buf[2+6],cmd_frame_buf[3+6],cmd_frame_buf[4+6],cmd_frame_buf[5+6] );
	
	/*Initialize the socket connection*/
	s_pbrc_struct.sock = pbrc_create_sock(p_initparam->dev_string, &(s_pbrc_struct.dev));
	if(NULL == s_pbrc_struct.sock) {
		pbrc_debug("Angela: socket init error!\n");
		return PBRC_E_INIT;
	}
	
	/*initialized the Linux timer*/
	init_timer(&s_pbrc_struct.timer);
	s_pbrc_struct.timer.function = cmd_timeout_handler;
	flag_sending_task = 0;
	/*create the reply dispatching task*/
	
	rev = kernel_thread_rt ("dallas_dispatch", &tid, 50, pbrc_frame_dispatching_task, NULL);
	if (tid < 0) {
		printk("Failed to launch kernel thread dallas_diapatch!.\n");
		return rev;
	}
	rev = kernel_thread_rt ("dallas_sending", &tid, 50, pbrc_frame_sending_task, NULL);
	if (tid < 0) {
		printk("Failed to launch kernel thread dallas_sending!.\n");
		return rev;
	}
	/*PBRC module initialized, set the flag*/
	s_pbrc_struct.init_flag=true;
	
	return rev;
}

unsigned long pbrc_set(unsigned long dst_addr, unsigned long *p_value, unsigned long length, unsigned long timeout, unsigned long user_id)
{
	struct pbrc_struct_t *p_pbrc = (struct pbrc_struct_t *)&s_pbrc_struct;
	unsigned long return_no;
	struct msgbuf_pbrc_t s_cmdmsg;
	struct cmd_msg_t s_cmd;
	
	/*check whether PBRC protocol module has been initialized*/
	if(!p_pbrc->init_flag)
		return PBRC_E_INIT;
	
	s_cmd.command = PBRC_CMD_SET;
	s_cmd.address = dst_addr;
	s_cmd.timeout = (unsigned long)(timeout/10);    /* convert to jiffies */
	s_cmd.length = length;
	s_cmd.p_data = p_value;
	
	s_cmdmsg.p_cmd_msg=&s_cmd;
	
	init_MUTEX_LOCKED(&s_cmdmsg.mutex_rply);
	
	/*lock spinlock*/
	spin_lock(&p_pbrc->spin_pbrc_rsrc);	
	if( pbrcq_enqueue(p_pbrc->msg_qid, &s_cmdmsg)<0) {
		pbrc_debug("Angela: command msg enqueue error!\n");
		
		/*unlock the spinlock*/
		spin_unlock(&p_pbrc->spin_pbrc_rsrc);
		return PBRC_E_UNKNOWN;
	}

	spin_unlock(&p_pbrc->spin_pbrc_rsrc);
	/*wait for reply frame*/
	down(&s_cmdmsg.mutex_rply);
	
	return_no = s_cmdmsg.errorno;
	
	return return_no;
}

unsigned long pbrc_set_field(unsigned long dst_addr, unsigned long value, unsigned long mask, unsigned long timeout, unsigned long user_id)
{
	struct pbrc_struct_t *p_pbrc = (struct pbrc_struct_t *)&s_pbrc_struct;
	unsigned long return_no;
	struct msgbuf_pbrc_t s_cmdmsg;
	struct cmd_msg_t s_cmd;
	
	/*check whether PBRC protocol module has been initialized*/
	if(!p_pbrc->init_flag)
		return PBRC_E_INIT;
	
	s_cmd.command = PBRC_CMD_SET_FIELD;
	s_cmd.address = dst_addr;
	s_cmd.timeout = (unsigned long)(timeout/10);    /* convert to jiffies */
	s_cmd.mask = mask;
	s_cmd.value = value;
	
	/*Update the command message structure*/
	s_cmdmsg.p_cmd_msg=&s_cmd;
	s_cmdmsg.p_cmd_msg->p_data = 0;
	
	init_MUTEX_LOCKED(&s_cmdmsg.mutex_rply);
	
	/*lock spinlock*/
	spin_lock(&p_pbrc->spin_pbrc_rsrc);
	if( pbrcq_enqueue(p_pbrc->msg_qid, &s_cmdmsg)<0) {
		pbrc_debug("Angela: command msg enqueue error!\n");

		/*unlock the spinlock*/
		spin_unlock(&p_pbrc->spin_pbrc_rsrc);
		return PBRC_E_UNKNOWN;
	}
	
	spin_unlock(&p_pbrc->spin_pbrc_rsrc);	
	/*wait for reply frame*/
	down(&s_cmdmsg.mutex_rply);
	return_no = s_cmdmsg.errorno;
	
	return return_no;
}

unsigned long pbrc_get(unsigned long src_addr, unsigned long *p_value, unsigned long length, unsigned long timeout, unsigned long user_id)
{
	struct pbrc_struct_t *p_pbrc = (struct pbrc_struct_t *)&s_pbrc_struct;
	unsigned long return_no;
	struct msgbuf_pbrc_t s_cmdmsg;
	struct cmd_msg_t s_cmd;
	/*check whether PBRC protocol module has been initialized*/
	if(!p_pbrc->init_flag)
		return PBRC_E_INIT;
	
	/*do the command processing*/    
	s_cmd.command = PBRC_CMD_GET;
	s_cmd.address = src_addr;
	s_cmd.timeout = (unsigned long)(timeout/10);    /* convert to jiffies */
	s_cmd.length = length;
	s_cmd.p_data = p_value;
	
	/*Update the command message structure*/
	s_cmdmsg.p_cmd_msg=&s_cmd;
	s_cmdmsg.p_cmd_msg->p_data = p_value;
	
	init_MUTEX_LOCKED(&s_cmdmsg.mutex_rply);
	/*lock spinlock*/
	spin_lock(&p_pbrc->spin_pbrc_rsrc);
	if( pbrcq_enqueue(p_pbrc->msg_qid, &s_cmdmsg)<0) {
		pbrc_debug("Angela: command msg enqueue error!\n");
		
		/*unlock the spinlock*/
		spin_unlock(&p_pbrc->spin_pbrc_rsrc);
		return PBRC_E_UNKNOWN;
	}
	spin_unlock(&p_pbrc->spin_pbrc_rsrc);
	/*wait for reply frame*/
	down(&s_cmdmsg.mutex_rply);
	return_no = s_cmdmsg.errorno;
	return return_no;
}

unsigned long pbrc_branch(unsigned long dst_addr, unsigned long timeout, unsigned long user_id)
{
	struct pbrc_struct_t *p_pbrc = (struct pbrc_struct_t *)&s_pbrc_struct;
	unsigned long return_no;
	struct msgbuf_pbrc_t s_cmdmsg;
	struct cmd_msg_t s_cmd;
	
	/*check whether PBRC protocol module has been initialized*/
	if(!p_pbrc->init_flag)
		return PBRC_E_INIT;
	
	s_cmd.command = PBRC_CMD_BRANCH;
	s_cmd.address = dst_addr;
	s_cmd.timeout = (unsigned long)(timeout/10);    /* convert to jiffies */
	
	/*Update the command message structure*/
	s_cmdmsg.p_cmd_msg=&s_cmd;
	s_cmdmsg.p_cmd_msg->p_data = 0;
	
	init_MUTEX_LOCKED(&s_cmdmsg.mutex_rply);
	
	/*lock spinlock*/
	spin_lock(&p_pbrc->spin_pbrc_rsrc);
	if( pbrcq_enqueue(p_pbrc->msg_qid, &s_cmdmsg)<0) {
		pbrc_debug("Angela: command msg enqueue error!\n");
		/*unlock the spinlock*/
		spin_unlock(&p_pbrc->spin_pbrc_rsrc);
		return PBRC_E_UNKNOWN;
	}

	spin_unlock(&p_pbrc->spin_pbrc_rsrc);	
	/*wait for reply frame*/
	down_interruptible(&s_cmdmsg.mutex_rply);
	
	return_no = s_cmdmsg.errorno;
	
	return return_no;
}

unsigned long pbrc_read_uni_mac(unsigned long src_addr, unsigned long *p_value, unsigned long timeout, unsigned long user_id)
{
	unsigned short offset = 0;
	struct pbrc_struct_t *p_pbrc = (struct pbrc_struct_t *)&s_pbrc_struct;
	unsigned long retval = 0;
	unsigned long read_command = 0;
	int is_busy = 1;
	int i;

	/* check parameter */
	if (( src_addr < UNI_0_CONFIG_BASE_ADDR ) ||( src_addr >= (UNI_1_CONFIG_BASE_ADDR + UNI_MAC_SIZE) )) {
		return PBRC_E_ADDRESS;
	}
	//get relevant part of address
	offset = src_addr & UNI_MAC_ADDR_OFFSET_MASK;
	read_command |= MA_GENERATE_BIT( 1, ETH_reg_mnem_READ_MAC_SC_OFFSET );
	read_command += offset;
	
	spin_lock(&p_pbrc->spin_pbrc_indirect_access);
	/* if the register is busy? */
	for (i = 0; i < 3; i ++) {
		
		retval = pbrc_get(ETH_reg_mnem_M_PM_83_STATUS, p_value, sizeof(unsigned long), timeout, user_id);
		if (retval != PBRC_OK) {
			continue;
		}
		is_busy = (*p_value) & 0x1;
		if ( !is_busy ) {
			break;
		}
	}
	if ( is_busy ) {
		spin_unlock(&p_pbrc->spin_pbrc_indirect_access);
		return (PBRC_E_UNKNOWN);
	}
	/* send address */
	retval = pbrc_set(ETH_reg_mnem_M_MAC_CONFIGURATION, &read_command, sizeof(unsigned long), timeout, user_id);
	/* check if it was written */
	for (i = 0; i < 3; i ++) {
		retval = pbrc_get(ETH_reg_mnem_M_PM_83_STATUS, p_value, sizeof(unsigned long), timeout, user_id);
		if (retval != PBRC_OK) {
			continue;
		}
		is_busy = (*p_value) & 0x1;
		if ( !is_busy ) {
			break;
		}
	}
	if ( is_busy ) {
		spin_unlock(&p_pbrc->spin_pbrc_indirect_access);
		return (PBRC_E_UNKNOWN);
	}
	/* get data */
	retval = pbrc_get(ETH_reg_mnem_M_79_DATA, p_value, sizeof(unsigned long), timeout, user_id);
	spin_unlock(&p_pbrc->spin_pbrc_indirect_access);
	
	return retval;
}

unsigned long pbrc_write_uni_mac(unsigned long dst_addr, unsigned long p_value, unsigned long timeout, unsigned long user_id)
{
	unsigned short offset = 0;
	struct pbrc_struct_t *p_pbrc = (struct pbrc_struct_t *)&s_pbrc_struct;
	unsigned long retval = 0;
	unsigned long write_command = 0;
	int is_busy = 1;
	int i;
	unsigned long tmp_data;

	if (( dst_addr < UNI_0_CONFIG_BASE_ADDR ) ||( dst_addr >= (UNI_1_CONFIG_BASE_ADDR + UNI_MAC_SIZE) )) {
		return PBRC_E_ADDRESS;
	}
	//get relevant part of address
	offset = dst_addr & UNI_MAC_ADDR_OFFSET_MASK;
	write_command |= MA_GENERATE_BIT( 1, ETH_reg_mnem_WRITE_MAC_SC_OFFSET );
	write_command += offset;
	
	spin_lock(&p_pbrc->spin_pbrc_indirect_access);
	/* if the register is busy? */
	for (i = 0; i < 3; i ++) {
		
		retval = pbrc_get(ETH_reg_mnem_M_PM_83_STATUS, &tmp_data, sizeof(unsigned long), timeout, user_id);
		if (retval != PBRC_OK) {
			continue;
		}
		is_busy = tmp_data & 0x1;
		if ( !is_busy ) {
			break;
		}
	}
	if ( is_busy ) {
		spin_unlock(&p_pbrc->spin_pbrc_indirect_access);
		return (PBRC_E_UNKNOWN);
	}
	/* send address & data*/
	retval = pbrc_set(ETH_reg_mnem_M_WR_DATA_MAC_CONFIGURATION, &p_value, sizeof(unsigned long), timeout, user_id);
	retval = pbrc_set(ETH_reg_mnem_M_MAC_CONFIGURATION, &write_command, sizeof(unsigned long), timeout, user_id);
	/* check if it was written */
	for (i = 0; i < 3; i ++) {
		
		retval = pbrc_get(ETH_reg_mnem_M_PM_83_STATUS, &tmp_data, sizeof(unsigned long), timeout, user_id);
		if (retval != PBRC_OK) {
			continue;
		}
		is_busy = tmp_data & 0x1;
		if ( !is_busy ) {
			break;
		}
	}
	if ( is_busy ) {
		spin_unlock(&p_pbrc->spin_pbrc_indirect_access);
		return (PBRC_E_UNKNOWN);
	}
	spin_unlock(&p_pbrc->spin_pbrc_indirect_access);

	return (PBRC_OK);
}

unsigned long pbrc_read_lag(unsigned long src_addr, unsigned long *p_value, unsigned long timeout, unsigned long user_id)
{
	unsigned short offset = 0;
	struct pbrc_struct_t *p_pbrc = (struct pbrc_struct_t *)&s_pbrc_struct;
	unsigned long retval = 0;
	unsigned long read_command = 0;
	int is_busy = 1;
	int i;

	/* check parameter */
	if (( src_addr < LAG_CONFIG_BASE_ADDR ) ||( src_addr >= (LAG_CONFIG_BASE_ADDR + LAG_SIZE) )) {
		return PBRC_E_ADDRESS;
	}
	
	offset = src_addr & LAG_ADDR_OFFSET_MASK;
	read_command |= MA_GENERATE_BIT( 1, ETH_reg_mnem_READ_LAG_SC_OFFSET );
	read_command += offset;
	
	spin_lock(&p_pbrc->spin_pbrc_indirect_access);
	
	/* if the register is busy? */
	for (i = 0; i < 3; i ++) {
		
		retval = pbrc_get(ETH_reg_mnem_M_PM_83_STATUS, p_value, sizeof(unsigned long), timeout, user_id);
		if (retval != PBRC_OK) {
			continue;
		}
		is_busy = (*p_value) & 0x1;
		if ( !is_busy ) {
			break;
		}
	}
	if ( is_busy ) {
		spin_unlock(&p_pbrc->spin_pbrc_indirect_access);
		return (PBRC_E_UNKNOWN);
	}
	/* send address */
	retval = pbrc_set(ETH_reg_mnem_M_LAG_CONFIGURATION, &read_command, sizeof(unsigned long), timeout, user_id);

	/* check if it was written */
	for (i = 0; i < 3; i ++) {
		
		retval = pbrc_get(ETH_reg_mnem_M_PM_83_STATUS, p_value, sizeof(unsigned long), timeout, user_id);
		if (retval != PBRC_OK) {
			continue;
		}
		is_busy = (*p_value) & 0x1;
		if ( !is_busy ) {
			break;
		}
	}
	if ( is_busy ) {
		spin_unlock(&p_pbrc->spin_pbrc_indirect_access);
		return (PBRC_E_UNKNOWN);
	}
	/* get data */
	retval = pbrc_get(ETH_reg_mnem_M_41_DATA, p_value, sizeof(unsigned long), timeout, user_id);

	spin_unlock(&p_pbrc->spin_pbrc_indirect_access);
	return retval;
}

unsigned long pbrc_write_lag(unsigned long dst_addr, unsigned long p_value, unsigned long timeout, unsigned long user_id)
{
	unsigned short offset = 0;
	struct pbrc_struct_t *p_pbrc = (struct pbrc_struct_t *)&s_pbrc_struct;
	unsigned long retval = 0;
	unsigned long write_command = 0;
	int is_busy = 1;
	int i;
	unsigned long tmp_data;
	

	/* check parameter */
	if (( dst_addr < LAG_CONFIG_BASE_ADDR ) ||( dst_addr >= (LAG_CONFIG_BASE_ADDR + LAG_SIZE) )) {
		return PBRC_E_ADDRESS;
	}
	
	offset = dst_addr & LAG_ADDR_OFFSET_MASK;
	/* prepare write data	*/
	write_command |= MA_GENERATE_BIT( 1, ETH_reg_mnem_WRITE_LAG_SC_OFFSET );
	write_command += offset;
	
	spin_lock(&p_pbrc->spin_pbrc_indirect_access);

	/* if the register is busy? */
	for (i = 0; i < 3; i ++) {
		
		retval = pbrc_get(ETH_reg_mnem_M_PM_83_STATUS, &tmp_data, sizeof(unsigned long), timeout, user_id);
		if (retval != PBRC_OK) {
			continue;
		}
		is_busy = tmp_data & 0x1;
		if ( !is_busy ) {
			break;
		}
	}
	if ( is_busy ) {
		spin_unlock(&p_pbrc->spin_pbrc_indirect_access);
		return (PBRC_E_UNKNOWN);
	}

	/* send address & data*/


	retval = pbrc_set(ETH_reg_mnem_M_WR_DATA_LAG_CONFIGURATION, &p_value, sizeof(unsigned long), timeout, user_id);
	retval = pbrc_set(ETH_reg_mnem_M_LAG_CONFIGURATION, &write_command, sizeof(unsigned long), timeout, user_id);

	/* check if it was written */
	for (i = 0; i < 3; i ++) {
		
		retval = pbrc_get(ETH_reg_mnem_M_PM_83_STATUS, &tmp_data, sizeof(unsigned long), timeout, user_id);
		if (retval != PBRC_OK) {
			continue;
		}
		is_busy = tmp_data & 0x1;
		if ( !is_busy ) {
			break;
		}
	}
	if ( is_busy ) {
		spin_unlock(&p_pbrc->spin_pbrc_indirect_access);
		return (PBRC_E_UNKNOWN);
	}
	spin_unlock(&p_pbrc->spin_pbrc_indirect_access);
	return (PBRC_OK);
}

unsigned long pbrc_dallas_rpc(unsigned long ioctl_number, unsigned long *legal_size, long *status, unsigned long * data, unsigned long timeout, unsigned long user_id)
{
	struct pbrc_struct_t *p_pbrc = (struct pbrc_struct_t *)&s_pbrc_struct;
	unsigned long return_no;
	struct msgbuf_pbrc_t s_cmdmsg;
	struct cmd_msg_t s_cmd;
	/*check whether PBRC protocol module has been initialized*/
	if(!p_pbrc->init_flag)
		return PBRC_E_INIT;
	
	/*do the command processing*/    
	s_cmd.command = PBRC_CMD_RPC;
	s_cmd.timeout = (unsigned long)(timeout/10);    /* convert to jiffies */
	s_cmd.legal_size = legal_size;
	s_cmd.ioctl_number = ioctl_number;
	s_cmd.p_data = data;
	s_cmd.status = status;
	s_cmdmsg.errorno = PBRC_OK;
	
	/*Update the command message structure*/
	s_cmdmsg.p_cmd_msg=&s_cmd;
	s_cmdmsg.p_cmd_msg->p_data = data;
	
	init_MUTEX_LOCKED(&s_cmdmsg.mutex_rply);
	/*lock spinlock*/
	spin_lock(&p_pbrc->spin_pbrc_rsrc);
	if( pbrcq_enqueue(p_pbrc->msg_qid, &s_cmdmsg)<0) {
		pbrc_debug("Angela: command msg enqueue error!\n");
		
		/*unlock the spinlock*/
		spin_unlock(&p_pbrc->spin_pbrc_rsrc);
		return PBRC_E_UNKNOWN;
	}
	spin_unlock(&p_pbrc->spin_pbrc_rsrc);
	/*wait for reply frame*/
	down_interruptible(&s_cmdmsg.mutex_rply);
	return_no = s_cmdmsg.errorno;
	if (return_no == RPC_S_TIMEOUT ) {
		*status = RPC_S_TIMEOUT;
	}
	
	return return_no;
}

unsigned long pbrc_uninit()
{
	struct pbrc_struct_t *p_pbrc = (struct pbrc_struct_t *)&s_pbrc_struct;
	
	/*check whether PBRC protocol module has been initialized*/
	if(!p_pbrc->init_flag)
		return PBRC_E_INIT;

	/*clear the init flag*/
	/*release all the pending tasks ??*/
	p_pbrc->init_flag = false;
	/* delete timer */
	del_timer(&p_pbrc->timer);
	/*uninitial the command queue*/
	pbrcq_uninit(p_pbrc->msg_qid);
	/* send message to quit thread sending */
	up(&p_pbrc->mutex_send);
	schedule();
	down(&p_pbrc->mutex_send_exit);
	down(&p_pbrc->mutex_exit);
	
	
	/*release the socket connection*/
	kclose(p_pbrc->sock);
	
	pbrc_debug("Angela: PBRC module uninitialized!\n");
	
	return PBRC_OK;
}

static void pbrc_frame_sending_task(void *arg)
{
	struct pbrc_struct_t *p_pbrc = (struct pbrc_struct_t *)&s_pbrc_struct;
	struct pbrc_queue *p_queue = (struct pbrc_queue *)p_pbrc->msg_qid;
	unsigned long dst_addr, length;
	struct msgbuf_pbrc_t *p_cmd;
	struct msgbuf_pbrc_t *temp_cmd;
	int snd_bytes = 60;
	int i;
	unsigned long	frame_cmd_head[4]={0};
	unsigned long	rpc_frame_cmd_head[3]={0};
	unsigned int frame_id;
	int rev;
	unsigned long *p_data;
	
	pbrc_debug("Sending task runningn!\n\n");
	for (;;)
	{
		if ((rev =down_interruptible(&p_pbrc->mutex_send)) != 0) {
			/* set the flag for terminating the dispatching task */
			p_pbrc->init_flag = false;
		}
		
		if (p_pbrc->init_flag == false) {
			pbrc_debug("pbrc_frame_sending_task exiting!\n");
			up(&p_pbrc->mutex_send_exit);
			return;
		}
		
		if(p_queue->msg_count==0) {
			pbrc_debug("pbrc_frame_sending_task: no command in queue!\n");
			continue;
		}

		spin_lock(&p_pbrc->spin_pbrc_rsrc);
		p_cmd = p_queue->p_head;
		do{
			snd_bytes = 60;
			if (p_cmd->snd_count == 0 || (time_after(jiffies, p_cmd->p_cmd_msg->check_time)
				&& time_before(jiffies, p_cmd->p_cmd_msg->enqueue_time + p_cmd->p_cmd_msg->timeout))) {
				if (p_cmd->p_cmd_msg->command != PBRC_CMD_RPC) {				
					dst_addr = p_cmd->p_cmd_msg->address;
					length = p_cmd->p_cmd_msg->length;
					frame_id = p_cmd->waiting_frameid;
					frame_cmd_head[0] = p_cmd->p_cmd_msg->command;
					/* add frame id */
					frame_cmd_head[0] |= (frame_id & 0xFFFF) << 8;
					/* add ack flag */
					if (p_cmd->p_cmd_msg->timeout > 0) {
						frame_cmd_head[0] |= 0x80 << 24; /*need ack*/
					}
					
					frame_cmd_head[1] = dst_addr;
					if (p_cmd->p_cmd_msg->command == PBRC_CMD_SET || p_cmd->p_cmd_msg->command == PBRC_CMD_GET) {
						frame_cmd_head[2] = length;
					}
					
					if (p_cmd->p_cmd_msg->command == PBRC_CMD_SET_FIELD) {
						frame_cmd_head[2] = p_cmd->p_cmd_msg->mask;
						frame_cmd_head[3] = p_cmd->p_cmd_msg->value;
					}
					/* convert endian */
					for (i = 0; i < 4; i++) {
						frame_cmd_head[i] = convert_endian(frame_cmd_head[i]);
					}
					memset(&cmd_frame_buf[12], 0, sizeof(cmd_frame_buf) - 12);
					memcpy(&cmd_frame_buf[12], public_cmd_head, 4);
					memcpy(&cmd_frame_buf[16], pbrc_magic_number, 4);
					memcpy(&cmd_frame_buf[20], frame_cmd_head, 4*sizeof(unsigned long));
					if (p_cmd->p_cmd_msg->command == PBRC_CMD_SET) {
						memcpy(&cmd_frame_buf[36], p_cmd->p_cmd_msg->p_data, length);
						snd_bytes = 36+length;
						
						/* convert endian */
						p_data = (unsigned long *)(&cmd_frame_buf[36]);
						for (i = 0; i < length/4; i++) {
							*(p_data + i) = convert_endian(*(p_data + i));
						}
					}
				} else {
					memset(&cmd_frame_buf[12], 0, sizeof(cmd_frame_buf) - 12);
					memcpy(&cmd_frame_buf[12], rpc_cmd_head, 2);
					memcpy(&cmd_frame_buf[14], rpc_magic_number, 4);
					
					memset(rpc_frame_cmd_head, 0, sizeof(rpc_frame_cmd_head));
					rpc_frame_cmd_head[0] = p_cmd->waiting_frameid;
					rpc_frame_cmd_head[1] = p_cmd->p_cmd_msg->ioctl_number;
					length = ((*p_cmd->p_cmd_msg->legal_size % 4) == 0) ? *p_cmd->p_cmd_msg->legal_size : (*p_cmd->p_cmd_msg->legal_size + 4 - *p_cmd->p_cmd_msg->legal_size % 4);
					rpc_frame_cmd_head[2] = *p_cmd->p_cmd_msg->legal_size;
					memcpy(&cmd_frame_buf[18], rpc_frame_cmd_head, sizeof(rpc_frame_cmd_head));
					memcpy(&cmd_frame_buf[18 + sizeof(rpc_frame_cmd_head)], p_cmd->p_cmd_msg->p_data, length);
					/* convert endian */
					p_data = (unsigned long *)(&cmd_frame_buf[18]);
					snd_bytes = 18 + sizeof(rpc_frame_cmd_head) + length;
				}
				
				if (snd_bytes < 60) {
					snd_bytes = 60;
				} else if (snd_bytes > 1518) {
					snd_bytes = 1518;
				}
				
				pbrc_debug("\nSending packet, length=%ld, snd_bytes=%d:\n", length, snd_bytes);
				for(i = 0; i < 60; i++) {
					if (i%4 == 0 && i != 0)
						pbrc_debug("\n");
					pbrc_debug("%02x ", cmd_frame_buf[i]);
				}
				pbrc_debug("\n\n");
				
				/* send the packet */
				if( (rev = ksend(p_pbrc->sock, cmd_frame_buf, snd_bytes, 0)) < 0 ) {
					pbrc_debug("Error: send packets error, rev=%d.\n", rev);
					/* do nothing, just ignore the error */
				}
				
				p_cmd->snd_count += 1;
				/*Update cmd structure*/
				p_cmd->p_cmd_msg->check_time += PBRC_TIMEOUT_UNIT/10;
				
				if (p_cmd->p_cmd_msg->timeout == 0) {
					pbrcq_dequeue(p_pbrc->msg_qid, &temp_cmd, p_cmd->waiting_frameid);
					up(&p_cmd->mutex_rply);
				}
			}
			p_cmd = p_cmd->p_next;
		}while(p_cmd != NULL);

		spin_unlock(&p_pbrc->spin_pbrc_rsrc);
	}
}

static void pbrc_frame_dispatching_task(void *arg)
{
	struct pbrc_struct_t *p_pbrc = (struct pbrc_struct_t *)&s_pbrc_struct;
	int rcv_byte, i;
	int pkt_length;
	unsigned short rcv_frame_id;
	struct msgbuf_pbrc_t *p_temp;
	unsigned long *p_reply;
	unsigned long opcode = 0;
	unsigned long error_code = 0;
	unsigned long magic_number = 0;
	unsigned long length = 0;
	long status = 0;
	
	pbrc_debug("Angela: dispatching task running!\n\n");
	/* waiting for reply packet */
	do {
		/*clear the reply frame buffer*/
		memset(rpl_frame_buf, 0x00, sizeof(rpl_frame_buf)); 
		
		/*waiting for the frame from Dallas*/
		rcv_byte = krecv(p_pbrc->sock, rpl_frame_buf, sizeof(rpl_frame_buf),0); 
		
		if(rcv_byte>0) {
			pbrc_debug("PBRC: received %d bytes!\n", rcv_byte);
			for(i = 0; i < rcv_byte; i++) {
				if (i%4 == 0 && i != 0)
				pbrc_debug("\n");
				pbrc_debug("%02x ", rpl_frame_buf[i]);
			}
			pbrc_debug("\n\n");
			p_reply = (unsigned long *)(&rpl_frame_buf[16]);
			magic_number = *p_reply;
			magic_number = convert_endian(magic_number);
			
			

			if (magic_number == PBRC_MAGIC_NUMBER) {
				p_reply = (unsigned long *)(&rpl_frame_buf[20]);
				/* convert endian */
				pkt_length = ((rcv_byte - 20)%4 == 0 )? rcv_byte : (rcv_byte + 4 - (rcv_byte - 20)%4) ;
				pkt_length = (pkt_length > 1516)? 1516 : pkt_length;
				for (i = 0; i < (pkt_length -20)/4; i++) {
					*(p_reply + i) = convert_endian(*(p_reply + i));
				}
				opcode = *p_reply;
				error_code = *(p_reply + 1);
				pbrc_debug("opcode = 0x%08lx, error_code=0x%08lx.\n", opcode, error_code);
				/*check whether this is a valid frame*/
				if((opcode & PBRC_OPCODE_FLAG_ERR) || (opcode & PBRC_OPCODE_FLAG_ACK)) {
					/*valid reply frame received*/
					
					rcv_frame_id = (opcode << 8) >> 16;
					pbrc_debug("PBRC: rcv_frame_id = 0x%x!\n", rcv_frame_id);
					/*use spinlock instead*/
					spin_lock(&p_pbrc->spin_pbrc_rsrc);

					if (pbrcq_dequeue(p_pbrc->msg_qid, &p_temp, rcv_frame_id) < 0) {
						pbrc_debug("PBRC: error frame id = 0x%x!\n", rcv_frame_id);
						spin_unlock(&p_pbrc->spin_pbrc_rsrc);
						continue;
					}
	
					pbrc_debug("PBRC: reply frame head is: 0x%08lx,\n0x%08lx,\n0x%08lx,\n0x%08lx,\n",
					*(p_reply + 0),
					*(p_reply + 1),
					*(p_reply + 2),
					*(p_reply + 3));
					
					if((opcode & PBRC_CMD_GET) && (!(opcode & PBRC_OPCODE_FLAG_ERR))) {
						memcpy(p_temp->p_cmd_msg->p_data, p_reply + 4, p_temp->p_cmd_msg->length);
						pbrc_debug("PBRC: reply data: 0x%lx\n",p_temp->p_cmd_msg->p_data[0]);
					}
					
					if (opcode & PBRC_OPCODE_FLAG_ERR) {
						p_temp->errorno = (error_code <= 3 )? (PBRC_E_UNKNOWN + error_code) : PBRC_E_UNKNOWN;
						pbrc_debug("PBRC ack: error ACK %d!\n", p_temp->errorno);
					} else {
						p_temp->errorno = PBRC_OK;
					}
					/*unlock the spinlock*/       
					spin_unlock(&p_pbrc->spin_pbrc_rsrc);
					up(&p_temp->mutex_rply); 
				} else {
					/*invalid frame received, simply drop it*/ 
					/*Qestion: should we add a schedule() here?*/
					
					pbrc_debug("PBRC: error frame got! Opcode_Flag=0x%lx\n", opcode & 0xFF);
				}
			} else  {
				p_reply = (unsigned long *)(&rpl_frame_buf[18]);
				/* convert endian */
				pkt_length = ((rcv_byte - 18)%4 == 0 )? rcv_byte : (rcv_byte + 4 - (rcv_byte - 18)%4) ;
				pkt_length = (pkt_length > 1516)? 1516 : pkt_length;
				rcv_frame_id = *p_reply;
				spin_lock(&p_pbrc->spin_pbrc_rsrc);
				if (pbrcq_dequeue(p_pbrc->msg_qid, &p_temp, rcv_frame_id) < 0) {
					pbrc_debug("PBRC: error frame id = 0x%x!\n", rcv_frame_id);
					spin_unlock(&p_pbrc->spin_pbrc_rsrc);
					continue;
				}
				
				status = (long)(*(p_reply + 1));
				length = *(p_reply + 2);
				*p_temp->p_cmd_msg->status = status;
				*p_temp->p_cmd_msg->legal_size = length;
				pbrc_debug("PBRC: rcv_frame_id = 0x%x, status=%ld, length=%ld.\n", rcv_frame_id, status, length);
				memcpy(p_temp->p_cmd_msg->p_data, p_reply + 3, length);
				/*unlock the spinlock*/       
				spin_unlock(&p_pbrc->spin_pbrc_rsrc);
				up(&p_temp->mutex_rply); 
			}
		} else {	/*rcv_byte>0*/
			schedule();
		}
	}while( p_pbrc->init_flag );
	
	pbrc_debug("pbrc_frame_dispatching_task exiting!\n");
	/*task exit*/
	up(&p_pbrc->mutex_exit);
	return ;
}

EXPORT_SYMBOL(pbrc_set);
EXPORT_SYMBOL(pbrc_set_field);
EXPORT_SYMBOL(pbrc_get);
EXPORT_SYMBOL(pbrc_branch);
EXPORT_SYMBOL(pbrc_write_uni_mac);
EXPORT_SYMBOL(pbrc_read_uni_mac);
EXPORT_SYMBOL(pbrc_write_lag);
EXPORT_SYMBOL(pbrc_read_lag);
EXPORT_SYMBOL(pbrc_dallas_rpc);
