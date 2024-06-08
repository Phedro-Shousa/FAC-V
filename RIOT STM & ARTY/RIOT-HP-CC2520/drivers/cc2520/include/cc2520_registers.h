/*
 *
 */

/**
 * @ingroup     drivers_cc2520
 * @{
 *
 * @file
 * @brief       Register and command definitions for CC2520
 *
 */

#ifndef CC2520_REGISTERS_H
#define CC2520_REGISTERS_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @name    Internal device option flags
 * @{
 */
#define CC2520_OPT_FRAME_FILTER     (0x0001)    /**< frame filtering active */  
#define CC2520_OPT_AUTOACK          (0x0002)    /**< auto ACKs active */
#define CC2520_OPT_AUTOCRC          (0x0003)    /**< auto CRC active */

#define CC2520_OPT_CSMA             (0x0004)    /**< CSMA active */
#define CC2520_OPT_PROMISCUOUS      (0x0008)    /**< promiscuous mode
                                                 *   active */
#define CC2520_OPT_PRELOADING       (0x0010)    /**< preloading enabled */  
#define CC2520_OPT_TELL_TX_START    (0x0020)    /**< notify MAC layer on TX
                                                 *   start */
#define CC2520_OPT_TELL_TX_END      (0x0040)    /**< notify MAC layer on TX
                                                 *   finished */
#define CC2520_OPT_TELL_RX_START    (0x0080)    /**< notify MAC layer on RX
                                                *   start */
#define CC2520_OPT_TELL_RX_END      (0x0100)    /**< notify MAC layer on RX
                                                 *   finished */
/** @} */

/**
 * @name    Possible device states
 * @{
 */
enum {
    CC2520_STATE_IDLE               =  (0x00),
    CC2520_STATE_RX_CALIBRATION     =  (0x02),
    CC2520_STATE_SFD_WAIT_LOW       =  (0x03),
    CC2520_STATE_SFD_WAIT_HIGH      =  (0x06),
    CC2520_STATE_RX_LOW             =  (0x07),
    CC2520_STATE_RX_HIGH            =  (0x0D),
    CC2520_STATE_RXRX_WAIT          =  (0x0E),
    CC2520_STATE_RXFIFO_RESET       =  (0x10),
    CC2520_STATE_RX_OVERFLOW        =  (0x11),
    CC2520_STATE_TX_CALIBRATION     =  (0x20),
    CC2520_STATE_TX_LOW             =  (0x22),
    CC2520_STATE_TX_HIGH            =  (0x26),
    CC2520_STATE_TX_FINAL           =  (0x27),
    CC2520_STATE_TXRX_TRANSIT       =  (0x28),
    CC2520_STATE_ACK_CALIBRATION    =  (0x30),
    CC2520_STATE_ACK_LOW            =  (0x31),
    CC2520_STATE_ACK_HIGH           =  (0x36),
    CC2520_STATE_ACK_DELAY          =  (0x37),
    CC2520_STATE_TX_UDERFLOW        =  (0x38),
    CC2520_STATE_TX_SHUTDOWN_LOW    =  (0x1A),
    CC2520_STATE_TX_SHUTDOWN_HIGH   =  (0x39),
};

/*
 * All constants are from the Chipcon cc2520 Data Sheet that at one
 * point in time could be found at
 * http://www.chipcon.com/files/cc2520_Data_Sheet_1_4.pdf
 *
 * The page numbers below refer to pages in this document.
 */

/* Page 27. */
enum cc2520_status_byte {
  CC2520_XOSC16M_STABLE     =  (0x80),
  CC2520_RSSI_VALID	        =  (0x40),
  CC2520_EXCEPTION_CHA      =  (0x20),
  CC2520_EXCEPTION_CHB      =  (0x10),
  CC2520_DPU_H	            =  (0x08),
  CC2520_DPU_L	            =  (0x04),
  CC2520_TX_ACTIVE	        =  (0x02),
  CC2520_RX_ACTIVE	        =  (0x01),
};

#define TX_FRM_DONE             0x02
#define RX_FRM_DONE             0x01
#define RX_FRM_ABORTED          0x20
#define RX_FRM_UNDERFLOW        0x20

/* Page 27. */
enum cc2520_memory_size {
  CC2520_RAM_SIZE	= 640,
  CC2520_FIFO_SIZE	= 128,
};

/* Page 29. */
enum cc2520_ram_address {
  CC2520RAM_TXFIFO	        =  (0x100),
  CC2520RAM_RXFIFO	        =  (0x180),
  CC2520RAM_IEEEADDR	    =  (0x3EA),
  CC2520RAM_PANID           =  (0x3F2),
  CC2520RAM_SHORTADDR       =  (0x3F4),
};

/**
 * @name    Frame control 0 register bitfields
 * @{
 */
#define CC2520_FRMCTRL0_APPEND_DATA_MODE        (0x80)
#define CC2520_FRMCTRL0_AUTOCRC                 (0x40)
#define CC2520_FRMCTRL0_AUTOACK                 (0x20)
#define CC2520_FRMCTRL0_ENERGY_SCAN             (0x10)
#define CC2520_FRMCTRL0_RX_MODE_1               (0x08)
#define CC2520_FRMCTRL0_RX_MODE_0               (0x04)
#define CC2520_FRMCTRL0_TX_MODE_1               (0x02)
#define CC2520_FRMCTRL0_TX_MODE_0               (0x01)
/** @} */

/**
 * @name    Frame filter 0 register bitfields
 * @{
 */
#define CC2520_FRMFILT0_RESERVED                (0x80)
#define CC2520_FRMFILT0_FCF_RESERVED_MASK_2     (0x40)
#define CC2520_FRMFILT0_FCF_RESERVED_MASK_1     (0x20)
#define CC2520_FRMFILT0_FCF_RESERVED_MASK_0     (0x10)
#define CC2520_FRMFILT0_MAX_FRAME_VERSION_1     (0x08)
#define CC2520_FRMFILT0_MAX_FRAME_VERSION_0     (0x04)
#define CC2520_FRMFILT0_PAN_COORDINATOR         (0x02)
#define CC2520_FRMFILT0_FRAME_FILTER_EN         (0x01)
/** @} */

/**
 * @name    Frame filter 1 register bitfields
 * @{
 */
#define CC2520_FRMFILT1_ACCEPT_FT_4TO7_RESERVED (0x80)
#define CC2520_FRMFILT1_ACCEPT_FT_3_MAC_CMD     (0x40)
#define CC2520_FRMFILT1_ACCEPT_FT_2_ACK         (0x20)
#define CC2520_FRMFILT1_ACCEPT_FT_1_DATA        (0x10)
#define CC2520_FRMFILT1_ACCEPT_FT_0_BEACON      (0x08)
#define CC2520_FRMFILT1_MODIFY_FT_FILTER_1      (0x04)
#define CC2520_FRMFILT1_MODIFY_FT_FILTER_0      (0x02)
#define CC2520_FRMFILT1_RESERVED                (0x01)
/** @} */


/*  FREG definitions (BSET/BCLR supported)  */
#define CC2520_FRMFILT0                         (0x000)
#define CC2520_FRMFILT1                         (0x001)
#define CC2520_SRCMATCH                         (0x002)
#define CC2520_SRCSHORTEN0                      (0x004)
#define CC2520_SRCSHORTEN1                      (0x005)
#define CC2520_SRCSHORTEN2                      (0x006)
#define CC2520_SRCEXTEN0                        (0x008)
#define CC2520_SRCEXTEN1                        (0x009)
#define CC2520_SRCEXTEN2                        (0x00A)
#define CC2520_FRMCTRL0                         (0x00C)
#define CC2520_FRMCTRL1                         (0x00D)
#define CC2520_RXENABLE0                        (0x00E)
#define CC2520_RXENABLE1                        (0x00F)
#define CC2520_EXCFLAG0                         (0x010)
#define CC2520_EXCFLAG1                         (0x011)
#define CC2520_EXCFLAG2                         (0x012)
#define CC2520_EXCMASKA0                        (0x014)
#define CC2520_EXCMASKA1                        (0x015)
#define CC2520_EXCMASKA2                        (0x016)
#define CC2520_EXCMASKB0                        (0x018)
#define CC2520_EXCMASKB1                        (0x019)
#define CC2520_EXCMASKB2                        (0x01A)
#define CC2520_EXCBINDX0                        (0x01C)
#define CC2520_EXCBINDX1                        (0x01D)
#define CC2520_EXCBINDY0                        (0x01E)
#define CC2520_EXCBINDY1                        (0x01F)
#define CC2520_GPIOCTRL0                        (0x020)
#define CC2520_GPIOCTRL1                        (0x021)
#define CC2520_GPIOCTRL2                        (0x022)
#define CC2520_GPIOCTRL3                        (0x023)
#define CC2520_GPIOCTRL4                        (0x024)
#define CC2520_GPIOCTRL5                        (0x025)
#define CC2520_GPIOPOLARITY                     (0x026)
#define CC2520_GPIOCTRL                         (0x028)
#define CC2520_DPUCON                           (0x02A)
#define CC2520_DPUSTAT                          (0x02C)
#define CC2520_FREQCTRL                         (0x02E)
#define CC2520_FREQTUNE                         (0x02F)
#define CC2520_TXPOWER                          (0x030)
#define CC2520_TXCTRL                           (0x031)
#define CC2520_FSMSTAT0                         (0x032)
#define CC2520_FSMSTAT1                         (0x033)
#define CC2520_FIFOPCTRL                        (0x034)
#define CC2520_FSMCTRL                          (0x035)
#define CC2520_CCACTRL0                         (0x036)
#define CC2520_CCACTRL1                         (0x037)
#define CC2520_RSSI                             (0x038)
#define CC2520_RSSISTAT                         (0x039)
#define CC2520_TXFIFO_BUF                       (0x03A)
#define CC2520_RXFIRST                          (0x03C)
#define CC2520_RXFIFOCNT                        (0x03E)
#define CC2520_TXFIFOCNT                        (0x03F)

/*  SREG definitions (BSET/BCLR unsupported)  */
#define CC2520_CHIPID                           (0x040)
#define CC2520_VERSION                          (0x042)
#define CC2520_EXTCLOCK                         (0x044)
#define CC2520_MDMCTRL0                         (0x046)
#define CC2520_MDMCTRL1                         (0x047)
#define CC2520_FREQEST                          (0x048)
#define CC2520_RXCTRL                           (0x04A)
#define CC2520_FSCTRL                           (0x04C)
#define CC2520_FSCAL0                           (0x04E)
#define CC2520_FSCAL1                           (0x04F)
#define CC2520_FSCAL2                           (0x050)
#define CC2520_FSCAL3                           (0x051)
#define CC2520_AGCCTRL0                         (0x052)
#define CC2520_AGCCTRL1                         (0x053)
#define CC2520_AGCCTRL2                         (0x054)
#define CC2520_AGCCTRL3                         (0x055)
#define CC2520_ADCTEST0                         (0x056)
#define CC2520_ADCTEST1                         (0x057)
#define CC2520_ADCTEST2                         (0x058)
#define CC2520_MDMTEST0                         (0x05A)
#define CC2520_MDMTEST1                         (0x05B)
#define CC2520_DACTEST0                         (0x05C)
#define CC2520_DACTEST1                         (0x05D)
#define CC2520_ATEST                            (0x05E)
#define CC2520_DACTEST2                         (0x05F)
#define CC2520_PTEST0                           (0x060)
#define CC2520_PTEST1                           (0x061)
#define CC2520_RESERVED                         (0x062)
#define CC2520_DPUBIST                          (0x07A)
#define CC2520_ACTBIST                          (0x07C)
#define CC2520_RAMBIST                          (0x07E)

// Instruction implementation
#define CC2520_INS_SNOP                         (0x00)
#define CC2520_INS_IBUFLD                       (0x02)
#define CC2520_INS_SIBUFEX                      (0x03)
#define CC2520_INS_SSAMPLECCA                   (0x04)
#define CC2520_INS_SRES                         (0x0F)
#define CC2520_INS_MEMRD                        (0x10)
#define CC2520_INS_MEMWR                        (0x20)
#define CC2520_INS_RXBUF                        (0x30)
#define CC2520_INS_RXBUFCP                      (0x38)
#define CC2520_INS_RXBUFMOV                     (0x32)
#define CC2520_INS_TXBUF                        (0x3A)
#define CC2520_INS_TXBUFCP                      (0x3E)
#define CC2520_INS_RANDOM                       (0x3C)
#define CC2520_INS_SXOSCON                      (0x40)
#define CC2520_INS_STXCAL                       (0x41)
#define CC2520_INS_SRXON                        (0x42)
#define CC2520_INS_STXON                        (0x43)
#define CC2520_INS_STXONCCA                     (0x44)
#define CC2520_INS_SRFOFF                       (0x45)
#define CC2520_INS_SXOSCOFF                     (0x46)
#define CC2520_INS_SFLUSHRX                     (0x47)
#define CC2520_INS_SFLUSHTX                     (0x48)
#define CC2520_INS_SACK                         (0x49)
#define CC2520_INS_SACKPEND                     (0x4A)
#define CC2520_INS_SNACK                        (0x4B)
#define CC2520_INS_SRXMASKBITSET                (0x4C)
#define CC2520_INS_SRXMASKBITCLR                (0x4D)
#define CC2520_INS_RXMASKAND                    (0x4E)
#define CC2520_INS_RXMASKOR                     (0x4F)
#define CC2520_INS_MEMCP                        (0x50)
#define CC2520_INS_MEMCPR                       (0x52)
#define CC2520_INS_MEMXCP                       (0x54)
#define CC2520_INS_MEMXWR                       (0x56)
#define CC2520_INS_BCLR                         (0x58)
#define CC2520_INS_BSET                         (0x59)
#define CC2520_INS_CTR                          (0x60)
#define CC2520_INS_CBCMAC                       (0x64)
#define CC2520_INS_UCBCMAC                      (0x66)
#define CC2520_INS_CCM                          (0x68)
#define CC2520_INS_UCCM                         (0x6A)
#define CC2520_INS_ECB                          (0x70)
#define CC2520_INS_ECBO                         (0x72)
#define CC2520_INS_ECBX                         (0x74)
#define CC2520_INS_ECBXO                        (0x76)
#define CC2520_INS_INC                          (0x78)
#define CC2520_INS_ABORT                        (0x7F)
#define CC2520_INS_REGRD                        (0x80)
#define CC2520_INS_REGWR                        (0xC0)


/**
 * @name    CC2420 strobe commands
 * @see     Datasheet section 37, pages 61--62
 * @{
 */
#define CC2520_STROBE_NOP           CC2520_INS_SNOP     /**< no operation */
#define CC2520_STROBE_XOSCON        CC2520_INS_SXOSCON  /**< turn transceiver on */
#define CC2520_STROBE_TXCAL         CC2520_INS_STXCAL   /**< calibrate TX freq and wait */
#define CC2520_STROBE_RXON          CC2520_INS_SRXON    /**< switch to RX mode */
#define CC2520_STROBE_TXON          CC2520_INS_STXON    /**< switch to TX mode */
#define CC2520_STROBE_TXONCCA       CC2520_INS_STXONCCA /**< switch to TX after CCA*/
#define CC2520_STROBE_RFOFF         CC2520_INS_SRFOFF   /**< switch to IDLE mode */
#define CC2520_STROBE_XOSCOFF       CC2520_INS_SXOSCOFF /**< power down */
#define CC2520_STROBE_FLUSHRX       CC2520_INS_SFLUSHRX /**< flush RX FIFO */
#define CC2520_STROBE_FLUSHTX       CC2520_INS_SFLUSHTX /**< flush TX FIFO */
#define CC2520_STROBE_ACK           CC2520_INS_SACK     /**< send ACK with pending cleared */
#define CC2520_STROBE_ACKPEND       CC2520_INS_SACKPEND /**< send ACK with pending set */
/** @} */

/**
 * @brief  CRC/Correlation bit masks
 * @{
 */
#define CC2520_CRCCOR_CRC_MASK      (0x80)
#define CC2520_CRCCOR_COR_MASK      (0x7F)
/** @} */


#ifdef __cplusplus
}
#endif

#endif /* CC2520_REGISTERS_H */
/** @} */
