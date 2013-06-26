/*++
Copyright (c) Realtek Semiconductor Corp. All rights reserved.

Module Name:
	Hal88XXTxDesc.c

Abstract:
	Defined RTL88XX HAL tx desc common function

Major Change History:
	When            Who                         What
	---------- ---------------   -------------------------------
	2012-03-29  Lun-Wu Yeh            Add PrepareTxDesc88XX().
--*/

#ifndef __ECOS
#include "HalPrecomp.h"
#else
#include "../HalPrecomp.h"
#endif

void DumpTxBDesc88XX(
    IN      HAL_PADAPTER    Adapter,
    IN      u4Byte          q_num 
)
{
#if IS_EXIST_RTL8881AEM
    u4Byte TXBD_NUM_8881A[HCI_TX_DMA_QUEUE_MAX_NUM] =
    {
        TX_MGQ_TXBD_NUM,
        TX_BKQ_TXBD_NUM, TX_BEQ_TXBD_NUM, TX_VIQ_TXBD_NUM, TX_VOQ_TXBD_NUM,
        TX_HI0Q_TXBD_NUM, TX_HI1Q_TXBD_NUM, TX_HI2Q_TXBD_NUM, TX_HI3Q_TXBD_NUM,
        TX_HI4Q_TXBD_NUM, TX_HI5Q_TXBD_NUM, TX_HI6Q_TXBD_NUM, TX_HI7Q_TXBD_NUM,
        TX_BCNQ_TXBD_NUM_8881A
    };
#endif  //IS_EXIST_RTL8881AEM
        
#if IS_EXIST_RTL8192EE
    u4Byte TXBD_NUM_8192E[HCI_TX_DMA_QUEUE_MAX_NUM] =
    {
        TX_MGQ_TXBD_NUM,
        TX_BKQ_TXBD_NUM, TX_BEQ_TXBD_NUM, TX_VIQ_TXBD_NUM, TX_VOQ_TXBD_NUM,
        TX_HI0Q_TXBD_NUM, TX_HI1Q_TXBD_NUM, TX_HI2Q_TXBD_NUM, TX_HI3Q_TXBD_NUM,
        TX_HI4Q_TXBD_NUM, TX_HI5Q_TXBD_NUM, TX_HI6Q_TXBD_NUM, TX_HI7Q_TXBD_NUM,
        TX_BCNQ_TXBD_NUM_8192E
    };
#endif  //IS_EXIST_RTL8192EE

    // TXBD_RWPtr_Reg: no entry for beacon queue, set NULL here
    u4Byte  TXBD_RWPtr_Reg[HCI_TX_DMA_QUEUE_MAX_NUM] =
    {
        REG_MGQ_TXBD_IDX,
        REG_BKQ_TXBD_IDX, REG_BEQ_TXBD_IDX, REG_VIQ_TXBD_IDX, REG_VOQ_TXBD_IDX,
        REG_HI0Q_TXBD_IDX, REG_HI1Q_TXBD_IDX, REG_HI2Q_TXBD_IDX, REG_HI3Q_TXBD_IDX,
        REG_HI4Q_TXBD_IDX, REG_HI5Q_TXBD_IDX, REG_HI6Q_TXBD_IDX, REG_HI7Q_TXBD_IDX,        
        0
    };

    u4Byte  TXBD_Reg[HCI_TX_DMA_QUEUE_MAX_NUM] =
    {
        REG_MGQ_TXBD_DESA,
        REG_BKQ_TXBD_DESA, REG_BEQ_TXBD_DESA, REG_VIQ_TXBD_DESA, REG_VOQ_TXBD_DESA,
        REG_HI0Q_TXBD_DESA, REG_HI1Q_TXBD_DESA, REG_HI2Q_TXBD_DESA, REG_HI3Q_TXBD_DESA,
        REG_HI4Q_TXBD_DESA, REG_HI5Q_TXBD_DESA, REG_HI6Q_TXBD_DESA, REG_HI7Q_TXBD_DESA,
        REG_BCNQ_TXBD_DESA
    };
	PHCI_TX_DMA_MANAGER_88XX    ptx_dma         = (PHCI_TX_DMA_MANAGER_88XX)(_GET_HAL_DATA(Adapter)->PTxDMA88XX);;
	int                         i               = 0;
	PTX_DESC_88XX               ptx_desc_head   = ptx_dma->tx_queue[q_num].ptx_desc_head;
	PTX_BUFFER_DESCRIPTOR       ptxbd = ptx_dma->tx_queue[q_num].pTXBD_head;
    pu4Byte                     pTXBD_NUM;

#if IS_EXIST_RTL8881AEM
    if ( IS_HARDWARE_TYPE_8881AEM(Adapter) ) {
        pTXBD_NUM           = TXBD_NUM_8881A;
    }
#endif  //IS_EXIST_RTL8881AEM

#if IS_EXIST_RTL8192EE
    if ( IS_HARDWARE_TYPE_8192EE(Adapter) ) {
        pTXBD_NUM           = TXBD_NUM_8192E;
    }
#endif  //IS_EXIST_RTL8192EE    
		
	printk(" q_num:%d, hw_idx=%d,host_idx= %d\n", q_num, ptx_dma->tx_queue[q_num].hw_idx, ptx_dma->tx_queue[q_num].host_idx);

	printk("total_txbd_num=%d,avail_txbd_num= %d,reg_rwptr_idx:%x\n",
		ptx_dma->tx_queue[q_num].total_txbd_num, ptx_dma->tx_queue[q_num].avail_txbd_num, ptx_dma->tx_queue[q_num].reg_rwptr_idx);

	printk("RWreg(%x):%08x\n", TXBD_RWPtr_Reg[q_num], HAL_RTL_R32(TXBD_RWPtr_Reg[q_num]));

#ifdef CONFIG_NET_PCI
	if (((Adapter->pshare->type >> TYPE_SHIFT) & TYPE_MASK) == TYPE_PCI_BIOS) {
	printk("pTXBD_head=%08x, %08x, reg(%x):%08x\n",
		(u4Byte)ptx_dma->tx_queue[q_num].pTXBD_head , 
			_GET_HAL_DATA(Adapter)->txBD_dma_ring_addr[q_num],
		TXBD_Reg[q_num], HAL_RTL_R32(TXBD_Reg[q_num]));

		for (i=0;i<pTXBD_NUM[q_num];i++ ){
			printk("ptxbd[%d], addr:%08x,%08x\n[0] Dword0: 0x%lx, Dword1: 0x%lx, [1] 0x%lx 0x%lx,\n",
				 i, \
				(u4Byte)&ptxbd[i],\
				(u4Byte)_GET_HAL_DATA(Adapter)->txBD_dma_ring_addr[q_num] + i * sizeof(TX_BUFFER_DESCRIPTOR),
				(u4Byte)GET_DESC(ptxbd[i].TXBD_ELE[0].Dword0), 
				(u4Byte)GET_DESC(ptxbd[i].TXBD_ELE[0].Dword1),
				(u4Byte)GET_DESC(ptxbd[i].TXBD_ELE[1].Dword0),
				(u4Byte)GET_DESC(ptxbd[i].TXBD_ELE[1].Dword1)  );
			printk("[2]: Dword0: 0x%lx, Dword1: 0x%lx, [3] 0x%lx 0x%lx,\n", \
				(u4Byte)GET_DESC(ptxbd[i].TXBD_ELE[2].Dword0), 
				(u4Byte)GET_DESC(ptxbd[i].TXBD_ELE[2].Dword1),
				(u4Byte)GET_DESC(ptxbd[i].TXBD_ELE[3].Dword0),
				(u4Byte)GET_DESC(ptxbd[i].TXBD_ELE[4].Dword1)  );

			printk("ptx_desc_head[%d], addr:%08x,%08x\n", \
				i, \
				(u4Byte)&ptx_desc_head[i],
				(u4Byte)_GET_HAL_DATA(Adapter)->txDesc_dma_ring_addr[q_num] + i * sizeof(TX_DESC_88XX));
				printk("%08x %08x %08x %08x ",  ptx_desc_head[i].Dword0,  ptx_desc_head[i].Dword1,  ptx_desc_head[i].Dword2,  ptx_desc_head[i].Dword3);
				printk("%08x %08x %08x %08x ",  ptx_desc_head[i].Dword4,  ptx_desc_head[i].Dword5,  ptx_desc_head[i].Dword6,  ptx_desc_head[i].Dword7);
				printk("%08x %08x\n", ptx_desc_head[i].Dword8,  ptx_desc_head[i].Dword9);
		}
	}else 
#endif
	{
		printk("pTXBD_head=%08x, %08x, reg(%x):%08x\n",
			(u4Byte)ptx_dma->tx_queue[q_num].pTXBD_head , 
			HAL_VIRT_TO_BUS1(Adapter, (PVOID)ptx_dma->tx_queue[q_num].pTXBD_head,sizeof(TX_BUFFER_DESCRIPTOR) * pTXBD_NUM[q_num], HAL_PCI_DMA_TODEVICE),
			TXBD_Reg[q_num], HAL_RTL_R32(TXBD_Reg[q_num]));

	for (i=0;i<pTXBD_NUM[q_num];i++ ){
		printk("ptxbd[%d], addr:%08x,%08x: Dword0: 0x%lx, Dword1: 0x%lx, [1] 0x%lx 0x%lx,\n",
                    i, \
                    (u4Byte)&ptxbd[i],\
                    (u4Byte)HAL_VIRT_TO_BUS1(Adapter, (PVOID)&ptxbd[i],sizeof(TX_BUFFER_DESCRIPTOR), HAL_PCI_DMA_TODEVICE),
                    (u4Byte)GET_DESC(ptxbd[i].TXBD_ELE[0].Dword0), 
                    (u4Byte)GET_DESC(ptxbd[i].TXBD_ELE[0].Dword1),
                    (u4Byte)GET_DESC(ptxbd[i].TXBD_ELE[1].Dword0),
                    (u4Byte)GET_DESC(ptxbd[i].TXBD_ELE[1].Dword1)  );
		printk("[2]: Dword0: 0x%lx, Dword1: 0x%lx, [3] 0x%lx 0x%lx,\n", \
                    (u4Byte)GET_DESC(ptxbd[i].TXBD_ELE[2].Dword0), 
                    (u4Byte)GET_DESC(ptxbd[i].TXBD_ELE[2].Dword1),
                    (u4Byte)GET_DESC(ptxbd[i].TXBD_ELE[3].Dword0),
                    (u4Byte)GET_DESC(ptxbd[i].TXBD_ELE[4].Dword1)  );

		printk("ptx_desc_head[%d], addr:%08x,%08x\n", \
                    i, \
                    (u4Byte)&ptx_desc_head[i],
                    (u4Byte)HAL_VIRT_TO_BUS1(Adapter, (PVOID)& ptx_desc_head[i],sizeof(TX_DESC_88XX), HAL_PCI_DMA_TODEVICE) );
		printk("%08x %08x %08x %08x ",  ptx_desc_head[i].Dword0,  ptx_desc_head[i].Dword1,  ptx_desc_head[i].Dword2,  ptx_desc_head[i].Dword3);
		printk("%08x %08x %08x %08x ",  ptx_desc_head[i].Dword4,  ptx_desc_head[i].Dword5,  ptx_desc_head[i].Dword6,  ptx_desc_head[i].Dword7);
		printk("%08x %08x\n", ptx_desc_head[i].Dword8,  ptx_desc_head[i].Dword9);
	}
	}
	printk("\n");
}

// TODO: 
//Note: PrepareTXBD88XX is necessary to be done after calling PrepareRXBD88XX
RT_STATUS
PrepareTXBD88XX(
    IN      HAL_PADAPTER Adapter
)
{    
    PHCI_RX_DMA_MANAGER_88XX    prx_dma;
    PHCI_TX_DMA_MANAGER_88XX    ptx_dma;
    PTX_BUFFER_DESCRIPTOR       ptxbd_head;
    PTX_DESC_88XX               ptx_desc_head;
    PTX_BUFFER_DESCRIPTOR       ptxbd;
    
    PTX_BUFFER_DESCRIPTOR       ptxbd_bcn_head;
    PTX_DESC_88XX               ptxdesc_bcn_head;
    PTX_BUFFER_DESCRIPTOR       ptxbd_bcn_cur;    

    PTX_DESC_88XX               ptx_desc;
    
    HCI_TX_DMA_QUEUE_88XX       q_num;
    pu4Byte                     pTXBD_NUM;
    u4Byte                      i;
    u4Byte                      beacon_offset;
    u4Byte                      TotalTXBDNum_NoBcn;


#if IS_EXIST_RTL8881AEM
    u4Byte TXBD_NUM_8881A[HCI_TX_DMA_QUEUE_MAX_NUM] =
    {
        TX_MGQ_TXBD_NUM,
        TX_BKQ_TXBD_NUM, TX_BEQ_TXBD_NUM, TX_VIQ_TXBD_NUM, TX_VOQ_TXBD_NUM,
        TX_HI0Q_TXBD_NUM, TX_HI1Q_TXBD_NUM, TX_HI2Q_TXBD_NUM, TX_HI3Q_TXBD_NUM,
        TX_HI4Q_TXBD_NUM, TX_HI5Q_TXBD_NUM, TX_HI6Q_TXBD_NUM, TX_HI7Q_TXBD_NUM,
        TX_BCNQ_TXBD_NUM_8881A
    };
#endif  //IS_EXIST_RTL8881AEM
        
#if IS_EXIST_RTL8192EE
    u4Byte TXBD_NUM_8192E[HCI_TX_DMA_QUEUE_MAX_NUM] =
    {
        TX_MGQ_TXBD_NUM,
        TX_BKQ_TXBD_NUM, TX_BEQ_TXBD_NUM, TX_VIQ_TXBD_NUM, TX_VOQ_TXBD_NUM,
        TX_HI0Q_TXBD_NUM, TX_HI1Q_TXBD_NUM, TX_HI2Q_TXBD_NUM, TX_HI3Q_TXBD_NUM,
        TX_HI4Q_TXBD_NUM, TX_HI5Q_TXBD_NUM, TX_HI6Q_TXBD_NUM, TX_HI7Q_TXBD_NUM,
        TX_BCNQ_TXBD_NUM_8192E
    };
#endif  //IS_EXIST_RTL8192EE

    // TXBD_RWPtr_Reg: no entry for beacon queue, set NULL here
    u4Byte  TXBD_RWPtr_Reg[HCI_TX_DMA_QUEUE_MAX_NUM] =
    {
        REG_MGQ_TXBD_IDX,
        REG_BKQ_TXBD_IDX, REG_BEQ_TXBD_IDX, REG_VIQ_TXBD_IDX, REG_VOQ_TXBD_IDX,
        REG_HI0Q_TXBD_IDX, REG_HI1Q_TXBD_IDX, REG_HI2Q_TXBD_IDX, REG_HI3Q_TXBD_IDX,
        REG_HI4Q_TXBD_IDX, REG_HI5Q_TXBD_IDX, REG_HI6Q_TXBD_IDX, REG_HI7Q_TXBD_IDX,        
        0
    };

    u4Byte  TXBD_Reg[HCI_TX_DMA_QUEUE_MAX_NUM] =
    {
        REG_MGQ_TXBD_DESA,
        REG_BKQ_TXBD_DESA, REG_BEQ_TXBD_DESA, REG_VIQ_TXBD_DESA, REG_VOQ_TXBD_DESA,
        REG_HI0Q_TXBD_DESA, REG_HI1Q_TXBD_DESA, REG_HI2Q_TXBD_DESA, REG_HI3Q_TXBD_DESA,
        REG_HI4Q_TXBD_DESA, REG_HI5Q_TXBD_DESA, REG_HI6Q_TXBD_DESA, REG_HI7Q_TXBD_DESA,
        REG_BCNQ_TXBD_DESA
    };
    u4Byte tmp_addr=0;

#if IS_EXIST_RTL8881AEM
    if ( IS_HARDWARE_TYPE_8881AEM(Adapter) ) {
        pTXBD_NUM           = TXBD_NUM_8881A;
        TotalTXBDNum_NoBcn  = TOTAL_NUM_TXBD_NO_BCN;
    }
#endif  //IS_EXIST_RTL8881AEM

#if IS_EXIST_RTL8192EE
    if ( IS_HARDWARE_TYPE_8192EE(Adapter) ) {
        pTXBD_NUM           = TXBD_NUM_8192E;
        TotalTXBDNum_NoBcn  = TOTAL_NUM_TXBD_NO_BCN;        
    }
#endif  //IS_EXIST_RTL8192EE

    prx_dma         = (PHCI_RX_DMA_MANAGER_88XX)(_GET_HAL_DATA(Adapter)->PRxDMA88XX);
    ptx_dma         = (PHCI_TX_DMA_MANAGER_88XX)(_GET_HAL_DATA(Adapter)->PTxDMA88XX);

#ifdef CONFIG_NET_PCI
    u4Byte tmp_tx_dma_ring_addr =0, tmp_tx_dma_ring_addr2=0;
    u4Byte tmp_tx_dma_ring_addr3 =0, tmp_tx_dma_ring_addr4=0;

    if (!(((Adapter->pshare->type >> TYPE_SHIFT) & TYPE_MASK) == TYPE_PCI_BIOS)) 
        goto original;

    //No Beacon
    printk("head:%08x, ring_dma_addr:%08x, size:%x\n", prx_dma->rx_queue[HCI_RX_DMA_QUEUE_MAX_NUM-1].pRXBD_head, 
        _GET_HAL_DATA(Adapter)->ring_dma_addr, prx_dma->rx_queue[HCI_RX_DMA_QUEUE_MAX_NUM-1].total_rxbd_num * sizeof(RX_BUFFER_DESCRIPTOR));
        tmp_tx_dma_ring_addr4 = (u4Byte)prx_dma->rx_queue[HCI_RX_DMA_QUEUE_MAX_NUM-1].pRXBD_head +  prx_dma->rx_queue[HCI_RX_DMA_QUEUE_MAX_NUM-1].total_rxbd_num * sizeof(RX_BUFFER_DESCRIPTOR);
    ptxbd_head  = (PTX_BUFFER_DESCRIPTOR)( tmp_tx_dma_ring_addr4);

    for (i=0;i<HCI_RX_DMA_QUEUE_MAX_NUM;i++){
        tmp_tx_dma_ring_addr = _GET_HAL_DATA(Adapter)->ring_dma_addr + prx_dma->rx_queue[i].total_rxbd_num * sizeof(RX_BUFFER_DESCRIPTOR);
    }


    printk("ptxbd_head:%08x, tmp_tx_dma_ring_addr:%08x, tmp_tx_dma_ring_addr4:%08x\n", ptxbd_head, tmp_tx_dma_ring_addr, tmp_tx_dma_ring_addr4);
    tmp_tx_dma_ring_addr4 =0;
    //No Beacon
    // TODO: need to bug fix
    ptx_desc_head   = (PTX_DESC_88XX)((pu1Byte)ptxbd_head + \
                        sizeof(TX_BUFFER_DESCRIPTOR) * TotalTXBDNum_NoBcn);

    tmp_tx_dma_ring_addr2 = tmp_tx_dma_ring_addr + sizeof(TX_BUFFER_DESCRIPTOR) * TotalTXBDNum_NoBcn;
    printk("ptx_desc_head:%08x, tmp_tx_dma_ring_addr2:%08x, , size: %x, %x\n", ptx_desc_head, tmp_tx_dma_ring_addr2, sizeof(TX_BUFFER_DESCRIPTOR) , sizeof(TX_DESC_88XX) );
    ptxbd_bcn_head  = (PTX_BUFFER_DESCRIPTOR)((pu1Byte)ptx_desc_head + \
                        sizeof(TX_DESC_88XX) * TotalTXBDNum_NoBcn);

    tmp_tx_dma_ring_addr3 = tmp_tx_dma_ring_addr2 + sizeof(TX_DESC_88XX) * TotalTXBDNum_NoBcn;
 printk("ptxbd_bcn_head:%08x, tmp_tx_dma_ring_addr3:%08x, \n", ptxbd_bcn_head, tmp_tx_dma_ring_addr3);
#if IS_RTL8881A_SERIES
    if (IS_HARDWARE_TYPE_8881AEM(Adapter)) {
        ptxdesc_bcn_head = (PTX_DESC_88XX)((pu1Byte)ptxbd_bcn_head + \
                                (1+HAL_NUM_VWLAN) * TXBD_BEACON_OFFSET_8881A);
        beacon_offset = TXBD_BEACON_OFFSET_8881A;
        tmp_tx_dma_ring_addr4 = tmp_tx_dma_ring_addr3+(1+HAL_NUM_VWLAN) * TXBD_BEACON_OFFSET_8881A;
    }
#endif
#if IS_RTL8192E_SERIES
    if (IS_HARDWARE_TYPE_8192E(Adapter)) {
        ptxdesc_bcn_head = (PTX_DESC_88XX)((pu1Byte)ptxbd_bcn_head + \
                                (1+HAL_NUM_VWLAN) * TXBD_BEACON_OFFSET_8192E);
        beacon_offset = TXBD_BEACON_OFFSET_8192E;
        tmp_tx_dma_ring_addr4 = tmp_tx_dma_ring_addr3+(1+HAL_NUM_VWLAN) * TXBD_BEACON_OFFSET_8192E;
    }
#endif
printk("ptxdesc_bcn_head:%08x, tmp_tx_dma_ring_addr4:%08x, \n", ptxdesc_bcn_head, tmp_tx_dma_ring_addr4);
    // initiate all tx queue data structures 
    for (q_num = 0; q_num < HCI_TX_DMA_QUEUE_MAX_NUM; q_num++)
    {
        ptx_dma->tx_queue[q_num].hw_idx         = 0;
        ptx_dma->tx_queue[q_num].host_idx       = 0;
        ptx_dma->tx_queue[q_num].total_txbd_num = pTXBD_NUM[q_num];
        ptx_dma->tx_queue[q_num].avail_txbd_num = pTXBD_NUM[q_num];
        ptx_dma->tx_queue[q_num].reg_rwptr_idx  = TXBD_RWPtr_Reg[q_num];

        if ( 0 == q_num ) {
            ptx_dma->tx_queue[q_num].pTXBD_head    = ptxbd_head;
            ptx_dma->tx_queue[q_num].ptx_desc_head = ptx_desc_head;
        }
        else {
            if ( HCI_TX_DMA_QUEUE_BCN != q_num ) {
                ptx_dma->tx_queue[q_num].pTXBD_head    = (PTX_BUFFER_DESCRIPTOR)((pu1Byte)ptx_dma->tx_queue[q_num-1].pTXBD_head + sizeof(TX_BUFFER_DESCRIPTOR) * pTXBD_NUM[q_num-1]);
                ptx_dma->tx_queue[q_num].ptx_desc_head = (PTX_DESC_88XX)((pu1Byte)ptx_dma->tx_queue[q_num-1].ptx_desc_head + sizeof(TX_DESC_88XX)*pTXBD_NUM[q_num-1]);

                tmp_tx_dma_ring_addr += sizeof(TX_BUFFER_DESCRIPTOR) *pTXBD_NUM[q_num-1] ;
                tmp_tx_dma_ring_addr2 += sizeof(TX_DESC_88XX) *pTXBD_NUM[q_num-1] ;
            }
            else {
                ptx_dma->tx_queue[q_num].pTXBD_head    = ptxbd_bcn_head;
                ptx_dma->tx_queue[q_num].ptx_desc_head = ptxdesc_bcn_head;
                tmp_tx_dma_ring_addr = tmp_tx_dma_ring_addr3;
                tmp_tx_dma_ring_addr2 = tmp_tx_dma_ring_addr4;
            }
        }
        
        ptxbd    = ptx_dma->tx_queue[q_num].pTXBD_head;
        ptx_desc = ptx_dma->tx_queue[q_num].ptx_desc_head;

        HAL_RTL_W32(TXBD_RWPtr_Reg[q_num], 0);
        HAL_RTL_W32(TXBD_Reg[q_num], tmp_tx_dma_ring_addr);
        _GET_HAL_DATA(Adapter)->txBD_dma_ring_addr[q_num] = tmp_tx_dma_ring_addr;
        _GET_HAL_DATA(Adapter)->txDesc_dma_ring_addr[q_num] = tmp_tx_dma_ring_addr2;
    
printk("%s(%d), q_num:%d, TXBD_RWPtr_Reg:%d, TXBD_Reg:%x, ptxbd:%08x %08x, ptx_desc:%08x %08x\n", __FUNCTION__, __LINE__, q_num, TXBD_RWPtr_Reg[q_num], TXBD_Reg[q_num], ptxbd, tmp_tx_dma_ring_addr, ptx_desc, tmp_tx_dma_ring_addr2);

        // assign LowAddress and TxDescLength to each TXBD element
        if (q_num != HCI_TX_DMA_QUEUE_BCN) {
            for(i = 0; i < pTXBD_NUM[q_num]; i++)        
            {   
                SET_DESC_FIELD_CLR(ptxbd[i].TXBD_ELE[0].Dword0, sizeof(TX_DESC_88XX), TXBD_DW0_TXBUFSIZE_MSK, TXBD_DW0_TXBUFSIZE_SH);
                SET_DESC_FIELD_CLR(ptxbd[i].TXBD_ELE[0].Dword1, tmp_tx_dma_ring_addr2 + sizeof(TX_DESC_88XX)*i,TXBD_DW1_PHYADDR_LOW_MSK, TXBD_DW1_PHYADDR_LOW_SH);
            }
        } else {
            // beacon...
            for (i = 0; i < 1+HAL_NUM_VWLAN; i++)
            {
                ptxbd_bcn_cur = (pu1Byte)ptxbd + beacon_offset * i;
                SET_DESC_FIELD_CLR(ptxbd_bcn_cur->TXBD_ELE[0].Dword0, \
                        sizeof(TX_DESC_88XX), \
                        TXBD_DW0_TXBUFSIZE_MSK, TXBD_DW0_TXBUFSIZE_SH);
               
               SET_DESC_FIELD_CLR(ptxbd_bcn_cur->TXBD_ELE[0].Dword1, tmp_tx_dma_ring_addr2 + sizeof(TX_DESC_88XX)*i, TXBD_DW1_PHYADDR_LOW_MSK, TXBD_DW1_PHYADDR_LOW_SH);
#if 1
                printk ("ptxbd_bcn[%ld]: 0x%lx, Dword0: 0x%lx, Dword1: 0x%lx \n", \
                                                i, (u4Byte)ptxbd_bcn_cur, \
                                                (u4Byte)GET_DESC(ptxbd_bcn_cur->TXBD_ELE[0].Dword0), \
                                                (u4Byte)GET_DESC(ptxbd_bcn_cur->TXBD_ELE[0].Dword1)
                                                );             
#endif
            }
        }        
    }   

    return RT_STATUS_SUCCESS;
#endif

original:
    //No Beacon
    ptxbd_head      = (PTX_BUFFER_DESCRIPTOR)(prx_dma->rx_queue[HCI_RX_DMA_QUEUE_MAX_NUM-1].pRXBD_head + \
                        prx_dma->rx_queue[HCI_RX_DMA_QUEUE_MAX_NUM-1].total_rxbd_num);

    //No Beacon
    ptx_desc_head   = (PTX_DESC_88XX)((pu1Byte)ptxbd_head + \
                        sizeof(TX_BUFFER_DESCRIPTOR) * TotalTXBDNum_NoBcn);
    
    ptxbd_bcn_head  = (PTX_BUFFER_DESCRIPTOR)((pu1Byte)ptx_desc_head + \
                        sizeof(TX_DESC_88XX) * TotalTXBDNum_NoBcn);

#if IS_RTL8881A_SERIES
    if (IS_HARDWARE_TYPE_8881AEM(Adapter)) {
        ptxdesc_bcn_head = (PTX_DESC_88XX)((pu1Byte)ptxbd_bcn_head + \
                                (1+HAL_NUM_VWLAN) * TXBD_BEACON_OFFSET_8881A);
        beacon_offset = TXBD_BEACON_OFFSET_8881A;
    }
#endif
#if IS_RTL8192E_SERIES
    if (IS_HARDWARE_TYPE_8192E(Adapter)) {
        ptxdesc_bcn_head = (PTX_DESC_88XX)((pu1Byte)ptxbd_bcn_head + \
                                (1+HAL_NUM_VWLAN) * TXBD_BEACON_OFFSET_8192E);
        beacon_offset = TXBD_BEACON_OFFSET_8192E;
    }
#endif

    // initiate all tx queue data structures 
    for (q_num = 0; q_num < HCI_TX_DMA_QUEUE_MAX_NUM; q_num++)
    {
        ptx_dma->tx_queue[q_num].hw_idx         = 0;
        ptx_dma->tx_queue[q_num].host_idx       = 0;
        ptx_dma->tx_queue[q_num].total_txbd_num = pTXBD_NUM[q_num];
        ptx_dma->tx_queue[q_num].avail_txbd_num = pTXBD_NUM[q_num];
        ptx_dma->tx_queue[q_num].reg_rwptr_idx  = TXBD_RWPtr_Reg[q_num];

        if ( 0 == q_num ) {
            ptx_dma->tx_queue[q_num].pTXBD_head    = ptxbd_head;
            ptx_dma->tx_queue[q_num].ptx_desc_head = ptx_desc_head;
        }
        else {
            if ( HCI_TX_DMA_QUEUE_BCN != q_num ) {
                ptx_dma->tx_queue[q_num].pTXBD_head    = ptx_dma->tx_queue[q_num-1].pTXBD_head + pTXBD_NUM[q_num-1];
                ptx_dma->tx_queue[q_num].ptx_desc_head = ((PTX_DESC_88XX)ptx_dma->tx_queue[q_num-1].ptx_desc_head) + pTXBD_NUM[q_num-1];
            }
            else {
                ptx_dma->tx_queue[q_num].pTXBD_head    = ptxbd_bcn_head;
                ptx_dma->tx_queue[q_num].ptx_desc_head = ptxdesc_bcn_head;
            }
        }
        
        ptxbd    = ptx_dma->tx_queue[q_num].pTXBD_head;
        ptx_desc = ptx_dma->tx_queue[q_num].ptx_desc_head;

        HAL_RTL_W32(TXBD_RWPtr_Reg[q_num], 0);
        HAL_RTL_W32(TXBD_Reg[q_num], HAL_VIRT_TO_BUS((u4Byte)ptxbd));

#if 0
        RT_TRACE_F(COMP_INIT, DBG_TRACE, ("QNum: %ld, TXBDHead: 0x%lx, TXDESCHead: 0x%lx\n", \
                                            (u4Byte)q_num, \
                                            (u4Byte)ptx_dma->tx_queue[q_num].pTXBD_head, \
                                            (u4Byte)ptx_dma->tx_queue[q_num].ptx_desc_head
                                            ));
#endif

        // assign LowAddress and TxDescLength to each TXBD element
        if (q_num != HCI_TX_DMA_QUEUE_BCN) {
            for(i = 0; i < pTXBD_NUM[q_num]; i++)        
            {   
                SET_DESC_FIELD_CLR(ptxbd[i].TXBD_ELE[0].Dword0, \
                        sizeof(TX_DESC_88XX), \
                        TXBD_DW0_TXBUFSIZE_MSK, TXBD_DW0_TXBUFSIZE_SH);
                SET_DESC_FIELD_CLR(ptxbd[i].TXBD_ELE[0].Dword1, \
                        HAL_VIRT_TO_BUS((u4Byte)&ptx_desc[i]), \
                        TXBD_DW1_PHYADDR_LOW_MSK, TXBD_DW1_PHYADDR_LOW_SH);
#if 0

               RT_TRACE_F(COMP_INIT, DBG_TRACE, ("ptxbd[%ld]: Dword0: 0x%lx, Dword1: 0x%lx\n", \
                                                i, \
                                                (u4Byte)GET_DESC(ptxbd[i].TXBD_ELE[0].Dword0), \
                                                (u4Byte)GET_DESC(ptxbd[i].TXBD_ELE[0].Dword1)
                                                ));
#endif

            }
        } else {
            // beacon...
            for (i = 0; i < 1+HAL_NUM_VWLAN; i++)
            {
                ptxbd_bcn_cur = (pu1Byte)ptxbd + beacon_offset * i;
                SET_DESC_FIELD_CLR(ptxbd_bcn_cur->TXBD_ELE[0].Dword0, \
                        sizeof(TX_DESC_88XX), \
                        TXBD_DW0_TXBUFSIZE_MSK, TXBD_DW0_TXBUFSIZE_SH);
                SET_DESC_FIELD_CLR(ptxbd_bcn_cur->TXBD_ELE[0].Dword1, \
                        HAL_VIRT_TO_BUS((u4Byte)&ptx_desc[i]), \
                        TXBD_DW1_PHYADDR_LOW_MSK, TXBD_DW1_PHYADDR_LOW_SH);
#if 1
                RT_TRACE_F(COMP_INIT, DBG_TRACE, ("ptxbd_bcn[%ld]: 0x%lx, Dword0: 0x%lx, Dword1: 0x%lx \n", \
                                                i, (u4Byte)ptxbd_bcn_cur, \
                                                (u4Byte)GET_DESC(ptxbd_bcn_cur->TXBD_ELE[0].Dword0), \
                                                (u4Byte)GET_DESC(ptxbd_bcn_cur->TXBD_ELE[0].Dword1)
                                                ));             
#endif
            }
        }        
    }   

    return RT_STATUS_SUCCESS;
}


static BOOLEAN 
IsTXBDFull88XX(
    IN   HAL_PADAPTER               Adapter,
    IN   u4Byte                     queueIndex  //HCI_TX_DMA_QUEUE_88XX
)
{
    PHCI_TX_DMA_MANAGER_88XX        ptx_dma;
    PHCI_TX_DMA_QUEUE_STRUCT_88XX   cur_q;    

    ptx_dma     = (PHCI_TX_DMA_MANAGER_88XX)(_GET_HAL_DATA(Adapter)->PTxDMA88XX);
    cur_q       = &(ptx_dma->tx_queue[queueIndex]);
    
    if (HAL_CIRC_SPACE_RTK(cur_q->host_idx, cur_q->hw_idx, cur_q->total_txbd_num) == 0) {
        // case: full
        RT_TRACE(COMP_SEND, DBG_LOUD, ("TXBD is Full !!!\n") );
        return _TRUE;
    }
    return _FALSE;
}


static VOID
SetTxDescQSel88XX(
    IN  HAL_PADAPTER    Adapter,
    IN  u4Byte          queueIndex,  //HCI_TX_DMA_QUEUE_88XX
    IN  PTX_DESC_88XX   ptx_desc,
    IN  u1Byte          drvQSel
)
{
    u1Byte  q_select;
    
	switch (queueIndex) {
    	case HCI_TX_DMA_QUEUE_HI0:
            SET_DESC_FIELD_NO_CLR(ptx_desc->Dword1, 1, TX_DW1_MOREDATA_MSK, TX_DW1_MOREDATA_SH);
            //Set MACIDMask to zero, but we have memset before
    		q_select = TXDESC_QSEL_HIGH;
    		break;
#if  CFG_HAL_SUPPORT_MBSSID
    	case HCI_TX_DMA_QUEUE_HI1:
            SET_DESC_FIELD_NO_CLR(ptx_desc->Dword1, 1, TX_DW1_MOREDATA_MSK, TX_DW1_MOREDATA_SH);
            SET_DESC_FIELD_NO_CLR(ptx_desc->Dword1, 1, TX_DW1_MACID_MSK, TX_DW1_MACID_SH);
            q_select = TXDESC_QSEL_HIGH;
    		break;
    	case HCI_TX_DMA_QUEUE_HI2:
            SET_DESC_FIELD_NO_CLR(ptx_desc->Dword1, 1, TX_DW1_MOREDATA_MSK, TX_DW1_MOREDATA_SH);
            SET_DESC_FIELD_NO_CLR(ptx_desc->Dword1, 2, TX_DW1_MACID_MSK, TX_DW1_MACID_SH);
            q_select = TXDESC_QSEL_HIGH;
    		break;
    	case HCI_TX_DMA_QUEUE_HI3:	
            SET_DESC_FIELD_NO_CLR(ptx_desc->Dword1, 1, TX_DW1_MOREDATA_MSK, TX_DW1_MOREDATA_SH);
            SET_DESC_FIELD_NO_CLR(ptx_desc->Dword1, 3, TX_DW1_MACID_MSK, TX_DW1_MACID_SH);
            q_select = TXDESC_QSEL_HIGH;
    		break;
    	case HCI_TX_DMA_QUEUE_HI4:
            SET_DESC_FIELD_NO_CLR(ptx_desc->Dword1, 1, TX_DW1_MOREDATA_MSK, TX_DW1_MOREDATA_SH);
            SET_DESC_FIELD_NO_CLR(ptx_desc->Dword1, 4, TX_DW1_MACID_MSK, TX_DW1_MACID_SH);
            q_select = TXDESC_QSEL_HIGH;
    		break;
    	case HCI_TX_DMA_QUEUE_HI5:
            SET_DESC_FIELD_NO_CLR(ptx_desc->Dword1, 1, TX_DW1_MOREDATA_MSK, TX_DW1_MOREDATA_SH);
            SET_DESC_FIELD_NO_CLR(ptx_desc->Dword1, 5, TX_DW1_MACID_MSK, TX_DW1_MACID_SH);
            q_select = TXDESC_QSEL_HIGH;
    		break;
    	case HCI_TX_DMA_QUEUE_HI6:
            SET_DESC_FIELD_NO_CLR(ptx_desc->Dword1, 1, TX_DW1_MOREDATA_MSK, TX_DW1_MOREDATA_SH);
            SET_DESC_FIELD_NO_CLR(ptx_desc->Dword1, 6, TX_DW1_MACID_MSK, TX_DW1_MACID_SH);
            q_select = TXDESC_QSEL_HIGH;
    		break;
    	case HCI_TX_DMA_QUEUE_HI7:
            SET_DESC_FIELD_NO_CLR(ptx_desc->Dword1, 1, TX_DW1_MOREDATA_MSK, TX_DW1_MOREDATA_SH);
            SET_DESC_FIELD_NO_CLR(ptx_desc->Dword1, 7, TX_DW1_MACID_MSK, TX_DW1_MACID_SH);
            q_select = TXDESC_QSEL_HIGH;
    		break;
#endif  //CFG_HAL_SUPPORT_MBSSID

    	case HCI_TX_DMA_QUEUE_MGT:
    		q_select = TXDESC_QSEL_MGT;
    		break;
            
#if CFG_HAL_MAC_LOOPBACK && CFG_HAL_WIFI_WMM
    	case HCI_TX_DMA_QUEUE_BE:
    		q_select = TXDESC_QSEL_TID0;
    		break;
#endif  //CFG_HAL_MAC_LOOPBACK && CFG_HAL_WIFI_WMM

    	default:
    		// data packet
#if CFG_HAL_RTL_MANUAL_EDCA
    		if (HAL_VAR_MANUAL_EDCA) {
    			switch (queueIndex) {
        			case HCI_TX_DMA_QUEUE_VO:
        				q_select = TXDESC_QSEL_TID6;
        				break;
        			case HCI_TX_DMA_QUEUE_VI:
        				q_select = TXDESC_QSEL_TID4;
                		break;
        			case HCI_TX_DMA_QUEUE_BE:
        				q_select = TXDESC_QSEL_TID0;
        	    		break;
        		    default:
        				q_select = TXDESC_QSEL_TID1;
        				break;
    			}
    		}
    		else {
                q_select = drvQSel;
    		}
#else
            q_select = drvQSel;
#endif  //CFG_HAL_RTL_MANUAL_EDCA
            break;
	}    

    SET_DESC_FIELD_NO_CLR(ptx_desc->Dword1, q_select, TX_DW1_QSEL_MSK, TX_DW1_QSEL_SH);
}

static VOID
SetSecType(
    IN  HAL_PADAPTER    Adapter,
    IN  PTX_DESC_88XX   ptx_desc,
    IN  PVOID           pDescData 
)
{
    PTX_DESC_DATA_88XX  pdesc_data  = (PTX_DESC_DATA_88XX)pDescData;
    
    switch(pdesc_data->secType) {
    case _WEP_40_PRIVACY_:
    case _WEP_104_PRIVACY_:
    case _TKIP_PRIVACY_:
        SET_DESC_FIELD_CLR(ptx_desc->Dword1, TXDESC_SECTYPE_WEP40_OR_TKIP,
                                        TX_DW1_SECTYPE_MSK, TX_DW1_SECTYPE_SH);
        break;
#if CFG_HAL_RTL_HW_WAPI_SUPPORT
    case _WAPI_SMS4_:
        SET_DESC_FIELD_CLR(ptx_desc->Dword1, TXDESC_SECTYPE_WAPI,
                                        TX_DW1_SECTYPE_MSK, TX_DW1_SECTYPE_SH);
        break;
#endif        
    case _CCMP_PRIVACY_:
        SET_DESC_FIELD_CLR(ptx_desc->Dword1, TXDESC_SECTYPE_AES,
                                        TX_DW1_SECTYPE_MSK, TX_DW1_SECTYPE_SH);
        break;
    default:
#if 0
        SET_DESC_FIELD_CLR(ptx_desc->Dword1, TXDESC_SECTYPE_NO_ENCRYPTION,
                                        TX_DW1_SECTYPE_MSK, TX_DW1_SECTYPE_SH);        
#endif
        break;
    }   
}

static VOID
FillTxDesc88XX (
    IN  HAL_PADAPTER    Adapter,
    IN  u4Byte          queueIndex,  //HCI_TX_DMA_QUEUE_88XX
    IN  PVOID           pDescData
)
{
    PHCI_TX_DMA_MANAGER_88XX        ptx_dma;
    PHCI_TX_DMA_QUEUE_STRUCT_88XX   cur_q;
    PTX_DESC_88XX                   ptx_desc;
    PTX_DESC_DATA_88XX              pdesc_data = (PTX_DESC_DATA_88XX)pDescData;

    //Dword 0
    u2Byte  TX_DESC_TXPKTSIZE        = pdesc_data->hdrLen + pdesc_data->llcLen + pdesc_data->frLen;
    u1Byte  TX_DESC_OFFSET           = HAL_TXDESC_OFFSET_SIZE;
    BOOLEAN TX_DESC_BMC              = (HAL_IS_MCAST(GetAddr1Ptr((pu1Byte)pdesc_data->pHdr))) ? 1 : 0;   // when multicast or broadcast, BMC = 1        
//    BOOLEAN TX_DESC_HT               = 0;
//    BOOLEAN TX_DESC_LINIP            = 0;
//    BOOLEAN TX_DESC_NOACM            = 0;
//    BOOLEAN TX_DESC_GF               = 0;

    //Dword 1 
    u1Byte  TX_DESC_MACID            = pdesc_data->macId; // MACID/MBSSID ?
    u1Byte  TX_DESC_RATE_ID          = pdesc_data->rateId;
//    BOOLEAN TX_DESC_EN_DESC_ID       = 0; // EN_DESC_ID ?  
//    u1Byte  TX_DESC_PKT_OFFSET       = 0; // early mode ?
    
    //Dword 2
    BOOLEAN TX_DESC_AGG_EN           = pdesc_data->aggEn; 
    BOOLEAN TX_DESC_BK               = pdesc_data->bk;
    BOOLEAN TX_DESC_MOREFRAG         = pdesc_data->frag;
    u1Byte  TX_DESC_AMPDU_DENSITY    = pdesc_data->ampduDensity;

    //Dword 3
    BOOLEAN TX_DESC_USERATE          = pdesc_data->useRate;    
    BOOLEAN TX_DESC_DISRTSFB         = pdesc_data->disRTSFB;
    BOOLEAN TX_DESC_DISDATAFB        = pdesc_data->disDataFB;
    BOOLEAN TX_DESC_CTS2SELF         = pdesc_data->CTS2Self;
    BOOLEAN TX_DESC_RTS_EN           = pdesc_data->RTSEn;
    BOOLEAN TX_DESC_HW_RTS_EN        = pdesc_data->HWRTSEn;
    BOOLEAN TX_DESC_NAVUSEHDR        = pdesc_data->navUseHdr;
    u1Byte  TX_DESC_MAX_AGG_NUM      = pdesc_data->maxAggNum;


    //Dword 4
    u1Byte  TX_DESC_DATERATE         = pdesc_data->dataRate;
    u1Byte  TX_DESC_DATA_RATEFB_LMT  = pdesc_data->dataRateFBLmt;
    BOOLEAN TX_DESC_RTY_LMT_EN       = pdesc_data->rtyLmtEn;   
    u1Byte  TX_DESC_DATA_RT_LMT      = pdesc_data->dataRtyLmt;
    u1Byte  TX_DESC_RTSRATE          = pdesc_data->RTSRate;

    //Dword 5
    u1Byte  TX_DESC_DATA_SC          = pdesc_data->dataSC;
    u1Byte  TX_DESC_DATA_SHORT       = pdesc_data->dataShort;
    u1Byte  TX_DESC_DATA_BW          = pdesc_data->dataBW;
    u1Byte  TX_DESC_DATA_STBC        = pdesc_data->dataStbc;
    u1Byte  TX_DESC_RTS_SHORT        = pdesc_data->RTSShort;
    u1Byte  TX_DESC_RTS_SC           = pdesc_data->RTSSC;   
    u1Byte	TX_DESC_POWER_OFFSET	 = pdesc_data->TXPowerOffset;

    //Dword 7
    // use for CFG_HAL_TX_SHORTCUT
    u2Byte  TX_DESC_TXBUFF           = pdesc_data->frLen;

    //Dword 9
    u2Byte TX_DESC_SEQ               = GetSequence(pdesc_data->pHdr);

    ptx_dma     = (PHCI_TX_DMA_MANAGER_88XX)(_GET_HAL_DATA(Adapter)->PTxDMA88XX);
    cur_q       = &(ptx_dma->tx_queue[queueIndex]);
    ptx_desc    = ((PTX_DESC_88XX)(cur_q->ptx_desc_head)) + cur_q->host_idx;

    //Clear All Bit
    PlatformZeroMemory((PVOID)ptx_desc, sizeof(TX_DESC_88XX));

    if (pdesc_data->secType != _NO_PRIVACY_) {
        if (pdesc_data->swCrypt == FALSE) {
            SetSecType(Adapter, ptx_desc, pdesc_data);
            // for hw sec: 1) WEP's iv, 2) TKIP's iv and eiv, 3) CCMP's ccmp header are all in pdesc_data->iv
            TX_DESC_TXPKTSIZE += pdesc_data->iv;
        } else {
            // for sw sec
            TX_DESC_TXPKTSIZE += (pdesc_data->iv + pdesc_data->icv + pdesc_data->mic);
        }
    }

    //4 Set Dword0
    SET_DESC_FIELD_NO_CLR(ptx_desc->Dword0, TX_DESC_TXPKTSIZE, TX_DW0_TXPKSIZE_MSK, TX_DW0_TXPKSIZE_SH);
    SET_DESC_FIELD_NO_CLR(ptx_desc->Dword0, TX_DESC_OFFSET, TX_DW0_OFFSET_MSK, TX_DW0_OFFSET_SH);
    SET_DESC_FIELD_NO_CLR(ptx_desc->Dword0, TX_DESC_BMC, TX_DW0_BMC_MSK, TX_DW0_BMC_SH);

    // TODO: MACID filed will be changed in normal chip !!!
    //4 Set Dword1
    SetTxDescQSel88XX(Adapter, queueIndex, ptx_desc, pdesc_data->qSel);        
    if ( (queueIndex >= HCI_TX_DMA_QUEUE_HI0) && (queueIndex <= HCI_TX_DMA_QUEUE_HI7) ) {
        //MacID has written in SetTxDescQSel88XX()
    }
    else {
        SET_DESC_FIELD_NO_CLR(ptx_desc->Dword1, TX_DESC_MACID, TX_DW1_MACID_MSK, TX_DW1_MACID_SH);
    }
    SET_DESC_FIELD_NO_CLR(ptx_desc->Dword1, TX_DESC_RATE_ID, TX_DW1_RATE_ID_MSK, TX_DW1_RATE_ID_SH);

    //4 Set Dword2
    SET_DESC_FIELD_NO_CLR(ptx_desc->Dword2, TX_DESC_AGG_EN, TX_DW2_AGG_EN_MSK, TX_DW2_AGG_EN_SH);    
    SET_DESC_FIELD_NO_CLR(ptx_desc->Dword2, TX_DESC_BK, TX_DW2_BK_MSK, TX_DW2_BK_SH);
    SET_DESC_FIELD_NO_CLR(ptx_desc->Dword2, TX_DESC_MOREFRAG, TX_DW2_MOREFRAG_MSK, TX_DW2_MOREFRAG_SH);
    SET_DESC_FIELD_NO_CLR(ptx_desc->Dword2, TX_DESC_AMPDU_DENSITY, TX_DW2_AMPDU_DENSITY_MSK, TX_DW2_AMPDU_DENSITY_SH);    

    //4 Set Dword3
    SET_DESC_FIELD_NO_CLR(ptx_desc->Dword3, TX_DESC_USERATE, TX_DW3_USERATE_MSK, TX_DW3_USERATE_SH);
    SET_DESC_FIELD_NO_CLR(ptx_desc->Dword3, TX_DESC_DISRTSFB, TX_DW3_DISRTSFB_MSK, TX_DW3_DISRTSFB_SH);
    SET_DESC_FIELD_NO_CLR(ptx_desc->Dword3, TX_DESC_DISDATAFB, TX_DW3_DISDATAFB_MSK, TX_DW3_DISDATAFB_SH);
    SET_DESC_FIELD_NO_CLR(ptx_desc->Dword3, TX_DESC_CTS2SELF, TX_DW3_CTS2SELF_MSK, TX_DW3_CTS2SELF_SH);
    SET_DESC_FIELD_NO_CLR(ptx_desc->Dword3, TX_DESC_RTS_EN, TX_DW3_RTSEN_MSK, TX_DW3_RTSEN_SH);
    SET_DESC_FIELD_NO_CLR(ptx_desc->Dword3, TX_DESC_HW_RTS_EN, TX_DW3_HW_RTS_EN_MSK, TX_DW3_HW_RTS_EN_SH);
    SET_DESC_FIELD_NO_CLR(ptx_desc->Dword3, TX_DESC_NAVUSEHDR, TX_DW3_NAVUSEHDR_MSK, TX_DW3_NAVUSEHDR_SH);
    SET_DESC_FIELD_NO_CLR(ptx_desc->Dword3, TX_DESC_MAX_AGG_NUM, TX_DW3_MAX_AGG_NUM_MSK, TX_DW3_MAX_AGG_NUM_SH);

    //4 Set Dword4
    SET_DESC_FIELD_NO_CLR(ptx_desc->Dword4, TX_DESC_DATERATE, TX_DW4_DATARATE_MSK, TX_DW4_DATARATE_SH);
    SET_DESC_FIELD_NO_CLR(ptx_desc->Dword4, TX_DESC_DATA_RATEFB_LMT, TX_DW4_DATA_RATEFB_LMT_MSK, TX_DW4_DATA_RATEFB_LMT_SH);
    SET_DESC_FIELD_NO_CLR(ptx_desc->Dword4, TX_DESC_RTY_LMT_EN, TX_DW4_RTY_LMT_EN_MSK, TX_DW4_RTY_LMT_EN_SH);
    SET_DESC_FIELD_NO_CLR(ptx_desc->Dword4, TX_DESC_DATA_RT_LMT, TX_DW4_DATA_RT_LMT_MSK, TX_DW4_DATA_RT_LMT_SH);
    SET_DESC_FIELD_NO_CLR(ptx_desc->Dword4, TX_DESC_RTSRATE, TX_DW4_RTSRATE_MSK, TX_DW4_RTSRATE_SH);

    //4 Set Dword5
    SET_DESC_FIELD_NO_CLR(ptx_desc->Dword5, TX_DESC_DATA_SC, TX_DW5_DATA_SC_MSK, TX_DW5_DATA_SC_SH);
    SET_DESC_FIELD_NO_CLR(ptx_desc->Dword5, TX_DESC_DATA_SHORT, TX_DW5_DATA_SHORT_MSK, TX_DW5_DATA_SHORT_SH);
    SET_DESC_FIELD_NO_CLR(ptx_desc->Dword5, TX_DESC_DATA_BW, TX_DW5_DATA_BW_MSK, TX_DW5_DATA_BW_SH);
    SET_DESC_FIELD_NO_CLR(ptx_desc->Dword5, TX_DESC_DATA_STBC, TX_DW5_DATA_STBC_MSK, TX_DW5_DATA_STBC_SH);
    SET_DESC_FIELD_NO_CLR(ptx_desc->Dword5, TX_DESC_RTS_SHORT, TX_DW5_RTS_SHORT_MSK, TX_DW5_RTS_SHORT_SH);
    SET_DESC_FIELD_NO_CLR(ptx_desc->Dword5, TX_DESC_RTS_SC, TX_DW5_RTS_SC_MSK, TX_DW5_RTS_SC_SH);
    SET_DESC_FIELD_NO_CLR(ptx_desc->Dword5, TX_DESC_POWER_OFFSET, TX_DW5_TXPWR_OFSET_MSK, TX_DW5_TXPWR_OFSET_SH);

    //4 Set Dword6
#if CFG_HAL_SUPPORT_MBSSID
    if (HAL_IS_VAP_INTERFACE(Adapter)) {

    // set MBSSID for each VAP_ID
    SET_DESC_FIELD_NO_CLR(ptx_desc->Dword6,HAL_VAR_VAP_INIT_SEQ, TX_DW6_MBSSID_MSK, TX_DW6_MBSSID_SH);  

    }         
#endif //#if CFG_HAL_SUPPORT_MBSSID

    //4 Set Dword7
    SET_DESC_FIELD_NO_CLR(ptx_desc->Dword7, TX_DESC_TXBUFF, TX_DW7_SW_TXBUFF_MSK, TX_DW7_SW_TXBUFF_SH);

    //4 Set Dword9
    SET_DESC_FIELD_NO_CLR(ptx_desc->Dword9, TX_DESC_SEQ, TX_DW9_SEQ_MSK, TX_DW9_SEQ_SH);    
}

static VOID
UpdateSWTXBDHostIdx88XX (
    IN      HAL_PADAPTER                    Adapter,
    IN      PHCI_TX_DMA_QUEUE_STRUCT_88XX   cur_q
)
{
    cur_q->host_idx++;
    cur_q->host_idx = cur_q->host_idx % cur_q->total_txbd_num;
    cur_q->avail_txbd_num--;
}

HAL_IMEM
RT_STATUS
SyncSWTXBDHostIdxToHW88XX (
    IN      HAL_PADAPTER    Adapter,
    IN      u4Byte          queueIndex  //HCI_TX_DMA_QUEUE_88XX
)
{
    PHCI_TX_DMA_MANAGER_88XX        ptx_dma;
    PHCI_TX_DMA_QUEUE_STRUCT_88XX   cur_q;
    u2Byte                          LastHostIdx;

    ptx_dma = (PHCI_TX_DMA_MANAGER_88XX)(_GET_HAL_DATA(Adapter)->PTxDMA88XX);
    cur_q   = &(ptx_dma->tx_queue[queueIndex]);

    //Avoid that IO operation is completed before Packet is put into DRAM
    //So, add one read operation
    if ( cur_q->host_idx == 0 ) {
        LastHostIdx = cur_q->total_txbd_num - 1;
    }
    else {
        LastHostIdx = cur_q->host_idx - 1;
    }
    
    if ( 0 == cur_q->pTXBD_head[LastHostIdx].TXBD_ELE[1].Dword1 ) {
        RT_TRACE(COMP_INIT, DBG_SERIOUS, ("LastHostIdx: 0x%x, Address(0x%lx) Error\n",  LastHostIdx, cur_q->pTXBD_head[LastHostIdx].TXBD_ELE[1].Dword1));
        return RT_STATUS_FAILURE;
    }

    HAL_RTL_W16(cur_q->reg_rwptr_idx, cur_q->host_idx);

//    RT_TRACE_F(COMP_SEND, DBG_TRACE, ("cur_q->host_idx:0x%lx \n", cur_q->host_idx))

    return RT_STATUS_SUCCESS;
}

//Note: This function can't be used by Beacon
static VOID
SetTxBufferDesc88XX (
    IN      HAL_PADAPTER    Adapter,
    IN      u4Byte          queueIndex,  //HCI_TX_DMA_QUEUE_88XX
    IN      PVOID           pDescData 
)
{
    PTX_DESC_DATA_88XX              pdesc_data = (PTX_DESC_DATA_88XX)pDescData;
    PHCI_TX_DMA_MANAGER_88XX        ptx_dma;
    PHCI_TX_DMA_QUEUE_STRUCT_88XX   cur_q;
    PTX_BUFFER_DESCRIPTOR           cur_txbd;
    u1Byte                          i;
    u4Byte                          TotalLen    = 0;
    u4Byte                          PSBLen;    
    // if each queue num is different, need modify this number....
    u4Byte                          TXBDSegNum  = TXBD_ELE_NUM; 
    u4Byte                          hdrLen      = pdesc_data->hdrLen + pdesc_data->llcLen;
    u4Byte                          payloadLen  = pdesc_data->frLen;

    ptx_dma     = (PHCI_TX_DMA_MANAGER_88XX)(_GET_HAL_DATA(Adapter)->PTxDMA88XX);
    cur_q       = &(ptx_dma->tx_queue[queueIndex]);
    cur_txbd    = cur_q->pTXBD_head + cur_q->host_idx;

    // TODO: wapi..
    if (pdesc_data->secType != _NO_PRIVACY_) {
        // WEP:  1) icv for hw encrypt
        // TKIP: 1) iv contains eiv,   2) payload contains mic, 3) icv for hw encrypt
        // CCMP: 1) iv is CCMP header, 2) mic for hw encrypt
        hdrLen += pdesc_data->iv;
    }


	PlatformZeroMemory(&(cur_txbd->TXBD_ELE[1]), sizeof(TXBD_ELEMENT)*(TXBD_ELE_NUM-1));
	
    SET_DESC_FIELD_NO_CLR(cur_txbd->TXBD_ELE[1].Dword0,
                hdrLen,
                TXBD_DW0_TXBUFSIZE_MSK, TXBD_DW0_TXBUFSIZE_SH);

    SET_DESC_FIELD_NO_CLR(cur_txbd->TXBD_ELE[1].Dword1,
                HAL_VIRT_TO_BUS1(Adapter, (PVOID)pdesc_data->pHdr, hdrLen, HAL_PCI_DMA_TODEVICE),
                TXBD_DW1_PHYADDR_LOW_MSK, TXBD_DW1_PHYADDR_LOW_SH);

#if (TXBD_ELE_NUM >= 4)
    SET_DESC_FIELD_NO_CLR(cur_txbd->TXBD_ELE[2].Dword0,
                payloadLen,
                TXBD_DW0_TXBUFSIZE_MSK, TXBD_DW0_TXBUFSIZE_SH);

    if (payloadLen) {
        SET_DESC_FIELD_NO_CLR(cur_txbd->TXBD_ELE[2].Dword1,
                HAL_VIRT_TO_BUS1(Adapter, (PVOID)pdesc_data->pBuf, payloadLen, HAL_PCI_DMA_TODEVICE),
                TXBD_DW1_PHYADDR_LOW_MSK, TXBD_DW1_PHYADDR_LOW_SH);
    }
    
    // for sw encryption: 1) WEP's icv and TKIP's icv, 2) CCMP's mic, 3) no security
    if (pdesc_data->pIcv != NULL) {
        SET_DESC_FIELD_NO_CLR(cur_txbd->TXBD_ELE[3].Dword0,
                    pdesc_data->icv,
                    TXBD_DW0_TXBUFSIZE_MSK, TXBD_DW0_TXBUFSIZE_SH);
        
        SET_DESC_FIELD_NO_CLR(cur_txbd->TXBD_ELE[3].Dword1,
                    HAL_VIRT_TO_BUS1(Adapter, (PVOID)pdesc_data->pIcv, pdesc_data->icv, HAL_PCI_DMA_TODEVICE),
                    TXBD_DW1_PHYADDR_LOW_MSK, TXBD_DW1_PHYADDR_LOW_SH);

    } else if (pdesc_data->pMic != NULL) {
        SET_DESC_FIELD_NO_CLR(cur_txbd->TXBD_ELE[3].Dword0,
                    pdesc_data->mic,
                    TXBD_DW0_TXBUFSIZE_MSK, TXBD_DW0_TXBUFSIZE_SH);
        
        SET_DESC_FIELD_NO_CLR(cur_txbd->TXBD_ELE[3].Dword1,
                    HAL_VIRT_TO_BUS1(Adapter, (PVOID)pdesc_data->pMic, pdesc_data->mic, HAL_PCI_DMA_TODEVICE),
                    TXBD_DW1_PHYADDR_LOW_MSK, TXBD_DW1_PHYADDR_LOW_SH);
    } else {
        cur_txbd->TXBD_ELE[3].Dword0 = SET_DESC(0);
    }    
#endif  //   (TXBD_ELE_NUM >= 4) 

    // count total length for "dword0 Length0"
    for (i = 0; i < TXBDSegNum; i++) {
        TotalLen += GET_DESC_FIELD(cur_txbd->TXBD_ELE[i].Dword0, 
                    TXBD_DW0_TXBUFSIZE_MSK, TXBD_DW0_TXBUFSIZE_SH);        
    }

#if IS_EXIST_RTL8192EE
    if ( IS_HARDWARE_TYPE_8192EE(Adapter) ) {
        PSBLen   = (TotalLen%PBP_PSTX_SIZE) == 0 ? (TotalLen/PBP_PSTX_SIZE):((TotalLen/PBP_PSTX_SIZE)+1);        
    }
#endif
#if IS_EXIST_RTL8881AEM
    if ( IS_HARDWARE_TYPE_8881AEM(Adapter) ) {
        if (IS_HAL_TEST_CHIP(Adapter)) {
            PSBLen   = TotalLen;
        }
        else {
            PSBLen   = (TotalLen%PBP_PSTX_SIZE) == 0 ? (TotalLen/PBP_PSTX_SIZE):((TotalLen/PBP_PSTX_SIZE)+1);                    
        }
    }
#endif
    
    SET_DESC_FIELD_CLR(cur_txbd->TXBD_ELE[0].Dword0, PSBLen, 
                    TXBD_DW0_PSLEN_MSK, TXBD_DW0_PSLEN_SH);    

    //4 Cache flush
#ifdef CONFIG_NET_PCI
     if (((Adapter->pshare->type >> TYPE_SHIFT) & TYPE_MASK) == TYPE_PCI_BIOS){
          HAL_CACHE_SYNC_WBACK(Adapter, _GET_HAL_DATA(Adapter)->txBD_dma_ring_addr[queueIndex] + cur_q->host_idx * sizeof(TX_BUFFER_DESCRIPTOR), sizeof(TX_BUFFER_DESCRIPTOR), HAL_PCI_DMA_TODEVICE);
     } else 
#endif
     {
          HAL_CACHE_SYNC_WBACK(Adapter, cur_txbd, sizeof(TX_BUFFER_DESCRIPTOR), HAL_PCI_DMA_TODEVICE);
     }
    
    HAL_CACHE_SYNC_WBACK(Adapter, 
        GET_DESC_FIELD(cur_txbd->TXBD_ELE[0].Dword1, TXBD_DW1_PHYADDR_LOW_MSK, TXBD_DW1_PHYADDR_LOW_SH),
        SIZE_TXDESC_88XX, 
        HAL_PCI_DMA_TODEVICE);
    
    if (hdrLen) {
        HAL_CACHE_SYNC_WBACK(Adapter, 
            GET_DESC_FIELD(cur_txbd->TXBD_ELE[1].Dword1, TXBD_DW1_PHYADDR_LOW_MSK, TXBD_DW1_PHYADDR_LOW_SH), 
            hdrLen, 
            HAL_PCI_DMA_TODEVICE);
    }
    
    if (payloadLen) {
        HAL_CACHE_SYNC_WBACK(Adapter, 
            GET_DESC_FIELD(cur_txbd->TXBD_ELE[2].Dword1, TXBD_DW1_PHYADDR_LOW_MSK, TXBD_DW1_PHYADDR_LOW_SH), 
            payloadLen, 
            HAL_PCI_DMA_TODEVICE);
    }

    if (pdesc_data->pIcv != NULL) {
        HAL_CACHE_SYNC_WBACK(Adapter, 
            GET_DESC_FIELD(cur_txbd->TXBD_ELE[3].Dword1, TXBD_DW1_PHYADDR_LOW_MSK, TXBD_DW1_PHYADDR_LOW_SH), 
            pdesc_data->icv, 
            HAL_PCI_DMA_TODEVICE);
    } else if (pdesc_data->pMic != NULL) {
        HAL_CACHE_SYNC_WBACK(Adapter, 
            GET_DESC_FIELD(cur_txbd->TXBD_ELE[3].Dword1, TXBD_DW1_PHYADDR_LOW_MSK, TXBD_DW1_PHYADDR_LOW_SH),
            pdesc_data->mic, 
            HAL_PCI_DMA_TODEVICE);
    }
    
#if 0 // CFG_HAL_DBG

    RT_TRACE_F(COMP_SEND, DBG_TRACE, ("\n\nq_idx: %d, txbd[%d], \n", queueIndex, cur_q->host_idx));

    //TXDESC
    RT_TRACE_F(COMP_SEND, DBG_TRACE, ("TXBD_ELE[%d], Dword0: 0x%x, Dword1: 0x%lx\n", 0, GET_DESC(cur_txbd->TXBD_ELE[0].Dword0), GET_DESC(cur_txbd->TXBD_ELE[0].Dword1)));
    PTX_DESC_88XX   ptx_desc = ((PTX_DESC_88XX)(cur_q->ptx_desc_head)) + cur_q->host_idx;
    RT_PRINT_DATA(COMP_SEND, DBG_TRACE, "TXDESC:\n", ptx_desc, sizeof(TX_DESC_88XX));

    //Header
//    RT_TRACE_F(COMP_SEND, DBG_TRACE, ("TXBD_ELE[%d], Dword0: 0x%lx, Dword1: 0x%lx\n", 1, GET_DESC(cur_txbd->TXBD_ELE[1].Dword0), GET_DESC(cur_txbd->TXBD_ELE[1].Dword1)));
//    if(pdesc_data->iv != 0) {
//        RT_PRINT_DATA(COMP_SEND, DBG_TRACE, "Header(+iv):\n", pdesc_data->pHdr, pdesc_data->hdrLen + pdesc_data->iv + pdesc_data->llcLen);
//    } else {
//        RT_PRINT_DATA(COMP_SEND, DBG_TRACE, "Header:\n", pdesc_data->pHdr, pdesc_data->hdrLen + pdesc_data->llcLen);
//    }

    //Payload
//    RT_TRACE_F(COMP_SEND, DBG_TRACE, ("TXBD_ELE[%d], Dword0: 0x%lx, Dword1: 0x%lx\n", 2, GET_DESC(cur_txbd->TXBD_ELE[2].Dword0), GET_DESC(cur_txbd->TXBD_ELE[2].Dword1)));
//    RT_PRINT_DATA(COMP_SEND, DBG_TRACE, "Payload:\n", pdesc_data->pBuf, pdesc_data->frLen);    

    //MIC or ICV
//    RT_TRACE_F(COMP_SEND, DBG_TRACE, ("TXBD_ELE[%d], Dword0: 0x%lx, Dword1: 0x%lx\n", 3, GET_DESC(cur_txbd->TXBD_ELE[3].Dword0), GET_DESC(cur_txbd->TXBD_ELE[3].Dword1)));    
//    RT_PRINT_DATA(COMP_SEND, DBG_TRACE, "Icv:\n", pdesc_data->pIcv, pdesc_data->icv);

#endif  //CFG_HAL_DBG

    UpdateSWTXBDHostIdx88XX(Adapter, cur_q);

}


BOOLEAN
FillTxHwCtrl88XX(
    IN      HAL_PADAPTER    Adapter,
    IN      u4Byte          queueIndex,  //HCI_TX_DMA_QUEUE_88XX
    IN      PVOID           pDescData
)
{
    FillTxDesc88XX(Adapter, queueIndex, pDescData);
    SetTxBufferDesc88XX(Adapter, queueIndex, pDescData);
    
    return _TRUE;
}

HAL_IMEM
BOOLEAN
QueryTxConditionMatch88XX(
    IN	HAL_PADAPTER        Adapter
)
{
    PHCI_TX_DMA_MANAGER_88XX    ptx_dma;
    HCI_TX_DMA_QUEUE_88XX       QueueIdx;
    HCI_TX_DMA_QUEUE_88XX       q_max;
    u32                         count = TX_CONDITION_MATCH_TXBD_CNT;

#ifdef CFG_HAL_SUPPORT_MBSSID
    // excluding beacon queue...
    q_max = HCI_TX_DMA_QUEUE_HI7;
#else
    // only check MGT, VO, VI, BE, BK, HI0 queue
    q_max = HCI_TX_DMA_QUEUE_HI0;
#endif

    ptx_dma = (PHCI_TX_DMA_MANAGER_88XX)(_GET_HAL_DATA(Adapter)->PTxDMA88XX);

    for (QueueIdx = 0; QueueIdx <= q_max; QueueIdx++)
    {
        if (HAL_CIRC_CNT_RTK(ptx_dma->tx_queue[QueueIdx].host_idx, \
                    ptx_dma->tx_queue[QueueIdx].hw_idx, \
                    ptx_dma->tx_queue[QueueIdx].total_txbd_num) > count)
            return _TRUE;
    }
    return _FALSE;
}


VOID
TxPolling88XX(
    IN	HAL_PADAPTER        Adapter,
    IN	u1Byte              QueueIndex
)
{
    //QueueIndex, ex. TXPOLL_BEACON_QUEUE

    if ( TXPOLL_BEACON_QUEUE == QueueIndex ) {
        PlatformEFIOWrite2Byte(Adapter, REG_RX_RXBD_NUM, PlatformEFIORead2Byte(Adapter, REG_RX_RXBD_NUM) | BIT12);
    }
    else {
        //Do Nothing
    }
}


static VOID
FillBeaconDesc88XX
(
    IN	HAL_PADAPTER        Adapter,
    IN  PTX_DESC_88XX       pdesc,
    IN  PVOID               dat_content,
    IN  u2Byte              txLength,
    IN  BOOLEAN             bForceUpdate
)
{
    PlatformZeroMemory(pdesc, SIZE_TXDESC_88XX);

    //Dword0
    SET_DESC_FIELD_CLR(pdesc->Dword0, 1, TX_DW0_BMC_MSK, TX_DW0_BMC_SH);
    SET_DESC_FIELD_CLR(pdesc->Dword0, HAL_TXDESC_OFFSET_SIZE, TX_DW0_OFFSET_MSK, TX_DW0_OFFSET_SH);
    SET_DESC_FIELD_CLR(pdesc->Dword0, txLength, TX_DW0_TXPKSIZE_MSK, TX_DW0_TXPKSIZE_SH);

    //Dword1
    SET_DESC_FIELD_CLR(pdesc->Dword1, TXDESC_QSEL_BCN, TX_DW1_QSEL_MSK, TX_DW1_QSEL_SH);


#if CFG_HAL_SUPPORT_MBSSID
        if (HAL_IS_VAP_INTERFACE(Adapter)) {
    
        // set MBSSID for each VAP_ID
      
        SET_DESC_FIELD_CLR(pdesc->Dword1, HAL_VAR_VAP_INIT_SEQ, TX_DW1_MACID_MSK, TX_DW1_MACID_SH);
        SET_DESC_FIELD_CLR(pdesc->Dword6, HAL_VAR_VAP_INIT_SEQ, TX_DW6_MBSSID_MSK, TX_DW6_MBSSID_SH);  
    
        }         
#endif //#if CFG_HAL_SUPPORT_MBSSID


    SET_DESC_FIELD_CLR(pdesc->Dword9, GetSequence(dat_content), TX_DW9_SEQ_MSK, TX_DW9_SEQ_SH);

    SET_DESC_FIELD_CLR(pdesc->Dword3, 1, TX_DW3_DISDATAFB_MSK, TX_DW3_DISDATAFB_SH);
    SET_DESC_FIELD_CLR(pdesc->Dword3, 1, TX_DW3_USERATE_MSK, TX_DW3_USERATE_SH);

    if (HAL_VAR_IS_40M_BW) {
        if (HAL_VAR_OFFSET_2ND_CHANNEL == HT_2NDCH_OFFSET_BELOW) {
            SET_DESC_FIELD_CLR(pdesc->Dword5, TXDESC_DATASC_LOWER, TX_DW5_DATA_SC_MSK, TX_DW5_DATA_SC_SH);
        }
		else {
            SET_DESC_FIELD_CLR(pdesc->Dword5, TXDESC_DATASC_UPPER, TX_DW5_DATA_SC_MSK, TX_DW5_DATA_SC_SH);
		}
	}

    // TODO: Why ?
    HAL_VAR_IS_40M_BW_BAK   = HAL_VAR_IS_40M_BW;
    HAL_VAR_TX_BEACON_LEN   = txLength;

    SET_DESC_FIELD_CLR(pdesc->Dword7, txLength, TX_DW7_SW_TXBUFF_MSK, TX_DW7_SW_TXBUFF_SH);


#if 0   // TODO: Filen: test code ?
#if (defined(UNIVERSAL_REPEATER) || defined(MBSSID))
    if (IS_ROOT_INTERFACE(Adapter)) {
		if (Adapter->pshare->rf_ft_var.swq_dbg	== 30) {
			pdesc->Dword9 |= set_desc((1122 & TXdesc_92E_TX_SeqMask)  << TXdesc_92E_TX_SeqSHIFT);
		}
		else {
			pdesc->Dword9 |= set_desc((5566 & TXdesc_92E_TX_SeqMask)  << TXdesc_92E_TX_SeqSHIFT);
		}
    }
    else {
        pdesc->Dword9 |= set_desc((GetSequence(dat_content) & TXdesc_92E_TX_SeqMask)  << TXdesc_92E_TX_SeqSHIFT);        
    }
#else
    if (Adapter->pshare->rf_ft_var.swq_dbg == 30) {
        pdesc->Dword9 |= set_desc((1122 & TXdesc_92E_TX_SeqMask)  << TXdesc_92E_TX_SeqSHIFT);
    }
    else {
        pdesc->Dword9 |= set_desc((5566 & TXdesc_92E_TX_SeqMask)  << TXdesc_92E_TX_SeqSHIFT);
    }
#endif  //(defined(UNIVERSAL_REPEATER) || defined(MBSSID))
#endif

    // Group Bit Control
    SET_DESC_FIELD_CLR(pdesc->Dword9, (HAL_VAR_TIM_OFFSET-24), TX_DW9_GROUPBIT_IE_OFFSET_MSK, TX_DW9_GROUPBIT_IE_OFFSET_SH);
    SET_DESC_FIELD_CLR(pdesc->Dword9, 1, TX_DW9_GROUPBIT_IE_ENABLE_MSK, TX_DW9_GROUPBIT_IE_ENABLE_SH);

    // TODO: Check with Button
}


VOID
SigninBeaconTXBD88XX
(
    IN	HAL_PADAPTER        Adapter,
    IN  pu4Byte             beaconbuf,
    IN  u2Byte              frlen
)
{   
    PTX_BUFFER_DESCRIPTOR           pTXBD;
    PTX_DESC_88XX                   ptx_desc;
    PHCI_TX_DMA_MANAGER_88XX        ptx_dma = (PHCI_TX_DMA_MANAGER_88XX)(_GET_HAL_DATA(Adapter)->PTxDMA88XX);
    u4Byte                          TXBDBeaconOffset;
    BOOLEAN                         bUpdate = 0;
    u4Byte                          TotalLen;
    u4Byte                          PSBLen;

    //3 Get TXBD PTR & Get TXDESC PTR
#if IS_RTL8192E_SERIES
    if ( IS_HARDWARE_TYPE_8192EE(Adapter) ) {
        TXBDBeaconOffset = TXBD_BEACON_OFFSET_8192E;        
    }
#endif  //IS_RTL8192E_SERIES
    
#if IS_RTL8881A_SERIES
    if ( IS_HARDWARE_TYPE_8881A(Adapter) ) {
        TXBDBeaconOffset = TXBD_BEACON_OFFSET_8881A;        
    }
#endif  //IS_RTL8881A_SERIES

    #if CFG_HAL_DBG
    //Error Check
    if ( TXBDBeaconOffset % sizeof(TX_BUFFER_DESCRIPTOR) != 0 ) {
        RT_TRACE(COMP_SEND, DBG_SERIOUS, ("TXBDBeaconOffset is mismatched\n")); 
        return;
    }
    #endif  //CFG_HAL_DBG
    
#if CFG_HAL_SUPPORT_MBSSID
    if (HAL_IS_VAP_INTERFACE(Adapter)) {
        pTXBD       = (PTX_BUFFER_DESCRIPTOR)((u4Byte)ptx_dma->tx_queue[HCI_TX_DMA_QUEUE_BCN].pTXBD_head + \
                (HAL_VAR_VAP_INIT_SEQ * TXBDBeaconOffset));
        
        ptx_desc    = (PTX_DESC_88XX)((u4Byte)ptx_dma->tx_queue[HCI_TX_DMA_QUEUE_BCN].ptx_desc_head + 
                    HAL_VAR_VAP_INIT_SEQ*sizeof(TX_DESC_88XX));
    }
    else {
        pTXBD       = ptx_dma->tx_queue[HCI_TX_DMA_QUEUE_BCN].pTXBD_head;
        ptx_desc    = (PTX_DESC_88XX)ptx_dma->tx_queue[HCI_TX_DMA_QUEUE_BCN].ptx_desc_head;
    }
#else
    pTXBD       = ptx_dma->tx_queue[HCI_TX_DMA_QUEUE_BCN].pTXBD_head;
    ptx_desc    = (PTX_DESC_88XX)ptx_dma->tx_queue[HCI_TX_DMA_QUEUE_BCN].ptx_desc_head;
#endif

#if 0
        //TXDESC
        RT_TRACE_F(COMP_SEND, DBG_TRACE, ("TXBD_ELE[%d], Dword0: 0x%lx, Dword1: 0x%lx\n", 0, GET_DESC(pTXBD->TXBD_ELE[0].Dword0), GET_DESC(pTXBD->TXBD_ELE[0].Dword1)));
        RT_PRINT_DATA(COMP_SEND, DBG_TRACE, "TXDESC:\n", ptx_desc, sizeof(TX_DESC_88XX));

        //Header + Payload
        RT_TRACE_F(COMP_SEND, DBG_TRACE, ("TXBD_ELE[%d], Dword0: 0x%lx, Dword1: 0x%lx\n", 1, GET_DESC(pTXBD->TXBD_ELE[1].Dword0), GET_DESC(pTXBD->TXBD_ELE[1].Dword1)));
        RT_PRINT_DATA(COMP_SEND, DBG_TRACE, "Content:\n", beaconbuf, frlen);

        RT_TRACE_F(COMP_SEND, DBG_TRACE, ("TXBD_ELE[%d], Dword0: 0x%lx, Dword1: 0x%lx\n", 2, GET_DESC(pTXBD->TXBD_ELE[2].Dword0), GET_DESC(pTXBD->TXBD_ELE[2].Dword1)));
        RT_TRACE_F(COMP_SEND, DBG_TRACE, ("TXBD_ELE[%d], Dword0: 0x%lx, Dword1: 0x%lx\n\n\n", 3, GET_DESC(pTXBD->TXBD_ELE[3].Dword0), GET_DESC(pTXBD->TXBD_ELE[3].Dword1)));

        DumpTxPktBuf(Adapter);

        PHCI_RX_DMA_MANAGER_88XX prx_dma = (PHCI_RX_DMA_MANAGER_88XX)(_GET_HAL_DATA(Adapter)->PRxDMA88XX);
        PHCI_TX_DMA_MANAGER_88XX ptx_dma = (PHCI_TX_DMA_MANAGER_88XX)(_GET_HAL_DATA(Adapter)->PTxDMA88XX);

        //No Beacon
        PTX_BUFFER_DESCRIPTOR ptxbd_head = (PTX_BUFFER_DESCRIPTOR)(prx_dma->rx_queue[HCI_RX_DMA_QUEUE_MAX_NUM-1].pRXBD_head + \
        prx_dma->rx_queue[HCI_RX_DMA_QUEUE_MAX_NUM-1].total_rxbd_num);

        PTX_DESC_88XX ptx_desc_head = (PTX_DESC_88XX)((pu1Byte)ptxbd_head + \
        sizeof(TX_BUFFER_DESCRIPTOR) * TOTAL_NUM_TXBD_NO_BCN);

        PTX_BUFFER_DESCRIPTOR ptxbd_bcn_head  = (PTX_BUFFER_DESCRIPTOR)((pu1Byte)ptx_desc_head + \
        sizeof(TX_DESC_88XX) * TOTAL_NUM_TXBD_NO_BCN);

        PTX_DESC_88XX ptxdesc_bcn_head = (PTX_DESC_88XX)((pu1Byte)ptxbd_bcn_head + \
          (1+HAL_NUM_VWLAN) * TXBD_BEACON_OFFSET_8192E);
          
        PTX_BUFFER_DESCRIPTOR ptxbd_bcn_cur;

        PTX_BUFFER_DESCRIPTOR ptxbd = ptx_dma->tx_queue[HCI_TX_DMA_QUEUE_BCN].pTXBD_head;
        PTX_DESC_88XX ptx_desc      = ptx_dma->tx_queue[HCI_TX_DMA_QUEUE_BCN].ptx_desc_head;
        u4Byte i;

        for (i = 0; i < 1+HAL_NUM_VWLAN; i++)
        {
           ptxbd_bcn_cur = (pu1Byte)ptxbd + TXBD_BEACON_OFFSET_8192E * i;
               RT_TRACE_F(COMP_INIT, DBG_TRACE, ("ptxbd_bcn[%ld]: Dword0: 0x%lx, Dword1: 0x%lx\n", \
                          i, \
                          (u4Byte)GET_DESC(ptxbd_bcn_cur->TXBD_ELE[0].Dword0), \
                          (u4Byte)GET_DESC(ptxbd_bcn_cur->TXBD_ELE[0].Dword1)
                          ));             
        }

#endif

    // TODO: Pedro, it should be move to start location of update_beacon...
    // TODO: because preparing beacon content also need to be protected...
    //Clear Own Bit
    SET_DESC_FIELD_CLR(pTXBD->TXBD_ELE[0].Dword0, 0, TXBD_DW0_BCN_OWN_MSK, TXBD_DW0_BCN_OWN_SH);

#if 0   // TODO: Filen
#ifdef DFS
        if (!priv->pmib->dot11DFSEntry.disable_DFS &&
            (timer_pending(&GET_ROOT(priv)->ch_avail_chk_timer))) {
            pdesc->Dword0 &= set_desc(~(TX_OWN));
            RTL_W16(PCIE_CTRL_REG, RTL_R16(PCIE_CTRL_REG)| (BCNQSTOP));
    
            return;
        }
#endif
#endif

    FillBeaconDesc88XX(Adapter, ptx_desc, (PVOID)beaconbuf, frlen, _FALSE);

    //Segment 1: Payload
    SET_DESC_FIELD_CLR(pTXBD->TXBD_ELE[1].Dword0, frlen, TXBD_DW0_TXBUFSIZE_MSK, TXBD_DW0_TXBUFSIZE_SH);
    SET_DESC_FIELD_CLR(pTXBD->TXBD_ELE[1].Dword1, \
                HAL_VIRT_TO_BUS1(Adapter, (PVOID)beaconbuf, frlen, HAL_PCI_DMA_TODEVICE), \
                TXBD_DW1_PHYADDR_LOW_MSK, TXBD_DW1_PHYADDR_LOW_SH);

    //Segment 0: Wifi Info
    //PrepareTxDesc88XX has done    
    TotalLen = sizeof(TX_DESC_88XX) + frlen;

#if IS_EXIST_RTL8192EE
    if ( IS_HARDWARE_TYPE_8192EE(Adapter) ) {
        PSBLen   = (TotalLen%PBP_PSTX_SIZE) == 0 ? (TotalLen/PBP_PSTX_SIZE):((TotalLen/PBP_PSTX_SIZE)+1);        
    }
#endif

#if IS_EXIST_RTL8881AEM
    if ( IS_HARDWARE_TYPE_8881AEM(Adapter) ) {
        PSBLen   = TotalLen;
    }
#endif
    
    SET_DESC_FIELD_CLR(pTXBD->TXBD_ELE[0].Dword0, PSBLen, TXBD_DW0_BCN_PSLEN_MSK, TXBD_DW0_BCN_PSLEN_SH);

    pTXBD->TXBD_ELE[2].Dword0 = SET_DESC(0);
    pTXBD->TXBD_ELE[3].Dword0 = SET_DESC(0);    

    //3 Write Cache Sync Back
    //write back cache: TXBD
#ifdef CONFIG_NET_PCI
    if (((Adapter->pshare->type >> TYPE_SHIFT) & TYPE_MASK) == TYPE_PCI_BIOS) {
        u4Byte uiTmp=0;
#if CFG_HAL_SUPPORT_MBSSID
        if (HAL_IS_VAP_INTERFACE(Adapter)) {
            uiTmp=HAL_VAR_VAP_INIT_SEQ * TXBDBeaconOffset;
        }
#endif
        HAL_CACHE_SYNC_WBACK(Adapter, _GET_HAL_DATA(Adapter)->txBD_dma_ring_addr[HCI_TX_DMA_QUEUE_BCN] + uiTmp, sizeof(TX_BUFFER_DESCRIPTOR), HAL_PCI_DMA_TODEVICE);
    } else 
 #endif
    {
        HAL_CACHE_SYNC_WBACK(Adapter, HAL_VIRT_TO_BUS1(Adapter, (PVOID)pTXBD, sizeof(TX_BUFFER_DESCRIPTOR), HAL_PCI_DMA_TODEVICE),
                        sizeof(TX_BUFFER_DESCRIPTOR), HAL_PCI_DMA_TODEVICE);
    }
    
    //write back cache: TXDESC    
    HAL_CACHE_SYNC_WBACK(Adapter,
        GET_DESC_FIELD(pTXBD->TXBD_ELE[0].Dword1, TXBD_DW1_PHYADDR_LOW_MSK, TXBD_DW1_PHYADDR_LOW_SH),
        sizeof(TX_DESC_88XX), HAL_PCI_DMA_TODEVICE);
    
    //write back cache: Payload    
    HAL_CACHE_SYNC_WBACK(Adapter,
        GET_DESC_FIELD(pTXBD->TXBD_ELE[1].Dword1, TXBD_DW1_PHYADDR_LOW_MSK, TXBD_DW1_PHYADDR_LOW_SH),
        (u4Byte)frlen, HAL_PCI_DMA_TODEVICE);
   
    //Set Own Bit
    SET_DESC_FIELD_CLR(pTXBD->TXBD_ELE[0].Dword0, 1, TXBD_DW0_BCN_OWN_MSK, TXBD_DW0_BCN_OWN_SH);
}



u2Byte
GetTxQueueHWIdx88XX
(
    IN	HAL_PADAPTER        Adapter,
    IN  u4Byte              q_num       //enum _TX_QUEUE_
)
{
    PHCI_TX_DMA_MANAGER_88XX    ptx_dma;

    ptx_dma = (PHCI_TX_DMA_MANAGER_88XX)(_GET_HAL_DATA(Adapter)->PTxDMA88XX);

    return (BIT_MASK_QUEUE_IDX &
        (HAL_RTL_R32(ptx_dma->tx_queue[MappingTxQueue88XX(Adapter, q_num)].reg_rwptr_idx)>>BIT_SHIFT_QUEUE_HW_IDX));    
}

#if CFG_HAL_TX_SHORTCUT

#if 0
PVOID
GetShortCutTxDesc88XX(
    IN  HAL_PADAPTER    Adapter
)
{
    // TODO: pre-allocate a TXDESC pool when system startup
    return (PVOID)HALMalloc(Adapter, sizeof(TX_DESC_88XX));
}

VOID
ReleaseShortCutTxDesc88XX(
    IN  HAL_PADAPTER    Adapter,
    IN  PVOID           pTxDesc
)
{
    // TODO: right ?, release to the TXDESC pool
    HAL_free(pTxDesc);
}
#endif

HAL_IMEM
VOID
SetShortCutTxBuffSize88XX(
    IN  HAL_PADAPTER    Adapter,
    IN  PVOID           pTxDesc,
    IN  u2Byte          txBuffSize
)
{
    PTX_DESC_88XX   ptx_desc = (PTX_DESC_88XX) pTxDesc;
    SET_DESC_FIELD_CLR(ptx_desc->Dword7, txBuffSize, TX_DW7_SW_TXBUFF_MSK, TX_DW7_SW_TXBUFF_SH);
}

HAL_IMEM
u2Byte
GetShortCutTxBuffSize88XX(
    IN  HAL_PADAPTER    Adapter,
    IN  PVOID           pTxDesc
)
{
    PTX_DESC_88XX   ptx_desc = (PTX_DESC_88XX) pTxDesc;
    return  (u2Byte)GET_DESC_FIELD(ptx_desc->Dword7, TX_DW7_SW_TXBUFF_MSK, TX_DW7_SW_TXBUFF_SH);
}

/**
 * direction: 
 *      1) 0x01: store current TXBD's txdesc to driver layer
 *      2) 0x02: copy backup txdesc from driver layer to current TXBD's TXDESC
 */
HAL_IMEM
PVOID
CopyShortCutTxDesc88XX(
    IN  HAL_PADAPTER    Adapter,
    IN  u4Byte          queueIndex, //HCI_TX_DMA_QUEUE_88XX    
    IN  PVOID           pTxDesc,
    IN  u4Byte          direction
)
{
    PHCI_TX_DMA_MANAGER_88XX        ptx_dma;
    PHCI_TX_DMA_QUEUE_STRUCT_88XX   cur_q;
    PTX_DESC_88XX                   cur_txdesc;
#if 1// CFG_HAL_DBG
    PTX_BUFFER_DESCRIPTOR           cur_txbd;
#endif // CFG_HAL_DBG

    ptx_dma     = (PHCI_TX_DMA_MANAGER_88XX)(_GET_HAL_DATA(Adapter)->PTxDMA88XX);
    cur_q       = &(ptx_dma->tx_queue[queueIndex]);
    cur_txdesc  = (PTX_DESC_88XX)cur_q->ptx_desc_head + cur_q->host_idx;

#if 0 // CFG_HAL_DBG
    cur_txbd    = cur_q->pTXBD_head + cur_q->host_idx;

    if (HAL_VIRT_TO_BUS((u4Byte)cur_txdesc) != GET_DESC(cur_txbd->TXBD_ELE[0].Dword1)) {
        printk("%s(%d): cur_txdesc: 0x%08x, cur_txbd[0].Dword1: 0x%08x \n", __FUNCTION__, __LINE__,
            HAL_VIRT_TO_BUS((u4Byte)cur_txdesc), GET_DESC(cur_txbd->TXBD_ELE[0].Dword1));
    }    
#endif // CFG_HAL_DBG

    if (0x01 == direction) {
        HAL_memcpy(pTxDesc, cur_txdesc, SIZE_TXDESC_88XX);
    } else { // 0x02 == direction
        HAL_memcpy(cur_txdesc, pTxDesc, SIZE_TXDESC_88XX);
    }

    return cur_txdesc;
}

HAL_IMEM
static VOID
FillShortCutTxDesc88XX(
    IN      HAL_PADAPTER    Adapter,
    IN      u4Byte          queueIndex,  //HCI_TX_DMA_QUEUE_88XX
    IN      PVOID           pDescData,
    IN      PVOID           pTxDesc
)
{
    PTX_DESC_DATA_88XX  pdesc_data  = (PTX_DESC_DATA_88XX)pDescData;
    PTX_DESC_88XX       ptx_desc    = (PTX_DESC_88XX)pTxDesc;
   
    // tx shortcut can reuse TXDESC while 1) no security or 2) hw security
    // if no security iv == 0, so adding iv is ok for no security and hw security
    u2Byte  TX_DESC_TXPKTSIZE   = pdesc_data->hdrLen + pdesc_data->llcLen + pdesc_data->frLen + pdesc_data->iv;
    BOOLEAN TX_DESC_BK          = pdesc_data->bk;
    BOOLEAN TX_DESC_RTY_LMT_EN  = pdesc_data->rtyLmtEn;   
    u1Byte  TX_DESC_DATA_RT_LMT = pdesc_data->dataRtyLmt;
    BOOLEAN TX_DESC_NAVUSEHDR   = pdesc_data->navUseHdr;
    u2Byte  TX_DESC_SEQ         = GetSequence(pdesc_data->pHdr);

    SET_DESC_FIELD_CLR(ptx_desc->Dword0, TX_DESC_TXPKTSIZE, TX_DW0_TXPKSIZE_MSK, TX_DW0_TXPKSIZE_SH);
    SET_DESC_FIELD_CLR(ptx_desc->Dword2, TX_DESC_BK, TX_DW2_BK_MSK, TX_DW2_BK_SH);
    SET_DESC_FIELD_CLR(ptx_desc->Dword3, TX_DESC_NAVUSEHDR, TX_DW3_NAVUSEHDR_MSK, TX_DW3_NAVUSEHDR_SH);
    SET_DESC_FIELD_CLR(ptx_desc->Dword4, TX_DESC_RTY_LMT_EN, TX_DW4_RTY_LMT_EN_MSK, TX_DW4_RTY_LMT_EN_SH);
    SET_DESC_FIELD_CLR(ptx_desc->Dword4, TX_DESC_DATA_RT_LMT, TX_DW4_DATA_RT_LMT_MSK, TX_DW4_DATA_RT_LMT_SH);
    SET_DESC_FIELD_CLR(ptx_desc->Dword9, TX_DESC_SEQ, TX_DW9_SEQ_MSK, TX_DW9_SEQ_SH);
}

HAL_IMEM
BOOLEAN
FillShortCutTxHwCtrl88XX(
    IN      HAL_PADAPTER    Adapter,
    IN      u4Byte          queueIndex,  //HCI_TX_DMA_QUEUE_88XX
    IN      PVOID           pDescData,
    IN      PVOID           pTxDesc,
    IN      u4Byte          direction    
)
{
    PTX_DESC_88XX ptx_desc;
    PTX_DESC_DATA_88XX pdesc_data  = (PTX_DESC_DATA_88XX)pDescData;

    if (0x01 == direction) {
        FillTxDesc88XX(Adapter, queueIndex, pDescData);
        CopyShortCutTxDesc88XX(Adapter, queueIndex, pTxDesc, direction);
    } else {    // 0x02 == direction
        ptx_desc = (PTX_DESC_88XX)CopyShortCutTxDesc88XX(Adapter, queueIndex, pTxDesc, direction);
        FillShortCutTxDesc88XX(Adapter, queueIndex, pDescData, ptx_desc);
    }

    SetTxBufferDesc88XX(Adapter, queueIndex, pDescData);

    return _TRUE;
}
#endif // CFG_HAL_TX_SHORTCUT



