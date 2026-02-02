/*
 * mx25_defs.h
 *
 *  Created on: 20 Feb. 2026.
 *      Author: priss
 */

/*
 * mx25_defs.h
 *
 * Macronix MX25xx SPI NOR Flash definitions
 *
 * Supported family:
 *   MX25Lxxxx / MX25Rxxxx
 *
 * Interface:
 *   SPI (Mode 0 / Mode 3)
 *
 * This file is bootloader-independent and application-level safe.
 */

#ifndef MX25_DEFS_H_
#define MX25_DEFS_H_

// -----------------------------------------------------------------------------
//                       Includes
// ---------------------------------------------------------------------------

#include <stdint.h>

#define MX25R8035F

/* Note:
   Synchronous IO     : MCU will polling WIP bit after
                        sending prgram/erase command
   Non-synchronous IO : MCU can do other operation after
                        sending prgram/erase command
   Default is synchronous IO
*/
//#define    NON_SYNCHRONOUS_IO

#define MX25_DUMMY_BYTE     0x00  //dummy MUST be 0x00, in "read manufacturer"

/* ============================================================================
 * JEDEC IDs (most common MX25 parts)
 * ========================================================================== */
#define MX25_MANUFACTURER_ID        0xC2  /* Macronix */

#define MX25_DEV_ID_MX25L3206E    (0x2016)
#define MX25_DEV_ID_MX25L6406E    (0x2017)
#define MX25_DEV_ID_MX25L12835F   (0x2018)
#define MX25_DEV_ID_MX25L25645G   (0x2019)
#define MX25_DEV_ID_MX25R8035F    (0x2814)
/* ============================================================================
 * SPI Flash Commands
 * ========================================================================== */

/* ** MX25 series command hex code definition ** */
/* --- Identification --- */
#define MX25_CMD_RDID              0x9F  /* Read JEDEC ID */
#define MX25_CMD_RES               0xAB  /* Read Electronic ID */
#define MX25_CMD_REMS              0x90  /* Read Electronic Manufacturer & device ID */

/* --- Read --- */
#define MX25_CMD_READ              0x03  /* Normal read */
#define MX25_CMD_FAST_READ         0x0B  /* Fast read (1 dummy byte) */

/* --- Write --- */
#define MX25_CMD_WREN              0x06  /* Write Enable */
#define MX25_CMD_WRDI              0x04  /* Write Disable */
#define MX25_CMD_PP                0x02  /* Page Program */

/* --- Erase --- */
#define MX25_CMD_SE                0x20  /* Sector Erase (4 KB) */
#define MX25_CMD_BE32              0x52  /* Block Erase (32 KB) */
#define MX25_CMD_BE64              0xD8  /* Block Erase (64 KB) */
#define MX25_CMD_CE                0xC7  /* Chip Erase */
#define MX25_CMD_CE_ALT            0x60  /* Chip Erase (alt) */

/* --- Register comands --- */
/* --- Status / Control --- */
#define MX25_CMD_WRSR              0x01  /* Write Status Register */
#define MX25_CMD_RDSR              0x05  /* Read Status Register */
#define MX25_CMD_WRSCUR            0x2F  /* Write Security Register  */
#define MX25_CMD_RDSCUR            0x2B  /* Read Security Register */
#define MX25_CMD_RDCR              0x15  /* Read Configuration Register */

#define MX25_CMD_RDFSR             0x70  /* Read Flag Status Register */

/* Mode setting comands */
/* --- Power --- */
#define MX25_CMD_DP            0xB9 /* Deep Power Down */
#define MX25_CMD_ENSO          0xB1 /* Enter Secured OTP */
#define MX25_CMD_EXSO          0xC1 /* Exit Secured OTP */
#ifdef SBL_CMD_0x77
#define MX25_CMD_SBL           0x77 /* Set Burst Length  new: 0x77*/
#else
#define MX25_CMD_SBL           0xC0 /* Set Burst Length  Old: 0xC0*/
#endif

/* Reset comands */
#define MX25_CMD_RSTEN     0x66    //RSTEN (Reset Enable)
#define MX25_CMD_RST       0x99    //RST (Reset Memory)

/* Suspend/Resume comands */
#define MX25_CMD_PGM_ERS_S    0xB0    /* PGM/ERS Suspend (Suspends Program/Erase) */
#define MX25_CMD_PGM_ERS_R    0x30    /* PGM/ERS Erase (Resumes Program/Erase)  */
#define MX25_CMD_NOP          0x00    /* NOP (No Operation)  */

/* ============================================================================
 * Status Register bits
 * ========================================================================== */
#define MX25_SR_WIP                ((uint8_t)(1U << 0)) /* Write In Progress */
#define MX25_SR_WEL                ((uint8_t)(1U << 1)) /* Write Enable Latch */
#define MX25_SR_BP0                ((uint8_t)(1U << 2))
#define MX25_SR_BP1                ((uint8_t)(1U << 3))
#define MX25_SR_BP2                ((uint8_t)(1U << 4))
#define MX25_SR_BP3                ((uint8_t)(1U << 5))
#define MX25_SR_QE                 ((uint8_t)(1U << 6)) /* QuadEnable */
#define MX25_SR_SRWD               ((uint8_t)(1U << 7)) /* Write Protect Disable */

/* ============================================================================
 * Flag Status Register bits (if supported)
 * ========================================================================== */
#define MX25_FSR_P_FAIL             ((uint8_t)(1U << 5))
#define MX25_FSR_E_FAIL             ((uint8_t)(1U << 6))



/*
  System Information Define
*/
#define    CLK_PERIOD                26 // unit: ns
#define    Min_Cycle_Per_Inst        1  // cycle count of one instruction
#define    One_Loop_Inst             10 // instruction count of one loop (estimate)


/*
  Flash ID, Timing Information Define
  (The following information could get from device specification)
*/

#ifdef MX25R8035F
#define    FlashID          0xc22814
#define    ElectronicID     0x14
#define    RESID0           0xc214
#define    RESID1           0x14c2
#define    CE_period        15625000       // tCE /  ( CLK_PERIOD * Min_Cycle_Per_Inst *One_Loop_Inst)
#define    tW               40000000       // 40ms
#define    tDP              10000          // 10us
#define    tBP              100000         // 100us
#define    tPP              10000000       // 10ms
#define    tSE              240000000      // 240ms
#define    tBE32            1750000000     // 1.75s
#define    tBE              3500000000     // 3.5s
#define    tPUW             10000000       // 10ms
#define    tWSR             tBP
// Support I/O mode
#define    SIO              0
#define    DIO              1
#define    QIO              2
#endif


// Flash information define
#define    WriteStatusRegCycleTime     tW / (CLK_PERIOD * Min_Cycle_Per_Inst * One_Loop_Inst)
#define    PageProgramCycleTime        tPP / (CLK_PERIOD * Min_Cycle_Per_Inst * One_Loop_Inst)
#define    SectorEraseCycleTime        tSE / (CLK_PERIOD * Min_Cycle_Per_Inst * One_Loop_Inst)
#define    BlockEraseCycleTime         tBE / (CLK_PERIOD * Min_Cycle_Per_Inst * One_Loop_Inst)
#define    ChipEraseCycleTime          CE_period
#define    FlashFullAccessTime         tPUW / (CLK_PERIOD * Min_Cycle_Per_Inst * One_Loop_Inst)

#ifdef tBP
#define    ByteProgramCycleTime        tBP / (CLK_PERIOD * Min_Cycle_Per_Inst * One_Loop_Inst)
#endif
#ifdef tWSR
#define    WriteSecuRegCycleTime       tWSR / (CLK_PERIOD * Min_Cycle_Per_Inst * One_Loop_Inst)
#endif
#ifdef tBE32
#define    BlockErase32KCycleTime      tBE32 / (CLK_PERIOD * Min_Cycle_Per_Inst * One_Loop_Inst)
#endif
#ifdef tWREAW
#define    WriteExtRegCycleTime        tWREAW / (CLK_PERIOD * Min_Cycle_Per_Inst * One_Loop_Inst)
#endif

/* ============================================================================
 * Geometry (defaults, override per part if needed)
 * ========================================================================== */
#define MX25_FLASH_SIZE              (1048576L)    // 0x00100000  //1MB-8Mb total size (bytes)
#define DEVICE_SIZE_8M               MX25_FLASH_SIZE

#define MX25_PAGE_SIZE              (256U)         //0x00100 //256b Page size (bits)
#define MX25_SECTOR_SIZE            (4U  * 1024U)  //0x01000 //  4K Sector size(bytes)
#define MX25_BLOCK32_SIZE           (32U * 1024U)  //0x08000 // 32K Block size (bytes)
#define MX25_BLOCK64_SIZE           (64U * 1024U)  //0x10000 // 64k Block size (bytes)

#define MX25_BLOCK_SIZE             (MX25_BLOCK64_SIZE)
#define MX25_Block_Num              (MX25_FLASH_SIZE / MX25_BLOCK64_SIZE)

#define MX25_PageToSector(PageNumber)      ((PageNumber * MX25_PAGE_SIZE) / MX25_SECTOR_SIZE)
#define MX25_PageToBlock(PageNumber)       ((PageNumber * MX25_PAGE_SIZE) / MX25_BLOCK_SIZE)
#define MX25_SectorToBlock(SectorNumber)   ((SectorNumber * MX25_SECTOR_SIZE) / MX25_BLOCK_SIZE)
#define MX25_SectorToPage(SectorNumber)    ((SectorNumber * MX25_SECTOR_SIZE) / MX25_PAGE_SIZE)
#define MX25_BlockToPage(BlockNumber)      ((BlockNumber * MX25_BLOCK_SIZE) / MX25_PAGE_SIZE)
#define MX25_PageToAddress(PageNumber)     (PageNumber * MX25_PAGE_SIZE)
#define MX25_SectorToAddress(SectorNumber) (SectorNumber * MX25_SECTOR_SIZE)
#define MX25_BlockToAddress(BlockNumber)   (BlockNumber * MX25_BLOCK_SIZE)
#define MX25_AddressToPage(Address)        (Address / MX25_PAGE_SIZE)
#define MX25_AddressToSector(Address)      (Address / MX25_SECTOR_SIZE)
#define MX25_AddressToBlock(Address)       (Address / MX25_BLOCK_SIZE)

/* ============================================================================
 * Addressing
 * ========================================================================== */
#define MX25_ADDR_BYTES             3U     /* 24-bit addressing */

/* ============================================================================
 * Timing (typical, for reference)
 * ========================================================================== */
#define MX25_T_PP_MAX_MS             3U    /* Page program */
#define MX25_T_SE_MAX_MS           150U    /* Sector erase */
#define MX25_T_BE64_MAX_MS        2000U    /* 64K block erase */
#define MX25_T_CE_MAX_MS         10000U    /* Chip erase */

/* ============================================================================
 * Helpers
 * ========================================================================== */
#define MX25_MIN(a,b)               ((a) < (b) ? (a) : (b))

/* ============================================================================
 * Sanity checks
 * ========================================================================== */
#if (MX25_PAGE_SIZE & (MX25_PAGE_SIZE - 1)) != 0
#error "MX25_PAGE_SIZE must be power of two"
#endif

#endif /* MX25_DEFS_H_ */
