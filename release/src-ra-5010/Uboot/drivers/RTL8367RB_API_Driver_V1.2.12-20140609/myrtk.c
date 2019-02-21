#include <rtk_api.h>
#include <rtk_api_ext.h>
#include <rtl8367b_asicdrv_port.h>

#include <rt_mmap.h>
#include <common.h>		/* for cpu_to_le32() and cpu_to_le32() */
#include <command.h>
#include <ralink_gpio.h>
#include <smi.h>

#if defined(SMI_SCK_GPIO)
#define SMI_SCK	SMI_SCK_GPIO	/* Use SMI_SCK_GPIO as SMI_SCK */
#else
#define SMI_SCK	2		/* Use SMI_SCK/GPIO#2 as SMI_SCK */
#endif

#if defined(SMI_SDA_GPIO)
#define SMI_SDA	SMI_SDA_GPIO	/* Use SMI_SDA_GPIO as SMI_SDA */
#else
#define SMI_SDA	1		/* Use SMI_SDA/GPIO#1 as SMI_SDA */
#endif

#define	PHY_CONTROL_REG			0
#define	CONTROL_REG_PORT_POWER_BIT	0x800

extern int rtl8367r_switch_inited;

int LANWANPartition_8367r(void)
{
	int r, ret = RT_ERR_OK;

	// connect CPU port to all LAN port
	rtk_portmask_t portmask;
	portmask.bits[0]=0x1f;
	r = rtk_led_enable_set(LED_GROUP_0, portmask);
	if (r != RT_ERR_OK)
		ret = r;
	r = rtk_led_enable_set(LED_GROUP_1, portmask);
	if (r != RT_ERR_OK)
		ret = r;
	r = rtk_led_operation_set(LED_OP_PARALLEL);
	if (r != RT_ERR_OK)
		ret = r;
	r = rtk_led_groupConfig_set(LED_GROUP_0, LED_CONFIG_SPD10010ACT);
	if (r != RT_ERR_OK)
		ret = r;
	r = rtk_led_groupConfig_set(LED_GROUP_1, LED_CONFIG_SPD1000ACT);
	if (r != RT_ERR_OK)
		ret = r;

	r = rtk_cpu_enable_set(ENABLE);
	if (r != RT_ERR_OK)
		ret = r;
	r = rtk_cpu_tagPort_set(RTK_EXT_1_MAC,CPU_INSERT_TO_NONE);
	if (r != RT_ERR_OK)
		ret = r;

	return ret;
}

#define CLK_DURATION(clk) udelay(clk)

/* Test control interface between CPU and Realtek switch.
 * @return:
 * 	0:	Success
 *  otherwise:	Fail
 */
int test_smi_signal_and_wait(void)
{
#define MIN_GOOD_COUNT	3
	int i, good = 0, r = -1;
	rtk_uint32 data;
	rtk_api_ret_t retVal;

	for (i = 0; i < 150 && good < MIN_GOOD_COUNT; i++) {
		if ((retVal = rtl8367b_getAsicReg(0x1202, &data)) != RT_ERR_OK)
			printf("error = %d\n", retVal);

		if (data)
			printf("0x%x,", data);
		else
			printf(".");

		if (data == 0x88a8)
			good++;
		else
			good = 0;

		udelay(50000);
	}
	puts("\n");

	if (good >= MIN_GOOD_COUNT)
		r = 0;

	return r;
}

void rtl8367r_reset_switch(void)
{
	int r;
	rtk_api_ret_t retVal;
	u32 data;

	data = le32_to_cpu(*(volatile u32 *)(RALINK_REG_GPIOMODE));

	/* Configure I2C pin as GPIO mode or I2C mode*/
	if (SMI_SCK == 2 || SMI_SDA == 1)
		data |= RALINK_GPIOMODE_I2C;
	else
		data &= ~RALINK_GPIOMODE_I2C;

	/* Configure MDC/MDIO pin as GPIO mode or MDIO mode*/
	if (SMI_SCK == 23 || SMI_SDA == 22)
		data |= RALINK_GPIOMODE_MDIO;
	else
		data &= ~RALINK_GPIOMODE_MDIO;

	*((volatile uint32_t *)(RALINK_REG_GPIOMODE)) = cpu_to_le32(data);

	smi_init(0, SMI_SCK, SMI_SDA);

	test_smi_signal_and_wait();
	retVal = rtl8367b_setAsicRegBits(RTL8367B_REG_CHIP_RESET, RTL8367B_CHIP_RST_MASK, 1);
	printf("Realtek switch chip reset: %d\n", retVal);
}

void rtl8367r_port_power(int port, int powerOn)
{
	rtk_port_phy_data_t pData;

	if(port < 0 || port > 4)
		return;

	rtk_port_phyReg_get(port, PHY_CONTROL_REG, &pData);
	printf("** rtk_port_phyReg_get(%d) = %08x powerOn(%d)\n", port, pData, powerOn);
	if(powerOn)
		pData &= ~CONTROL_REG_PORT_POWER_BIT;
	else
		pData |= CONTROL_REG_PORT_POWER_BIT;
	rtk_port_phyReg_set(port, PHY_CONTROL_REG, pData);
}

int rtl8367r_switch_init_pre(void)
{
	int r;
	rtk_api_ret_t retVal;
	int input_txDelay = 1;
	int input_rxDelay = 2;

	r = test_smi_signal_and_wait();
	if (r) {
		printf("!!!!!! Test control interface between CPU and Realtek switch failed.\n");
		return -1;
	}

	retVal = rtk_switch_init();
	printf("rtk_switch_init(): return %d\n", retVal);
	if (retVal != RT_ERR_OK) return retVal;

	/* RALINK uses RGMII to connect switch IC directly
	 * we need to set the MDIO mode here
	 */
	rtk_port_mac_ability_t mac_cfg;
	mac_cfg.forcemode = MAC_FORCE;
	mac_cfg.speed = SPD_1000M;
	mac_cfg.duplex = FULL_DUPLEX;
	mac_cfg.link = PORT_LINKUP;
	mac_cfg.nway = DISABLED;
	mac_cfg.txpause = ENABLED;
	mac_cfg.rxpause = ENABLED;
	retVal = rtk_port_macForceLinkExt_set (EXT_PORT_1, MODE_EXT_RGMII,&mac_cfg);
	printf("rtk_port_macForceLinkExt_set(EXT_PORT_1): return %d\n", retVal);
	retVal = rtk_port_macForceLinkExt_set (EXT_PORT_2, MODE_EXT_RGMII,&mac_cfg);
	printf("rtk_port_macForceLinkExt_set(EXT_PORT_2): return %d\n", retVal);

	printf("input_txDelay:%d, input_rxDelay:%d\n", input_txDelay, input_rxDelay);
	retVal = rtk_port_rgmiiDelayExt_set(EXT_PORT_1, input_txDelay, input_rxDelay);
	printf("rtk_port_rgmiiDelayExt_set(EXT_PORT_1): return %d\n", retVal);
	retVal = rtk_port_rgmiiDelayExt_set(EXT_PORT_2, input_txDelay, input_rxDelay);
	printf("rtk_port_rgmiiDelayExt_set(EXT_PORT_2): return %d\n", retVal);

	// power down all LAN ports
	// this is to force DHCP IP address new when PC cable connects to LAN port

	rtl8367r_port_power(0, 0);
	rtl8367r_port_power(1, 0);
	rtl8367r_port_power(2, 0);
	rtl8367r_port_power(3, 0);
	rtl8367r_port_power(4, 0);

	{ //avoid MII iNIC packet forward to other ports.
		int port;
		rtk_portmask_t portmask;

		portmask.bits[0] = 1 << RTK_EXT_1_MAC;
		for(port = 0; port <= 4; port++)
		{
			retVal = rtk_port_isolation_set(port, portmask);
			printf("rtk_port_isolation_set(%d, %08x) return %d\n", port, portmask.bits[0], retVal);
		}

		portmask.bits[0] = 0x1F | (1<<RTK_EXT_2_MAC);
		retVal = rtk_port_isolation_set(RTK_EXT_1_MAC, portmask);
		printf("rtk_port_isolation_set(RTK_EXT_1_MAC, %08x) return %d\n", portmask.bits[0], retVal);

		portmask.bits[0] = 1 << RTK_EXT_1_MAC;
		retVal = rtk_port_isolation_set(RTK_EXT_2_MAC, portmask);
		printf("rtk_port_isolation_set(RTK_EXT_2_MAC, %08x) return %d\n", portmask.bits[0], retVal);
	}

	rtl8367r_switch_inited = 1;

	return RT_ERR_OK;
}

int rtl8367r_switch_init(void)
{
	// Power up all LAN ports
	rtl8367r_port_power(0, 1);
	rtl8367r_port_power(1, 1);
	rtl8367r_port_power(2, 1);
	rtl8367r_port_power(3, 1);
	rtl8367r_port_power(4, 1);

	LANWANPartition_8367r();

	rtl8367r_switch_inited = 1;

	return RT_ERR_OK;
}

int rtk_switch_reg_access(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	int cmd = 0, cnt = 1, ret = 0, r, infinit = 0;
	uint32_t reg, data, data1;

	if (!strcmp(argv[0], "rtkswreg.r") && (argc >= 2 && argc <= 3)) {
		cmd = 1;
		if (argc == 3)
			cnt = simple_strtoul(argv[2], NULL, 0);
	} else if (!strcmp(argv[0], "rtkswreg.w") && (argc >= 3 && argc <= 4)) {
		cmd = 2;
		if (argc == 4) {
			cnt = simple_strtoul(argv[3], NULL, 0);
		}
	}
	if (cnt >= 99999)
		infinit = 1;

	if (!infinit && cnt > 1)
		printf("Repeat rtkswreg command %d times\n", cnt);
	while (infinit || cnt-- > 0) {
		switch (cmd) {
		case 1:
			reg = simple_strtoul(argv[1], NULL, 0);
			r = smi_read(reg, &data);
			if (r == RT_ERR_OK)
				printf("Realtek Switch register 0x%x = 0x%x\n", reg, data);
			else {
				printf("%s() smi_read(0x%x) failed. return %d\n", __func__, reg, r);
				ret = 2;
			}
			break;
		case 2:
			reg = simple_strtoul(argv[1], NULL, 0);
			data = simple_strtoul(argv[2], NULL, 0);
			data1 = ~data;
			r = smi_write(reg, data);
			if (r == RT_ERR_OK)
				printf("Realtek Switch register 0x%x = 0x%x\n", reg, data);
			else {
				printf("%s() smi_write(0x%x) failed. return %d\n", __func__, reg, r);
				ret = 3;
			}

			/* Verify */
			if (!ret) {
				r = smi_read(reg, &data1);
				if (r == RT_ERR_OK && data == data1) {
					printf("Write 0x%x to Realtek Switch register 0x%x. Verify OK.\n", data, reg);
				}
				else if (r == RT_ERR_OK) {
					printf("Write 0x%x to Realtek Switch register 0x%x. Got 0x%x. Mismatch.\n", data, reg, data1);
					ret = 4;
				} else {
					printf("%s() smi_read(0x%x) failed. return %d\n", __func__, reg, r);
					ret = 5;
				}
			}
			break;
		default:
#ifdef	CFG_LONGHELP
			printf ("%s\n%s\n", cmdtp->usage, cmdtp->help);
#else
			printf ("Usage:\n%s\n", cmdtp->usage);
#endif
			cnt = 0;
			ret = 1;
		}
	}

	return ret;
}

U_BOOT_CMD(
	rtkswreg,	4,	1,	rtk_switch_reg_access,
	"rtkswreg - Read/Write Realtek Switch register through MDIO interface\n",
	"Usage:\n"
	"rtkswreg.r register_address [count]\n"
	"rtkswreg.w register_address data [count]\n"
);
