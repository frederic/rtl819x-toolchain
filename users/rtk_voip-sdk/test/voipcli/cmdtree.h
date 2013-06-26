#ifndef __CMD_TREE_H__
#define __CMD_TREE_H__

/* ================================================================ */
/* Arguments */
/* ================================================================ */

/* Arguments type */
typedef enum {
	ATYPE_NONE,
	ATYPE_IN_BEGIN,	/* -------- begin of input argument */
	ATYPE_IN,		/* 		numerical type */
	ATYPE_IN_STR,	/* 		string type */
	ATYPE_IN_BIN,	/*		binary type */
	ATYPE_IN_IA4,	/*		IP address (4bytes) */
	ATYPE_IN_END,	/* -------- end of input argument */
	ATYPE_OUT_BEGIN,/* -------- begin of output argument */
	ATYPE_OUT,		/* 		numerical type */
	ATYPE_OUT_END,	/* -------- end of output argument */
} atype_t;

/* Arguments */
typedef struct {
	const char * name;
	unsigned long offset;	/* structure offset */
	unsigned long size;		/* field size */
	atype_t type;
} args_t;

extern unsigned long aBufferSize;
extern unsigned char aBuffer[];

/* ================================================================ */
/* Internal node */
/* ================================================================ */

/* Node type */
typedef enum {
	NTYPE_NONE,
	NTYPE_INODE,	/* internal node */
	NTYPE_LEAF,		/* leaf */
} ntype_t;

/* Node */
typedef struct node_s {
	unsigned long id;
	const char * name;
	ntype_t type;
	const struct node_s *next;	/* if type is internal node, we can see its next node */
	const args_t *args;		/* if type is leaf, we can see its arguments */
	unsigned long asize;	/* argument structure size */
} node_t;

extern const node_t nRoot[];

/* ================================================================ */
/* IOCTL */
/* ================================================================ */
extern int CmdNodeExecute( const node_t *pNode );


#endif /* __CMD_TREE_H__  */

