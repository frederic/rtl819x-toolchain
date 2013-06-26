#ifndef __AIPC_DATA_SHM_H__
#define __AIPC_DATA_SHM_H__

#include "soc_type.h"

/*****************************************************************************
*   Data Plane
*****************************************************************************/
/*
*	Mailbox related define
*/
//	CPU -> DSP
#if 1	//this mechanism could need more memory space. use union can save more

#define TODSP_MAIL_TOTAL (64*4+1)	//jitter buffer*active sessions+1. left 1 for circle queue
#define TODSP_MAIL_SIZE  160		//max pcm data size

typedef struct _todsp_mail{
	u8_t	m[TODSP_MAIL_TOTAL][TODSP_MAIL_SIZE];
} todsp_mail_t;

#else	//can save more space using the following union mechanism

#if 0
typedef struct g711u g711u_t;
typedef struct g711a g711a_t;
typedef struct g729  g729_t;
typedef struct g7222 g7222_t;

typedef union _todsp_mail{
	g711u_t m[TODSP_MAIL_TOTAL];
	g711a_t n[TODSP_MAIL_TOTAL];
	g729_t  o[TODSP_MAIL_TOTAL];
	g7222_t p[TODSP_MAIL_TOTAL];
} todsp_mail_t;
#endif

#endif

typedef struct _todsp_mbox{
	void * 	todsp_mb[TODSP_MAIL_TOTAL];	//CPU -> DSP Mailbox Array
	u32_t	todsp_mb_ins;
	u32_t	todsp_mb_del;
	void * 	todsp_bc[TODSP_MAIL_TOTAL];	//CPU -> DSP Buffer Circulation Array
	u32_t	todsp_bc_ins;
	u32_t	todsp_bc_del;
	todsp_mail_t todsp_mail;			//CPU -> DSP mail
} todsp_mbox_t;

//	CPU <- DSP
#define TOCPU_MAIL_TOTAL	(4+1)	//Active session number. left 1 for circle queue
//#define ETH_DSP_HDR			40		//need to check real size in SD4 design
//#define ETH_RTP_PAYLOAD		1500							//MAX Ethernet MTU
//#define TOCPU_MAIL_SIZE  (ETH_DSP_HDR+(ETH_RTP_PAYLOAD*2))	//max payload size
#define	TOCPU_MAIL_SIZE		2048

typedef struct _tocpu_mail{
	u8_t	m[TOCPU_MAIL_TOTAL][TOCPU_MAIL_SIZE];
}  tocpu_mail_t;

typedef struct _tocpu_mbox{
	void * 	tocpu_mb[TOCPU_MAIL_TOTAL];//CPU <- DSP Mailbox Array
	u32_t	tocpu_mb_ins;
	u32_t	tocpu_mb_del;
	void * 	tocpu_bc[TOCPU_MAIL_TOTAL];//CPU <- DSP Buffer Circulation Array
	u32_t	tocpu_bc_ins;
	u32_t	tocpu_bc_del;
	tocpu_mail_t tocpu_mail;			//CPU <- DSP mail
} tocpu_mbox_t;

/*
*	AIPC interrupt related define
*/
#define TODSP_HIQ_SIZE 		64
#define TODSP_LOWQ_SIZE 	64
#define TOCPU_HIQ_SIZE		64
#define TOCPU_LOWQ_SIZE		64

//	AIPC Interrupt Queue CPU -> DSP
typedef struct _todsp_int_q{
	u32_t 	todsp_int_hiq[TODSP_HIQ_SIZE]; 		//Interrupt High Priority Queue
	u32_t	todsp_int_hiq_ins;
	u32_t	todsp_int_hiq_del;
	u32_t 	todsp_int_lowq[TODSP_LOWQ_SIZE];	//Interrupt Low Priority Queue
	u32_t	todsp_int_lowq_ins;
	u32_t	todsp_int_lowq_del;
} todsp_int_q_t;

//	AIPC Interrupt Queue CPU <- DSP
typedef struct _tocpu_int_q{
	u32_t 	tocpu_int_hiq[TOCPU_HIQ_SIZE]; 	//Interrupt High Priority Queue
	u32_t	tocpu_int_hiq_ins;
	u32_t	tocpu_int_hiq_del;
	u32_t 	tocpu_int_lowq[TOCPU_LOWQ_SIZE];	//Interrupt Low Priority Queue
	u32_t	tocpu_int_lowq_ins;
	u32_t	tocpu_int_lowq_del;
} tocpu_int_q_t;

/*
*	Data plane related structure
*/
//	Data plane related shared memory structure
typedef struct _aipc_data_shm{
	todsp_mbox_t	todsp_mbox;			//CPU -> DSP mailbox
	tocpu_mbox_t	tocpu_mbox;			//CPU <- DSP mailbox

	todsp_int_q_t	todsp_int_q;		//CPU -> DSP interrupt queue
	tocpu_int_q_t	tocpu_int_q;		//CPU <- DSP interrupt queue
} aipc_data_shm_t; 

extern aipc_data_shm_t aipc_data_shm;


/*****************************************************************************
*   Control Plane
*****************************************************************************/
#define AIPC_CTRL_BUF_LEN		4096
#define AIPC_CTRL_BUF_NUM		8

#define AIPC_EVENT_BUF_LEN		512
#define AIPC_EVENT_BUF_NUM		8

typedef enum cmd_type_e {
    TYPE_POST = 0 ,
    TYPE_POST_GET ,		// FIXME: Need review  
	TYPE_MAX
}cmd_type_t;

typedef enum event_type_e {
    EVENT_POST = 0 ,
    EVENT_POST_GET ,	// FIXME: Need review  
	EVENT_MAX
}event_type_t;

typedef enum owned_by_e{
    OWN_NONE = 0,
	OWN_CPU_ALLOC,
    OWN_CPU,
	OWN_DSP_ALLOC,
    OWN_DSP,
	OWN_MAX
}owned_by_t;

typedef struct {
    u32_t  type		:	1;
    u32_t  own		:	3;
    u32_t  cseq 	:	12;
	u32_t  size		:	16;
    u8_t   buf[AIPC_CTRL_BUF_LEN];
} aipc_ctrl_buf_t;

typedef struct {
    u32_t  type		:	1;
    u32_t  own		:	3;
    u32_t  cseq 	:	12;
	u32_t  size		:	16;
    u8_t   buf[AIPC_EVENT_BUF_LEN];
} aipc_event_buf_t;


#if 1
extern aipc_ctrl_buf_t aipc_ctrl_buf[AIPC_CTRL_BUF_NUM];
#else
#if (AIPC_CTRL_BUF_NUM==8)
aipc_ctrl_buf_t aipc_ctrl_buf[AIPC_CTRL_BUF_NUM] = {
    [0] = {
        .type   =   TYPE_POST ,
        .own    =   OWN_NONE ,
        .cseq   =   0,
        .size   =   0,
        .buf    =   {0}
        },
    [1] = {
        .type   =   TYPE_POST ,
        .own    =   OWN_NONE ,
        .cseq   =   0,
        .size   =   0,
        .buf    =   {0}
        } 
    [2] = {
        .type   =   TYPE_POST ,
        .own    =   OWN_NONE ,
        .cseq   =   0,
        .size   =   0,
        .buf    =   {0}
        },
    [3] = {
        .type   =   TYPE_POST ,
        .own    =   OWN_NONE ,
        .cseq   =   0,
        .size   =   0,
        .buf    =   {0}
        } 
    [4] = {
        .type   =   TYPE_POST ,
        .own    =   OWN_NONE ,
        .cseq   =   0,
        .size   =   0,
        .buf    =   {0}
        },
    [5] = {
        .type   =   TYPE_POST ,
        .own    =   OWN_NONE ,
        .cseq   =   0,
        .size   =   0,
        .buf    =   {0}
        } 
    [6] = {
        .type   =   TYPE_POST ,
        .own    =   OWN_NONE ,
        .cseq   =   0,
        .size   =   0,
        .buf    =   {0}
        },
    [7] = {
        .type   =   TYPE_POST ,
        .own    =   OWN_NONE ,
        .cseq   =   0,
        .size   =   0,
        .buf    =   {0}
        } 
};
#else
#error "Error control buffer size"
#endif
#endif

#if 1
extern aipc_event_buf_t aipc_event_buf[AIPC_EVENT_BUF_NUM];
#else
#if (AIPC_EVENT_BUF_NUM==8)
aipc_event_buf_t aipc_event_buf[AIPC_EVENT_BUF_NUM] = {
    [0] = {
        .type   =   EVENT_POST ,
        .own    =   OWN_NONE ,
        .cseq   =   0,
        .size   =   0,
        .buf    =   {0}
        },
    [1] = {
        .type   =   EVENT_POST ,
        .own    =   OWN_NONE ,
        .cseq   =   0,
        .size   =   0,
        .buf    =   {0}
        } 
    [2] = {
        .type   =   EVENT_POST ,
        .own    =   OWN_NONE ,
        .cseq   =   0,
        .size   =   0,
        .buf    =   {0}
        },
    [3] = {
        .type   =   EVENT_POST ,
        .own    =   OWN_NONE ,
        .cseq   =   0,
        .size   =   0,
        .buf    =   {0}
        } 
    [4] = {
        .type   =   EVENT_POST ,
        .own    =   OWN_NONE ,
        .cseq   =   0,
        .size   =   0,
        .buf    =   {0}
        },
    [5] = {
        .type   =   EVENT_POST ,
        .own    =   OWN_NONE ,
        .cseq   =   0,
        .size   =   0,
        .buf    =   {0}
        } 
    [6] = {
        .type   =   EVENT_POST ,
        .own    =   OWN_NONE ,
        .cseq   =   0,
        .size   =   0,
        .buf    =   {0}
        },
    [7] = {
        .type   =   EVENT_POST ,
        .own    =   OWN_NONE ,
        .cseq   =   0,
        .size   =   0,
        .buf    =   {0}
        } 
};
#else
#error "Error control buffer size"
#endif
#endif

/*****************************************************************************
*  share memory initialization (CPU only)
*****************************************************************************/
extern int apic_init_shm( void );

/*****************************************************************************
*  addtional definition 
*****************************************************************************/
#define MAX_HANDLE_CNT	1

#endif /* __AIPC_DATA_SHM_H__ */

