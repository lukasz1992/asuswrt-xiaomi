#ifdef CONFIG_PROJECT_7628
#ifndef __MTK_PHY_7628_H
#define __MTK_PHY_7628_H

#define U2_SR_COEF_7628 32

///////////////////////////////////////////////////////////////////////////////

struct u2phy_reg {
	//0x0
	PHY_LE32 u2phyac0;
	PHY_LE32 u2phyac1;
	PHY_LE32 u2phyac2;
	PHY_LE32 reserve0;
	//0x10
	PHY_LE32 u2phyacr0;
	PHY_LE32 u2phyacr1;
	PHY_LE32 u2phyacr2;
	PHY_LE32 u2phyacr3;
	//0x20
	PHY_LE32 u2phyacr4;
	PHY_LE32 u2phyamon0;
	PHY_LE32 reserve1[2];
	//0x30~0x50
	PHY_LE32 reserve2[12];
	//0x60
	PHY_LE32 u2phydcr0;
	PHY_LE32 u2phydcr1;
	PHY_LE32 u2phydtm0;
	PHY_LE32 u2phydtm1;
	//0x70
	PHY_LE32 u2phydmon0;
	PHY_LE32 u2phydmon1;
	PHY_LE32 u2phydmon2;
	PHY_LE32 u2phydmon3;
	//0x80
	PHY_LE32 u2phybc12c;
	PHY_LE32 u2phybc12c1;
	PHY_LE32 reserve3[2];
	//0x90~0xe0
	PHY_LE32 reserve4[24];
	//0xf0
	PHY_LE32 reserve6[3];
	PHY_LE32 regfcom;
};

//U3D_U2PHYAC0
#define RG_USB20_USBPLL_DIVEN                     (0x7<<28) //30:28
#define RG_USB20_USBPLL_CKCTRL                    (0x3<<26) //27:26
#define RG_USB20_USBPLL_PREDIV                    (0x3<<24) //25:24
#define RG_USB20_USBPLL_FORCE_ON                  (0x1<<23) //23:23
#define RG_USB20_USBPLL_FBDIV                     (0x7f<<16) //22:16
#define RG_USB20_REF_EN                           (0x1<<15) //15:15
#define RG_USB20_INTR_EN                          (0x1<<14) //14:14
#define RG_USB20_BG_TRIM                          (0xf<<8) //11:8
#define RG_USB20_BG_RBSEL                         (0x3<<6) //7:6
#define RG_USB20_BG_RASEL                         (0x3<<4) //5:4
#define RG_USB20_BGR_DIV                          (0x3<<2) //3:2
#define RG_SIFSLV_CHP_EN                          (0x1<<1) //1:1
#define RG_SIFSLV_BGR_EN                          (0x1<<0) //0:0

//U3D_U2PHYAC1
#define RG_USB20_VRT_VREF_SEL                     (0x7<<28) //30:28
#define RG_USB20_TERM_VREF_SEL                    (0x7<<24) //26:24
#define RG_USB20_MPX_SEL                          (0xff<<16) //23:16
#define RG_USB20_MPX_OUT_SEL                      (0x3<<12) //13:12
#define RG_USB20_TX_PH_ROT_SEL                    (0x7<<8) //10:8
#define RG_USB20_USBPLL_ACCEN                     (0x1<<3) //3:3
#define RG_USB20_USBPLL_LF                        (0x1<<2) //2:2
#define RG_USB20_USBPLL_BR                        (0x1<<1) //1:1
#define RG_USB20_USBPLL_BP                        (0x1<<0) //0:0

//U3D_U2PHYAC2
#define RG_SIFSLV_MAC_BANDGAP_EN                  (0x1<<17) //17:17
#define RG_SIFSLV_MAC_CHOPPER_EN                  (0x1<<16) //16:16
#define RG_USB20_CLKREF_REV                       (0xff<<0) //7:0

//U3D_U2PHYACR0
#define RG_USB20_ICUSB_EN                         (0x1<<24) //24:24
#define RG_USB20_HSTX_SRCAL_EN                    (0x1<<23) //23:23
#define RG_USB20_HSTX_SRCTRL                      (0x7<<16) //18:16
#define RG_USB20_LS_CR                            (0x7<<12) //14:12
#define RG_USB20_FS_CR                            (0x7<<8) //10:8
#define RG_USB20_LS_SR                            (0x7<<4) //6:4
#define RG_USB20_FS_SR                            (0x7<<0) //2:0

//U3D_U2PHYACR1
#define RG_USB20_INIT_SQ_EN_DG                    (0x3<<28) //29:28
#define RG_USB20_SQD                              (0x3<<24) //25:24
#define RG_USB20_HSTX_TMODE_SEL                   (0x3<<20) //21:20
#define RG_USB20_HSTX_TMODE_EN                    (0x1<<19) //19:19
#define RG_USB20_PHYD_MONEN                       (0x1<<18) //18:18
#define RG_USB20_INLPBK_EN                        (0x1<<17) //17:17
#define RG_USB20_CHIRP_EN                         (0x1<<16) //16:16
#define RG_USB20_DM_ABIST_SOURCE_EN               (0x1<<15) //15:15
#define RG_USB20_DM_ABIST_SELE                    (0xf<<8) //11:8
#define RG_USB20_DP_ABIST_SOURCE_EN               (0x1<<7) //7:7
#define RG_USB20_DP_ABIST_SELE                    (0xf<<0) //3:0

//U3D_U2PHYACR2
#define RG_USB20_OTG_ABIST_SELE                   (0x7<<29) //31:29
#define RG_USB20_OTG_ABIST_EN                     (0x1<<28) //28:28
#define RG_USB20_OTG_VBUSCMP_EN                   (0x1<<27) //27:27
#define RG_USB20_OTG_VBUSTH                       (0x7<<24) //26:24
#define RG_USB20_DISC_FIT_EN                      (0x1<<22) //22:22
#define RG_USB20_DISCD                            (0x3<<20) //21:20
#define RG_USB20_DISCTH                           (0xf<<16) //19:16
#define RG_USB20_SQCAL_EN                         (0x1<<15) //15:15
#define RG_USB20_SQCAL                            (0xf<<8) //11:8
#define RG_USB20_SQTH                             (0xf<<0) //3:0

//U3D_U2PHYACR3
#define RG_USB20_HSTX_DBIST                       (0xf<<28) //31:28
#define RG_USB20_HSTX_BIST_EN                     (0x1<<26) //26:26
#define RG_USB20_HSTX_I_EN_MODE                   (0x3<<24) //25:24
#define RG_USB20_HSRX_TMODE_EN                    (0x1<<23) //23:23
#define RG_USB20_HSRX_BIAS_EN_SEL                 (0x3<<20) //21:20
#define RG_USB20_USB11_TMODE_EN                   (0x1<<19) //19:19
#define RG_USB20_TMODE_FS_LS_TX_EN                (0x1<<18) //18:18
#define RG_USB20_TMODE_FS_LS_RCV_EN               (0x1<<17) //17:17
#define RG_USB20_TMODE_FS_LS_MODE                 (0x1<<16) //16:16
#define RG_USB20_HS_TERM_EN_MODE                  (0x3<<13) //14:13
#define RG_USB20_PUPD_BIST_EN                     (0x1<<12) //12:12
#define RG_USB20_EN_PU_DM                         (0x1<<11) //11:11
#define RG_USB20_EN_PD_DM                         (0x1<<10) //10:10
#define RG_USB20_EN_PU_DP                         (0x1<<9) //9:9
#define RG_USB20_EN_PD_DP                         (0x1<<8) //8:8
#define RG_USB20_PHY_REV                          (0xff<<0) //7:0

//U3D_U2PHYACR4
#define RG_USB20_DP_100K_MODE                     (0x1<<18) //18:18
#define RG_USB20_DM_100K_EN                       (0x1<<17) //17:17
#define USB20_DP_100K_EN                          (0x1<<16) //16:16
#define USB20_GPIO_DM_I                           (0x1<<15) //15:15
#define USB20_GPIO_DP_I                           (0x1<<14) //14:14
#define USB20_GPIO_DM_OE                          (0x1<<13) //13:13
#define USB20_GPIO_DP_OE                          (0x1<<12) //12:12
#define RG_USB20_GPIO_CTL                         (0x1<<9) //9:9
#define USB20_GPIO_MODE                           (0x1<<8) //8:8
#define RG_USB20_TX_BIAS_EN                       (0x1<<5) //5:5
#define RG_USB20_TX_VCMPDN_EN                     (0x1<<4) //4:4
#define RG_USB20_HS_SQ_EN_MODE                    (0x3<<2) //3:2
#define RG_USB20_HS_RCV_EN_MODE                   (0x3<<0) //1:0

//U3D_U2PHYAMON0
#define RGO_USB20_GPIO_DM_O                       (0x1<<1) //1:1
#define RGO_USB20_GPIO_DP_O                       (0x1<<0) //0:0

//U3D_U2PHYDCR0
#define RG_USB20_CDR_TST                          (0x3<<30) //31:30
#define RG_USB20_GATED_ENB                        (0x1<<29) //29:29
#define RG_USB20_TESTMODE                         (0x3<<26) //27:26
#define RG_USB20_PLL_STABLE                       (0x1<<25) //25:25
#define RG_USB20_PLL_FORCE_ON                     (0x1<<24) //24:24
#define RG_USB20_PHYD_RESERVE                     (0xffff<<8) //23:8
#define RG_USB20_EBTHRLD                          (0x1<<7) //7:7
#define RG_USB20_EARLY_HSTX_I                     (0x1<<6) //6:6
#define RG_USB20_TX_TST                           (0x1<<5) //5:5
#define RG_USB20_NEGEDGE_ENB                      (0x1<<4) //4:4
#define RG_USB20_CDR_FILT                         (0xf<<0) //3:0

//U3D_U2PHYDCR1
#define RG_USB20_PROBE_SEL                        (0xff<<24) //31:24
#define RG_USB20_DRVVBUS                          (0x1<<23) //23:23
#define RG_DEBUG_EN                               (0x1<<22) //22:22
#define RG_USB20_OTG_PROBE                        (0x3<<20) //21:20
#define RG_USB20_SW_PLLMODE                       (0x3<<18) //19:18
#define RG_USB20_BERTH                            (0x3<<16) //17:16
#define RG_USB20_LBMODE                           (0x3<<13) //14:13
#define RG_USB20_FORCE_TAP                        (0x1<<12) //12:12
#define RG_USB20_TAPSEL                           (0xfff<<0) //11:0

//U3D_U2PHYDTM0
#define RG_UART_MODE                              (0x3<<30) //31:30
#define FORCE_UART_I                              (0x1<<29) //29:29
#define FORCE_UART_BIAS_EN                        (0x1<<28) //28:28
#define FORCE_UART_TX_OE                          (0x1<<27) //27:27
#define FORCE_UART_EN                             (0x1<<26) //26:26
#define FORCE_USB_CLKEN                           (0x1<<25) //25:25
#define FORCE_DRVVBUS                             (0x1<<24) //24:24
#define FORCE_DATAIN                              (0x1<<23) //23:23
#define FORCE_TXVALID                             (0x1<<22) //22:22
#define FORCE_DM_PULLDOWN                         (0x1<<21) //21:21
#define FORCE_DP_PULLDOWN                         (0x1<<20) //20:20
#define FORCE_XCVRSEL                             (0x1<<19) //19:19
#define FORCE_SUSPENDM                            (0x1<<18) //18:18
#define FORCE_TERMSEL                             (0x1<<17) //17:17
#define FORCE_OPMODE                              (0x1<<16) //16:16
#define UTMI_MUXSEL                               (0x1<<15) //15:15
#define RG_RESET                                  (0x1<<14) //14:14
#define RG_DATAIN                                 (0xf<<10) //13:10
#define RG_TXVALIDH                               (0x1<<9) //9:9
#define RG_TXVALID                                (0x1<<8) //8:8
#define RG_DMPULLDOWN                             (0x1<<7) //7:7
#define RG_DPPULLDOWN                             (0x1<<6) //6:6
#define RG_XCVRSEL                                (0x3<<4) //5:4
#define RG_SUSPENDM                               (0x1<<3) //3:3
#define RG_TERMSEL                                (0x1<<2) //2:2
#define RG_OPMODE                                 (0x3<<0) //1:0

//U3D_U2PHYDTM1
#define RG_USB20_PRBS7_EN                         (0x1<<31) //31:31
#define RG_USB20_PRBS7_BITCNT                     (0x3f<<24) //29:24
#define RG_USB20_CLK48M_EN                        (0x1<<23) //23:23
#define RG_USB20_CLK60M_EN                        (0x1<<22) //22:22
#define RG_UART_I                                 (0x1<<19) //19:19
#define RG_UART_BIAS_EN                           (0x1<<18) //18:18
#define RG_UART_TX_OE                             (0x1<<17) //17:17
#define RG_UART_EN                                (0x1<<16) //16:16
#define FORCE_VBUSVALID                           (0x1<<13) //13:13
#define FORCE_SESSEND                             (0x1<<12) //12:12
#define FORCE_BVALID                              (0x1<<11) //11:11
#define FORCE_AVALID                              (0x1<<10) //10:10
#define FORCE_IDDIG                               (0x1<<9) //9:9
#define FORCE_IDPULLUP                            (0x1<<8) //8:8
#define RG_VBUSVALID                              (0x1<<5) //5:5
#define RG_SESSEND                                (0x1<<4) //4:4
#define RG_BVALID                                 (0x1<<3) //3:3
#define RG_AVALID                                 (0x1<<2) //2:2
#define RG_IDDIG                                  (0x1<<1) //1:1
#define RG_IDPULLUP                               (0x1<<0) //0:0

//U3D_U2PHYDMON0
#define RG_USB20_PRBS7_BERTH                      (0xff<<0) //7:0

//U3D_U2PHYDMON1
#define USB20_UART_O                              (0x1<<31) //31:31
#define RGO_USB20_LB_PASS                         (0x1<<30) //30:30
#define RGO_USB20_LB_DONE                         (0x1<<29) //29:29
#define AD_USB20_BVALID                           (0x1<<28) //28:28
#define USB20_IDDIG                               (0x1<<27) //27:27
#define AD_USB20_VBUSVALID                        (0x1<<26) //26:26
#define AD_USB20_SESSEND                          (0x1<<25) //25:25
#define AD_USB20_AVALID                           (0x1<<24) //24:24
#define USB20_LINE_STATE                          (0x3<<22) //23:22
#define USB20_HST_DISCON                          (0x1<<21) //21:21
#define USB20_TX_READY                            (0x1<<20) //20:20
#define USB20_RX_ERROR                            (0x1<<19) //19:19
#define USB20_RX_ACTIVE                           (0x1<<18) //18:18
#define USB20_RX_VALIDH                           (0x1<<17) //17:17
#define USB20_RX_VALID                            (0x1<<16) //16:16
#define USB20_DATA_OUT                            (0xffff<<0) //15:0

//U3D_U2PHYDMON2
#define RGO_TXVALID_CNT                           (0xff<<24) //31:24
#define RGO_RXACTIVE_CNT                          (0xff<<16) //23:16
#define RGO_USB20_LB_BERCNT                       (0xff<<8) //15:8
#define USB20_PROBE_OUT                           (0xff<<0) //7:0

//U3D_U2PHYDMON3
#define RGO_USB20_PRBS7_ERRCNT                    (0xffff<<16) //31:16
#define RGO_USB20_PRBS7_DONE                      (0x1<<3) //3:3
#define RGO_USB20_PRBS7_LOCK                      (0x1<<2) //2:2
#define RGO_USB20_PRBS7_PASS                      (0x1<<1) //1:1
#define RGO_USB20_PRBS7_PASSTH                    (0x1<<0) //0:0

//U3D_U2PHYBC12C
#define RG_SIFSLV_CHGDT_DEGLCH_CNT                (0xf<<28) //31:28
#define RG_SIFSLV_CHGDT_CTRL_CNT                  (0xf<<24) //27:24
#define RG_SIFSLV_CHGDT_FORCE_MODE                (0x1<<16) //16:16
#define RG_CHGDT_ISRC_LEV                         (0x3<<14) //15:14
#define RG_CHGDT_VDATSRC                          (0x1<<13) //13:13
#define RG_CHGDT_BGVREF_SEL                       (0x7<<10) //12:10
#define RG_CHGDT_RDVREF_SEL                       (0x3<<8) //9:8
#define RG_CHGDT_ISRC_DP                          (0x1<<7) //7:7
#define RG_SIFSLV_CHGDT_OPOUT_DM                  (0x1<<6) //6:6
#define RG_CHGDT_VDAT_DM                          (0x1<<5) //5:5
#define RG_CHGDT_OPOUT_DP                         (0x1<<4) //4:4
#define RG_SIFSLV_CHGDT_VDAT_DP                   (0x1<<3) //3:3
#define RG_SIFSLV_CHGDT_COMP_EN                   (0x1<<2) //2:2
#define RG_SIFSLV_CHGDT_OPDRV_EN                  (0x1<<1) //1:1
#define RG_CHGDT_EN                               (0x1<<0) //0:0

//U3D_U2PHYBC12C1
#define RG_CHGDT_REV                              (0xff<<0) //7:0

//U3D_REGFCOM
#define RG_PAGE                                   (0xff<<24) //31:24
#define I2C_MODE                                  (0x1<<16) //16:16


/* OFFSET  */

//U3D_U2PHYAC0
#define RG_USB20_USBPLL_DIVEN_OFST                (28)
#define RG_USB20_USBPLL_CKCTRL_OFST               (26)
#define RG_USB20_USBPLL_PREDIV_OFST               (24)
#define RG_USB20_USBPLL_FORCE_ON_OFST             (23)
#define RG_USB20_USBPLL_FBDIV_OFST                (16)
#define RG_USB20_REF_EN_OFST                      (15)
#define RG_USB20_INTR_EN_OFST                     (14)
#define RG_USB20_BG_TRIM_OFST                     (8)
#define RG_USB20_BG_RBSEL_OFST                    (6)
#define RG_USB20_BG_RASEL_OFST                    (4)
#define RG_USB20_BGR_DIV_OFST                     (2)
#define RG_SIFSLV_CHP_EN_OFST                     (1)
#define RG_SIFSLV_BGR_EN_OFST                     (0)

//U3D_U2PHYAC1
#define RG_USB20_VRT_VREF_SEL_OFST                (28)
#define RG_USB20_TERM_VREF_SEL_OFST               (24)
#define RG_USB20_MPX_SEL_OFST                     (16)
#define RG_USB20_MPX_OUT_SEL_OFST                 (12)
#define RG_USB20_TX_PH_ROT_SEL_OFST               (8)
#define RG_USB20_USBPLL_ACCEN_OFST                (3)
#define RG_USB20_USBPLL_LF_OFST                   (2)
#define RG_USB20_USBPLL_BR_OFST                   (1)
#define RG_USB20_USBPLL_BP_OFST                   (0)

//U3D_U2PHYAC2
#define RG_SIFSLV_MAC_BANDGAP_EN_OFST             (17)
#define RG_SIFSLV_MAC_CHOPPER_EN_OFST             (16)
#define RG_USB20_CLKREF_REV_OFST                  (0)

//U3D_U2PHYACR0
#define RG_USB20_ICUSB_EN_OFST                    (24)
#define RG_USB20_HSTX_SRCAL_EN_OFST               (23)
#define RG_USB20_HSTX_SRCTRL_OFST                 (16)
#define RG_USB20_LS_CR_OFST                       (12)
#define RG_USB20_FS_CR_OFST                       (8)
#define RG_USB20_LS_SR_OFST                       (4)
#define RG_USB20_FS_SR_OFST                       (0)

//U3D_U2PHYACR1
#define RG_USB20_INIT_SQ_EN_DG_OFST               (28)
#define RG_USB20_SQD_OFST                         (24)
#define RG_USB20_HSTX_TMODE_SEL_OFST              (20)
#define RG_USB20_HSTX_TMODE_EN_OFST               (19)
#define RG_USB20_PHYD_MONEN_OFST                  (18)
#define RG_USB20_INLPBK_EN_OFST                   (17)
#define RG_USB20_CHIRP_EN_OFST                    (16)
#define RG_USB20_DM_ABIST_SOURCE_EN_OFST          (15)
#define RG_USB20_DM_ABIST_SELE_OFST               (8)
#define RG_USB20_DP_ABIST_SOURCE_EN_OFST          (7)
#define RG_USB20_DP_ABIST_SELE_OFST               (0)

//U3D_U2PHYACR2
#define RG_USB20_OTG_ABIST_SELE_OFST              (29)
#define RG_USB20_OTG_ABIST_EN_OFST                (28)
#define RG_USB20_OTG_VBUSCMP_EN_OFST              (27)
#define RG_USB20_OTG_VBUSTH_OFST                  (24)
#define RG_USB20_DISC_FIT_EN_OFST                 (22)
#define RG_USB20_DISCD_OFST                       (20)
#define RG_USB20_DISCTH_OFST                      (16)
#define RG_USB20_SQCAL_EN_OFST                    (15)
#define RG_USB20_SQCAL_OFST                       (8)
#define RG_USB20_SQTH_OFST                        (0)

//U3D_U2PHYACR3
#define RG_USB20_HSTX_DBIST_OFST                  (28)
#define RG_USB20_HSTX_BIST_EN_OFST                (26)
#define RG_USB20_HSTX_I_EN_MODE_OFST              (24)
#define RG_USB20_HSRX_TMODE_EN_OFST               (23)
#define RG_USB20_HSRX_BIAS_EN_SEL_OFST            (20)
#define RG_USB20_USB11_TMODE_EN_OFST              (19)
#define RG_USB20_TMODE_FS_LS_TX_EN_OFST           (18)
#define RG_USB20_TMODE_FS_LS_RCV_EN_OFST          (17)
#define RG_USB20_TMODE_FS_LS_MODE_OFST            (16)
#define RG_USB20_HS_TERM_EN_MODE_OFST             (13)
#define RG_USB20_PUPD_BIST_EN_OFST                (12)
#define RG_USB20_EN_PU_DM_OFST                    (11)
#define RG_USB20_EN_PD_DM_OFST                    (10)
#define RG_USB20_EN_PU_DP_OFST                    (9)
#define RG_USB20_EN_PD_DP_OFST                    (8)
#define RG_USB20_PHY_REV_OFST                     (0)

//U3D_U2PHYACR4
#define RG_USB20_DP_100K_MODE_OFST                (18)
#define RG_USB20_DM_100K_EN_OFST                  (17)
#define USB20_DP_100K_EN_OFST                     (16)
#define USB20_GPIO_DM_I_OFST                      (15)
#define USB20_GPIO_DP_I_OFST                      (14)
#define USB20_GPIO_DM_OE_OFST                     (13)
#define USB20_GPIO_DP_OE_OFST                     (12)
#define RG_USB20_GPIO_CTL_OFST                    (9)
#define USB20_GPIO_MODE_OFST                      (8)
#define RG_USB20_TX_BIAS_EN_OFST                  (5)
#define RG_USB20_TX_VCMPDN_EN_OFST                (4)
#define RG_USB20_HS_SQ_EN_MODE_OFST               (2)
#define RG_USB20_HS_RCV_EN_MODE_OFST              (0)

//U3D_U2PHYAMON0
#define RGO_USB20_GPIO_DM_O_OFST                  (1)
#define RGO_USB20_GPIO_DP_O_OFST                  (0)

//U3D_U2PHYDCR0
#define RG_USB20_CDR_TST_OFST                     (30)
#define RG_USB20_GATED_ENB_OFST                   (29)
#define RG_USB20_TESTMODE_OFST                    (26)
#define RG_USB20_PLL_STABLE_OFST                  (25)
#define RG_USB20_PLL_FORCE_ON_OFST                (24)
#define RG_USB20_PHYD_RESERVE_OFST                (8)
#define RG_USB20_EBTHRLD_OFST                     (7)
#define RG_USB20_EARLY_HSTX_I_OFST                (6)
#define RG_USB20_TX_TST_OFST                      (5)
#define RG_USB20_NEGEDGE_ENB_OFST                 (4)
#define RG_USB20_CDR_FILT_OFST                    (0)

//U3D_U2PHYDCR1
#define RG_USB20_PROBE_SEL_OFST                   (24)
#define RG_USB20_DRVVBUS_OFST                     (23)
#define RG_DEBUG_EN_OFST                          (22)
#define RG_USB20_OTG_PROBE_OFST                   (20)
#define RG_USB20_SW_PLLMODE_OFST                  (18)
#define RG_USB20_BERTH_OFST                       (16)
#define RG_USB20_LBMODE_OFST                      (13)
#define RG_USB20_FORCE_TAP_OFST                   (12)
#define RG_USB20_TAPSEL_OFST                      (0)

//U3D_U2PHYDTM0
#define RG_UART_MODE_OFST                         (30)
#define FORCE_UART_I_OFST                         (29)
#define FORCE_UART_BIAS_EN_OFST                   (28)
#define FORCE_UART_TX_OE_OFST                     (27)
#define FORCE_UART_EN_OFST                        (26)
#define FORCE_USB_CLKEN_OFST                      (25)
#define FORCE_DRVVBUS_OFST                        (24)
#define FORCE_DATAIN_OFST                         (23)
#define FORCE_TXVALID_OFST                        (22)
#define FORCE_DM_PULLDOWN_OFST                    (21)
#define FORCE_DP_PULLDOWN_OFST                    (20)
#define FORCE_XCVRSEL_OFST                        (19)
#define FORCE_SUSPENDM_OFST                       (18)
#define FORCE_TERMSEL_OFST                        (17)
#define FORCE_OPMODE_OFST                         (16)
#define UTMI_MUXSEL_OFST                          (15)
#define RG_RESET_OFST                             (14)
#define RG_DATAIN_OFST                            (10)
#define RG_TXVALIDH_OFST                          (9)
#define RG_TXVALID_OFST                           (8)
#define RG_DMPULLDOWN_OFST                        (7)
#define RG_DPPULLDOWN_OFST                        (6)
#define RG_XCVRSEL_OFST                           (4)
#define RG_SUSPENDM_OFST                          (3)
#define RG_TERMSEL_OFST                           (2)
#define RG_OPMODE_OFST                            (0)

//U3D_U2PHYDTM1
#define RG_USB20_PRBS7_EN_OFST                    (31)
#define RG_USB20_PRBS7_BITCNT_OFST                (24)
#define RG_USB20_CLK48M_EN_OFST                   (23)
#define RG_USB20_CLK60M_EN_OFST                   (22)
#define RG_UART_I_OFST                            (19)
#define RG_UART_BIAS_EN_OFST                      (18)
#define RG_UART_TX_OE_OFST                        (17)
#define RG_UART_EN_OFST                           (16)
#define FORCE_VBUSVALID_OFST                      (13)
#define FORCE_SESSEND_OFST                        (12)
#define FORCE_BVALID_OFST                         (11)
#define FORCE_AVALID_OFST                         (10)
#define FORCE_IDDIG_OFST                          (9)
#define FORCE_IDPULLUP_OFST                       (8)
#define RG_VBUSVALID_OFST                         (5)
#define RG_SESSEND_OFST                           (4)
#define RG_BVALID_OFST                            (3)
#define RG_AVALID_OFST                            (2)
#define RG_IDDIG_OFST                             (1)
#define RG_IDPULLUP_OFST                          (0)

//U3D_U2PHYDMON0
#define RG_USB20_PRBS7_BERTH_OFST                 (0)

//U3D_U2PHYDMON1
#define USB20_UART_O_OFST                         (31)
#define RGO_USB20_LB_PASS_OFST                    (30)
#define RGO_USB20_LB_DONE_OFST                    (29)
#define AD_USB20_BVALID_OFST                      (28)
#define USB20_IDDIG_OFST                          (27)
#define AD_USB20_VBUSVALID_OFST                   (26)
#define AD_USB20_SESSEND_OFST                     (25)
#define AD_USB20_AVALID_OFST                      (24)
#define USB20_LINE_STATE_OFST                     (22)
#define USB20_HST_DISCON_OFST                     (21)
#define USB20_TX_READY_OFST                       (20)
#define USB20_RX_ERROR_OFST                       (19)
#define USB20_RX_ACTIVE_OFST                      (18)
#define USB20_RX_VALIDH_OFST                      (17)
#define USB20_RX_VALID_OFST                       (16)
#define USB20_DATA_OUT_OFST                       (0)

//U3D_U2PHYDMON2
#define RGO_TXVALID_CNT_OFST                      (24)
#define RGO_RXACTIVE_CNT_OFST                     (16)
#define RGO_USB20_LB_BERCNT_OFST                  (8)
#define USB20_PROBE_OUT_OFST                      (0)

//U3D_U2PHYDMON3
#define RGO_USB20_PRBS7_ERRCNT_OFST               (16)
#define RGO_USB20_PRBS7_DONE_OFST                 (3)
#define RGO_USB20_PRBS7_LOCK_OFST                 (2)
#define RGO_USB20_PRBS7_PASS_OFST                 (1)
#define RGO_USB20_PRBS7_PASSTH_OFST               (0)


///////////////////////////////////////////////////////////////////////////////

struct sifslv_fm_feg {
//0x0
PHY_LE32 fmcr0;
PHY_LE32 fmcr1;
PHY_LE32 fmcr2;
PHY_LE32 fmmonr0;
//0x10
PHY_LE32 fmmonr1;
};

//U3D_FMCR0
#define RG_LOCKTH                                 (0xf<<28) //31:28
#define RG_MONCLK_SEL                             (0x3<<26) //27:26
#define RG_FM_MODE                                (0x1<<25) //25:25
#define RG_FREQDET_EN                             (0x1<<24) //24:24
#define RG_CYCLECNT                               (0xffffff<<0) //23:0

//U3D_FMCR1
#define RG_TARGET                                 (0xffffffff<<0) //31:0

//U3D_FMCR2
#define RG_OFFSET                                 (0xffffffff<<0) //31:0

//U3D_FMMONR0
#define USB_FM_OUT                                (0xffffffff<<0) //31:0

//U3D_FMMONR1
#define RG_MONCLK_SEL_3                           (0x1<<9) //9:9
#define RG_FRCK_EN                                (0x1<<8) //8:8
#define USBPLL_LOCK                               (0x1<<1) //1:1
#define USB_FM_VLD                                (0x1<<0) //0:0


/* OFFSET */

//U3D_FMCR0
#define RG_LOCKTH_OFST                            (28)
#define RG_MONCLK_SEL_OFST                        (26)
#define RG_FM_MODE_OFST                           (25)
#define RG_FREQDET_EN_OFST                        (24)
#define RG_CYCLECNT_OFST                          (0)

//U3D_FMCR1
#define RG_TARGET_OFST                            (0)

//U3D_FMCR2
#define RG_OFFSET_OFST                            (0)

//U3D_FMMONR0
#define USB_FM_OUT_OFST                           (0)

//U3D_FMMONR1
#define RG_MONCLK_SEL_3_OFST                      (9)
#define RG_FRCK_EN_OFST                           (8)
#define USBPLL_LOCK_OFST                          (1)
#define USB_FM_VLD_OFST                           (0)


///////////////////////////////////////////////////////////////////////////////

PHY_INT32 phy_init(struct u3phy_info *info);
PHY_INT32 phy_change_pipe_phase(struct u3phy_info *info, PHY_INT32 phy_drv, PHY_INT32 pipe_phase);
PHY_INT32 eyescan_init(struct u3phy_info *info);
PHY_INT32 phy_eyescan(struct u3phy_info *info, PHY_INT32 x_t1, PHY_INT32 y_t1, PHY_INT32 x_br, PHY_INT32 y_br, PHY_INT32 delta_x, PHY_INT32 delta_y
		, PHY_INT32 eye_cnt, PHY_INT32 num_cnt, PHY_INT32 PI_cal_en, PHY_INT32 num_ignore_cnt);
PHY_INT32 u2_save_cur_en(struct u3phy_info *info);
PHY_INT32 u2_save_cur_re(struct u3phy_info *info);
PHY_INT32 u2_slew_rate_calibration(struct u3phy_info *info);

#endif
#endif
