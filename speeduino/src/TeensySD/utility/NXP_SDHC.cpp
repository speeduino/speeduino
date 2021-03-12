// adapted for Arduino SD library by Paul Stoffregen
// following code is modified by Walter Zimmer from
// from version provided by
// Petr Gargulak (NXP Employee)
//https://community.nxp.com/servlet/JiveServlet/download/339474-1-263510/SDHC_K60_Baremetal.ZIP
//see also
//https://community.nxp.com/thread/99202

#if defined(__MK64FX512__) || defined(__MK66FX1M0__) || defined(__IMXRT1062__)

#include "core_pins.h"  // include calls to kinetis.h or imxrt.h
#include "usb_serial.h" // for Serial

#include "NXP_SDHC.h"
#include "src/coru/coru.h"

/******************************************************************************
  Constants
******************************************************************************/

enum {
  SDHC_RESULT_OK = 0,             /* 0: Successful */
  SDHC_RESULT_ERROR,              /* 1: R/W Error */
  SDHC_RESULT_WRPRT,              /* 2: Write Protected */
  SDHC_RESULT_NOT_READY,          /* 3: Not Ready */
  SDHC_RESULT_PARERR,             /* 4: Invalid Parameter */
  SDHC_RESULT_NO_RESPONSE         /* 5: No Response */ // from old diskio.h
};

/*void print_result(int n)
  {
        switch (n) {
        case SDHC_RESULT_OK: serial_print("OK\n"); break;
        case SDHC_RESULT_ERROR: serial_print("R/W Error\n"); break;
        case SDHC_RESULT_WRPRT: serial_print("Write Protect\n"); break;
        case SDHC_RESULT_NOT_READY: serial_print("Not Ready\n"); break;
        case SDHC_RESULT_PARERR: serial_print("Invalid Param\n"); break;
        case SDHC_RESULT_NO_RESPONSE: serial_print("No Response\n"); break;
        default: serial_print("Unknown result\n");
        }
  }*/


#define IO_SDHC_ATTRIBS (IO_DEV_ATTR_READ | IO_DEV_ATTR_REMOVE | IO_DEV_ATTR_SEEK | IO_DEV_ATTR_WRITE | IO_DEV_ATTR_BLOCK_MODE)

#define SDHC_XFERTYP_RSPTYP_NO              (0x00)
#define SDHC_XFERTYP_RSPTYP_136             (0x01)
#define SDHC_XFERTYP_RSPTYP_48              (0x02)
#define SDHC_XFERTYP_RSPTYP_48BUSY          (0x03)

#define SDHC_XFERTYP_CMDTYP_ABORT           (0x03)

#define SDHC_PROCTL_EMODE_INVARIANT         (0x02)

#define SDHC_PROCTL_DTW_1BIT                (0x00)
#define SDHC_PROCTL_DTW_4BIT                (0x01)
#define SDHC_PROCTL_DTW_8BIT                (0x10)

#define SDHC_INITIALIZATION_MAX_CNT 100000

/* SDHC commands */
#define SDHC_CMD0                           (0)
#define SDHC_CMD1                           (1)
#define SDHC_CMD2                           (2)
#define SDHC_CMD3                           (3)
#define SDHC_CMD4                           (4)
#define SDHC_CMD5                           (5)
#define SDHC_CMD6                           (6)
#define SDHC_CMD7                           (7)
#define SDHC_CMD8                           (8)
#define SDHC_CMD9                           (9)
#define SDHC_CMD10                          (10)
#define SDHC_CMD11                          (11)
#define SDHC_CMD12                          (12)
#define SDHC_CMD13                          (13)
#define SDHC_CMD15                          (15)
#define SDHC_CMD16                          (16)
#define SDHC_CMD17                          (17)
#define SDHC_CMD18                          (18)
#define SDHC_CMD20                          (20)
#define SDHC_CMD24                          (24)
#define SDHC_CMD25                          (25)
#define SDHC_CMD26                          (26)
#define SDHC_CMD27                          (27)
#define SDHC_CMD28                          (28)
#define SDHC_CMD29                          (29)
#define SDHC_CMD30                          (30)
#define SDHC_CMD32                          (32)
#define SDHC_CMD33                          (33)
#define SDHC_CMD34                          (34)
#define SDHC_CMD35                          (35)
#define SDHC_CMD36                          (36)
#define SDHC_CMD37                          (37)
#define SDHC_CMD38                          (38)
#define SDHC_CMD39                          (39)
#define SDHC_CMD40                          (40)
#define SDHC_CMD42                          (42)
#define SDHC_CMD52                          (52)
#define SDHC_CMD53                          (53)
#define SDHC_CMD55                          (55)
#define SDHC_CMD56                          (56)
#define SDHC_CMD59                          (59)
#define SDHC_CMD60                          (60)
#define SDHC_CMD61                          (61)
#define SDHC_ACMD6                          (0x40 + 6)
#define SDHC_ACMD13                         (0x40 + 13)
#define SDHC_ACMD22                         (0x40 + 22)
#define SDHC_ACMD23                         (0x40 + 23)
#define SDHC_ACMD41                         (0x40 + 41)
#define SDHC_ACMD42                         (0x40 + 42)
#define SDHC_ACMD51                         (0x40 + 51)

#define SDHC_FIFO_BUFFER_SIZE               16
#define SDHC_BLOCK_SIZE                     512

#if defined(__IMXRT1062__)
#define MAKE_REG_MASK(m,s) (((uint32_t)(((uint32_t)(m) << s))))
#define MAKE_REG_GET(x,m,s) (((uint32_t)(((uint32_t)(x)>>s) & m)))
#define MAKE_REG_SET(x,m,s) (((uint32_t)(((uint32_t)(x) & m) << s)))

#define SDHC_BLKATTR_BLKSIZE_MASK MAKE_REG_MASK(0x1FFF,0) //uint32_t)(((n) & 0x1FFF)<<0) // Transfer Block Size Mask
#define SDHC_BLKATTR_BLKSIZE(n)   MAKE_REG_SET(n,0x1FFF,0) //uint32_t)(((n) & 0x1FFF)<<0) // Transfer Block Size
#define SDHC_BLKATTR_BLKCNT_MASK  MAKE_REG_MASK(0x1FFF,16) //((uint32_t)0x1FFF<<16)
#define SDHC_BLKATTR_BLKCNT(n)    MAKE_REG_SET(n,0x1FFF,16) //(uint32_t)(((n) & 0x1FFF)<<16) // Blocks Count For Current Transfer

#define SDHC_XFERTYP_CMDINX(n)  MAKE_REG_SET(n,0x3F,24) //(uint32_t)(((n) & 0x3F)<<24)// Command Index
#define SDHC_XFERTYP_CMDTYP(n)  MAKE_REG_SET(n,0x3,22) //(uint32_t)(((n) & 0x3)<<22)  // Command Type
#define SDHC_XFERTYP_DPSEL    MAKE_REG_MASK(0x1,21) //((uint32_t)0x00200000)    // Data Present Select
#define SDHC_XFERTYP_CICEN    MAKE_REG_MASK(0x1,20) //((uint32_t)0x00100000)    // Command Index Check Enable
#define SDHC_XFERTYP_CCCEN    MAKE_REG_MASK(0x1,19) //((uint32_t)0x00080000)    // Command CRC Check Enable
#define SDHC_XFERTYP_RSPTYP(n)  MAKE_REG_SET(n,0x3,16) //(uint32_t)(((n) & 0x3)<<16)  // Response Type Select
#define SDHC_XFERTYP_MSBSEL   MAKE_REG_MASK(0x1,5) //((uint32_t)0x00000020)   // Multi/Single Block Select
#define SDHC_XFERTYP_DTDSEL   MAKE_REG_MASK(0x1,4) //((uint32_t)0x00000010)   // Data Transfer Direction Select
#define SDHC_XFERTYP_AC12EN   MAKE_REG_MASK(0x1,2) //((uint32_t)0x00000004)   // Auto CMD12 Enable
#define SDHC_XFERTYP_BCEN   MAKE_REG_MASK(0x1,1) //((uint32_t)0x00000002)   // Block Count Enable
#define SDHC_XFERTYP_DMAEN    MAKE_REG_MASK(0x3,0) //((uint32_t)0x00000001)   // DMA Enable

#define SDHC_PRSSTAT_DLSL_MASK    MAKE_REG_MASK(0xFF,24)  //((uint32_t)0xFF000000)    // DAT Line Signal Level
#define SDHC_PRSSTAT_CLSL     MAKE_REG_MASK(0x1,23) //((uint32_t)0x00800000)    // CMD Line Signal Level
#define SDHC_PRSSTAT_WPSPL      MAKE_REG_MASK(0x1,19) //
#define SDHC_PRSSTAT_CDPL     MAKE_REG_MASK(0x1,18) //
#define SDHC_PRSSTAT_CINS     MAKE_REG_MASK(0x1,16) //((uint32_t)0x00010000)    // Card Inserted
#define SDHC_PRSSTAT_TSCD     MAKE_REG_MASK(0x1,15)
#define SDHC_PRSSTAT_RTR      MAKE_REG_MASK(0x1,12)
#define SDHC_PRSSTAT_BREN     MAKE_REG_MASK(0x1,11) //((uint32_t)0x00000800)    // Buffer Read Enable
#define SDHC_PRSSTAT_BWEN     MAKE_REG_MASK(0x1,10) //((uint32_t)0x00000400)    // Buffer Write Enable
#define SDHC_PRSSTAT_RTA      MAKE_REG_MASK(0x1,9) //((uint32_t)0x00000200)   // Read Transfer Active
#define SDHC_PRSSTAT_WTA      MAKE_REG_MASK(0x1,8) //((uint32_t)0x00000100)   // Write Transfer Active
#define SDHC_PRSSTAT_SDOFF      MAKE_REG_MASK(0x1,7) //((uint32_t)0x00000080)   // SD Clock Gated Off Internally
#define SDHC_PRSSTAT_PEROFF     MAKE_REG_MASK(0x1,6) //((uint32_t)0x00000040)   // SDHC clock Gated Off Internally
#define SDHC_PRSSTAT_HCKOFF     MAKE_REG_MASK(0x1,5) //((uint32_t)0x00000020)   // System Clock Gated Off Internally
#define SDHC_PRSSTAT_IPGOFF     MAKE_REG_MASK(0x1,4) //((uint32_t)0x00000010)   // Bus Clock Gated Off Internally
#define SDHC_PRSSTAT_SDSTB      MAKE_REG_MASK(0x1,3) //((uint32_t)0x00000008)   // SD Clock Stable
#define SDHC_PRSSTAT_DLA      MAKE_REG_MASK(0x1,2) //((uint32_t)0x00000004)   // Data Line Active
#define SDHC_PRSSTAT_CDIHB      MAKE_REG_MASK(0x1,1) //((uint32_t)0x00000002)   // Command Inhibit (DAT)
#define SDHC_PRSSTAT_CIHB     MAKE_REG_MASK(0x1,0) //((uint32_t)0x00000001)   // Command Inhibit (CMD)

#define SDHC_PROTCT_NONEXACT_BLKRD  MAKE_REG_MASK(0x1,30) //
#define SDHC_PROTCT_BURST_LENEN(n)  MAKE_REG_SET(n,0x7,12) //
#define SDHC_PROCTL_WECRM     MAKE_REG_MASK(0x1,26) //((uint32_t)0x04000000)    // Wakeup Event Enable On SD Card Removal
#define SDHC_PROCTL_WECINS      MAKE_REG_MASK(0x1,25) //((uint32_t)0x02000000)    // Wakeup Event Enable On SD Card Insertion
#define SDHC_PROCTL_WECINT      MAKE_REG_MASK(0x1,24) //((uint32_t)0x01000000)    // Wakeup Event Enable On Card Interrupt
#define SDHC_PROCTL_RD_DONE_NOBLK MAKE_REG_MASK(0x1,20) //
#define SDHC_PROCTL_IABG      MAKE_REG_MASK(0x1,19) //((uint32_t)0x00080000)    // Interrupt At Block Gap
#define SDHC_PROCTL_RWCTL     MAKE_REG_MASK(0x1,18) //((uint32_t)0x00040000)    // Read Wait Control
#define SDHC_PROCTL_CREQ      MAKE_REG_MASK(0x1,17) //((uint32_t)0x00020000)    // Continue Request
#define SDHC_PROCTL_SABGREQ     MAKE_REG_MASK(0x1,16) //((uint32_t)0x00010000)    // Stop At Block Gap Request
#define SDHC_PROCTL_DMAS(n)     MAKE_REG_SET(n,0x3,8) //(uint32_t)(((n) & 0x3)<<8)  // DMA Select
#define SDHC_PROCTL_CDSS      MAKE_REG_MASK(0x1,7) //((uint32_t)0x00000080)   // Card Detect Signal Selection
#define SDHC_PROCTL_CDTL      MAKE_REG_MASK(0x1,6) //((uint32_t)0x00000040)   // Card Detect Test Level
#define SDHC_PROCTL_EMODE(n)    MAKE_REG_SET(n,0x3,4) //(uint32_t)(((n) & 0x3)<<4)  // Endian Mode
#define SDHC_PROCTL_EMODE_MASK  MAKE_REG_MASK(0x3,4) //(uint32_t)((0x3)<<4) // Endian Mode
#define SDHC_PROCTL_D3CD      MAKE_REG_MASK(0x1,3) //((uint32_t)0x00000008)   // DAT3 As Card Detection Pin
#define SDHC_PROCTL_DTW(n)      MAKE_REG_SET(n,0x3,1) //(uint32_t)(((n) & 0x3)<<1)  // Data Transfer Width, 0=1bit, 1=4bit, 2=8bit
#define SDHC_PROCTL_DTW_MASK    MAKE_REG_MASK(0x3,1) //((uint32_t)0x00000006)
#define SDHC_PROCTL_LCTL      MAKE_REG_MASK(0x1,0) //((uint32_t)0x00000001)   // LED Control

#define SDHC_SYSCTL_RSTT      MAKE_REG_MASK(0x1,28) //
#define SDHC_SYSCTL_INITA     MAKE_REG_MASK(0x1,27) //((uint32_t)0x08000000)    // Initialization Active
#define SDHC_SYSCTL_RSTD      MAKE_REG_MASK(0x1,26) //((uint32_t)0x04000000)    // Software Reset For DAT Line
#define SDHC_SYSCTL_RSTC      MAKE_REG_MASK(0x1,25) //((uint32_t)0x02000000)    // Software Reset For CMD Line
#define SDHC_SYSCTL_RSTA      MAKE_REG_MASK(0x1,24) //((uint32_t)0x01000000)    // Software Reset For ALL
#define SDHC_SYSCTL_DTOCV(n)  MAKE_REG_SET(n,0xF,16) //(uint32_t)(((n) & 0xF)<<16)  // Data Timeout Counter Value
#define SDHC_SYSCTL_DTOCV_MASK    MAKE_REG_MASK(0xF,16) //((uint32_t)0x000F0000)
#define SDHC_SYSCTL_SDCLKFS(n)    MAKE_REG_SET(n,0xFF,8) //(uint32_t)(((n) & 0xFF)<<8)  // SDCLK Frequency Select
#define SDHC_SYSCTL_SDCLKFS_MASK  MAKE_REG_MASK(0xFF,8) //((uint32_t)0x0000FF00)
#define SDHC_SYSCTL_DVS(n)      MAKE_REG_SET(n,0xF,4) //(uint32_t)(((n) & 0xF)<<4)  // Divisor
#define SDHC_SYSCTL_DVS_MASK    MAKE_REG_MASK(0xF,4)  //((uint32_t)0x000000F0)

#define SDHC_SYSCTL_SDCLKEN   ((uint32_t)0x00000008)    // SD Clock Enable
#define SDHC_SYSCTL_PEREN     ((uint32_t)0x00000004)    // Peripheral Clock Enable
#define SDHC_SYSCTL_HCKEN     ((uint32_t)0x00000002)    // System Clock Enable
#define SDHC_SYSCTL_IPGEN     ((uint32_t)0x00000001)    // IPG Clock Enable

#define SDHC_IRQSTAT_DMAE     MAKE_REG_MASK(0x1,28) //((uint32_t)0x10000000)    // DMA Error
#define SDHC_IRQSTAT_TNE      MAKE_REG_MASK(0x1,26) //
#define SDHC_IRQSTAT_AC12E    MAKE_REG_MASK(0x1,24) //((uint32_t)0x01000000)    // Auto CMD12 Error
#define SDHC_IRQSTAT_DEBE     MAKE_REG_MASK(0x1,22) //((uint32_t)0x00400000)    // Data End Bit Error
#define SDHC_IRQSTAT_DCE      MAKE_REG_MASK(0x1,21) //((uint32_t)0x00200000)    // Data CRC Error
#define SDHC_IRQSTAT_DTOE     MAKE_REG_MASK(0x1,20) //((uint32_t)0x00100000)    // Data Timeout Error
#define SDHC_IRQSTAT_CIE      MAKE_REG_MASK(0x1,19) //((uint32_t)0x00080000)    // Command Index Error
#define SDHC_IRQSTAT_CEBE     MAKE_REG_MASK(0x1,18) //((uint32_t)0x00040000)    // Command End Bit Error
#define SDHC_IRQSTAT_CCE      MAKE_REG_MASK(0x1,17) //((uint32_t)0x00020000)    // Command CRC Error
#define SDHC_IRQSTAT_CTOE     MAKE_REG_MASK(0x1,16) //((uint32_t)0x00010000)    // Command Timeout Error
#define SDHC_IRQSTAT_TP       MAKE_REG_MASK(0x1,14) //
#define SDHC_IRQSTAT_RTE      MAKE_REG_MASK(0x1,12) //
#define SDHC_IRQSTAT_CINT     MAKE_REG_MASK(0x1,8) //((uint32_t)0x00000100)   // Card Interrupt
#define SDHC_IRQSTAT_CRM      MAKE_REG_MASK(0x1,7) //((uint32_t)0x00000080)   // Card Removal
#define SDHC_IRQSTAT_CINS     MAKE_REG_MASK(0x1,6) //((uint32_t)0x00000040)   // Card Insertion
#define SDHC_IRQSTAT_BRR      MAKE_REG_MASK(0x1,5) //((uint32_t)0x00000020)   // Buffer Read Ready
#define SDHC_IRQSTAT_BWR      MAKE_REG_MASK(0x1,4) //((uint32_t)0x00000010)   // Buffer Write Ready
#define SDHC_IRQSTAT_DINT     MAKE_REG_MASK(0x1,3) //((uint32_t)0x00000008)   // DMA Interrupt
#define SDHC_IRQSTAT_BGE      MAKE_REG_MASK(0x1,2) //((uint32_t)0x00000004)   // Block Gap Event
#define SDHC_IRQSTAT_TC       MAKE_REG_MASK(0x1,1) //((uint32_t)0x00000002)   // Transfer Complete
#define SDHC_IRQSTAT_CC       MAKE_REG_MASK(0x1,0) //((uint32_t)0x00000001)   // Command Complete

#define SDHC_IRQSTATEN_DMAESEN    MAKE_REG_MASK(0x1,28) //((uint32_t)0x10000000)    // DMA Error Status Enable
#define SDHC_IRQSTATEN_TNESEN   MAKE_REG_MASK(0x1,26) //
#define SDHC_IRQSTATEN_AC12ESEN   MAKE_REG_MASK(0x1,24) //((uint32_t)0x01000000)    // Auto CMD12 Error Status Enable
#define SDHC_IRQSTATEN_DEBESEN    MAKE_REG_MASK(0x1,22) //((uint32_t)0x00400000)    // Data End Bit Error Status Enable
#define SDHC_IRQSTATEN_DCESEN   MAKE_REG_MASK(0x1,21) //((uint32_t)0x00200000)    // Data CRC Error Status Enable
#define SDHC_IRQSTATEN_DTOESEN    MAKE_REG_MASK(0x1,20) //((uint32_t)0x00100000)    // Data Timeout Error Status Enable
#define SDHC_IRQSTATEN_CIESEN   MAKE_REG_MASK(0x1,19) //((uint32_t)0x00080000)    // Command Index Error Status Enable
#define SDHC_IRQSTATEN_CEBESEN    MAKE_REG_MASK(0x1,18) //((uint32_t)0x00040000)    // Command End Bit Error Status Enable
#define SDHC_IRQSTATEN_CCESEN   MAKE_REG_MASK(0x1,17) //((uint32_t)0x00020000)    // Command CRC Error Status Enable
#define SDHC_IRQSTATEN_CTOESEN    MAKE_REG_MASK(0x1,16) //((uint32_t)0x00010000)    // Command Timeout Error Status Enable
#define SDHC_IRQSTATEN_TPSEN    MAKE_REG_MASK(0x1,14) //
#define SDHC_IRQSTATEN_RTESEN   MAKE_REG_MASK(0x1,12) //
#define SDHC_IRQSTATEN_CINTSEN    MAKE_REG_MASK(0x1,8) //((uint32_t)0x00000100)   // Card Interrupt Status Enable
#define SDHC_IRQSTATEN_CRMSEN   MAKE_REG_MASK(0x1,7) //((uint32_t)0x00000080)   // Card Removal Status Enable
#define SDHC_IRQSTATEN_CINSEN   MAKE_REG_MASK(0x1,6) //((uint32_t)0x00000040)   // Card Insertion Status Enable
#define SDHC_IRQSTATEN_BRRSEN   MAKE_REG_MASK(0x1,5) //((uint32_t)0x00000020)   // Buffer Read Ready Status Enable
#define SDHC_IRQSTATEN_BWRSEN   MAKE_REG_MASK(0x1,4) //((uint32_t)0x00000010)   // Buffer Write Ready Status Enable
#define SDHC_IRQSTATEN_DINTSEN    MAKE_REG_MASK(0x1,3) //((uint32_t)0x00000008)   // DMA Interrupt Status Enable
#define SDHC_IRQSTATEN_BGESEN   MAKE_REG_MASK(0x1,2) //((uint32_t)0x00000004)   // Block Gap Event Status Enable
#define SDHC_IRQSTATEN_TCSEN    MAKE_REG_MASK(0x1,1) //((uint32_t)0x00000002)   // Transfer Complete Status Enable
#define SDHC_IRQSTATEN_CCSEN    MAKE_REG_MASK(0x1,0) //((uint32_t)0x00000001)   // Command Complete Status Enable

#define SDHC_IRQSIGEN_DMAEIEN   MAKE_REG_MASK(0x1,28) //((uint32_t)0x10000000)    // DMA Error Interrupt Enable
#define SDHC_IRQSIGEN_TNEIEN    MAKE_REG_MASK(0x1,26) //
#define SDHC_IRQSIGEN_AC12EIEN    MAKE_REG_MASK(0x1,24) //((uint32_t)0x01000000)    // Auto CMD12 Error Interrupt Enable
#define SDHC_IRQSIGEN_DEBEIEN   MAKE_REG_MASK(0x1,22) //((uint32_t)0x00400000)    // Data End Bit Error Interrupt Enable
#define SDHC_IRQSIGEN_DCEIEN    MAKE_REG_MASK(0x1,21) //((uint32_t)0x00200000)    // Data CRC Error Interrupt Enable
#define SDHC_IRQSIGEN_DTOEIEN   MAKE_REG_MASK(0x1,20) //((uint32_t)0x00100000)    // Data Timeout Error Interrupt Enable
#define SDHC_IRQSIGEN_CIEIEN    MAKE_REG_MASK(0x1,19) //((uint32_t)0x00080000)    // Command Index Error Interrupt Enable
#define SDHC_IRQSIGEN_CEBEIEN   MAKE_REG_MASK(0x1,18) //((uint32_t)0x00040000)    // Command End Bit Error Interrupt Enable
#define SDHC_IRQSIGEN_CCEIEN    MAKE_REG_MASK(0x1,17) //((uint32_t)0x00020000)    // Command CRC Error Interrupt Enable
#define SDHC_IRQSIGEN_CTOEIEN   MAKE_REG_MASK(0x1,16) //((uint32_t)0x00010000)    // Command Timeout Error Interrupt Enable
#define SDHC_IRQSIGEN_TPIEN     MAKE_REG_MASK(0x1,14) //
#define SDHC_IRQSIGEN_RTEIEN    MAKE_REG_MASK(0x1,12) //
#define SDHC_IRQSIGEN_CINTIEN   MAKE_REG_MASK(0x1,8)  //((uint32_t)0x00000100)    // Card Interrupt Interrupt Enable
#define SDHC_IRQSIGEN_CRMIEN    MAKE_REG_MASK(0x1,7)  //((uint32_t)0x00000080)    // Card Removal Interrupt Enable
#define SDHC_IRQSIGEN_CINSIEN   MAKE_REG_MASK(0x1,6)  //((uint32_t)0x00000040)    // Card Insertion Interrupt Enable
#define SDHC_IRQSIGEN_BRRIEN    MAKE_REG_MASK(0x1,5)  //((uint32_t)0x00000020)    // Buffer Read Ready Interrupt Enable
#define SDHC_IRQSIGEN_BWRIEN    MAKE_REG_MASK(0x1,4)  //((uint32_t)0x00000010)    // Buffer Write Ready Interrupt Enable
#define SDHC_IRQSIGEN_DINTIEN   MAKE_REG_MASK(0x1,3)  //((uint32_t)0x00000008)    // DMA Interrupt Interrupt Enable
#define SDHC_IRQSIGEN_BGEIEN    MAKE_REG_MASK(0x1,2)  //((uint32_t)0x00000004)    // Block Gap Event Interrupt Enable
#define SDHC_IRQSIGEN_TCIEN     MAKE_REG_MASK(0x1,1)  //((uint32_t)0x00000002)    // Transfer Complete Interrupt Enable
#define SDHC_IRQSIGEN_CCIEN     MAKE_REG_MASK(0x1,0)  //((uint32_t)0x00000001)    // Command Complete Interrupt Enable

#define SDHC_AC12ERR_SMPLCLK_SEL  MAKE_REG_MASK(0x1,23) //
#define SDHC_AC12ERR_EXEC_TUNING  MAKE_REG_MASK(0x1,22) //
#define SDHC_AC12ERR_CNIBAC12E    MAKE_REG_MASK(0x1,7)  //((uint32_t)0x00000080)    // Command Not Issued By Auto CMD12 Error
#define SDHC_AC12ERR_AC12IE     MAKE_REG_MASK(0x1,4)  //((uint32_t)0x00000010)    // Auto CMD12 Index Error
#define SDHC_AC12ERR_AC12CE     MAKE_REG_MASK(0x1,3)  //((uint32_t)0x00000008)    // Auto CMD12 CRC Error
#define SDHC_AC12ERR_AC12EBE    MAKE_REG_MASK(0x1,2)  //((uint32_t)0x00000004)    // Auto CMD12 End Bit Error
#define SDHC_AC12ERR_AC12TOE    MAKE_REG_MASK(0x1,1)  //((uint32_t)0x00000002)    // Auto CMD12 Timeout Error
#define SDHC_AC12ERR_AC12NE     MAKE_REG_MASK(0x1,0)  //((uint32_t)0x00000001)    // Auto CMD12 Not Executed

#define SDHC_HTCAPBLT_VS18      MAKE_REG_MASK(0x1,26) //
#define SDHC_HTCAPBLT_VS30      MAKE_REG_MASK(0x1,25) //
#define SDHC_HTCAPBLT_VS33      MAKE_REG_MASK(0x1,24) //
#define SDHC_HTCAPBLT_SRS     MAKE_REG_MASK(0x1,23) //
#define SDHC_HTCAPBLT_DMAS      MAKE_REG_MASK(0x1,22) //
#define SDHC_HTCAPBLT_HSS     MAKE_REG_MASK(0x1,21) //
#define SDHC_HTCAPBLT_ADMAS     MAKE_REG_MASK(0x1,20) //
#define SDHC_HTCAPBLT_MBL_VAL   MAKE_REG_GET((USDHC1_HOST_CTRL_CAP),0x7,16) //
#define SDHC_HTCAPBLT_RETUN_MODE  MAKE_REG_GET((USDHC1_HOST_CTRL_CAP),0x3,14) //
#define SDHC_HTCAPBLT_TUNE_SDR50  MAKE_REG_MASK(0x1,13) //
#define SDHC_HTCAPBLT_TIME_RETUN(n) MAKE_REG_SET(n,0xF,8) //

#define SDHC_WML_WR_BRSTLEN_MASK    MAKE_REG_MASK(0x1F,24)  //
#define SDHC_WML_RD_BRSTLEN_MASK    MAKE_REG_MASK(0x1F,8)   //
#define SDHC_WML_WR_WML_MASK      MAKE_REG_MASK(0xFF,16)  //
#define SDHC_WML_RD_WML_MASK      MAKE_REG_MASK(0xFF,0)   //
#define SDHC_WML_WR_BRSTLEN(n)    MAKE_REG_SET(n,0x1F,24) //(uint32_t)(((n) & 0x7F)<<16)  // Write Burst Len
#define SDHC_WML_RD_BRSTLEN(n)    MAKE_REG_SET(n,0x1F,8)  //(uint32_t)(((n) & 0x7F)<<0)   // Read Burst Len
#define SDHC_WML_WR_WML(n)      MAKE_REG_SET(n,0xFF,16) //(uint32_t)(((n) & 0x7F)<<16)  // Write Watermark Level
#define SDHC_WML_RD_WML(n)      MAKE_REG_SET(n,0xFF,0)  //(uint32_t)(((n) & 0x7F)<<0)   // Read Watermark Level
#define SDHC_WML_WRWML(n)     MAKE_REG_SET(n,0xFF,16) //(uint32_t)(((n) & 0x7F)<<16)  // Write Watermark Level
#define SDHC_WML_RDWML(n)     MAKE_REG_SET(n,0xFF,0)  //(uint32_t)(((n) & 0x7F)<<0)   // Read Watermark Level

// Teensy 4.0 only
#define SDHC_MIX_CTRL_DMAEN     MAKE_REG_MASK(0x1,0)  //
#define SDHC_MIX_CTRL_BCEN      MAKE_REG_MASK(0x1,1)  //
#define SDHC_MIX_CTRL_AC12EN    MAKE_REG_MASK(0x1,2)  //
#define SDHC_MIX_CTRL_DDR_EN    MAKE_REG_MASK(0x1,3)  //
#define SDHC_MIX_CTRL_DTDSEL    MAKE_REG_MASK(0x1,4)  //
#define SDHC_MIX_CTRL_MSBSEL    MAKE_REG_MASK(0x1,5)  //
#define SDHC_MIX_CTRL_NIBBLE_POS  MAKE_REG_MASK(0x1,6)  //
#define SDHC_MIX_CTRL_AC23EN    MAKE_REG_MASK(0x1,7)  //

#define SDHC_FEVT_CINT        MAKE_REG_MASK(0x1,31) //((uint32_t)0x80000000)    // Force Event Card Interrupt
#define SDHC_FEVT_DMAE        MAKE_REG_MASK(0x1,28) //((uint32_t)0x10000000)    // Force Event DMA Error
#define SDHC_FEVT_AC12E       MAKE_REG_MASK(0x1,24) //((uint32_t)0x01000000)    // Force Event Auto CMD12 Error
#define SDHC_FEVT_DEBE        MAKE_REG_MASK(0x1,22) //((uint32_t)0x00400000)    // Force Event Data End Bit Error
#define SDHC_FEVT_DCE       MAKE_REG_MASK(0x1,21) //((uint32_t)0x00200000)    // Force Event Data CRC Error
#define SDHC_FEVT_DTOE        MAKE_REG_MASK(0x1,20) //((uint32_t)0x00100000)    // Force Event Data Timeout Error
#define SDHC_FEVT_CIE       MAKE_REG_MASK(0x1,19) //((uint32_t)0x00080000)    // Force Event Command Index Error
#define SDHC_FEVT_CEBE        MAKE_REG_MASK(0x1,18) //((uint32_t)0x00040000)    // Force Event Command End Bit Error
#define SDHC_FEVT_CCE       MAKE_REG_MASK(0x1,17) //((uint32_t)0x00020000)    // Force Event Command CRC Error
#define SDHC_FEVT_CTOE        MAKE_REG_MASK(0x1,16) //((uint32_t)0x00010000)    // Force Event Command Timeout Error
#define SDHC_FEVT_CNIBAC12E     MAKE_REG_MASK(0x1,7)  //((uint32_t)0x00000080)    // Force Event Command Not Executed By Auto Command 12 Error
#define SDHC_FEVT_AC12IE      MAKE_REG_MASK(0x1,4)  //((uint32_t)0x00000010)    // Force Event Auto Command 12 Index Error
#define SDHC_FEVT_AC12EBE     MAKE_REG_MASK(0x1,3)  //((uint32_t)0x00000008)    // Force Event Auto Command 12 End Bit Error
#define SDHC_FEVT_AC12CE      MAKE_REG_MASK(0x1,2)  //((uint32_t)0x00000004)    // Force Event Auto Command 12 CRC Error
#define SDHC_FEVT_AC12TOE     MAKE_REG_MASK(0x1,1)  //((uint32_t)0x00000002)    // Force Event Auto Command 12 Time Out Error
#define SDHC_FEVT_AC12NE      MAKE_REG_MASK(0x1,0)  //((uint32_t)0x00000001)    // Force Event Auto Command 12 Not Executed

#define SDHC_ADMAES_ADMADCE     MAKE_REG_MASK(0x1,3)  //((uint32_t)0x00000008)
#define SDHC_ADMAES_ADMALME     MAKE_REG_MASK(0x1,2)  //((uint32_t)0x00000004)
#define SDHC_ADMAES_ADMAES_MASK   MAKE_REG_MASK(0x3,0)  //((uint32_t)0x00000003)

#define SDHC_MMCBOOT_BOOTBLKCNT(n)  MAKE_REG_MASK(0xFF,16)  //(uint32_t)(((n) & 0xFFF)<<16) // stop at block gap value of automatic mode
#define SDHC_MMCBOOT_AUTOSABGEN   MAKE_REG_MASK(0x1,7)  //((uint32_t)0x00000080)    // enable auto stop at block gap function
#define SDHC_MMCBOOT_BOOTEN     MAKE_REG_MASK(0x1,6)  //((uint32_t)0x00000040)    // Boot Mode Enable
#define SDHC_MMCBOOT_BOOTMODE   MAKE_REG_MASK(0x1,5)  //((uint32_t)0x00000020)    // Boot Mode Select
#define SDHC_MMCBOOT_BOOTACK    MAKE_REG_MASK(0x1,4)  //((uint32_t)0x00000010)    // Boot Ack Mode Select
#define SDHC_MMCBOOT_DTOCVACK(n)  MAKE_REG_MASK(0xF,0)  //(uint32_t)(((n) & 0xF)<<0)  // Boot ACK Time Out Counter Value
//#define SDHC_HOSTVER    (*(volatile uint32_t *)0x400B10FC) // Host Controller Version

#define CCM_ANALOG_PFD_528_PFD0_FRAC_MASK 0x3f
#define CCM_ANALOG_PFD_528_PFD0_FRAC(n) ((n) & CCM_ANALOG_PFD_528_PFD0_FRAC_MASK)
#define CCM_ANALOG_PFD_528_PFD1_FRAC_MASK (0x3f<<8)
#define CCM_ANALOG_PFD_528_PFD1_FRAC(n) (((n)<<8) & CCM_ANALOG_PFD_528_PFD1_FRAC_MASK)
#define CCM_ANALOG_PFD_528_PFD2_FRAC_MASK (0x3f<<16)
#define CCM_ANALOG_PFD_528_PFD2_FRAC(n) (((n)<<16) & CCM_ANALOG_PFD_528_PFD2_FRAC_MASK)
#define CCM_ANALOG_PFD_528_PFD3_FRAC_MASK ((0x3f<<24)
#define CCM_ANALOG_PFD_528_PFD3_FRAC(n) (((n)<<24) & CCM_ANALOG_PFD_528_PFD3_FRAC_MASK)

#define SDHC_DSADDR       (USDHC1_DS_ADDR ) // DMA System Address register
#define SDHC_BLKATTR      (USDHC1_BLK_ATT) // Block Attributes register
#define SDHC_CMDARG       (USDHC1_CMD_ARG) // Command Argument register
#define SDHC_XFERTYP      (USDHC1_CMD_XFR_TYP) // Transfer Type register
#define SDHC_CMDRSP0      (USDHC1_CMD_RSP0) // Command Response 0
#define SDHC_CMDRSP1      (USDHC1_CMD_RSP1) // Command Response 1
#define SDHC_CMDRSP2      (USDHC1_CMD_RSP2) // Command Response 2
#define SDHC_CMDRSP3      (USDHC1_CMD_RSP3) // Command Response 3
#define SDHC_DATPORT      (USDHC1_DATA_BUFF_ACC_PORT) // Buffer Data Port register
#define SDHC_PRSSTAT      (USDHC1_PRES_STATE) // Present State register
#define SDHC_PROCTL       (USDHC1_PROT_CTRL) // Protocol Control register
#define SDHC_SYSCTL       (USDHC1_SYS_CTRL) // System Control register
#define SDHC_IRQSTAT      (USDHC1_INT_STATUS) // Interrupt Status register
#define SDHC_IRQSTATEN      (USDHC1_INT_STATUS_EN) // Interrupt Status Enable register
#define SDHC_IRQSIGEN     (USDHC1_INT_SIGNAL_EN) // Interrupt Signal Enable register
#define SDHC_AC12ERR      (USDHC1_AUTOCMD12_ERR_STATUS) // Auto CMD12 Error Status Register
#define SDHC_HTCAPBLT     (USDHC1_HOST_CTRL_CAP) // Host Controller Capabilities
#define SDHC_WML        (USDHC1_WTMK_LVL) // Watermark Level Register
#define SDHC_MIX_CTRL     (USDHC1_MIX_CTRL) // Mixer Control
#define SDHC_FEVT       (USDHC1_FORCE_EVENT) // Force Event register
#define SDHC_ADMAES       (USDHC1_ADMA_ERR_STATUS) // ADMA Error Status register
#define SDHC_ADSADDR      (USDHC1_ADMA_SYS_ADDR) // ADMA System Addressregister
#define SDHC_VENDOR       (USDHC1_VEND_SPEC) // Vendor Specific register
#define SDHC_MMCBOOT      (USDHC1_MMC_BOOT) // MMC Boot register
#define SDHC_VENDOR2    (USDHC2_VEND_SPEC2) // Vendor Specific2 register
//
//#define IRQ_SDHC    IRQ_SDHC1

#define SDHC_MAX_DVS (0xF + 1U)
#define SDHC_MAX_CLKFS (0xFF + 1U)
#define SDHC_PREV_DVS(x) ((x) -= 1U)
#define SDHC_PREV_CLKFS(x, y) ((x) >>= (y))

#define CCM_CSCDR1_USDHC1_CLK_PODF_MASK (0x7<<11)
#define CCM_CSCDR1_USDHC1_CLK_PODF(n) (((n)&0x7)<<11)

#define IOMUXC_SW_PAD_CTL_PAD_SRE     ((0x1<)<0)
#define IOMUXC_SW_PAD_CTL_PAD_PKE     ((0x1)<<12)
#define IOMUXC_SW_PAD_CTL_PAD_PUE       ((0x1)<<13)
#define IOMUXC_SW_PAD_CTL_PAD_HYS       ((0x1)<<16)
#define IOMUXC_SW_PAD_CTL_PAD_SPEED(n)  (((n)&0x3)<<6)
#define IOMUXC_SW_PAD_CTL_PAD_PUS(n)    (((n)&0x3)<<14)
#define IOMUXC_SW_PAD_CTL_PAD_PUS_MASK  ((0x3)<<14)
#define IOMUXC_SW_PAD_CTL_PAD_DSE(n)    (((n)&0x7)<<3)
#define IOMUXC_SW_PAD_CTL_PAD_DSE_MASK  ((0x7)<<3)

#endif // __IMXRT1062__

#define SDHC_IRQSIGEN_DMA_MASK (SDHC_IRQSIGEN_TCIEN | SDHC_IRQSIGEN_DINTIEN | SDHC_IRQSIGEN_DMAEIEN)
#define CARD_STATUS_READY_FOR_DATA  (1UL << 8)

/******************************************************************************
  Types
******************************************************************************/
typedef struct {
  uint8_t  status;
  uint8_t  highCapacity;
  uint8_t  version2;
  uint8_t  tranSpeed;
  uint32_t address;
  uint32_t numBlocks;
  uint32_t lastCardStatus;
} SD_CARD_DESCRIPTOR;

/******************************************************************************
  Global functions
******************************************************************************/

/******************************************************************************
  Private variables
******************************************************************************/

static SD_CARD_DESCRIPTOR sdCardDesc;
static volatile uint32_t dmaDone=0;
//

/******************************************************************************
  Forward declaration of private functions
******************************************************************************/
static void sdhc_setSdclk(uint32_t kHzMax);

static uint8_t SDHC_Init(void);
static void SDHC_InitGPIO(void);
static void SDHC_ReleaseGPIO(void);
//static void SDHC_SetClock(uint32_t sysctl);
static uint32_t SDHC_WaitStatus(uint32_t mask);
static int SDHC_ReadBlock(uint32_t* pData);
static int SDHC_WriteBlock(const uint32_t* pData);
static int SDHC_CMD_Do(uint32_t xfertyp);
static int SDHC_CMD0_GoToIdle(void);
static int SDHC_CMD2_Identify(void);
static int SDHC_CMD3_GetAddress(void);
static int SDHC_ACMD6_SetBusWidth(uint32_t address, uint32_t width);
static int SDHC_CMD7_SelectCard(uint32_t address);
static int SDHC_CMD8_SetInterface(uint32_t cond);
static int SDHC_CMD9_GetParameters(uint32_t address);
static int SDHC_CMD12_StopTransfer(void);
static int SDHC_CMD12_StopTransferWaitForBusy(void);
static int SDHC_CMD16_SetBlockSize(uint32_t block_size);
static int SDHC_CMD17_ReadBlock(uint32_t sector);
static int SDHC_CMD24_WriteBlock(uint32_t sector);
static int SDHC_ACMD41_SendOperationCond(uint32_t cond);

/******************************************************************************

    Public functions

******************************************************************************/
uint8_t SDHC_CardGetType(void)
{
  if (sdCardDesc.status) return 0;
  if (sdCardDesc.version2 == 0) return 1; // SD_CARD_TYPE_SD1
  if (sdCardDesc.highCapacity == 0) return 2; // SD_CARD_TYPE_SD2
  return 3; // SD_CARD_TYPE_SDHC
}

//-----------------------------------------------------------------------------
// initialize the SDHC Controller and SD Card
// returns status of initialization(OK, nonInit, noCard, CardProtected)
uint8_t SDHC_CardInit(void)
{
  uint8_t resS;
  int resR;

  resS = SDHC_Init();

  sdCardDesc.status = resS;
  sdCardDesc.address = 0;
  sdCardDesc.highCapacity = 0;
  sdCardDesc.version2 = 0;
  sdCardDesc.numBlocks = 0;

  if (resS)
    return resS;

  SDHC_IRQSIGEN = 0;

  resR = SDHC_CMD0_GoToIdle();
  if (resR) {
    sdCardDesc.status = SDHC_STATUS_NOINIT;
    return SDHC_STATUS_NOINIT;
  }

  resR = SDHC_CMD8_SetInterface(0x000001AA); // 3.3V and AA check pattern
  if (resR == SDHC_RESULT_OK) {
      if (SDHC_CMDRSP0 != 0x000001AA) {
        sdCardDesc.status = SDHC_STATUS_NOINIT;
        return SDHC_STATUS_NOINIT;
      }
      sdCardDesc.highCapacity = 1;
  } else if (resR == SDHC_RESULT_NO_RESPONSE) {
      // version 1 cards do not respond to CMD8
  } else {
    sdCardDesc.status = SDHC_STATUS_NOINIT;
    return SDHC_STATUS_NOINIT;
  }

  if (SDHC_ACMD41_SendOperationCond(0))  return sdCardDesc.status = SDHC_STATUS_NOINIT;

  if (SDHC_CMDRSP0 & 0x300000) {
    uint32_t condition = 0x00300000;
    if (sdCardDesc.highCapacity) condition |= 0x40000000;
    //
    uint32_t ii = 0;
    do {
      ii++;
      if (SDHC_ACMD41_SendOperationCond(condition)) {
        resS = SDHC_STATUS_NOINIT;
        break;
      }
    } while ((!(SDHC_CMDRSP0 & 0x80000000)) && (ii < SDHC_INITIALIZATION_MAX_CNT));

    if (resS) return resS;

    if ((ii >= SDHC_INITIALIZATION_MAX_CNT) || (!(SDHC_CMDRSP0 & 0x40000000)))
      sdCardDesc.highCapacity = 0;
  }

  // Card identify
  if (SDHC_CMD2_Identify())  return sdCardDesc.status = SDHC_STATUS_NOINIT;

  // Get card address
  if (SDHC_CMD3_GetAddress())  return sdCardDesc.status = SDHC_STATUS_NOINIT;

  sdCardDesc.address = SDHC_CMDRSP0 & 0xFFFF0000;

  // Get card parameters
  if (SDHC_CMD9_GetParameters(sdCardDesc.address))  return sdCardDesc.status = SDHC_STATUS_NOINIT;

  if (!(SDHC_CMDRSP3 & 0x00C00000)) {
    uint32_t read_bl_len, c_size, c_size_mult;

    read_bl_len = (SDHC_CMDRSP2 >> 8) & 0x0F;
    c_size = SDHC_CMDRSP2 & 0x03;
    c_size = (c_size << 10) | (SDHC_CMDRSP1 >> 22);
    c_size_mult = (SDHC_CMDRSP1 >> 7) & 0x07;
    sdCardDesc.numBlocks = (c_size + 1) * (1 << (c_size_mult + 2)) * (1 << (read_bl_len - 9));
  } else {
    uint32_t c_size;
    sdCardDesc.version2 = 1;
    c_size = (SDHC_CMDRSP1 >> 8) & 0x003FFFFF;
    sdCardDesc.numBlocks = (c_size + 1) << 10;
  }

  // Select card
  if (SDHC_CMD7_SelectCard(sdCardDesc.address)) return sdCardDesc.status = SDHC_STATUS_NOINIT;

  // Set Block Size to 512
  // Block Size in SDHC Controller is already set to 512 by SDHC_Init();
  // Set 512 Block size in SD card
  if (SDHC_CMD16_SetBlockSize(SDHC_BLOCK_SIZE))  return sdCardDesc.status = SDHC_STATUS_NOINIT;

  // Set 4 bit data bus width
  if (SDHC_ACMD6_SetBusWidth(sdCardDesc.address, 2))  return sdCardDesc.status = SDHC_STATUS_NOINIT;

  // Set Data bus width also in SDHC controller
  SDHC_PROCTL &= ~SDHC_PROCTL_DTW_MASK;
  SDHC_PROCTL |= SDHC_PROCTL_DTW(SDHC_PROCTL_DTW_4BIT);

  // De-Init GPIO
  SDHC_ReleaseGPIO();

  // Set the SDHC default baud rate
  sdhc_setSdclk(25000);
  // SDHC_SetClock(SDHC_SYSCTL_25MHZ);
  // TODO: use CMD6 and CMD9 to detect if card supports 50 MHz
  // then use CMD4 to configure card to high speed mode,
  // and SDHC_SetClock() for 50 MHz config

  // Init GPIO
  SDHC_InitGPIO();

  return sdCardDesc.status;
}


//-----------------------------------------------------------------------------
// FUNCTION:    SDHC_CardReadBlock (disk_read)
// SCOPE:       SDHC public related function
// DESCRIPTION: Function read block to disk
//
// PARAMETERS:  buff - pointer on buffer where read data should be stored
//              sector - index of sector
//
// RETURNS:     result of operation
//-----------------------------------------------------------------------------
#if 1
// read a block from disk, using polling
//   buff - pointer on buffer where read data should be stored
//   sector - index of start sector
int SDHC_CardReadBlock(void * buff, uint32_t sector)
{
  int result;
  uint32_t* pData = (uint32_t*)buff;

  // Check if this is ready
  if (sdCardDesc.status != 0)
     return SDHC_RESULT_NOT_READY;

  // Convert LBA to uint8_t address if needed
  if (!sdCardDesc.highCapacity)
    sector *= 512;

  SDHC_IRQSTAT = 0xffff;
#if defined(__IMXRT1062__)
  SDHC_MIX_CTRL |= SDHC_MIX_CTRL_DTDSEL;
#endif

  // Just single block mode is needed
  result = SDHC_CMD17_ReadBlock(sector);
  if(result != SDHC_RESULT_OK) return result;
  result = SDHC_ReadBlock(pData);

  // finish up
  while (!(SDHC_IRQSTAT & SDHC_IRQSTAT_TC)) {coru_yield();}  // wait for transfer to complete
  SDHC_IRQSTAT = (SDHC_IRQSTAT_TC | SDHC_IRQSTAT_BRR | SDHC_IRQSTAT_AC12E);

  return result;
}
#else
// read a block from disk, using DMA & interrupts
int SDHC_CardReadBlock(void * buff, uint32_t sector)
{
  int result=0;
  uint32_t* pData = (uint32_t*)buff;
  /* check alignment for DMA */
  if (reinterpret_cast<uintptr_t>(static_cast<const void*>(buff)) % 4) {
    return -1;
  }
//  Serial.print("Sector: "); Serial.println(sector); Serial.flush();
  // Check if this is ready
  if (sdCardDesc.status != 0) return SDHC_RESULT_NOT_READY;

  while ((SDHC_PRSSTAT & SDHC_PRSSTAT_CIHB) || (SDHC_PRSSTAT & SDHC_PRSSTAT_CDIHB));

  // Convert LBA to BYTE address if needed
  if (!sdCardDesc.highCapacity)  sector *= 512;

  // clear status
  SDHC_IRQSTAT = SDHC_IRQSTAT;

  // use dma: disabling polling
  uint32_t irqstat = SDHC_IRQSTATEN;
  irqstat &= ~(SDHC_IRQSTATEN_BRRSEN | SDHC_IRQSTATEN_BWRSEN | SDHC_IRQSTATEN_CCSEN) ;
  irqstat &= ~(SDHC_IRQSTATEN_DCESEN | SDHC_IRQSTATEN_CCESEN) ;
  // enable status
  irqstat |= SDHC_IRQSTATEN_DMAESEN | SDHC_IRQSTATEN_DINTSEN | SDHC_IRQSTATEN_TCSEN ;
  SDHC_IRQSTATEN = irqstat;

  uint32_t sigen = SDHC_IRQSIGEN;
  sigen |= SDHC_IRQSIGEN_DMA_MASK ;

  SDHC_SYSCTL |= SDHC_SYSCTL_HCKEN;
  #if defined(__IMXRT1052__) || defined(__IMXRT1062__)
    SDHC_MIX_CTRL |= SDHC_MIX_CTRL_DTDSEL ; // read
    SDHC_MIX_CTRL |=  SDHC_MIX_CTRL_DMAEN ; // DMA
  #endif

  uint32_t xfertyp = SDHC_XFERTYP_CMDINX(SDHC_CMD17) | SDHC_XFERTYP_RSPTYP(SDHC_XFERTYP_RSPTYP_48) | SDHC_XFERTYP_DPSEL
                     | SDHC_XFERTYP_DTDSEL | SDHC_XFERTYP_DMAEN;

  dmaDone=0;
  SDHC_DSADDR  = (uint32_t)buff;
  SDHC_CMDARG = sector;
  SDHC_BLKATTR = SDHC_BLKATTR_BLKCNT(1) | SDHC_BLKATTR_BLKSIZE(512);
  SDHC_IRQSIGEN = sigen;
  SDHC_XFERTYP = xfertyp;
  //
  while(!dmaDone);
  SDHC_IRQSTAT &= (SDHC_IRQSTAT_CC | SDHC_IRQSTAT_TC);

  return result;
}
#endif

//-----------------------------------------------------------------------------
// FUNCTION:    SDHC_CardWriteBlock (disk_write)
// SCOPE:       SDHC public related function
// DESCRIPTION: Function write block to disk
//
// PARAMETERS:  buff - pointer on buffer where is stored data
//              sector - index of sector
//
// RETURNS:     result of operation
//-----------------------------------------------------------------------------
#if 1
int SDHC_CardWriteBlock(const void * buff, uint32_t sector)
{
  int result;
  const uint32_t *pData = (const uint32_t *)buff;

  // Check if this is ready
  if (sdCardDesc.status != 0) return SDHC_RESULT_NOT_READY;

  // Convert LBA to uint8_t address if needed
  if(!sdCardDesc.highCapacity)
    sector *= 512;

  //SDHC_IRQSTAT = 0xffff;
  SDHC_IRQSTAT = SDHC_IRQSTAT;
#if defined(__IMXRT1062__)
	SDHC_MIX_CTRL &= ~SDHC_MIX_CTRL_DTDSEL;
#endif

  // Just single block mode is needed
  result = SDHC_CMD24_WriteBlock(sector);
  if (result != SDHC_RESULT_OK) return result;
  result = SDHC_WriteBlock(pData);

  // finish up
  while (!(SDHC_IRQSTAT & SDHC_IRQSTAT_TC)) {coru_yield();}  // wait for transfer to complete
  SDHC_IRQSTAT = (SDHC_IRQSTAT_TC | SDHC_IRQSTAT_BWR | SDHC_IRQSTAT_AC12E);

  return result;
}
#else
int SDHC_CardWriteBlock(const void * buff, uint32_t sector)
{
  int result=0;
  const uint32_t *pData = (const uint32_t *)buff;
  /* check alignment for DMA */
  if (reinterpret_cast<uintptr_t>(static_cast<const void*>(buff)) % 4) {
    return -1;
  }

  // Check if this is ready
  if (sdCardDesc.status != 0) return SDHC_RESULT_NOT_READY;

  while ((SDHC_PRSSTAT & SDHC_PRSSTAT_CIHB) || (SDHC_PRSSTAT & SDHC_PRSSTAT_CDIHB)) ;

  // Convert LBA to uint8_t address if needed
  if (!sdCardDesc.highCapacity) sector *= 512;

  // clear status
  SDHC_IRQSTAT = SDHC_IRQSTAT;

  uint32_t irqstat = SDHC_IRQSTATEN;
  // use dma: disabling polling
  irqstat &= ~(SDHC_IRQSTATEN_BRRSEN | SDHC_IRQSTATEN_BWRSEN | SDHC_IRQSTATEN_CCSEN) ;
  irqstat &= ~(SDHC_IRQSTATEN_DCESEN | SDHC_IRQSTATEN_CCESEN) ;
  // enable status
  irqstat |= /*SDHC_IRQSTATEN_DCESEN | SDHC_IRQSTATEN_CCESEN | */SDHC_IRQSTATEN_DMAESEN ;
  irqstat |= SDHC_IRQSTATEN_DINTSEN | SDHC_IRQSTATEN_TCSEN ;
  SDHC_IRQSTATEN = irqstat;

  uint32_t sigen = SDHC_IRQSIGEN;
  sigen |= SDHC_IRQSIGEN_DMA_MASK ;

  SDHC_SYSCTL |= SDHC_SYSCTL_HCKEN;

  #if defined(__IMXRT1052__)
    SDHC_MIX_CTRL &= ~ SDHC_MIX_CTRL_DTDSEL;  // write
    SDHC_MIX_CTRL |=  SDHC_MIX_CTRL_DMAEN ;   //DMA
  #endif

  uint32_t xfertyp = SDHC_XFERTYP_CMDINX(SDHC_CMD24) | SDHC_XFERTYP_RSPTYP(SDHC_XFERTYP_RSPTYP_48) | SDHC_XFERTYP_DPSEL
                     | SDHC_XFERTYP_DMAEN;

  dmaDone=0;
  SDHC_DSADDR  = (uint32_t)buff;
  SDHC_CMDARG = sector;
  SDHC_BLKATTR = SDHC_BLKATTR_BLKCNT(1) | SDHC_BLKATTR_BLKSIZE(512);
  SDHC_IRQSIGEN = sigen;
  SDHC_XFERTYP = xfertyp;
  //
  while(!dmaDone);
  SDHC_IRQSTAT &= (SDHC_IRQSTAT_CC | SDHC_IRQSTAT_TC);

  while(SDHC_PRSSTAT & SDHC_PRSSTAT_DLA);

  //check for uSD status
  do
  { while ((SDHC_PRSSTAT & SDHC_PRSSTAT_CIHB) || (SDHC_PRSSTAT & SDHC_PRSSTAT_CDIHB)) ;
    SDHC_IRQSTATEN |= SDHC_IRQSTATEN_CCSEN;
    SDHC_IRQSTAT=SDHC_IRQSTAT;
    // CMD13 to check uSD status
    xfertyp = (SDHC_XFERTYP_CMDINX(SDHC_CMD13) | SDHC_XFERTYP_RSPTYP(SDHC_XFERTYP_RSPTYP_48));
    //
    SDHC_CMDARG = sdCardDesc.address;
    SDHC_XFERTYP = xfertyp;
    while(!(SDHC_IRQSTAT & SDHC_IRQSTAT_CC)); SDHC_IRQSTAT &= SDHC_IRQSTAT_CC;
  } while(SDHC_CMDRSP0 & 0x200); // while data?
//  } while(!(SDHC_CMDRSP0 & CARD_STATUS_READY_FOR_DATA ));

  return result;
}
#endif

/******************************************************************************

    Private functions

******************************************************************************/
#if defined(__MK64FX512__) || defined(__MK66FX1M0__)
  // Teensy 3.5 & 3.6
  // initialize the SDHC Controller signals
  static void SDHC_InitGPIO(void)
  {
    PORTE_PCR0 = PORT_PCR_MUX(4) | PORT_PCR_PS | PORT_PCR_PE | PORT_PCR_DSE; /* SDHC.D1  */
    PORTE_PCR1 = PORT_PCR_MUX(4) | PORT_PCR_PS | PORT_PCR_PE | PORT_PCR_DSE; /* SDHC.D0  */
    PORTE_PCR2 = PORT_PCR_MUX(4) | PORT_PCR_DSE;                             /* SDHC.CLK */
    PORTE_PCR3 = PORT_PCR_MUX(4) | PORT_PCR_PS | PORT_PCR_PE | PORT_PCR_DSE; /* SDHC.CMD */
    PORTE_PCR4 = PORT_PCR_MUX(4) | PORT_PCR_PS | PORT_PCR_PE | PORT_PCR_DSE; /* SDHC.D3  */
    PORTE_PCR5 = PORT_PCR_MUX(4) | PORT_PCR_PS | PORT_PCR_PE | PORT_PCR_DSE; /* SDHC.D2  */
  }

  // release the SDHC Controller signals
  static void SDHC_ReleaseGPIO(void)
  {
    PORTE_PCR0 = PORT_PCR_MUX(1) | PORT_PCR_PE | PORT_PCR_PS;   /* PULLUP SDHC.D1  */
    PORTE_PCR1 = PORT_PCR_MUX(1) | PORT_PCR_PE | PORT_PCR_PS;   /* PULLUP SDHC.D0  */
    PORTE_PCR2 = 0;           /* SDHC.CLK */
    PORTE_PCR3 = PORT_PCR_MUX(1) | PORT_PCR_PE | PORT_PCR_PS;   /* PULLUP SDHC.CMD */
    PORTE_PCR4 = PORT_PCR_MUX(1) | PORT_PCR_PE | PORT_PCR_PS; /* PULLUP SDHC.D3  */
    PORTE_PCR5 = PORT_PCR_MUX(1) | PORT_PCR_PE | PORT_PCR_PS;   /* PULLUP SDHC.D2  */
  }

  void initClock()
  {
  #ifdef HAS_KINETIS_MPU
    // Allow SDHC Bus Master access.
    MPU_RGDAAC0 |= 0x0C000000;
  #endif
    // Enable SDHC clock.
    SIM_SCGC3 |= SIM_SCGC3_SDHC;
  }

  uint32_t sdhcClock()
  { return F_CPU;
  }

#else
  // Teensy 4.0
  static void SDHC_InitGPIO(void)
  {
      IOMUXC_SW_MUX_CTL_PAD_GPIO_SD_B0_04 = 0; //DAT2  
      IOMUXC_SW_MUX_CTL_PAD_GPIO_SD_B0_05 = 0; //DAT3  
      IOMUXC_SW_MUX_CTL_PAD_GPIO_SD_B0_00 = 0; //CMD   
      //3.3V                                           
      IOMUXC_SW_MUX_CTL_PAD_GPIO_SD_B0_01 = 0; //CLK   
      //GND                                           
      IOMUXC_SW_MUX_CTL_PAD_GPIO_SD_B0_02 = 0; //DAT0 
      IOMUXC_SW_MUX_CTL_PAD_GPIO_SD_B0_03 = 0; //DAT1 
  
      const uint32_t CLOCK_MASK = IOMUXC_SW_PAD_CTL_PAD_PKE |
                                  IOMUXC_SW_PAD_CTL_PAD_DSE(1) |
                                  IOMUXC_SW_PAD_CTL_PAD_SPEED(2);
  
      const uint32_t DATA_MASK = CLOCK_MASK |
                                 (IOMUXC_SW_PAD_CTL_PAD_PUE | IOMUXC_SW_PAD_CTL_PAD_PUS(1));
  
      IOMUXC_SW_PAD_CTL_PAD_GPIO_SD_B0_04 = DATA_MASK;
      IOMUXC_SW_PAD_CTL_PAD_GPIO_SD_B0_05 = DATA_MASK;
      IOMUXC_SW_PAD_CTL_PAD_GPIO_SD_B0_00 = DATA_MASK;
      IOMUXC_SW_PAD_CTL_PAD_GPIO_SD_B0_01 = CLOCK_MASK;
      IOMUXC_SW_PAD_CTL_PAD_GPIO_SD_B0_02 = DATA_MASK;
      IOMUXC_SW_PAD_CTL_PAD_GPIO_SD_B0_03 = DATA_MASK;
  }
  
  static void SDHC_ReleaseGPIO(void)
  {
    IOMUXC_SW_MUX_CTL_PAD_GPIO_SD_B0_04 = 5; //GPIO3_IO16
    IOMUXC_SW_MUX_CTL_PAD_GPIO_SD_B0_05 = 5; //GPIO3_IO17
    IOMUXC_SW_MUX_CTL_PAD_GPIO_SD_B0_00 = 5; //GPIO3_IO12
    //3.3V
    IOMUXC_SW_MUX_CTL_PAD_GPIO_SD_B0_01 = 5; //GPIO3_IO13
    //GND
    IOMUXC_SW_MUX_CTL_PAD_GPIO_SD_B0_02 = 5; //GPIO3_IO14
    IOMUXC_SW_MUX_CTL_PAD_GPIO_SD_B0_03 = 5; //GPIO3_IO15
  }
  
  void initClock()
  {
    /* set PDF_528 PLL2PFD0 */
    CCM_ANALOG_PFD_528 |= (1 << 7);
    CCM_ANALOG_PFD_528 &= ~(0x3F << 0);
    CCM_ANALOG_PFD_528 |= ((24) & 0x3F << 0); // 12 - 35
    CCM_ANALOG_PFD_528 &= ~(1 << 7);
  
    /* Enable USDHC clock. */
    CCM_CCGR6 |= CCM_CCGR6_USDHC1(CCM_CCGR_ON);
    CCM_CSCDR1 &= ~(CCM_CSCDR1_USDHC1_CLK_PODF_MASK);
    //
    //  CCM_CSCMR1 &= ~(CCM_CSCMR1_USDHC1_CLK_SEL);     // PLL2PFD2
    CCM_CSCMR1 |= CCM_CSCMR1_USDHC1_CLK_SEL;          // PLL2PFD0
    CCM_CSCDR1 |= CCM_CSCDR1_USDHC1_CLK_PODF((7)); // &0x7
  
    // for testing
    //CCM_CCOSR = CCM_CCOSR_CLKO1_EN | CCM_CCOSR_CLKO1_DIV(7) | CCM_CCOSR_CLKO1_SEL(1); //(1: SYS_PLL/2)
    //IOMUXC_SW_MUX_CTL_PAD_GPIO_SD_B0_04 = 6; //CCM_CLKO1 (0 is USDHC1_DAT2)
    // for testing
    //CCM_CCOSR |= (CCM_CCOSR_CLKO2_EN | CCM_CCOSR_CLKO2_DIV(7) | CCM_CCOSR_CLKO2_SEL(3)); //(3: usdhc1_clk_root))
    //IOMUXC_SW_MUX_CTL_PAD_GPIO_SD_B0_05 = 6; //CCM_CLKO2 (0 is USDHC1_DAT3)
  }
  
  uint32_t sdhcClock()
  {
    uint32_t divider = ((CCM_CSCDR1 >> 11) & 0x7) + 1;
    uint32_t PLL2PFD0 = (528000000U * 3) / ((CCM_ANALOG_PFD_528 & 0x3F) / 6) / divider;
    return PLL2PFD0;
  }

#endif
/* //may be useful for debugging
static void printRegs()
{
// Serial.print("DS_ADDR: "); Serial.println(SDHC_DSADDR,HEX);        // DMA System Address register
 Serial.print("BLK_ATT:       "); Serial.println(SDHC_BLKATTR,HEX);       // Block Attributes register
// Serial.print("CMD_ARG: "); Serial.println(SDHC_CMDARG,HEX);        // Command Argument register
// Serial.print("CMD_XFR_TYP: "); Serial.println(SDHC_XFERTYP,HEX);    // Transfer Type register
// Serial.print("CMD_RSP0: "); Serial.println(SDHC_CMDRSP0,HEX);       // Command Response 0
// Serial.print("CMD_RSP1: "); Serial.println(SDHC_CMDRSP1,HEX);       // Command Response 1
// Serial.print("CMD_RSP2: "); Serial.println(SDHC_CMDRSP2,HEX);       // Command Response 2
// Serial.print("CMD_RSP3: "); Serial.println(SDHC_CMDRSP3,HEX);       // Command Response 3
// Serial.print("DATA_BUFF_ACC_POR: "); Serial.println(SDHC_DATPORT,HEX); // Buffer Data Port register
 Serial.print("PRES_STATE:    "); Serial.println(SDHC_PRSSTAT,HEX);     // Present State register
 Serial.print("PROT_CTRL:     "); Serial.println(SDHC_PROCTL,HEX);      // Protocol Control register
 Serial.print("SYS_CTRL:      "); Serial.println(SDHC_SYSCTL,HEX);        // System Control register
 Serial.print("INT_STATUS:    "); Serial.println(SDHC_IRQSTAT,HEX);      // Interrupt Status register
 Serial.print("INT_STATUS_EN: "); Serial.println(SDHC_IRQSTATEN,HEX);    // Interrupt Status Enable register
 Serial.print("INT_SIGNAL_EN: "); Serial.println(SDHC_IRQSIGEN,HEX);      // Interrupt Signal Enable register
// Serial.print("AUTOCMD12_ERR_STATUS: "); Serial.println(SDHC_AC12ERR,HEX); // Auto CMD12 Error Status Register
 Serial.print("HOST_CTRL_CAP: "); Serial.println(SDHC_HTCAPBLT,HEX);  // Host Controller Capabilities
 Serial.print("WTMK_LVL:      "); Serial.println(SDHC_WML,HEX);        // Watermark Level Register
 #if defined(__IMXRT1052__)
 Serial.print("MIX_CTRL:      "); Serial.println(SDHC_MIX_CTRL,HEX);     // Mixer Control
 #endif
// Serial.print("FORCE_EVENT:   "); Serial.println(SDHC_FEVT,HEX);      // Force Event register
// Serial.print("ADMA_ERR_STATUS: "); Serial.println(SDHC_ADMAES,HEX); // ADMA Error Status register
// Serial.print("ADMA_SYS_ADDR: "); Serial.println(SDHC_ADSADDR,HEX);  // ADMA System Addressregister
 Serial.print("VEND_SPEC:     "); Serial.println(SDHC_VENDOR,HEX);       // Vendor Specific register
// Serial.print("MMC_BOOT:      "); Serial.println(SDHC_MMCBOOT,HEX);      // MMC Boot register
 #if defined(__IMXRT1052__)
 Serial.print("VEND_SPEC2:    "); Serial.println(SDHC_VENDOR2,HEX);    // Vendor Specific2 register
 #endif
}
*/

static void sdhc_setSdclk(uint32_t kHzMax) {
  const uint32_t DVS_LIMIT = 0X10;
  const uint32_t SDCLKFS_LIMIT = 0X100;
  uint32_t dvs = 1;
  uint32_t sdclkfs = 1;
  uint32_t maxSdclk = 1000 * kHzMax;

  //  uint32_t f_pll = F_CPU;
  uint32_t f_pll = sdhcClock();

  while ((f_pll / (sdclkfs * DVS_LIMIT) > maxSdclk) && (sdclkfs < SDCLKFS_LIMIT)) {
    sdclkfs <<= 1;
  }
  while ((f_pll / (sdclkfs * dvs) > maxSdclk) && (dvs < DVS_LIMIT)) {
    dvs++;
  }
  //uint32_t m_sdClkKhz = f_pll / (1000 * sdclkfs * dvs);

  sdclkfs >>= 1;
  dvs--;

#if defined(__MK64FX512__) || defined(__MK66FX1M0__)
  // Disable SDHC clock.
  SDHC_SYSCTL &= ~SDHC_SYSCTL_SDCLKEN;
#endif

  // Change dividers.
  uint32_t sysctl = SDHC_SYSCTL & ~(SDHC_SYSCTL_DTOCV_MASK
                                    | SDHC_SYSCTL_DVS_MASK | SDHC_SYSCTL_SDCLKFS_MASK);

  SDHC_SYSCTL = sysctl | SDHC_SYSCTL_DTOCV(0x0E) | SDHC_SYSCTL_DVS(dvs)
                | SDHC_SYSCTL_SDCLKFS(sdclkfs);

  // Wait until the SDHC clock is stable.
  while (!(SDHC_PRSSTAT & SDHC_PRSSTAT_SDSTB)) { }

#if defined(__MK64FX512__) || defined(__MK66FX1M0__)
  // Enable the SDHC clock.
  SDHC_SYSCTL |= SDHC_SYSCTL_SDCLKEN;
#endif

//  Serial.printf("setSdclk: %d %d : %x %x\n\r", f_pll, m_sdClkKhz, sdclkfs, dvs);
}

#if 0
void sdhc_isr(void)
{ SDHC_IRQSIGEN &= ~SDHC_IRQSIGEN_DMA_MASK;
  //
//  Serial.print("IRQ1: "); Serial.println(SDHC_IRQSTAT,HEX);
  while(!(SDHC_IRQSTAT & SDHC_IRQSTAT_TC));//  SDHC_IRQSTAT &= ~SDHC_IRQSTAT_TC;

  #if defined(__IMXRT1052__)
    SDHC_MIX_CTRL &= ~(SDHC_MIX_CTRL_AC23EN | SDHC_MIX_CTRL_DMAEN) ;  
  #endif
  // for T3.6, seems not to hurt for T4  
  if(SDHC_SYSCTL & SDHC_SYSCTL_HCKEN) SDHC_SYSCTL &=  ~SDHC_SYSCTL_HCKEN;
  SDHC_PROCTL &= ~SDHC_PROCTL_D3CD; SDHC_PROCTL |=  SDHC_PROCTL_D3CD;

  dmaDone=1;
}
#endif

// initialize the SDHC Controller
// returns status of initialization(OK, nonInit, noCard, CardProtected)
static uint8_t SDHC_Init(void)
{
  initClock();

  // De-init GPIO - to prevent unwanted clocks on bus
  SDHC_ReleaseGPIO();

  #if defined (__IMXRT1062__)
    //SDHC_SYSCTL   |= 0xF;
    SDHC_MIX_CTRL = 0x80000000;
  #endif

  /* Reset SDHC */
  SDHC_SYSCTL |= SDHC_SYSCTL_RSTA | SDHC_SYSCTL_SDCLKFS(0x80);
  while (SDHC_SYSCTL & SDHC_SYSCTL_RSTA) ; // wait

  /* Set the SDHC initial baud rate divider and start */
  sdhc_setSdclk(400);

  /* Poll inhibit bits */
  while (SDHC_PRSSTAT & (SDHC_PRSSTAT_CIHB | SDHC_PRSSTAT_CDIHB)) ;

  /* Init GPIO again */
  SDHC_InitGPIO();

  /* Initial values */ // to do - Check values
  SDHC_BLKATTR = SDHC_BLKATTR_BLKCNT(1) | SDHC_BLKATTR_BLKSIZE(512);
  //SDHC_PROCTL &= ~SDHC_PROCTL_DMAS(3); // clear ADMA
  //SDHC_PROCTL |=  SDHC_PROCTL_D3CD;
  //SDHC_PROCTL = SDHC_PROCTL_EMODE(SDHC_PROCTL_EMODE_INVARIANT) | SDHC_PROCTL_D3CD;
  SDHC_PROCTL = (SDHC_PROCTL & ~(SDHC_PROCTL_EMODE(3)))
	| (SDHC_PROCTL_EMODE(SDHC_PROCTL_EMODE_INVARIANT) | SDHC_PROCTL_D3CD );
  //SDHC_WML = SDHC_WML_RDWML(SDHC_FIFO_BUFFER_SIZE) | SDHC_WML_WRWML(SDHC_FIFO_BUFFER_SIZE);
  //Serial.printf("SDHC_WML = %08X\n", SDHC_WML); // prints 08100810 (good)
  //#if defined(__IMXRT1062__)
    //SDHC_VENDOR = 0x2000F801; // (1<<29 | 0x1F<<11 | 1);
    //SDHC_VENDOR2 &= ~(1<<12); //switch off ACMD23 sharing SDMA
  //#endif

  /* Enable requests */
  // clear interrupt status
  SDHC_IRQSTAT = SDHC_IRQSTAT;

#if 1
  SDHC_IRQSTATEN = SDHC_IRQSTATEN_DMAESEN | SDHC_IRQSTATEN_AC12ESEN | SDHC_IRQSTATEN_DEBESEN |
	SDHC_IRQSTATEN_DCESEN | SDHC_IRQSTATEN_DTOESEN | SDHC_IRQSTATEN_CIESEN |
	SDHC_IRQSTATEN_CEBESEN | SDHC_IRQSTATEN_CCESEN | SDHC_IRQSTATEN_CTOESEN |
	SDHC_IRQSTATEN_BRRSEN | SDHC_IRQSTATEN_BWRSEN | SDHC_IRQSTATEN_DINTSEN |
	SDHC_IRQSTATEN_CRMSEN | SDHC_IRQSTATEN_TCSEN | SDHC_IRQSTATEN_CCSEN;
#else
  SDHC_IRQSTATEN =  //SDHC_IRQSTAT_CRM | SDHC_IRQSTATEN_CIESEN |
                    SDHC_IRQSTATEN_TCSEN | SDHC_IRQSTATEN_CCSEN;

  attachInterruptVector(IRQ_SDHC, sdhc_isr);
  NVIC_SET_PRIORITY(IRQ_SDHC, 6 * 16);
  NVIC_ENABLE_IRQ(IRQ_SDHC);
#endif

  // initial clocks... SD spec says only 74 clocks are needed, but if Teensy rebooted
  // while the card was in middle of an operation, thousands of clock cycles can be
  // needed to get the card to complete a prior command and return to a usable state.
  for (int ii = 0; ii < 1500; ii++) {
    SDHC_SYSCTL |= SDHC_SYSCTL_INITA;
    while (SDHC_SYSCTL & SDHC_SYSCTL_INITA) ;
  }

  // to do - check if this needed
  SDHC_IRQSTAT |= SDHC_IRQSTAT_CRM;
  // Check card
  if (SDHC_PRSSTAT & SDHC_PRSSTAT_CINS) {
    return 0;
  } else {
    return SDHC_STATUS_NODISK;
  }
}

/******************************************************************************

    Private SD Card functions

******************************************************************************/

// waits for status bits sets
static uint32_t SDHC_WaitStatus(uint32_t mask)
{
  uint32_t             result;
  uint32_t             timeout = 1 << 24;
  do
  { result = SDHC_IRQSTAT & mask;
    timeout--;
  } while (!result && (timeout));
  if (timeout) return result;
  return 0;
}

// reads one block
static int SDHC_ReadBlock(uint32_t* pData)
{
	uint32_t i, irqstat;
	const uint32_t i_max = ((SDHC_BLOCK_SIZE) / (4 * SDHC_FIFO_BUFFER_SIZE));

	for (i = 0; i < i_max; i++) {
		irqstat = SDHC_IRQSTAT;
		SDHC_IRQSTAT = irqstat | SDHC_IRQSTAT_BRR;
		if (irqstat & (SDHC_IRQSTAT_DEBE | SDHC_IRQSTAT_DCE | SDHC_IRQSTAT_DTOE)) {
			SDHC_IRQSTAT = irqstat | SDHC_IRQSTAT_BRR |
				SDHC_IRQSTAT_DEBE | SDHC_IRQSTAT_DCE | SDHC_IRQSTAT_DTOE;
			SDHC_CMD12_StopTransferWaitForBusy();
			return SDHC_RESULT_ERROR;
		}
		while (!(SDHC_PRSSTAT & SDHC_PRSSTAT_BREN)) { };
		*pData++ = SDHC_DATPORT;
		*pData++ = SDHC_DATPORT;
		*pData++ = SDHC_DATPORT;
		*pData++ = SDHC_DATPORT;
		*pData++ = SDHC_DATPORT;
		*pData++ = SDHC_DATPORT;
		*pData++ = SDHC_DATPORT;
		*pData++ = SDHC_DATPORT;
		*pData++ = SDHC_DATPORT;
		*pData++ = SDHC_DATPORT;
		*pData++ = SDHC_DATPORT;
		*pData++ = SDHC_DATPORT;
		*pData++ = SDHC_DATPORT;
		*pData++ = SDHC_DATPORT;
		*pData++ = SDHC_DATPORT;
		*pData++ = SDHC_DATPORT;
	}
	return SDHC_RESULT_OK;
}

// writes one block
static int SDHC_WriteBlock(const uint32_t* pData)
{
	uint32_t i, i_max, j;
	i_max = ((SDHC_BLOCK_SIZE) / (4 * SDHC_FIFO_BUFFER_SIZE));

	for(i = 0; i < i_max; i++) {
		while (!(SDHC_IRQSTAT & SDHC_IRQSTAT_BWR)) ; // wait
		if (SDHC_IRQSTAT & (SDHC_IRQSTAT_DEBE | SDHC_IRQSTAT_DCE | SDHC_IRQSTAT_DTOE)) {
			SDHC_IRQSTAT |= SDHC_IRQSTAT_DEBE | SDHC_IRQSTAT_DCE |
				SDHC_IRQSTAT_DTOE | SDHC_IRQSTAT_BWR;
			(void)SDHC_CMD12_StopTransferWaitForBusy();
			return SDHC_RESULT_ERROR;
		}
		for(j=0; j<SDHC_FIFO_BUFFER_SIZE; j++) {
			SDHC_DATPORT = *pData++;
		}
		SDHC_IRQSTAT |= SDHC_IRQSTAT_BWR;

		if (SDHC_IRQSTAT & (SDHC_IRQSTAT_DEBE | SDHC_IRQSTAT_DCE | SDHC_IRQSTAT_DTOE)) {
			SDHC_IRQSTAT |= SDHC_IRQSTAT_DEBE | SDHC_IRQSTAT_DCE |
				SDHC_IRQSTAT_DTOE | SDHC_IRQSTAT_BWR;
			(void)SDHC_CMD12_StopTransferWaitForBusy();
			return SDHC_RESULT_ERROR;
		}
	}
	return SDHC_RESULT_OK;
}

// sends the command to SDcard
static int SDHC_CMD_Do(uint32_t xfertyp)
{
  // Card removal check preparation
  SDHC_IRQSTAT |= SDHC_IRQSTAT_CRM;

  // Wait for cmd line idle // to do timeout PRSSTAT[CDIHB] and the PRSSTAT[CIHB]
  while ((SDHC_PRSSTAT & SDHC_PRSSTAT_CIHB) || (SDHC_PRSSTAT & SDHC_PRSSTAT_CDIHB)) { };
  SDHC_XFERTYP = xfertyp;

  /* Wait for response */
  const uint32_t mask = SDHC_IRQSTAT_CIE | SDHC_IRQSTAT_CEBE | SDHC_IRQSTAT_CCE | SDHC_IRQSTAT_CC;
  if (SDHC_WaitStatus(mask) != SDHC_IRQSTAT_CC) {
      //SDHC_IRQSTAT |= mask;
      SDHC_IRQSTAT |= (mask | SDHC_IRQSTAT_CTOE);
      return SDHC_RESULT_ERROR;
  }
  /* Check card removal */
  if (SDHC_IRQSTAT & SDHC_IRQSTAT_CRM) {
      SDHC_IRQSTAT |= SDHC_IRQSTAT_CTOE | SDHC_IRQSTAT_CC;
      return SDHC_RESULT_NOT_READY;
  }

  /* Get response, if available */
  if (SDHC_IRQSTAT & SDHC_IRQSTAT_CTOE) {
      SDHC_IRQSTAT |= SDHC_IRQSTAT_CTOE | SDHC_IRQSTAT_CC;
      return SDHC_RESULT_NO_RESPONSE;
  }
  SDHC_IRQSTAT |= SDHC_IRQSTAT_CC;

  return SDHC_RESULT_OK;
}

// sends CMD0 to put SDCARD to idle
static int SDHC_CMD0_GoToIdle(void)
{
  uint32_t xfertyp;
  int result;

  SDHC_CMDARG = 0;

  xfertyp = (SDHC_XFERTYP_CMDINX(SDHC_CMD0) | SDHC_XFERTYP_RSPTYP(SDHC_XFERTYP_RSPTYP_NO));

  result = SDHC_CMD_Do(xfertyp);

  if (result == SDHC_RESULT_OK) { (void)SDHC_CMDRSP0; }
  return result;
}

// sends CMD2 to identify card
static int SDHC_CMD2_Identify(void)
{
  uint32_t xfertyp;
  int result;

  SDHC_CMDARG = 0;

  xfertyp = (SDHC_XFERTYP_CMDINX(SDHC_CMD2) | SDHC_XFERTYP_CCCEN
            | SDHC_XFERTYP_RSPTYP(SDHC_XFERTYP_RSPTYP_136));

  result = SDHC_CMD_Do(xfertyp);

  if (result == SDHC_RESULT_OK) { (void)SDHC_CMDRSP0; }

  return result;
}

// sends CMD 3 to get address
static int SDHC_CMD3_GetAddress(void)
{
  uint32_t xfertyp;
  int result;

  SDHC_CMDARG = 0;

  xfertyp = (SDHC_XFERTYP_CMDINX(SDHC_CMD3) | SDHC_XFERTYP_CICEN |
             SDHC_XFERTYP_CCCEN | SDHC_XFERTYP_RSPTYP(SDHC_XFERTYP_RSPTYP_48));

  result = SDHC_CMD_Do(xfertyp);

  if (result == SDHC_RESULT_OK) { (void)SDHC_CMDRSP0; }
  return result;
}

// sends ACMD6 to set bus width
static int SDHC_ACMD6_SetBusWidth(uint32_t address, uint32_t width)
{
  uint32_t xfertyp;
  int result;

  SDHC_CMDARG = address;
  // first send CMD 55 Application specific command
  xfertyp = (SDHC_XFERTYP_CMDINX(SDHC_CMD55) | SDHC_XFERTYP_CICEN |
             SDHC_XFERTYP_CCCEN | SDHC_XFERTYP_RSPTYP(SDHC_XFERTYP_RSPTYP_48));

  result = SDHC_CMD_Do(xfertyp);
  if (result == SDHC_RESULT_OK) { (void)SDHC_CMDRSP0;} else { return result; }
  SDHC_CMDARG = width;

  // Send CMD6
  xfertyp = (SDHC_XFERTYP_CMDINX(SDHC_CMD6) | SDHC_XFERTYP_CICEN |
             SDHC_XFERTYP_CCCEN | SDHC_XFERTYP_RSPTYP(SDHC_XFERTYP_RSPTYP_48));

  result = SDHC_CMD_Do(xfertyp);

  if (result == SDHC_RESULT_OK) {  (void)SDHC_CMDRSP0; }
  return result;
}


// sends CMD 7 to select card
static int SDHC_CMD7_SelectCard(uint32_t address)
{
  uint32_t xfertyp;
  int result;

  SDHC_CMDARG = address;

  xfertyp = (SDHC_XFERTYP_CMDINX(SDHC_CMD7) | SDHC_XFERTYP_CICEN |
             SDHC_XFERTYP_CCCEN | SDHC_XFERTYP_RSPTYP(SDHC_XFERTYP_RSPTYP_48BUSY));

  result = SDHC_CMD_Do(xfertyp);

  if (result == SDHC_RESULT_OK) {(void)SDHC_CMDRSP0; }
  return result;
}

// CMD8 to send interface condition
static int SDHC_CMD8_SetInterface(uint32_t cond)
{
  uint32_t xfertyp;
  int result;

  SDHC_CMDARG = cond;

  xfertyp = (SDHC_XFERTYP_CMDINX(SDHC_CMD8) | SDHC_XFERTYP_CICEN |
             SDHC_XFERTYP_CCCEN | SDHC_XFERTYP_RSPTYP(SDHC_XFERTYP_RSPTYP_48));

  result = SDHC_CMD_Do(xfertyp);

  if (result == SDHC_RESULT_OK) { (void)SDHC_CMDRSP0; }
  return result;
}

// sends CMD 9 to get interface condition
static int SDHC_CMD9_GetParameters(uint32_t address)
{
  uint32_t xfertyp;
  int result;

  SDHC_CMDARG = address;

  xfertyp = (SDHC_XFERTYP_CMDINX(SDHC_CMD9) | SDHC_XFERTYP_CCCEN |
             SDHC_XFERTYP_RSPTYP(SDHC_XFERTYP_RSPTYP_136));

  result = SDHC_CMD_Do(xfertyp);

  if (result == SDHC_RESULT_OK) {
    //(void)SDHC_CMDRSP0;
    sdCardDesc.tranSpeed = SDHC_CMDRSP2 >> 24;
  }

  return result;
}

// sends CMD12 to stop transfer
static int SDHC_CMD12_StopTransfer(void)
{
  uint32_t xfertyp;
  int result;

  SDHC_CMDARG = 0;
  xfertyp = (SDHC_XFERTYP_CMDINX(SDHC_CMD12) | SDHC_XFERTYP_CMDTYP(SDHC_XFERTYP_CMDTYP_ABORT) |
             SDHC_XFERTYP_CICEN | SDHC_XFERTYP_CCCEN | SDHC_XFERTYP_RSPTYP(SDHC_XFERTYP_RSPTYP_48BUSY));

  result = SDHC_CMD_Do(xfertyp);

  if (result == SDHC_RESULT_OK) { }
  return result;
}

// sends CMD12 to stop transfer and first waits to ready SDCArd
static int SDHC_CMD12_StopTransferWaitForBusy(void)
{
  uint32_t timeOut = 1000;
  int result;
  do {
    result = SDHC_CMD12_StopTransfer();
    timeOut--;
  } while (timeOut && (SDHC_PRSSTAT & SDHC_PRSSTAT_DLA) && result == SDHC_RESULT_OK);

  if (result != SDHC_RESULT_OK)  return result;
  if (!timeOut)  return SDHC_RESULT_NO_RESPONSE;

  return SDHC_RESULT_OK;
}

// sends CMD16 to set block size
static int SDHC_CMD16_SetBlockSize(uint32_t block_size)
{
  uint32_t xfertyp;
  int result;

  SDHC_CMDARG = block_size;

  xfertyp = (SDHC_XFERTYP_CMDINX(SDHC_CMD16) | SDHC_XFERTYP_CICEN |
             SDHC_XFERTYP_CCCEN | SDHC_XFERTYP_RSPTYP(SDHC_XFERTYP_RSPTYP_48));

  result = SDHC_CMD_Do(xfertyp);

  if (result == SDHC_RESULT_OK) { (void)SDHC_CMDRSP0; }

  return result;
}

// sends CMD17 to read one block
static int SDHC_CMD17_ReadBlock(uint32_t sector)
{
  uint32_t xfertyp;
  int result;

  SDHC_CMDARG = sector;

  SDHC_BLKATTR = SDHC_BLKATTR_BLKCNT(1) | 512;

  xfertyp = (SDHC_XFERTYP_CMDINX(SDHC_CMD17) | SDHC_XFERTYP_CICEN |
             SDHC_XFERTYP_CCCEN | SDHC_XFERTYP_RSPTYP(SDHC_XFERTYP_RSPTYP_48) |
             SDHC_XFERTYP_DTDSEL | SDHC_XFERTYP_DPSEL);

  result = SDHC_CMD_Do(xfertyp);
  if (result == SDHC_RESULT_OK) { ( void)SDHC_CMDRSP0; }

  return result;
}

// sends CMD24 to write one block
static int SDHC_CMD24_WriteBlock(uint32_t sector)
{
  uint32_t xfertyp;
  int result;

  SDHC_CMDARG = sector;
  SDHC_BLKATTR = SDHC_BLKATTR_BLKCNT(1) | 512;

  xfertyp = (SDHC_XFERTYP_CMDINX(SDHC_CMD24) | SDHC_XFERTYP_CICEN |
             SDHC_XFERTYP_CCCEN | SDHC_XFERTYP_RSPTYP(SDHC_XFERTYP_RSPTYP_48) |
             SDHC_XFERTYP_DPSEL);

  result = SDHC_CMD_Do(xfertyp);
  if (result == SDHC_RESULT_OK) { (void)SDHC_CMDRSP0; }

  return result;
}

// ACMD 41 to send operation condition
static int SDHC_ACMD41_SendOperationCond(uint32_t cond)
{
  uint32_t xfertyp;
  int result;

  SDHC_CMDARG = 0;
  // first send CMD 55 Application specific command
  xfertyp = (SDHC_XFERTYP_CMDINX(SDHC_CMD55) | SDHC_XFERTYP_CICEN |
             SDHC_XFERTYP_CCCEN | SDHC_XFERTYP_RSPTYP(SDHC_XFERTYP_RSPTYP_48));

  result = SDHC_CMD_Do(xfertyp);

  if (result == SDHC_RESULT_OK) { (void)SDHC_CMDRSP0; } else { return result; }

  SDHC_CMDARG = cond;

  // Send 41CMD
  xfertyp = (SDHC_XFERTYP_CMDINX(SDHC_ACMD41) | SDHC_XFERTYP_RSPTYP(SDHC_XFERTYP_RSPTYP_48));
  result = SDHC_CMD_Do(xfertyp);

  if (result == SDHC_RESULT_OK) { (void)SDHC_CMDRSP0; }

  return result;
}
#endif // __MK64FX512__ or __MK66FX1M0__ or __IMXRT1052__
