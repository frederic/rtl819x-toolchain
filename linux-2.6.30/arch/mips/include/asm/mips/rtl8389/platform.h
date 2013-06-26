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

#define MHZ             20
#define SYSCLK          MHZ * 1000 * 1000

#define BAUDRATE        38400  /* ex. 19200 or 38400 or 57600 or 115200 */ 
                               /* For Early Debug */

/*
 * Interrupt IRQ Assignments
 */
#define UART0_IRQ       31
#define UART1_IRQ       30
#define TC0_IRQ         29
#define TC1_IRQ         28
#define OCPTO_IRQ       27
#define HLXTO_IRQ       26
#define SLXTO_IRQ       25
#define NIC_IRQ         24
#define GPIO_ABCD_IRQ   23
#define GPIO_EFGH_IRQ   22
#define RTC_IRQ         21

/*
 * Interrupt Routing Selection
 */
#define UART0_RS       2
#define UART1_RS       1
#define TC0_RS         5
#define TC1_RS         1
#define OCPTO_RS       1
#define HLXTO_RS       1
#define SLXTO_RS       1
#define NIC_RS         4
#define GPIO_ABCD_RS   4
#define GPIO_EFGH_RS   4
#define RTC_RS         4


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
 * Memory Controller
 */
#define MC_MCR          0xB8001000
   #define MC_MCR_VAL      0x00000000

#define MC_DCR          0xB8001004
   #define MC_DCR0_VAL     0x54480000

#define MC_DTCR         0xB8001008
   #define MC_DTCR_VAL     0xFFFF05C0


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
   #define UART0_IE        (1 << 31)
   #define UART1_IE        (1 << 30)
   #define TC0_IE          (1 << 29)
   #define TC1_IE          (1 << 28)
   #define OCPTO_IE        (1 << 27)
   #define HLXTO_IE        (1 << 26)
   #define SLXTO_IE        (1 << 25)
   #define NIC_IE          (1 << 24)
   #define GPIO_ABCD_IE    (1 << 23)
   #define GPIO_EFGH_IE    (1 << 22)
   #define RTC_IE          (1 << 21)

#define GISR            0xB8003004
   #define UART0_IP        (1 << 31)
   #define UART1_IP        (1 << 30)
   #define TC0_IP          (1 << 29)
   #define TC1_IP          (1 << 28)
   #define OCPTO_IP        (1 << 27)
   #define HLXTO_IP        (1 << 26)
   #define SLXTO_IP        (1 << 25)
   #define NIC_IP          (1 << 24)
   #define GPIO_ABCD_IP    (1 << 23)
   #define GPIO_EFGH_IP    (1 << 22)
   #define RTC_IP          (1 << 21)

#define IRR0            0xB8003008
#define IRR0_SETTING    ((UART0_RS  << 28) | \
                         (UART1_RS  << 24) | \
                         (TC0_RS    << 20) | \
                         (TC1_RS    << 16) | \
                         (OCPTO_RS  << 12) | \
                         (HLXTO_RS  << 8)  | \
                         (SLXTO_RS  << 4)  | \
                         (NIC_RS    << 0)    \
                        )

#define IRR1            0xB800300C
#define IRR1_SETTING    ((GPIO_ABCD_RS << 28) | \
                         (GPIO_EFGH_RS << 24) | \
                         (RTC_RS       << 20)   \
                        )

#define IRR2            0xB8003010
#define IRR2_SETTING    0

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


#endif /* _PLATFORM_H */
