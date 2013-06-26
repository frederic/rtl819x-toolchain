#include "aipc_ctrl_cpu.h"


// ==================================================
// ctrl / evnet 
// ==================================================
static int aipc_ctrl_event_init( void )
{
	extern int aipc_ctrl_init( void );
	int i;

	aipc_ctrl_init();

	//extern aipc_ctrl_buf_t aipc_ctrl_buf[AIPC_CTRL_BUF_NUM];

	for( i = 0; i < AIPC_CTRL_BUF_NUM; i ++ ) {
		aipc_ctrl_buf[ i ].type = TYPE_POST;
		aipc_ctrl_buf[ i ].own = OWN_NONE;
		aipc_ctrl_buf[ i ].cseq = 0;
		aipc_ctrl_buf[ i ].size = 0;
	}

	//extern aipc_event_buf_t aipc_event_buf[AIPC_EVENT_BUF_NUM];
	
	for( i = 0; i < AIPC_EVENT_BUF_NUM; i ++ ) {
		aipc_event_buf[ i ].type = EVENT_POST;
		aipc_event_buf[ i ].own = OWN_NONE;
		aipc_event_buf[ i ].cseq = 0;
		aipc_event_buf[ i ].size = 0;
	}

	return 0;
}

// ==================================================
// CPU -> DSP 
// ==================================================

static void
aipc_todsp_mb_init( void )
{
	aipc_data_shm.todsp_mbox.todsp_mb_ins = 0;
	aipc_data_shm.todsp_mbox.todsp_mb_del = 0;
}

static void
aipc_todsp_bc_init( void )
{
	int i;
	aipc_data_shm.todsp_mbox.todsp_bc_ins = TODSP_MAIL_TOTAL - 1;
	aipc_data_shm.todsp_mbox.todsp_bc_del = 0;
	
	for( i=0 ; i<TODSP_MAIL_TOTAL ; i++ ){
		aipc_data_shm.todsp_mbox.todsp_bc[i] = &aipc_data_shm.todsp_mbox.todsp_mail.m[i];
		//aipc_data_shm.todsp_mbox.todsp_bc_ins++;		
	}
}

static void
aipc_int_dsp_hiq_init( void )
{
	aipc_data_shm.todsp_int_q.todsp_int_hiq_ins = 0;
	aipc_data_shm.todsp_int_q.todsp_int_hiq_del = 0;
}

static void
aipc_int_dsp_lowq_init( void )
{
	aipc_data_shm.todsp_int_q.todsp_int_lowq_ins = 0;
	aipc_data_shm.todsp_int_q.todsp_int_lowq_del = 0;
}

static int aipc_todsp_mbox_init( void )
{
	aipc_todsp_mb_init();
	aipc_todsp_bc_init();

	aipc_int_dsp_hiq_init();
	aipc_int_dsp_lowq_init();
	
	return 0;
}

// ==================================================
// DSP -> CPU 
// ==================================================
static void
aipc_tocpu_mb_init( void )
{
	aipc_data_shm.tocpu_mbox.tocpu_mb_ins = 0;
	aipc_data_shm.tocpu_mbox.tocpu_mb_del = 0;
}

static void
aipc_tocpu_bc_init( void )
{
	int i;
	aipc_data_shm.tocpu_mbox.tocpu_bc_ins = TOCPU_MAIL_TOTAL - 1;
	aipc_data_shm.tocpu_mbox.tocpu_bc_del = 0;
	
	for( i=0 ; i<TOCPU_MAIL_TOTAL ; i++ ){
		aipc_data_shm.tocpu_mbox.tocpu_bc[i] = &aipc_data_shm.tocpu_mbox.tocpu_mail.m[i];
		//aipc_data_shm.tocpu_mbox.tocpu_bc_ins++;		
	}
}

static void
aipc_int_cpu_hiq_init( void )
{
	aipc_data_shm.tocpu_int_q.tocpu_int_hiq_ins = 0;
	aipc_data_shm.tocpu_int_q.tocpu_int_hiq_del = 0;
}

static void
aipc_int_cpu_lowq_init( void )
{
	aipc_data_shm.tocpu_int_q.tocpu_int_lowq_ins = 0;
	aipc_data_shm.tocpu_int_q.tocpu_int_lowq_del = 0;
}

static int aipc_tocpu_mbox_init( void )
{
	aipc_tocpu_mb_init();
	aipc_tocpu_bc_init();

	aipc_int_cpu_hiq_init();
	aipc_int_cpu_lowq_init();
	
	return 0;
}

// ==================================================
// ALL 
// ==================================================

int apic_init_shm( void )
{
	// CPU do this!! 
	
	// ctrl / event 
	aipc_ctrl_event_init();
	
	// cpu -> dsp mbox 
	aipc_todsp_mbox_init();
	
	// cpu <- dsp mbox 
	aipc_tocpu_mbox_init();
	
	return 0;
}

