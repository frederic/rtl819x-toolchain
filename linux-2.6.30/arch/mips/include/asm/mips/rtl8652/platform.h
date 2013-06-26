#ifndef _PLATFORM_H
#define _PLATFORM_H


/*
 *  =============
 *  Utilty Macros
 *  =============
 */
#define REG8(reg)    (*(volatile unsigned char *)((unsigned int)reg))
#define REG32(reg)   (*(volatile unsigned int *)((unsigned int)reg))


/*
 *  ====================================
 *  Platform Configurable Common Options
 *  ====================================
 */

#define PROM_DEBUG      0

#define MHZ             200
#define SYSCLK          MHZ * 1000 * 1000

#define BAUDRATE        38400  /* ex. 19200 or 38400 or 57600 or 115200 */ 
                               /* For Early Debug */

/*
 * Interrupt IRQ Assignments
 */
#define PCIB0TMO_IRQ    0
#define PCIB1TMO_IRQ    1
#define LBCTMOm0_IRQ    2
#define LBCTMOm1_IRQ    3
#define LBCTMOs_IRQ     4
#define TC0_IRQ         8
#define TC1_IRQ         9
#define USB_IRQ         10
#define UART0_IRQ       12
#define UART1_IRQ       13
#define PCI_IRQ         14
#define SWCORE_IRQ      15
#define GPIO_ABCD_IRQ   16
#define GPIO_EFGH_IRQ   17
#define HCI_IRQ         18
#define PCM_IRQ         19
#define CRYPTO_IRQ      20
#define GDMA_IRQ        21

/*
 * Interrupt Routing Selection
 */
#define PCIB0TMO_RS     2
#define PCIB1TMO_RS     2
#define LBCTMOm0_RS     2
#define LBCTMOm1_RS     2
#define LBCTMOs_RS      2
#define TC0_RS          7
#define TC1_RS          2
#define USB_H_RS        4
#define UART0_RS        3
#define UART1_RS        2
#define PCI_RS          5
#define SW_RS           6
#define GPIO_ABCD_RS    2
#define GPIO_EFGH_RS    2
#define HCI_RS          2
#define PCM_RS          2
#define CRYPTO_RS       2
#define GDMA_RS         2


#define DIVISOR         1000

#if DIVISOR > (1 << 16)
#error "Exceed the Maximum Value of DivFactor"
#endif

/*
 *  ==========================
 *  Platform Register Settings
 *  ==========================
 */

/*
 * CPU
 */
#define IMEM_BASE       0x00C00000
#define IMEM_TOP        0x00C03FFF

#define DMEM_BASE       0x00C04000
#define DMEM_TOP        0x00C05FFF

/*
 * Memory Controller
 */
#define MC_MCR          0xB8001000
   #define MC_MCR_VAL      0x92A28000

#define MC_MTCR0        0xB8001004
   #define MC_MTCR0_VAL    0x12120000

#define MC_MTCR1        0xB8001008
   #define MC_MTCR1_VAL    0x00000FEB

#define MC_PFCR         0xB8001010
   #define MC_PFCR_VAL     0x00000101


#define MC_BASE         0xB8001000
#define NCR             (MC_BASE + 0x100)
#define NSR             (MC_BASE + 0x104)
#define NCAR            (MC_BASE + 0x108)
#define NADDR           (MC_BASE + 0x10C)
#define NDR             (MC_BASE + 0x110)

#define SFCR            (MC_BASE + 0x200)
#define SFDR            (MC_BASE + 0x204)

/*
 * UART
 */
#define UART0_BASE      0xB8002000
#define UART0_RBR       (UART0_BASE + 0x000)
#define UART0_THR       (UART0_BASE + 0x000)
#define UART0_DLL       (UART0_BASE + 0x000)
#define UART0_IER       (UART0_BASE + 0x004)
#define UART0_DLM       (UART0_BASE + 0x004)
#define UART0_IIR       (UART0_BASE + 0x008)
#define UART0_FCR       (UART0_BASE + 0x008)
#define UART0_LCR       (UART0_BASE + 0x00C)
#define UART0_MCR       (UART0_BASE + 0x010)
#define UART0_LSR       (UART0_BASE + 0x014)

#define UART1_BASE      0xB8002100
#define UART1_RBR       (UART1_BASE + 0x000)
#define UART1_THR       (UART1_BASE + 0x000)
#define UART1_DLL       (UART1_BASE + 0x000)
#define UART1_IER       (UART1_BASE + 0x004)
#define UART1_DLM       (UART1_BASE + 0x004)
#define UART1_IIR       (UART1_BASE + 0x008)
#define UART1_FCR       (UART1_BASE + 0x008)
   #define FCR_EN          0x01
   #define FCR_RXRST       0x02
   #define     RXRST             0x02
   #define FCR_TXRST       0x04
   #define     TXRST             0x04
   #define FCR_DMA         0x08
   #define FCR_RTRG        0xC0
   #define     CHAR_TRIGGER_01   0x00
   #define     CHAR_TRIGGER_04   0x40
   #define     CHAR_TRIGGER_08   0x80
   #define     CHAR_TRIGGER_14   0xC0
#define UART1_LCR       (UART1_BASE + 0x00C)
   #define LCR_WLN         0x03
   #define     CHAR_LEN_5        0x00
   #define     CHAR_LEN_6        0x01
   #define     CHAR_LEN_7        0x02
   #define     CHAR_LEN_8        0x03
   #define LCR_STB         0x04
   #define     ONE_STOP          0x00
   #define     TWO_STOP          0x04
   #define LCR_PEN         0x08
   #define     PARITY_ENABLE     0x01
   #define     PARITY_DISABLE    0x00
   #define LCR_EPS         0x30
   #define     PARITY_ODD        0x00
   #define     PARITY_EVEN       0x10
   #define     PARITY_MARK       0x20
   #define     PARITY_SPACE      0x30
   #define LCR_BRK         0x40
   #define LCR_DLAB        0x80
   #define     DLAB              0x80
#define UART1_MCR       (UART1_BASE + 0x010)
#define UART1_LSR       (UART1_BASE + 0x014)
   #define LSR_DR          0x01
   #define     RxCHAR_AVAIL      0x01
   #define LSR_OE          0x02
   #define LSR_PE          0x04
   #define LSR_FE          0x08
   #define LSR_BI          0x10
   #define LSR_THRE        0x20
   #define     TxCHAR_AVAIL      0x00
   #define     TxCHAR_EMPTY      0x20
   #define LSR_TEMT        0x40
   #define LSR_RFE         0x80


/*
 * Interrupt Controller
 */
#define GIMR            0xB8003000
   #define GDMA_IE         (1 << 21)
   #define CRYPTO_IE       (1 << 20)
   #define PCM_IE          (1 << 19)
   #define HCI_IE          (1 << 18)
   #define GPIO_EFGH_IE    (1 << 17)
   #define GPIO_ABCD_IE    (1 << 16)
   #define SW_IE           (1 << 15)
   #define PCI_IE          (1 << 14)
   #define UART1_IE        (1 << 13)
   #define UART0_IE        (1 << 12)
   #define USB_H_IE        (1 << 10)
   #define TC1_IE          (1 << 9)
   #define TC0_IE          (1 << 8)
   #define LBCTMOs_IE      (1 << 4)
   #define LBCTMOm1_IE     (1 << 3)
   #define LBCTMOm0_IE     (1 << 2)
   #define PCIB1TO_IE      (1 << 1)
   #define PCIB0TO_IE      (1 << 0)

#define GISR            0xB8003004
   #define GDMA_IP         (1 << 21)
   #define CRYPTO_IP       (1 << 20)
   #define PCM_IP          (1 << 19)
   #define HCI_IP          (1 << 18)
   #define GPIO_EFGH_IP    (1 << 17)
   #define GPIO_ABCD_IP    (1 << 16)
   #define SW_IP           (1 << 15)
   #define PCI_IP          (1 << 14)
   #define UART1_IP        (1 << 13)
   #define UART0_IP        (1 << 12)
   #define USB_H_IP        (1 << 10)
   #define TC1_IP          (1 << 9)
   #define TC0_IP          (1 << 8)
   #define LBCTMOs_IP      (1 << 4)
   #define LBCTMOm1_IP     (1 << 3)
   #define LBCTMOm0_IP     (1 << 2)
   #define PCIB1TO_IP      (1 << 1)
   #define PCIB0TO_IP      (1 << 0)

#define IRR0            0xB8003008
#define IRR0_SETTING    ((LBCTMOs_RS  << 16) | \
                         (LBCTMOm1_RS << 12) | \
                         (LBCTMOm0_RS << 8)  | \
                         (PCIB1TMO_RS << 4)  | \
                         (PCIB0TMO_RS << 0)    \
                        )

#define IRR1            0xB800300C
#define IRR1_SETTING    ((SW_RS    << 28) | \
                         (PCI_RS   << 24) | \
                         (UART1_RS << 20) | \
                         (UART0_RS << 16) | \
                         (USB_H_RS << 8)  | \
                         (TC1_RS   << 4)  | \
                         (TC0_RS   << 0)    \
                        )

#define IRR2            0xB8003010
#define IRR2_SETTING    ((GDMA_RS      << 20) | \
                         (CRYPTO_RS    << 16) | \
                         (PCM_RS       << 12) | \
                         (HCI_RS       << 8)  | \
                         (GPIO_EFGH_RS << 4)  | \
                         (GPIO_ABCD_RS << 0)    \
                        )

#define IRR3            0xB8003014
#define IRR3_SETTING    0

/*
 * Timer/Counter
 */
#define TC_BASE         0xB8003100
#define TC0DATA         (TC_BASE + 0x00)
#define TC1DATA         (TC_BASE + 0x04)
   #define TCD_OFFSET      8
#define TC0CNT          (TC_BASE + 0x08)
#define TC1CNT          (TC_BASE + 0x0C)
#define TCCNR           (TC_BASE + 0x10)
   #define TC0EN           (1 << 31)
   #define TC0MODE_TIMER   (1 << 30)
   #define TC1EN           (1 << 29)
   #define TC1MODE_TIMER   (1 << 28)
#define TCIR            (TC_BASE + 0x14)
   #define TC0IE           (1 << 31)
   #define TC1IE           (1 << 30)
   #define TC0IP           (1 << 29)
   #define TC1IP           (1 << 28)
#define CDBR            (TC_BASE + 0x18)
   #define DIVF_OFFSET     16
#define WDTCNR          (TC_BASE + 0x1C)

/*
 * HCI
 */
#define HCI_BASE        0xB8007000
#define HCI_GCR         (HCI_BASE + 0x00)
#define HCI_TRFFDR      (HCI_BASE + 0x04)
#define HCI_TRFFAR      (HCI_BASE + 0x08)
#define HCI_FFCR_CS(i)  (HCI_BASE + 0x0C + ((i) << 2))
#define HCI_IER_CS(i)   (HCI_BASE + 0x1C + ((i) << 2))
#define HCI_LSR_CS(i)   (HCI_BASE + 0x2C + ((i) << 2))
#define HCI_CR_CS(i)    (HCI_BASE + 0x3C + ((i) << 2))
#define HCI_TR_CS(i)    (HCI_BASE + 0x4C + ((i) << 2))

#endif /* _PLATFORM_H */
