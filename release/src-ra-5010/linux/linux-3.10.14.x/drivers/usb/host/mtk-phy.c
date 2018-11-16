#include <linux/gfp.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#define U3_PHY_LIB
#include "mtk-phy.h"
#ifdef CONFIG_C60802_SUPPORT
#include "mtk-phy-c60802.h"
#endif
#ifdef CONFIG_D60802_SUPPORT
#include "mtk-phy-d60802.h"
#endif
#ifdef CONFIG_PROJECT_7662
#include "mtk-phy-7662.h"
#endif
#ifdef CONFIG_PROJECT_5399
#include "mtk-phy-5399.h"
#endif
#ifdef CONFIG_PROJECT_7621
#include "mtk-phy-7621.h"
#endif
#ifdef CONFIG_PROJECT_7628
#include "mtk-phy-7628.h"
#endif
#ifdef CONFIG_C60802_SUPPORT
static const struct u3phy_operator c60802_operators = {
	.init = phy_init_c60802,
	.change_pipe_phase = phy_change_pipe_phase_c60802,
	.eyescan_init = eyescan_init_c60802,
	.eyescan = phy_eyescan_c60802,
	.u2_save_current_entry = u2_save_cur_en_c60802,
	.u2_save_current_recovery = u2_save_cur_re_c60802,
	.u2_slew_rate_calibration = u2_slew_rate_calibration_c60802,
};
#endif
#ifdef CONFIG_D60802_SUPPORT
static const struct u3phy_operator d60802_operators = {
	.init = phy_init_d60802,
	.change_pipe_phase = phy_change_pipe_phase_d60802,
	.eyescan_init = eyescan_init_d60802,
	.eyescan = phy_eyescan_d60802,
	//.u2_save_current_entry = u2_save_cur_en_d60802,
	//.u2_save_current_recovery = u2_save_cur_re_d60802,	
	.u2_slew_rate_calibration = u2_slew_rate_calibration_d60802,
};
#endif
#ifdef CONFIG_PROJECT_PHY
static struct u3phy_operator project_operators = {
	.init = phy_init,
	.change_pipe_phase = phy_change_pipe_phase,
	.eyescan_init = eyescan_init,
	.eyescan = phy_eyescan,
	.u2_slew_rate_calibration = u2_slew_rate_calibration,
};
#endif


PHY_INT32 u3phy_init(){
#ifndef CONFIG_PROJECT_PHY
	PHY_INT32 u3phy_version;
#endif
	
	if (u3phy != NULL)
		return PHY_TRUE;

	u3phy = kmalloc(sizeof(struct u3phy_info), GFP_NOIO);
	if (u3phy == NULL)
		return PHY_FALSE;

#if defined (CONFIG_USB_MT7621_XHCI_PLATFORM)
	u3phy_p1 = kmalloc(sizeof(struct u3phy_info), GFP_NOIO);
	if (u3phy_p1 == NULL)
		return PHY_FALSE;
#endif
#ifdef CONFIG_U3_PHY_GPIO_SUPPORT
	u3phy->phyd_version_addr = 0x2000e4;
#if defined (CONFIG_USB_MT7621_XHCI_PLATFORM)
	u3phy_p1->phyd_version_addr = 0x2000e4;
#endif
#else
#if defined (CONFIG_RALINK_MT7628)
	u3phy->phyd_version_addr = U2_PHY_BASE + 0xf0;
	printk("******MT7628 mtk phy\n");
#else
	u3phy->phyd_version_addr = U3_PHYD_B2_BASE + 0xe4;
#if defined (CONFIG_USB_MT7621_XHCI_PLATFORM)
	u3phy_p1->phyd_version_addr = U3_PHYD_B2_BASE_P1 + 0xe4;
#endif
#endif
#endif

#ifdef CONFIG_PROJECT_PHY
	printk("*****run project phy.\n");
	u3phy->u2phy_regs = (struct u2phy_reg *)U2_PHY_BASE;
#if !defined (CONFIG_RALINK_MT7628)
	u3phy->u3phyd_regs = (struct u3phyd_reg *)U3_PHYD_BASE;
	u3phy->u3phyd_bank2_regs = (struct u3phyd_bank2_reg *)U3_PHYD_B2_BASE;
	u3phy->u3phya_regs = (struct u3phya_reg *)U3_PHYA_BASE;
	u3phy->u3phya_da_regs = (struct u3phya_da_reg *)U3_PHYA_DA_BASE;
	u3phy->sifslv_chip_regs = (struct sifslv_chip_reg *)SIFSLV_CHIP_BASE;		
#endif
	u3phy->sifslv_fm_regs = (struct sifslv_fm_feg *)SIFSLV_FM_FEG_BASE;	
	u3phy_ops = &project_operators;

#if defined (CONFIG_USB_MT7621_XHCI_PLATFORM)
	u3phy_p1->u2phy_regs = (struct u2phy_reg *)U2_PHY_BASE_P1;
	u3phy_p1->u3phyd_regs = (struct u3phyd_reg *)U3_PHYD_BASE_P1;
	u3phy_p1->u3phyd_bank2_regs = (struct u3phyd_bank2_reg *)U3_PHYD_B2_BASE_P1;
	u3phy_p1->u3phya_regs = (struct u3phya_reg *)U3_PHYA_BASE_P1;
	u3phy_p1->u3phya_da_regs = (struct u3phya_da_reg *)U3_PHYA_DA_BASE_P1;
	u3phy_p1->sifslv_chip_regs = (struct sifslv_chip_reg *)SIFSLV_CHIP_BASE;
	u3phy_p1->sifslv_fm_regs = (struct sifslv_fm_feg *)SIFSLV_FM_FEG_BASE;
#endif
#else
	
	//parse phy version
	u3phy_version = U3PhyReadReg32(u3phy->phyd_version_addr);
	printk(KERN_ERR "phy version: %x\n", u3phy_version);
	u3phy->phy_version = u3phy_version;

	if(u3phy_version == 0xc60802a){
	#ifdef CONFIG_C60802_SUPPORT	
	#ifdef CONFIG_U3_PHY_GPIO_SUPPORT
		u3phy->u2phy_regs_c = 0x0;
		u3phy->u3phyd_regs_c = 0x100000;
		u3phy->u3phyd_bank2_regs_c = 0x200000;
		u3phy->u3phya_regs_c = 0x300000;
		u3phy->u3phya_da_regs_c = 0x400000;
		u3phy->sifslv_chip_regs_c = 0x500000;
		u3phy->sifslv_fm_regs_c = 0xf00000;
	#else
		u3phy->u2phy_regs_c = U2_PHY_BASE;
		u3phy->u3phyd_regs_c = U3_PHYD_BASE;
		u3phy->u3phyd_bank2_regs_c = U3_PHYD_B2_BASE;
		u3phy->u3phya_regs_c = U3_PHYA_BASE;
		u3phy->u3phya_da_regs_c = U3_PHYA_DA_BASE;
		u3phy->sifslv_chip_regs_c = SIFSLV_CHIP_BASE;		
		u3phy->sifslv_fm_regs_c = SIFSLV_FM_FEG_BASE;		
	#endif
		u3phy_ops = &c60802_operators;
	#endif
	}
	else if(u3phy_version == 0xd60802a){
	#ifdef CONFIG_D60802_SUPPORT
	#ifdef CONFIG_U3_PHY_GPIO_SUPPORT
		u3phy->u2phy_regs_d = 0x0;
		u3phy->u3phyd_regs_d = 0x100000;
		u3phy->u3phyd_bank2_regs_d = 0x200000;
		u3phy->u3phya_regs_d = 0x300000;
		u3phy->u3phya_da_regs_d = 0x400000;
		u3phy->sifslv_chip_regs_d = 0x500000;
		u3phy->sifslv_fm_regs_d = 0xf00000;		
	#else
		u3phy->u2phy_regs_d = U2_PHY_BASE;
		u3phy->u3phyd_regs_d = U3_PHYD_BASE;
		u3phy->u3phyd_bank2_regs_d = U3_PHYD_B2_BASE;
		u3phy->u3phya_regs_d = U3_PHYA_BASE;
		u3phy->u3phya_da_regs_d = U3_PHYA_DA_BASE;
		u3phy->sifslv_chip_regs_d = SIFSLV_CHIP_BASE;		
		u3phy->sifslv_fm_regs_d = SIFSLV_FM_FEG_BASE;	
	#endif
		u3phy_ops = &d60802_operators;
	#endif
	}
	else if(u3phy_version == 0xe60802a){
	#ifdef CONFIG_E60802_SUPPORT
	#endif
	}
	else{
		printk(KERN_ERR "No match phy version\n");
		return PHY_FALSE;
	}
	
#endif

	return PHY_TRUE;
}

PHY_INT32 U3PhyWriteField8(PHY_INT32 addr, PHY_INT32 offset, PHY_INT32 mask, PHY_INT32 value){
	PHY_INT8 cur_value;
	PHY_INT8 new_value;

	cur_value = U3PhyReadReg8(addr);
	new_value = (cur_value & (~mask)) | (value << offset);
	//udelay(i2cdelayus);
	U3PhyWriteReg8(addr, new_value);
	return PHY_TRUE;
}

PHY_INT32 U3PhyWriteField32(PHY_INT32 addr, PHY_INT32 offset, PHY_INT32 mask, PHY_INT32 value){
	PHY_INT32 cur_value;
	PHY_INT32 new_value;

	cur_value = U3PhyReadReg32(addr);
	new_value = (cur_value & (~mask)) | ((value << offset) & mask);
	U3PhyWriteReg32(addr, new_value);
	//DRV_MDELAY(100);

	return PHY_TRUE;
}

PHY_INT32 U3PhyReadField8(PHY_INT32 addr,PHY_INT32 offset,PHY_INT32 mask){
	
	return ((U3PhyReadReg8(addr) & mask) >> offset);
}

PHY_INT32 U3PhyReadField32(PHY_INT32 addr, PHY_INT32 offset, PHY_INT32 mask){

	return ((U3PhyReadReg32(addr) & mask) >> offset);
}

