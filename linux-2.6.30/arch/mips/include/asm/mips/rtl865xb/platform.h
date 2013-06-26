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

#define MHZ             100
#define SYSCLK          MHZ * 1000 * 1000

#define BAUDRATE        38400  /* ex. 19200 or 38400 or 57600 or 115200 */ 
                               /* For Early Debug */

/*
 * Interrupt IRQ Assignments
 */
#define PCIBTMO_IRQ     16
#define GPIO_DEFGHI_IRQ 17
#define PCM_IRQ         18
#define AUTH_IRQ        19
#define CRYPTO_IRQ      20
#define LBCTMO_IRQ      21
#define EXT_IRQ         23
#define GPIO_ABC_IRQ    24
#define SWCORE_IRQ      25
#define PCI_IRQ         26
#define UART1_IRQ       27
#define UART0_IRQ       28
#define PCMCIA_IRQ      29
#define USB_IRQ         30
#define TC_IRQ          31

/*
 * Interrupt Routing Selection
 */
#define PCIBTMO_RS      0
#define GPIO_DEFGHI_RS  0
#define PCM_RS          0
#define AUTH_RS         0
#define CRYPTO_RS       0
#define LBCTMO_RS       0
#define EXT_RS          0
#define GPIO_ABC_RS     0
#define SWCORE_RS       3
#define PCI_RS          2
#define UART1_RS        0
#define UART0_RS        1
#define PCMCIA_RS       0
#define USB_RS          0
#define TC_RS           4

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
#define IMEM_TOP        0x00C01FFF

#define DMEM_BASE       0x00C02000
#define DMEM_TOP        0x00C03FFF

/*
 * Memory Controller
 */
#define MC_MCR          0xBD013000
   #define MC_MCR_VAL      0xFAA00000

#define MC_MTCR0        0xBD013004
   #define MC_MTCR0_VAL    0x1B1B0000

#define MC_MTCR1        0xBD013008
   #define MC_MTCR1_VAL    0x00000CEA

/*
 * UART
 */
#define UART0_BASE      0xBD011000
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

#define UART1_BASE      0xBD011100
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
#define GIMR            0xBD012000
   #define TC_IE           (1 << 31)
   #define USB_IE          (1 << 30)
   #define PCMICA_IE       (1 << 29)
   #define UART0_IE        (1 << 28)
   #define UART1_IE        (1 << 27)
   #define PCI_IE          (1 << 26)
   #define SWCORE_IE       (1 << 25)
   #define GPIO_ABC_IE     (1 << 24)
   #define EXT_IE          (1 << 23)
   #define LBCTMO_IE       (1 << 21)
   #define CRYPTO_IE       (1 << 20)
   #define AUTH_IE         (1 << 19)
   #define PCM_IE          (1 << 18)
   #define GPIO_DEFGHI_IE  (1 << 17)
   #define PCIBTMO_IE      (1 << 16)

#define GISR            0xBD012004
   #define TC_IP           (1 << 31)
   #define USB_IP          (1 << 30)
   #define PCMICA_IP       (1 << 29)
   #define UART0_IP        (1 << 28)
   #define UART1_IP        (1 << 27)
   #define PCI_IP          (1 << 26)
   #define SWCORE_IP       (1 << 25)
   #define GPIO_ABC_IP     (1 << 24)
   #define EXT_IP          (1 << 23)
   #define LBCTMO_IP       (1 << 21)
   #define CRYPTO_IP       (1 << 20)
   #define AUTH_IP         (1 << 19)
   #define PCM_IP          (1 << 18)
   #define GPIO_DEFGHI_IP  (1 << 17)
   #define PCIBTMO_IP      (1 << 16)

#define IRR1            0xBD012008
#define IRR1_SETTING    (((TC_RS          & 0x3) << 30) | \
                         ((USB_RS         & 0x3) << 28) | \
                         ((PCMCIA_RS      & 0x3) << 26) | \
                         ((UART0_RS       & 0x3) << 24) | \
                         ((UART1_RS       & 0x3) << 22) | \
                         ((PCI_RS         & 0x3) << 20) | \
                         ((SWCORE_RS      & 0x3) << 18) | \
                         ((GPIO_ABC_RS    & 0x3) << 16) | \
                         ((EXT_RS         & 0x3) << 14) | \
                         ((LBCTMO_RS      & 0x3) << 12) | \
                         ((TC_RS          & 0x4) >> 2 << 9) | \
                         ((USB_RS         & 0x4) >> 2 << 8) | \
                         ((PCMCIA_RS      & 0x4) >> 2 << 7) | \
                         ((UART0_RS       & 0x4) >> 2 << 6) | \
                         ((UART1_RS       & 0x4) >> 2 << 5) | \
                         ((PCI_RS         & 0x4) >> 2 << 4) | \
                         ((SWCORE_RS      & 0x4) >> 2 << 3) | \
                         ((GPIO_ABC_RS    & 0x4) >> 2 << 2) | \
                         ((EXT_RS         & 0x4) >> 2 << 1) | \
                         ((LBCTMO_RS      & 0x4) >> 2 << 0)   \
                        )

#define IRR2            0xBD0120A0
#define IRR2_SETTING    (((CRYPTO_RS      & 0x7) << 29) | \
                         ((AUTH_RS        & 0x7) << 26) | \
                         ((PCM_RS         & 0x7) << 23) | \
                         ((GPIO_DEFGHI_RS & 0x7) << 20) | \
                         ((PCIBTMO_RS     & 0x7) << 17)   \
                        )

/*
 * Timer/Counter
 */
#define TC_BASE         0xBD012020
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


#endif /* _PLATFORM_H */
