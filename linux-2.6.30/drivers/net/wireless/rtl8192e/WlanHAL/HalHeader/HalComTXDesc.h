#ifndef __HAL_COM_TXDESC_H__
#define __HAL_COM_TXDESC_H__
/*-------------------------Modification Log-----------------------------------    
-------------------------Modification Log-----------------------------------*/

/*--------------------------Include File--------------------------------------*/
#include "HalHWCfg.h"
/*--------------------------Include File--------------------------------------*/

#if CONFIG_WLANREG_SUPPORT & (SUPPORT_CHIP_8881A|SUPPORT_CHIP_8192E)
//3  TX Buffer Descriptor
//TXBD Desc bit Mask & Shift
//Dword 0 MSK
#define TXBD_DW0_TXBUFSIZE_MSK      0xFFFF

//---Normal Packet
#define TXBD_DW0_PSLEN_MSK          0xFFFF

//---Beacon Packet
#define TXBD_DW0_BCN_PSLEN_MSK      0x7FFF
#define TXBD_DW0_BCN_OWN_MSK        0x1

#define TXBD_DW0_EXTENDTXBUF_MSK    0x1

//Dword 0 SHIFT
#define TXBD_DW0_TXBUFSIZE_SH       0

//---Normal Packet
#define TXBD_DW0_PSLEN_SH           16

//---Beacon Packet
#define TXBD_DW0_BCN_PSLEN_SH       16
#define TXBD_DW0_BCN_OWN_SH         31

#define	TXBD_DW0_EXTENDTXBUF_SH     31


//Dword 1 MSK
#define TXBD_DW1_PHYADDR_LOW_MSK    0xFFFFFFFF

//Dword 1 SHIFT
#define TXBD_DW1_PHYADDR_LOW_SH     0


//Dword 2 MSK
#define TXBD_DW2_PHYADDR_HIGH_MSK   0xFFFFFFFF

//Dword 2 SHIFT
#define TXBD_DW2_PHYADDR_HIGH_SH    0


//Dword 3 MSK
#define TXBD_DW3_PHYADDR_RSVD_MSK   0xFFFFFFFF

//Dword 3 SHIFT
#define TXBD_DW3_PHYADDR_RSVD_SH    0


//3  TX WiFI Info
//TX Desc bit Mask & Shift
//Dword 0
#define TX_DW0_TXPKSIZE_MSK             0xFFFF
#define TX_DW0_OFFSET_MSK               0xFF
#define TX_DW0_BMC_MSK                  0x1
#define TX_DW0_HTC_MSK                  0x1
#define TX_DW0_RSVD_26_MSK              0x1
#define TX_DW0_RSVD_27_MSK              0x1
#define TX_DW0_LINIP_MSK                0x1
#define TX_DW0_NOACM_MSK                0x1
#define TX_DW0_GF_MSK                   0x1
#define TX_DW0_RSVD_31_MSK              0x1

//Dword 0
#define TX_DW0_TXPKSIZE_SH         0
#define TX_DW0_OFFSET_SH           16
#define TX_DW0_BMC_SH              24
#define TX_DW0_HTC_SH              25
#define TX_DW0_RSVD_26_SH          26
#define TX_DW0_RSVD_27_SH          27
#define TX_DW0_LINIP_SH            28
#define TX_DW0_NOACM_SH            29
#define TX_DW0_GF_SH               30
#define TX_DW0_RSVD_31_SH          31


//Dword 1
#define TX_DW1_MACID_MSK                0x7F
#define TX_DW1_RSVD_7_MSK               0x1
#define TX_DW1_QSEL_MSK                 0x1F
#define TX_DW1_PDG_NAV_EXT_MSK          0x1
#define TX_DW1_LSIG_TXOP_EN_MSK         0x1
#define TX_DW1_PIFS_MSK                 0x1
#define TX_DW1_RATE_ID_MSK              0x1F
#define TX_DW1_EN_DESC_ID_MSK           0x1
#define TX_DW1_SECTYPE_MSK              0x3
#define TX_DW1_PKT_OFFSET_MSK           0x1F
#define TX_DW1_MOREDATA_MSK             0x1
#define TX_DW1_TXOP_PS_CAP_MSK          0x1
#define TX_DW1_TXOP_PS_MODE_MSK         0x1

//Dword 1
#define TX_DW1_MACID_SH            0
#define TX_DW1_RSVD_7_SH           7
#define TX_DW1_QSEL_SH             8
#define TX_DW1_PDG_NAV_EXT_SH      13
#define TX_DW1_LSIG_TXOP_EN_SH     14
#define TX_DW1_PIFS_SH             15
#define TX_DW1_RATE_ID_SH          16
#define TX_DW1_EN_DESC_ID_SH       21
#define TX_DW1_SECTYPE_SH          22
#define TX_DW1_PKT_OFFSET_SH       24
#define TX_DW1_MOREDATA_SH         29
#define TX_DW1_TXOP_PS_CAP_SH      30
#define TX_DW1_TXOP_PS_MODE_SH     31


//Dword 2
#define TX_DW2_P_AID_MSK                0x1FF
#define TX_DW2_RSVD_9_MSK               0x1
#define TX_DW2_CCA_RTS_MSK              0x3
#define TX_DW2_AGG_EN_MSK               0x1
#define TX_DW2_RDG_EN_MSK               0x1
#define TX_DW2_NULL_0_MSK               0x1
#define TX_DW2_NULL_1_MSK               0x1
#define TX_DW2_BK_MSK                   0x1
#define TX_DW2_MOREFRAG_MSK             0x1
#define TX_DW2_RAW_MSK                  0x1
#define TX_DW2_SPE_RPT_MSK              0x1
#define TX_DW2_AMPDU_DENSITY_MSK        0x7
#define TX_DW2_BT_NULL_MSK              0x1
#define TX_DW2_G_ID_MSK                 0x3F
#define TX_DW2_RSVD30_31_MSK            0x3

//Dword 2
#define TX_DW2_P_AID_SH            0
#define TX_DW2_RSVD_9_SH           9
#define TX_DW2_CCA_RTS_SH          10
#define TX_DW2_AGG_EN_SH           12
#define TX_DW2_RDG_EN_SH           13
#define TX_DW2_NULL_0_SH           14
#define TX_DW2_NULL_1_SH           15
#define TX_DW2_BK_SH               16
#define TX_DW2_MOREFRAG_SH         17
#define TX_DW2_RAW_SH              18
#define TX_DW2_SPE_RPT_SH          19
#define TX_DW2_AMPDU_DENSITY_SH    20
#define TX_DW2_BT_NULL_SH          23
#define TX_DW2_G_ID_SH             24
#define TX_DW2_RSVD30_31_SH        30


//Dword 3
#define TX_DW3_WHEADER_MSK              0xF
#define TX_DW3_CHK_EN_MSK               0x1
#define TX_DW3_EARLY_RATE_MSK           0x1
#define TX_DW3_HW_SSN_SEL_MSK           0x3

#define TX_DW3_USERATE_MSK              0x1
#define TX_DW3_DISRTSFB_MSK             0x1
#define TX_DW3_DISDATAFB_MSK            0x1
#define TX_DW3_CTS2SELF_MSK             0x1
#define TX_DW3_RTSEN_MSK                0x1
#define TX_DW3_HW_RTS_EN_MSK            0x1
#define TX_DW3_PORT_ID_MSK              0x1
#define TX_DW3_NAVUSEHDR_MSK            0x1

#define TX_DW3_USE_MAX_LEN_MSK          0x1
#define TX_DW3_MAX_AGG_NUM_MSK          0x1F
#define TX_DW3_NDPA_MSK                 0x3

#define TX_DW3_AMPDU_MAX_TIME_MSK       0xFF

//Dword 3
#define TX_DW3_WHEADER_SH           0
#define TX_DW3_CHK_EN_SH            4
#define TX_DW3_EARLY_RATE_SH        5
#define TX_DW3_HW_SSN_SEL_SH        6
#define TX_DW3_USERATE_SH           8
#define TX_DW3_DISRTSFB_SH          9
#define TX_DW3_DISDATAFB_SH         10
#define TX_DW3_CTS2SELF_SH          11
#define TX_DW3_RTSEN_SH             12
#define TX_DW3_HW_RTS_EN_SH         13
#define TX_DW3_PORT_ID_SH           14
#define TX_DW3_NAVUSEHDR_SH         15
#define TX_DW3_USE_MAX_LEN_SH       16
#define TX_DW3_MAX_AGG_NUM_SH       17
#define TX_DW3_NDPA_SH              22
#define TX_DW3_AMPDU_MAX_TIME_SH    24


//Dword 4
#define TX_DW4_DATARATE_MSK             0x7F
#define TX_DW4_TYPE_RATE_MSK            0x1

#define TX_DW4_DATA_RATEFB_LMT_MSK      0x1F
#define TX_DW4_RTS_RATEFB_LMT_MSK       0xF
#define TX_DW4_RTY_LMT_EN_MSK           0x1
#define TX_DW4_DATA_RT_LMT_MSK          0x3F

#define TX_DW4_RTSRATE_MSK              0x1F
#define TX_DW4_PCTS_EN_MSK              0x1
#define TX_DW4_PCTS_MASK_EN_MSK         0x3

//Dword 4
#define TX_DW4_DATARATE_SH          0
#define TX_DW4_TYPE_RATE_SH         7
#define TX_DW4_DATA_RATEFB_LMT_SH   8
#define TX_DW4_RTS_RATEFB_LMT_SH    13
#define TX_DW4_RTY_LMT_EN_SH        17
#define TX_DW4_DATA_RT_LMT_SH       18
#define TX_DW4_RTSRATE_SH           24
#define TX_DW4_PCTS_EN_SH           29
#define TX_DW4_PCTS_MASK_EN_SH      30


//Dword 5
#define TX_DW5_DATA_SC_MSK              0xF
#define TX_DW5_DATA_SHORT_MSK           0x1
#define TX_DW5_DATA_BW_MSK              0x3
#define TX_DW5_DATA_LDPC_MSK            0x1

#define TX_DW5_DATA_STBC_MSK            0x3
#define TX_DW5_VCS_STBC_MSK             0x3
#define TX_DW5_RTS_SHORT_MSK            0x1
#define TX_DW5_RTS_SC_MSK               0xF
#define TX_DW5_RSVD17_23_MSK            0x7F

#define TX_DW5_TX_ANT_MSK               0xF
#define TX_DW5_TXPWR_OFSET_MSK          0x7
#define TX_DW5_RSVD31_MSK               0x1

//Dword 5
#define TX_DW5_DATA_SC_SH           0
#define TX_DW5_DATA_SHORT_SH        4
#define TX_DW5_DATA_BW_SH           5
#define TX_DW5_DATA_LDPC_SH         7
#define TX_DW5_DATA_STBC_SH         8
#define TX_DW5_VCS_STBC_SH          10
#define TX_DW5_RTS_SHORT_SH         12
#define TX_DW5_RTS_SC_SH            13
#define TX_DW5_RSVD17_23_SH         17
#define TX_DW5_TX_ANT_SH            24
#define TX_DW5_TXPWR_OFSET_SH       28
#define TX_DW5_RSVD31_SH            31


//Dword 6
#define TX_DW6_SW_DEFINE_MSK            0xFFF
#define TX_DW6_MBSSID_MSK               0xF


#define TX_DW6_ANTSEL_A_MSK             0x7
#define TX_DW6_ANTSEL_B_MSK             0x7
#define TX_DW6_ANTSEL_B_MSK             0x7
#define TX_DW6_ANTSEL_D_MSK             0x7
#define TX_DW6_RSVD28_31_MSK            0xF

//Dword 6
#define TX_DW6_SW_DEFINE_SH         0
#define TX_DW6_MBSSID_SH            12
#define TX_DW6_ANTSEL_A_SH          16
#define TX_DW6_ANTSEL_B_SH          19
#define TX_DW6_ANTSEL_C_SH          22
#define TX_DW6_ANTSEL_D_SH          25
#define TX_DW6_RSVD28_31_SH         28


//Dword 7
#define TX_DW7_SW_TXBUFF_MSK            0xFFFF
#define TX_DW7_RSVD16_23_MSK            0xFF
#define TX_DW7_USB_TXAGG_NUM_MSK        0xFF

//Dword 7
#define TX_DW7_SW_TXBUFF_SH         0
#define TX_DW7_RSVD16_23_SH         16
#define TX_DW7_USB_TXAGG_NUM_SH     24


//Dword 8
#define TX_DW8_RTS_RC_MSK               0x3F
#define TX_DW8_BAR_RTY_TH_MSK           0x3

#define TX_DW8_DATA_RC_MSK              0x3F
#define TX_DW8_RSVD14_MSK               0x1
#define TX_DW8_EN_HWSEQ_MSK             0x1

#define TX_DW8_NEXTHEADPAGE_MSK         0xFF
#define TX_DW8_TAILPAGE_MSK             0xFF

//Dword 8
#define TX_DW8_RTS_RC_SH            0
#define TX_DW8_BAR_RTY_TH_SH        6
#define TX_DW8_DATA_RC_SH           8
#define TX_DW8_RSVD14_SH            14
#define TX_DW8_EN_HWSEQ_SH          15
#define TX_DW8_NEXTHEADPAGE_SH      16
#define TX_DW8_TAILPAGE_SH          24


//Dword 9
//---Normal Packet
#define TX_DW9_PADDING_LEN_MSK          0x7FF

//---Beacon Packet
#define TX_DW9_GROUPBIT_IE_OFFSET_MSK   0x7F
#define TX_DW9_GROUPBIT_IE_ENABLE_MSK   0x1

#define TX_DW9_TXBFPATH_MSK             0x1
#define TX_DW9_SEQ_MSK                  0xFFF
#define TX_DW9_FINAL_DATA_RATE_MSK      0xFF

//Dword 9
//---Normal Packet
#define TX_DW9_PADDING_LEN_SH           0

//---Beacon Packet
#define TX_DW9_GROUPBIT_IE_OFFSET_SH    0
#define TX_DW9_GROUPBIT_IE_ENABLE_SH    7

#define TX_DW9_TXBFPATH_SH              11
#define TX_DW9_SEQ_SH                   12
#define TX_DW9_FINAL_DATA_RATE_SH       24

//3  TX Buffer Descriptor Detail
//Dword 0



//3  TX WiFI Info Detail
//Dword 0

//Dword 1
//---QSEL
#define TXDESC_QSEL_TID0        0
#define TXDESC_QSEL_TID1        1
#define TXDESC_QSEL_TID2        2
#define TXDESC_QSEL_TID3        3
#define TXDESC_QSEL_TID4        4

#define TXDESC_QSEL_TID5        5
#define TXDESC_QSEL_TID6        6
#define TXDESC_QSEL_TID7        7
#define TXDESC_QSEL_TID8        8
#define TXDESC_QSEL_TID9        9         

#define TXDESC_QSEL_TID10       10
#define TXDESC_QSEL_TID11       11
#define TXDESC_QSEL_TID12       12
#define TXDESC_QSEL_TID13       13
#define TXDESC_QSEL_TID14       14
#define TXDESC_QSEL_TID15       15

#define TXDESC_QSEL_BCN         16
#define TXDESC_QSEL_HIGH        17
#define TXDESC_QSEL_MGT         18
#define TXDESC_QSEL_CMD         19

//Dword 1

#define TXDESC_SECTYPE_NO_ENCRYPTION    0
#define TXDESC_SECTYPE_WEP40_OR_TKIP    1
#define TXDESC_SECTYPE_WAPI             2
#define TXDESC_SECTYPE_AES              3


//Dword 2

//Dword 5
//---DataSC
#define TXDESC_DATASC_DONT_CARE     0
#define TXDESC_DATASC_UPPER         1
#define TXDESC_DATASC_LOWER         2
#define TXDESC_DATASC_DUPLICATE     3

#endif	//CONFIG_WLANREG_SUPPORT & (SUPPORT_CHIP_8881A|SUPPORT_CHIP_8192E)



#endif  //__HAL_COM_TXDESC_H__

