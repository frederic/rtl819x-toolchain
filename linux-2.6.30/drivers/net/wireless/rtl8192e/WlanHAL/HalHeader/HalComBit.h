#ifndef __RTL_WLAN_BITDEF_H__
#define __RTL_WLAN_BITDEF_H__

/*-------------------------Modification Log-----------------------------------
    Base on MAC_Register.doc SVN391
-------------------------Modification Log-----------------------------------*/

/*--------------------------Include File--------------------------------------*/
#include "HalHWCfg.h"
/*--------------------------Include File--------------------------------------*/

//3 ============Programming guide Start=====================
/*
    1. For all bit define, it should be prefixed by "BIT_"
    2. For all bit mask, it should be prefixed by "BIT_MASK_"
    3. For all bit shift, it should be prefixed by "BIT_SHIFT_"
    4. For other case, prefix is not needed

Example:
#define BIT_SHIFT_MAX_TXDMA         16
#define BIT_MASK_MAX_TXDMA          0x7
#define BIT_MAX_TXDMA(x)                (((x) & BIT_MASK_MAX_TXDMA)<<BIT_SHIFT_MAX_TXDMA)

    
*/
//3 ============Programming guide End=====================




// TODO: It is necessary to add 8188E 
//4 HIMR0/HISR0 (0xB0~0xB4)
#if CONFIG_WLANREG_SUPPORT & SUPPORT_CHIP_8723A
//8723A is reserved (0xB0~0xB4)
#endif

#if CONFIG_WLANREG_SUPPORT & ~SUPPORT_CHIP_8723A
#define BIT_RXOK        					BIT(0)
#define BIT_RDU         					BIT(1)
#define BIT_VODOK   						BIT(2)
#define BIT_VIDOK   						BIT(3)
#define BIT_BEDOK   						BIT(4)
#define BIT_BKDOK   						BIT(5)
#define BIT_MGTDOK							BIT(6)
#define BIT_HIGHDOK							BIT(7)
#define BIT_CPWM_INT						BIT(8)
#define BIT_CPWM2_INT						BIT(9)
#define BIT_C2HCMD_INT  					BIT(10)

#if CONFIG_WLANREG_SUPPORT & (SUPPORT_CHIP_8881A|SUPPORT_CHIP_8192E|SUPPORT_CHIP_8723B|SUPPORT_CHIP_8821A|SUPPORT_CHIP_8812A)
//BIT11 RESERVED
#endif

#define BIT_CTWEND  						BIT(12)
//BIT13 RESERVED
#define BIT_BCNDMAINT_E						BIT(14)

#if CONFIG_WLANREG_SUPPORT & (SUPPORT_CHIP_8881A|SUPPORT_CHIP_8192E|SUPPORT_CHIP_8723B|SUPPORT_CHIP_8821A|SUPPORT_CHIP_8812A)
//BIT15 RESERVED
#endif

#define BIT_BCNDERR0                        BIT(16)
//BIT17~19 RESERVED
#define BIT_BCNDMAINT0     					BIT(20)

#if CONFIG_WLANREG_SUPPORT & (SUPPORT_CHIP_8881A|SUPPORT_CHIP_8192E|SUPPORT_CHIP_8723B|SUPPORT_CHIP_8821A|SUPPORT_CHIP_8812A)
//BIT21 ~ 23 RESERVED
#endif

#define BIT_TSF_BIT32_TOGGLE				BIT(24)
#define BIT_TXBCNOK   						BIT(25)
#define BIT_TXBCNERR						BIT(26)
#define BIT_GTINT3							BIT(27)
#define BIT_GTINT4  						BIT(28)
#define BIT_PSTIMEOUT						BIT(29)
#define BIT_TIMEOUT1   		    			BIT(30)
#define BIT_TIMEOUT2   		    			BIT(31)
#endif  //#if CONFIG_WLANREG_SUPPORT & ~SUPPORT_CHIP_8723A


// TODO: It is necessary to add 8188E 
// have synced RTL8195_MACREG_R56
//4 HIMR1/HISR1 (0xB8~0xBC)
#if CONFIG_WLANREG_SUPPORT & SUPPORT_CHIP_8723A
//8723A is reserved (0xB8~0xBC)
#endif

#if CONFIG_WLANREG_SUPPORT & ~SUPPORT_CHIP_8723A
//BIT0 ~ 7  RESERVED
#define BIT_FOVW                            BIT(8)
#define BIT_TXFOVW                          BIT(9)
#define BIT_RXERR_INT                       BIT(10)
#define BIT_TXERR_INT                       BIT(11)
#define BIT_ATIMEND                         BIT(12)
#define BIT_ATIMEND_E                       BIT(13)
#define BIT_BCNDERR1                        BIT(14)
#define BIT_BCNDERR2                        BIT(15)
#define BIT_BCNDERR3                        BIT(16)
#define BIT_BCNDERR4                        BIT(17)
#define BIT_BCNDERR5                        BIT(18)
#define BIT_BCNDERR6                        BIT(19)
#define BIT_BCNDERR7                        BIT(20)
#define BIT_BCNDMAINT1                      BIT(21)
#define BIT_BCNDMAINT2                      BIT(22)
#define BIT_BCNDMAINT3                      BIT(23)
#define BIT_BCNDMAINT4                      BIT(24)
#define BIT_BCNDMAINT5                      BIT(25)
#define BIT_BCNDMAINT6                      BIT(26)
#define BIT_BCNDMAINT7                      BIT(27)
#define BIT_MCU_ERR                         BIT(28)
//BIT29~31  RESERVED
#endif  //#if CONFIG_WLANREG_SUPPORT & ~SUPPORT_CHIP_8723A



//-----------------------------------------------------
//
//  0x0300h ~ 0x03FFh   PCIe/LBus
//
//-----------------------------------------------------
#if CONFIG_WLANREG_SUPPORT & (SUPPORT_CHIP_8881A|SUPPORT_CHIP_8192E)

#if CONFIG_WLANREG_SUPPORT & SUPPORT_CHIP_8192E
//4 REG_PCIE_CTRL1(0x300), 4 Bytes
#define BIT_PCIEIO_PERSTB_SEL       BIT(31)

#define BIT_MASK_PCIE_MAX_RXDMA     0x7
#define BIT_SHIFT_PCIE_MAX_RXDMA    28
#define BIT_PCIE_MAX_RXDMA(x)       (((x) & BIT_MASK_PCIE_MAX_RXDMA)<<BIT_SHIFT_PCIE_MAX_RXDMA)

#define BIT_MULRW                   BIT(27)

#define BIT_MASK_PCIE_MAX_TXDMA     0x7
#define BIT_SHIFT_PCIE_MAX_TXDMA    24
#define BIT_PCIE_MAX_TXDMA(x)       (((x) & BIT_MASK_PCIE_MAX_TXDMA)<<BIT_SHIFT_PCIE_MAX_TXDMA)

#define BIT_EN_CPL_TIMEOUT_PS       BIT(22)
#define BIT_REG_TXDMA_FAIL_PS       BIT(21)
#define BIT_PCIE_RST_TRXDMA_INTF    BIT(20)
#define BIT_EN_HWENTR_L1            BIT(19)
#define BIT_EN_ADV_CLKGATE          BIT(18)
#define BIT_PCIE_EN_SWENT_L23       BIT(17)
#define BIT_PCIE_EN_HWEXT_L1        BIT(16)
#endif

#if CONFIG_WLANREG_SUPPORT & SUPPORT_CHIP_8881A
//4 REG_LX_CTRL1(0x300)
#define BIT_WT_LIT_EDN              BIT(25)
#define BIT_RD_LITT_EDN             BIT(24)

#define BIT_SHIFT_MAX_RXDMA         20
#define BIT_MASK_MAX_RXDMA          0x7
#define BIT_MAX_RXDMA(x)            (((x) & BIT_MASK_MAX_RXDMA)<<BIT_SHIFT_MAX_RXDMA)

#define BIT_SHIFT_MAX_TXDMA         16
#define BIT_MASK_MAX_TXDMA          0x7
#define BIT_MAX_TXDMA(x)            (((x) & BIT_MASK_MAX_TXDMA)<<BIT_SHIFT_MAX_TXDMA)
#endif

#if CONFIG_WLANREG_SUPPORT & (SUPPORT_CHIP_8881A|SUPPORT_CHIP_8192E)
#define BIT_STOP_BCNQ               BIT(14)
#define BIT_STOP_MGQ                BIT(13)
#define BIT_STOP_VOQ                BIT(12)
#define BIT_STOP_VIQ                BIT(11)
#define BIT_STOP_BEQ                BIT(10)
#define BIT_STOP_BKQ                BIT(9)
#define BIT_STOP_RXQ                BIT(8)
#define BIT_STOP_HI7Q               BIT(7)
#define BIT_STOP_HI6Q               BIT(6)
#define BIT_STOP_HI5Q               BIT(5)
#define BIT_STOP_HI4Q               BIT(4)
#define BIT_STOP_HI3Q               BIT(3)
#define BIT_STOP_HI2Q               BIT(2)
#define BIT_STOP_HI1Q               BIT(1)
#define BIT_STOP_HI0Q               BIT(0)
#endif


//4 REG_INT_MIG_CFG(0x0304), 4 Bytes
#define BIT_SHIFT_TXTTIMER_MATCH_NUM                28
#define BIT_MASK_TXTTIMER_MATCH_NUM                 0xF
#define BIT_MAX_TXTTIMER_MATCH_NUM(x)               (((x) & BIT_MASK_TXTTIMER_MATCH_NUM)<<BIT_SHIFT_TXTTIMER_MATCH_NUM)

#define BIT_SHIFT_TXPKT_NUM_MATCH                   24
#define BIT_MASK_TXPKT_NUM_MATCH                    0xF
#define BIT_MAX_TXPKT_NUM_MATCH(x)                  (((x) & BIT_MASK_TXPKT_NUM_MATCH)<<BIT_SHIFT_TXPKT_NUM_MATCH)

#define BIT_SHIFT_RXTTIMER_MATCH_NUM                20
#define BIT_MASK_RXTTIMER_MATCH_NUM                 0xF
#define BIT_MAX_RXTTIMER_MATCH_NUM(x)               (((x) & BIT_MASK_RXTTIMER_MATCH_NUM)<<BIT_SHIFT_RXTTIMER_MATCH_NUM)

#define BIT_SHIFT_RXPKT_NUM_MATCH                   16
#define BIT_MASK_RXPKT_NUM_MATCH                    0xF
#define BIT_MAX_RXPKT_NUM_MATCH(x)                  (((x) & BIT_MASK_RXPKT_NUM_MATCH)<<BIT_SHIFT_RXPKT_NUM_MATCH)

#define BIT_SHIFT_MIGRATE_TIMER                     0
#define BIT_MASK_MIGRATE_TIMER                      0xFFFF
#define BIT_MAX_MIGRATE_TIMER(x)                    (((x) & BIT_MASK_MIGRATE_TIMER)<<BIT_SHIFT_MIGRATE_TIMER)

//4 #define REG_BCNQ_TXBD_DESA          0x0308  // 8 Bytes
//4 #define REG_MGQ_TXBD_DESA           0x0310  // 8 Bytes 
//4 #define REG_VOQ_TXBD_DESA           0x0318  // 8 Bytes
//4 #define REG_VIQ_TXBD_DESA           0x0320  // 8 Bytes
//4 #define REG_BEQ_TXBD_DESA           0x0328  // 8 Bytes
//4 #define REG_BKQ_TXBD_DESA           0x0330  // 8 Bytes
//4 #define REG_RXQ_RXBD_DESA           0x0338  // 8 Bytes
//4 #define REG_HI0Q_TXBD_DESA          0x0340  // 8 Bytes
//4 #define REG_HI1Q_TXBD_DESA          0x0348  // 8 Bytes
//4 #define REG_HI2Q_TXBD_DESA          0x0350  // 8 Bytes
//4 #define REG_HI3Q_TXBD_DESA          0x0358  // 8 Bytes
//4 #define REG_HI4Q_TXBD_DESA          0x0360  // 8 Bytes
//4 #define REG_HI5Q_TXBD_DESA          0x0368  // 8 Bytes
//4 #define REG_HI6Q_TXBD_DESA          0x0370  // 8 Bytes
//4 #define REG_HI7Q_TXBD_DESA          0x0378  // 8 Bytes


//4 #define REG_MGQ_TXBD_NUM            0x0380  // 2 Bytes
#define BIT_SHIFT_MGQ_DESC_MODE                      12
#define BIT_MASK_MGQ_DESC_MODE                       0x3
#define BIT_MAX_MGQ_DESC_MODE(x)                     (((x) & BIT_MASK_MGQ_DESC_MODE)<<BIT_SHIFT_MGQ_DESC_MODE)

#define BIT_SHIFT_MGQ_DESC_NUM                      0
#define BIT_MASK_MGQ_DESC_NUM                       0xFFF
#define BIT_MAX_MGQ_DESC_NUM(x)                     (((x) & BIT_MASK_MGQ_DESC_NUM)<<BIT_SHIFT_MGQ_DESC_NUM)


//4 #define REG_RX_RXBD_NUM             0x0382  // 2 Bytes
#define BIT_SHIFT_SYS_32_64                         15
#define BIT_SYS_32_64                               BIT(BIT_SHIFT_SYS_32_64)

#define BIT_SHIFT_BCNQ_DESC_MODE                    13
#define BIT_MASK_BCNQ_DESC_MODE                     0x3
#define BIT_MAX_BCNQ_DESC_MODE(x)                   (((x) & BIT_MASK_BCNQ_DESC_MODE)<<BIT_SHIFT_BCNQ_DESC_MODE)

#define BIT_BCNQ_FLAG                               BIT(12)

#define BIT_SHIFT_RXQ_DESC_NUM                      0
#define BIT_MASK_RXQ_DESC_NUM                       0xFFF
#define BIT_MAX_RXQ_DESC_NUM(x)                     (((x) & BIT_MASK_RXQ_DESC_NUM)<<BIT_SHIFT_RXQ_DESC_NUM)


//4 #define REG_VOQ_TXBD_NUM            0x0384  // 2 Bytes
#define BIT_VOQ_FLAG                                BIT(14)

#define BIT_SHIFT_VOQ_DESC_MODE                    12
#define BIT_MASK_VOQ_DESC_MODE                     0x3
#define BIT_MAX_VOQ_DESC_MODE(x)                   (((x) & BIT_MASK_VOQ_DESC_MODE)<<BIT_SHIFT_VOQ_DESC_MODE)

#define BIT_SHIFT_VOQ_DESC_NUM                      0
#define BIT_MASK_VOQ_DESC_NUM                       0xFFF
#define BIT_MAX_VOQ_DESC_NUM(x)                     (((x) & BIT_MASK_VOQ_DESC_NUM)<<BIT_SHIFT_VOQ_DESC_NUM)


//4 #define REG_VIQ_TXBD_NUM            0x0386  // 2 Bytes
#define BIT_VIQ_FLAG                                BIT(14)

#define BIT_SHIFT_VIQ_DESC_MODE                    12
#define BIT_MASK_VIQ_DESC_MODE                     0x3
#define BIT_MAX_VIQ_DESC_MODE(x)                   (((x) & BIT_MASK_VIQ_DESC_MODE)<<BIT_SHIFT_VIQ_DESC_MODE)

#define BIT_SHIFT_VIQ_DESC_NUM                      0
#define BIT_MASK_VIQ_DESC_NUM                       0xFFF
#define BIT_MAX_VIQ_DESC_NUM(x)                     (((x) & BIT_MASK_VIQ_DESC_NUM)<<BIT_SHIFT_VIQ_DESC_NUM)


//4 #define REG_BEQ_TXBD_NUM            0x0388  // 2 Bytes
#define BIT_BEQ_FLAG                                BIT(14)

#define BIT_SHIFT_BEQ_DESC_MODE                    12
#define BIT_MASK_BEQ_DESC_MODE                     0x3
#define BIT_MAX_BEQ_DESC_MODE(x)                   (((x) & BIT_MASK_BEQ_DESC_MODE)<<BIT_SHIFT_BEQ_DESC_MODE)

#define BIT_SHIFT_BEQ_DESC_NUM                      0
#define BIT_MASK_BEQ_DESC_NUM                       0xFFF
#define BIT_MAX_BEQ_DESC_NUM(x)                     (((x) & BIT_MASK_BEQ_DESC_NUM)<<BIT_SHIFT_BEQ_DESC_NUM)



//4 #define REG_BKQ_TXBD_NUM            0x038A  // 2 Bytes
#define BIT_BKQ_FLAG                                BIT(14)

#define BIT_SHIFT_BKQ_DESC_MODE                    12
#define BIT_MASK_BKQ_DESC_MODE                     0x3
#define BIT_MAX_BKQ_DESC_MODE(x)                   (((x) & BIT_MASK_BKQ_DESC_MODE)<<BIT_SHIFT_BKQ_DESC_MODE)

#define BIT_SHIFT_BKQ_DESC_NUM                      0
#define BIT_MASK_BKQ_DESC_NUM                       0xFFF
#define BIT_MAX_BKQ_DESC_NUM(x)                     (((x) & BIT_MASK_BKQ_DESC_NUM)<<BIT_SHIFT_BKQ_DESC_NUM)


//4 #define REG_HI0Q_TXBD_NUM            0x038C  // 2 Bytes
#define BIT_HI0Q_FLAG                                BIT(14)

#define BIT_SHIFT_HI0Q_DESC_MODE                    12
#define BIT_MASK_HI0Q_DESC_MODE                     0x3
#define BIT_MAX_HI0Q_DESC_MODE(x)                   (((x) & BIT_MASK_HI0Q_DESC_MODE)<<BIT_SHIFT_HI0Q_DESC_MODE)

#define BIT_SHIFT_HI0Q_DESC_NUM                      0
#define BIT_MASK_HI0Q_DESC_NUM                       0xFFF
#define BIT_MAX_HI0Q_DESC_NUM(x)                     (((x) & BIT_MASK_HI0Q_DESC_NUM)<<BIT_SHIFT_HI0Q_DESC_NUM)


//4 #define REG_HI1Q_TXBD_NUM            0x038E  // 2 Bytes
#define BIT_HI1Q_FLAG                                BIT(14)

#define BIT_SHIFT_HI1Q_DESC_MODE                    12
#define BIT_MASK_HI1Q_DESC_MODE                     0x3
#define BIT_MAX_HI1Q_DESC_MODE(x)                   (((x) & BIT_MASK_HI1Q_DESC_MODE)<<BIT_SHIFT_HI1Q_DESC_MODE)

#define BIT_SHIFT_HI1Q_DESC_NUM                      0
#define BIT_MASK_HI1Q_DESC_NUM                       0xFFF
#define BIT_MAX_HI1Q_DESC_NUM(x)                     (((x) & BIT_MASK_HI1Q_DESC_NUM)<<BIT_SHIFT_HI1Q_DESC_NUM)


//4 #define REG_HI2Q_TXBD_NUM            0x0390  // 2 Bytes
#define BIT_HI2Q_FLAG                                BIT(14)

#define BIT_SHIFT_HI2Q_DESC_MODE                    12
#define BIT_MASK_HI2Q_DESC_MODE                     0x3
#define BIT_MAX_HI2Q_DESC_MODE(x)                   (((x) & BIT_MASK_HI2Q_DESC_MODE)<<BIT_SHIFT_HI2Q_DESC_MODE)


#define BIT_SHIFT_HI2Q_DESC_NUM                      0
#define BIT_MASK_HI2Q_DESC_NUM                       0xFFF
#define BIT_MAX_HI2Q_DESC_NUM(x)                     (((x) & BIT_MASK_HI2Q_DESC_NUM)<<BIT_SHIFT_HI2Q_DESC_NUM)


//4 #define REG_HI3Q_TXBD_NUM            0x0392  // 2 Bytes
#define BIT_HI3Q_FLAG                                BIT(14)

#define BIT_SHIFT_HI3Q_DESC_MODE                    12
#define BIT_MASK_HI3Q_DESC_MODE                     0x3
#define BIT_MAX_HI3Q_DESC_MODE(x)                   (((x) & BIT_MASK_HI3Q_DESC_MODE)<<BIT_SHIFT_HI3Q_DESC_MODE)

#define BIT_SHIFT_HI3Q_DESC_NUM                      0
#define BIT_MASK_HI3Q_DESC_NUM                       0xFFF
#define BIT_MAX_HI3Q_DESC_NUM(x)                     (((x) & BIT_MASK_HI3Q_DESC_NUM)<<BIT_SHIFT_HI3Q_DESC_NUM)


//4 #define REG_HI4Q_TXBD_NUM            0x0394  // 2 Bytes
#define BIT_HI4Q_FLAG                                BIT(14)

#define BIT_SHIFT_HI4Q_DESC_MODE                    12
#define BIT_MASK_HI4Q_DESC_MODE                     0x3
#define BIT_MAX_HI4Q_DESC_MODE(x)                   (((x) & BIT_MASK_HI4Q_DESC_MODE)<<BIT_SHIFT_HI4Q_DESC_MODE)

#define BIT_SHIFT_HI4Q_DESC_NUM                      0
#define BIT_MASK_HI4Q_DESC_NUM                       0xFFF
#define BIT_MAX_HI4Q_DESC_NUM(x)                     (((x) & BIT_MASK_HI4Q_DESC_NUM)<<BIT_SHIFT_HI4Q_DESC_NUM)


//4 #define REG_HI5Q_TXBD_NUM            0x0396  // 2 Bytes
#define BIT_HI5Q_FLAG                                BIT(14)

#define BIT_SHIFT_HI5Q_DESC_MODE                    12
#define BIT_MASK_HI5Q_DESC_MODE                     0x3
#define BIT_MAX_HI5Q_DESC_MODE(x)                   (((x) & BIT_MASK_HI5Q_DESC_MODE)<<BIT_SHIFT_HI5Q_DESC_MODE)

#define BIT_SHIFT_HI5Q_DESC_NUM                      0
#define BIT_MASK_HI5Q_DESC_NUM                       0xFFF
#define BIT_MAX_HI5Q_DESC_NUM(x)                     (((x) & BIT_MASK_HI5Q_DESC_NUM)<<BIT_SHIFT_HI5Q_DESC_NUM)


//4 #define REG_HI6Q_TXBD_NUM            0x0398  // 2 Bytes
#define BIT_HI6Q_FLAG                                BIT(14)

#define BIT_SHIFT_HI6Q_DESC_MODE                    12
#define BIT_MASK_HI6Q_DESC_MODE                     0x3
#define BIT_MAX_HI6Q_DESC_MODE(x)                   (((x) & BIT_MASK_HI6Q_DESC_MODE)<<BIT_SHIFT_HI6Q_DESC_MODE)

#define BIT_SHIFT_HI6Q_DESC_NUM                      0
#define BIT_MASK_HI6Q_DESC_NUM                       0xFFF
#define BIT_MAX_HI6Q_DESC_NUM(x)                     (((x) & BIT_MASK_HI6Q_DESC_NUM)<<BIT_SHIFT_HI6Q_DESC_NUM)


//4 #define REG_HI7Q_TXBD_NUM            0x039A  // 2 Bytes
#define BIT_HI7Q_FLAG                                BIT(14)

#define BIT_SHIFT_HI7Q_DESC_MODE                    12
#define BIT_MASK_HI7Q_DESC_MODE                     0x3
#define BIT_MAX_HI7Q_DESC_MODE(x)                   (((x) & BIT_MASK_HI7Q_DESC_MODE)<<BIT_SHIFT_HI7Q_DESC_MODE)

#define BIT_SHIFT_HI7Q_DESC_NUM                      0
#define BIT_MASK_HI7Q_DESC_NUM                       0xFFF
#define BIT_MAX_HI7Q_DESC_NUM(x)                     (((x) & BIT_MASK_HI7Q_DESC_NUM)<<BIT_SHIFT_HI7Q_DESC_NUM)


//4 #define REG_TSFTIMER_HCI            0x039C  // 4 Bytes
#define BIT_SHIFT_TSFT2_HCI                           16
#define BIT_MASK_TSFT2_HCI                            0xFFFF
#define BIT_MAX_TSFT2_HCI(x)                         (((x) & BIT_MASK_TSFT2_HCI)<<BIT_SHIFT_TSFT2_HCI)

#define BIT_SHIFT_TSFT1_HCI                           0
#define BIT_MASK_TSFT1_HCI                            0xFFFF
#define BIT_MAX_TSFT1_HCI(x)                         (((x) & BIT_MASK_TSFT1_HCI)<<BIT_SHIFT_TSFT1_HCI)


//4 #define REG_BD_RWPTR_CLR            0x039C  // 4 Bytes
#define BIT_CLR_HI7Q_HW_IDX                             BIT(29)
#define BIT_CLR_HI6Q_HW_IDX                             BIT(28)
#define BIT_CLR_HI5Q_HW_IDX                             BIT(27)
#define BIT_CLR_HI4Q_HW_IDX                             BIT(26)
#define BIT_CLR_HI3Q_HW_IDX                             BIT(25)
#define BIT_CLR_HI2Q_HW_IDX                             BIT(24)
#define BIT_CLR_HI1Q_HW_IDX                             BIT(23)
#define BIT_CLR_HI0Q_HW_IDX                             BIT(22)
#define BIT_CLR_BKQ_HW_IDX                              BIT(21)
#define BIT_CLR_BEQ_HW_IDX                              BIT(20)
#define BIT_CLR_VIQ_HW_IDX                              BIT(19)
#define BIT_CLR_VOQ_HW_IDX                              BIT(18)
#define BIT_CLR_MGTQ_HW_IDX                             BIT(17)
#define BIT_CLR_RXQ_HW_IDX                              BIT(16)

#if CONFIG_WLANREG_SUPPORT & SUPPORT_CHIP_8881A
#define BIT_SRST_TX                                     BIT(15)
#define BIT_SRST_RX                                     BIT(14)
#endif  //CONFIG_WLANREG_SUPPORT & SUPPORT_CHIP_8881A

#define BIT_CLR_HI7Q_HOST_IDX                           BIT(13)
#define BIT_CLR_HI6Q_HOST_IDX                           BIT(12)
#define BIT_CLR_HI5Q_HOST_IDX                           BIT(11)
#define BIT_CLR_HI4Q_HOST_IDX                           BIT(10)
#define BIT_CLR_HI3Q_HOST_IDX                           BIT(9)
#define BIT_CLR_HI2Q_HOST_IDX                           BIT(8)
#define BIT_CLR_HI1Q_HOST_IDX                           BIT(7)
#define BIT_CLR_HI0Q_HOST_IDX                           BIT(6)
#define BIT_CLR_BKQ_HOST_IDX                            BIT(5)
#define BIT_CLR_BEQ_HOST_IDX                            BIT(4)
#define BIT_CLR_VIQ_HOST_IDX                            BIT(3)
#define BIT_CLR_VOQ_HOST_IDX                            BIT(2)
#define BIT_CLR_MGTQ_HOST_IDX                           BIT(1)
#define BIT_CLR_RXQ_HOST_IDX                            BIT(0)


//4 #define REG_VOQ_TXBD_IDX            0x03A0  // 4 Bytes
//4 #define REG_VIQ_TXBD_IDX            0x03A4  // 4 Bytes
//4 #define REG_BEQ_TXBD_IDX            0x03A8  // 4 Bytes
//4 #define REG_BKQ_TXBD_IDX            0x03AC  // 4 Bytes
//4 #define REG_MGQ_TXBD_IDX            0x03B0  // 4 Bytes
//4 #define REG_RXQ_RXBD_IDX            0x03B4  // 4 Bytes
//4 #define REG_HI0Q_TXBD_IDX           0x03B8  // 4 Bytes
//4 #define REG_HI1Q_TXBD_IDX           0x03BC  // 4 Bytes
//4 #define REG_HI2Q_TXBD_IDX           0x03C0  // 4 Bytes
//4 #define REG_HI3Q_TXBD_IDX           0x03C4  // 4 Bytes
//4 #define REG_HI4Q_TXBD_IDX           0x03C8  // 4 Bytes
//4 #define REG_HI5Q_TXBD_IDX           0x03CC  // 4 Bytes
//4 #define REG_HI6Q_TXBD_IDX           0x03D0  // 4 Bytes
//4 #define REG_HI7Q_TXBD_IDX           0x03D4  // 4 Bytes

//TXBD_IDX Common
#define BIT_SHIFT_QUEUE_HOST_IDX    0
#define BIT_SHIFT_QUEUE_HW_IDX      16
#define BIT_MASK_QUEUE_IDX          0x0FFF

#if CONFIG_WLANREG_SUPPORT & SUPPORT_CHIP_8192E
//4 #define REG_DBG_SEL_V1              0x03D8  // 1 Bytes
#endif

//4 #define REG_PCIE_HRPWM1_V1          0x03D9  // 1 Bytes
//4 #define REG_PCIE_HCPWM1_V1          0x03DA  // 1 Bytes

#if CONFIG_WLANREG_SUPPORT & SUPPORT_CHIP_8192E
//4 #define REG_PCIE_CTRL2              0x03DB  // 1 Bytes
#define BIT_DIS_TXDMA_PRE                           BIT(7)
#define BIT_DIS_RXDMA_PRE                           BIT(6)

#define BIT_SHIFT_HPS_CLKR_PCIE                     4
#define BIT_MASK_HPS_CLKR_PCIE                      0x3
#define BIT_HPS_CLKR_PCIE(x)                        (((x) & BIT_MASK_HPS_CLKR_PCIE)<<BIT_SHIFT_HPS_CLKR_PCIE)

#define BIT_PCIE_INT                                BIT(3)
#define BIT_TXFLAG_EXIT_L1_EN                       BIT(2)
#define BIT_EN_RXDMA_ALIGN                          BIT(1)
#define BIT_EN_TXDMA_ALIGN                          BIT(0)
#endif

#if CONFIG_WLANREG_SUPPORT & SUPPORT_CHIP_8881A
//4 #define REG_LX_CTRL2                0x03DB  // 1 Bytes
#define BIT_SHIFT_HPS_CLKR                          4
#define BIT_MASK_HPS_CLKR                           0x3
#define BIT_HPS_CLKR(x)                             (((x) & BIT_MASK_HPS_CLKR)<<BIT_SHIFT_HPS_CLKR)
#define BIT_LX_INT                                  BIT(3)
#endif

//4 #define REG_PCIE_HRPWM2_V1          0x03DC  // 2 Bytes
//4 #define REG_PCIE_HCPWM2_V1          0x03DE  // 2 Bytes
//4 #define REG_PCIE_H2C_MSG_V1         0x03E0  // 4 Bytes
//4 #define REG_PCIE_C2H_MSG_V1         0x03E4  // 4 Bytes

#if CONFIG_WLANREG_SUPPORT & SUPPORT_CHIP_8192E
//4 #define REG_DBI_WDATA_V1            0x03E8  // 4 Bytes
//4 #define REG_DBI_RDATA_V1            0x03EC  // 4 Bytes
//4 #define REG_DBI_FLAG_V1             0x03F0  // 4 Bytes
#define BIT_DBI_RFLAG                               BIT(17)
#define BIT_DBI_WFLAG                               BIT(16)

#define BIT_SHIFT_DBI_WREN                          12
#define BIT_MASK_DBI_WREN                           0xF
#define BIT_DBI_WREN(x)                             (((x) & BIT_MASK_DBI_WREN)<<BIT_SHIFT_DBI_WREN)

#define BIT_SHIFT_DBI_ADDR                          0
#define BIT_MASK_DBI_ADDR                           0xFFF
#define BIT_DBI_ADDR(x)                             (((x) & BIT_MASK_DBI_ADDR)<<BIT_SHIFT_DBI_ADDR)
#endif 

#if CONFIG_WLANREG_SUPPORT & SUPPORT_CHIP_8881A
//4 #define REG_LX_DMA_ISR              0x03E8  // 4 Bytes
#define BIT_BCN7DOK         BIT(23)
#define BIT_BCN6DOK         BIT(22)
#define BIT_BCN5DOK         BIT(21)
#define BIT_BCN4DOK         BIT(20)
#define BIT_BCN3DOK         BIT(19)
#define BIT_BCN2DOK         BIT(18)
#define BIT_BCN1DOK         BIT(17)
#define BIT_BCN0DOK         BIT(16)

#define BIT_M7DOK           BIT(15)
#define BIT_M6DOK           BIT(14)
#define BIT_M5DOK           BIT(13)
#define BIT_M4DOK           BIT(12)
#define BIT_M3DOK           BIT(11)
#define BIT_M2DOK           BIT(10)
#define BIT_M1DOK           BIT(9)
#define BIT_M0DOK           BIT(8)

#define BIT_MGTQDOK         BIT(6)
#define BIT_BKQDOK          BIT(5)
#define BIT_BEQDOK          BIT(4)
#define BIT_VIQDOK          BIT(3)
#define BIT_VOQDOK          BIT(2)
#define BIT_RDU             BIT(1)
#define BIT_RXDOK           BIT(0)

//4 #define REG_LX_DMA_IMR              0x03EC  // 4 Bytes
#define BIT_BCN7DOKM        BIT(23)
#define BIT_BCN6DOKM        BIT(22)
#define BIT_BCN5DOKM        BIT(21)
#define BIT_BCN4DOKM        BIT(20)
#define BIT_BCN3DOKM        BIT(19)
#define BIT_BCN2DOKM        BIT(18)
#define BIT_BCN1DOKM        BIT(17)
#define BIT_BCN0DOKM        BIT(16)

#define BIT_M7DOKM          BIT(15)
#define BIT_M6DOKM          BIT(14)
#define BIT_M5DOKM          BIT(13)
#define BIT_M4DOKM          BIT(12)
#define BIT_M3DOKM          BIT(11)
#define BIT_M2DOKM          BIT(10)
#define BIT_M1DOKM          BIT(9)
#define BIT_M0DOKM          BIT(8)

#define BIT_MGTQDOKM        BIT(6)
#define BIT_BKQDOKM         BIT(5)
#define BIT_BEQDOKM         BIT(4)
#define BIT_VIQDOKM         BIT(3)
#define BIT_VOQDOKM         BIT(2)
#define BIT_RDUM            BIT(1)
#define BIT_RXDOKM          BIT(0)

//4 #define REG_LX_DMA_DBG              0x03F0  // 4 Bytes
#define BIT_RX_OVER_RD_ERR              BIT(20)
#define BIT_RXDMA_STUCK                 BIT(19)

#define BIT_SHIFT_RX_STATE              16
#define BIT_MASK_RX_STATE               0x7
#define BIT_RX_STATE(x)                 (((x) & BIT_MASK_RX_STATE)<<BIT_SHIFT_RX_STATE)

#define BIT_TDE_NO_IDLE                 BIT(15)
#define BIT_TXDMA_STUCK                 BIT(14)
#define BIT_TDE_FULL_ERR                BIT(13)
#define BIT_HD_SIZE_ERR                 BIT(12)

#define BIT_SHIFT_TX_STATE              8
#define BIT_MASK_TX_STATE               0xF
#define BIT_TX_STATE(x)                 (((x) & BIT_MASK_TX_STATE)<<BIT_SHIFT_TX_STATE)

#define BIT_MST_BUSY                    BIT(3)
#define BIT_SLV_BUSY                    BIT(2)
#define BIT_RXDES_UNAVAIL               BIT(1)
#define BIT_EN_DBG_STUCK                BIT(0)
#endif

#if CONFIG_WLANREG_SUPPORT & SUPPORT_CHIP_8192E
//4 #define REG_MDIO_V1                 0x03F4  // 4 Bytes
#endif


//4 #define REG_PCIE_MIX_CFG            0x03F8  // 4 Bytes
//4 #define REG_BUS_MIX_CFG             0x03F8  // 4 Bytes
#define BIT_SHIFT_WATCH_DOG_RECORD              10
#define BIT_MASK_WATCH_DOG_RECORD               0x3FFF
#define BIT_WATCH_DOG_RECORD(x)                 (((x) & BIT_MASK_WATCH_DOG_RECORD)<<BIT_SHIFT_WATCH_DOG_RECORD)

#define BIT_R_IO_TIMEOUT_FLAG                   BIT(9)
#define BIT_EN_WATCH_DOG                        BIT(8)

#if CONFIG_WLANREG_SUPPORT & SUPPORT_CHIP_8192E
//4 //4 #define REG_PCIE_MIX_CFG            0x03F8  // 4 Bytes
#define BIT_ECRC_EN                             BIT(7)
#define BIT_MDIO_RFLAG                          BIT(6)
#define BIT_MDIO_WFLAG                          BIT(5)

#define BIT_SHIFT_MDIO_ADDRESS                  0
#define BIT_MASK_MDIO_ADDRESS                   0x1F
#define BIT_MDIO_ADDRESS(x)                     (((x) & BIT_MASK_MDIO_ADDRESS)<<BIT_SHIFT_MDIO_ADDRESS)
#endif

#if CONFIG_WLANREG_SUPPORT & SUPPORT_CHIP_8881A
//4 #define REG_BUS_MIX_CFG             0x03F8  // 4 Bytes
#endif

#endif // endif CONFIG_WLANREG_SUPPORT & (SUPPORT_CHIP_8881A|SUPPORT_CHIP_8192E)



#endif//__RTL_WLAN_BITDEF_H__
