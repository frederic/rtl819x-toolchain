/*
 *      Handling routines for Mesh in 802.11 SME (Station Management Entity)
 *
 *      PS: All extern function in ../8190n_headers.h
 */
#define _MESH_SECURITY_C_

#ifdef CONFIG_RTL8192CD
#include "../rtl8192cd/8192cd.h"
#include "../rtl8192cd/8192cd_headers.h"
#else
#include "../rtl8190/8190n.h"
#include "../rtl8190/8190n_headers.h"
#endif
#include "./mesh_security.h"

#ifdef CONFIG_RTK_MESH

void DOT11_InitQueue2(DOT11_QUEUE2 * q, int szMaxItem, int szMaxData)
{
	q->Head = 0;
	q->Tail = 0;
	q->NumItem = 0;
	q->MaxItem = szMaxItem;
	q->MaxData = szMaxData;
}

int DOT11_EnQueue2(unsigned long task_priv, DOT11_QUEUE2 *q, unsigned char *item, int itemsize)
{
	DRV_PRIV *priv = (DRV_PRIV *)task_priv;
	unsigned long flags;

	if(DOT11_IsFullQueue(q))
		return E_DOT11_QFULL;
	if(itemsize > q->MaxData)
		return E_DOT11_2LARGE;

	MESH_LOCK(lock_queue, flags);
	q->ItemArray[q->Tail].ItemSize = itemsize;
	memset(q->ItemArray[q->Tail].Item, 0, sizeof(q->ItemArray[q->Tail].Item));
	memcpy(q->ItemArray[q->Tail].Item, item, itemsize);
	q->NumItem++;
	if((q->Tail+1) == q->MaxItem)
		q->Tail = 0;
	else
		q->Tail++;

	MESH_UNLOCK(lock_queue, flags);
	return 0;
}

int DOT11_DeQueue2(unsigned long task_priv, DOT11_QUEUE2 *q, unsigned char *item, int *itemsize)
{
	DRV_PRIV *priv = (DRV_PRIV *)task_priv;
	unsigned long flags;

	if(DOT11_IsEmptyQueue(q))
		return E_DOT11_QEMPTY;

	MESH_LOCK(lock_queue, flags);
	memcpy(item, q->ItemArray[q->Head].Item, q->ItemArray[q->Head].ItemSize);
	*itemsize = q->ItemArray[q->Head].ItemSize;
	q->NumItem--;
	if((q->Head+1) == q->MaxItem)
		q->Head = 0;
	else
		q->Head++;

	MESH_UNLOCK(lock_queue, flags);
	return 0;
}

#endif
