#ifndef __CON_DEFER_INIT_H__
#define __CON_DEFER_INIT_H__

#include "voip_types.h"

typedef void ( *fn_defer_func_t )( uint32 p0, uint32 p1 );

typedef struct defer_init_s {
	fn_defer_func_t fn_defer_func;
	uint32 p0;
	uint32 p1;
	struct defer_init_s *next;
} defer_init_t;

// add initialization task that should defer 
extern void add_defer_initialization( defer_init_t *defer );

// complete defer initialization 
extern void complete_defer_initialization( void );

#endif /* __CON_DEFER_INIT_H__ */

