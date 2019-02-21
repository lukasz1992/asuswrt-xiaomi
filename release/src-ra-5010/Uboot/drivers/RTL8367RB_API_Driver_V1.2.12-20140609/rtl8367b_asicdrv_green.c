/*
 * Copyright (C) 2009 Realtek Semiconductor Corp.
 * All Rights Reserved.
 *
 * This program is the proprietary software of Realtek Semiconductor
 * Corporation and/or its licensors, and only be used, duplicated,
 * modified or distributed under the authorized license from Realtek.
 *
 * ANY USE OF THE SOFTWARE OTHER THAN AS AUTHORIZED UNDER
 * THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED.
 *
 * $Revision: 48858 $
 * $Date: 2014-06-24 20:26:15 +0800 (週二, 24 六月 2014) $
 *
 * Purpose : RTL8367B switch high-level API for RTL8367B
 * Feature : Green ethernet related functions
 *
 */
#include <rtl8367b_asicdrv_green.h>

/* Function Name:
 *      rtl8367b_getAsicGreenPortPage
 * Description:
 *      Get per-Port ingress page usage per second
 * Input:
 *      port 	- Physical port number (0~7)
 *      pPage 	- page number of ingress packet occuping per second
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 *      RT_ERR_PORT_ID  - Invalid port number
 * Note:
 *      Ingress traffic occuping page number per second for high layer green feature usage
 */
ret_t rtl8367b_getAsicGreenPortPage(rtk_uint32 port, rtk_uint32* pPage)
{
    ret_t retVal;
    rtk_uint32 regData;
    rtk_uint32 pageMeter;

    if(port > RTL8367B_PORTIDMAX)
        return RT_ERR_PORT_ID;

    retVal = rtl8367b_getAsicReg(RTL8367B_PAGEMETER_PORT_REG(port), &regData);
    if(retVal != RT_ERR_OK)
        return retVal;

   pageMeter = regData;

    retVal = rtl8367b_getAsicReg(RTL8367B_PAGEMETER_PORT_REG(port) + 1, &regData);
    if(retVal != RT_ERR_OK)
        return retVal;

    pageMeter = pageMeter + (regData << 16);

    *pPage = pageMeter;
    return RT_ERR_OK;
}
/* Function Name:
 *      rtl8367b_setAsicGreenTrafficType
 * Description:
 *      Set traffic type for each priority
 * Input:
 *      priority 	- internal priority (0~7)
 *      traffictype - high/low traffic type, 1:high priority traffic type, 0:low priority traffic type
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 				- Success
 *      RT_ERR_SMI  			- SMI access error
 *      RT_ERR_QOS_INT_PRIORITY - Invalid priority
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicGreenTrafficType(rtk_uint32 priority, rtk_uint32 traffictype)
{

    if(priority > RTL8367B_PRIMAX)
        return RT_ERR_QOS_INT_PRIORITY;

    return rtl8367b_setAsicRegBit(RTL8367B_REG_HIGHPRI_CFG, priority, (traffictype?1:0));
}
/* Function Name:
 *      rtl8367b_getAsicGreenTrafficType
 * Description:
 *      Get traffic type for each priority
 * Input:
 *      priority 	- internal priority (0~7)
 *      pTraffictype - high/low traffic type, 1:high priority traffic type, 0:low priority traffic type
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 				- Success
 *      RT_ERR_SMI  			- SMI access error
 *      RT_ERR_QOS_INT_PRIORITY - Invalid priority
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicGreenTrafficType(rtk_uint32 priority, rtk_uint32* pTraffictype)
{
    if(priority > RTL8367B_PRIMAX)
        return RT_ERR_QOS_INT_PRIORITY;

    return rtl8367b_getAsicRegBit(RTL8367B_REG_HIGHPRI_CFG, priority, pTraffictype);
}

/* Function Name:
 *      rtl8367b_setAsicGreenHighPriorityTraffic
 * Description:
 *      Set indicator which ASIC had received high priority traffic
 * Input:
 *      port 			- Physical port number (0~7)
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 *      RT_ERR_PORT_ID  - Invalid port number
 * Note:
 *      None
 */
ret_t rtl8367b_setAsicGreenHighPriorityTraffic(rtk_uint32 port)
{
    if(port > RTL8367B_PORTIDMAX)
        return RT_ERR_PORT_ID;

    return rtl8367b_setAsicRegBit(RTL8367B_REG_HIGHPRI_INDICATOR, port, 1);
}


/* Function Name:
 *      rtl8367b_getAsicGreenHighPriorityTraffic
 * Description:
 *      Get indicator which ASIC had received high priority traffic or not
 * Input:
 *      port 		- Physical port number (0~7)
 *      pIndicator 	- Have received high priority traffic indicator. If 1 means ASCI had received high priority in 1second checking priod
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 		- Success
 *      RT_ERR_SMI  	- SMI access error
 *      RT_ERR_PORT_ID  - Invalid port number
 * Note:
 *      None
 */
ret_t rtl8367b_getAsicGreenHighPriorityTraffic(rtk_uint32 port, rtk_uint32* pIndicator)
{
    if(port > RTL8367B_PORTIDMAX)
        return RT_ERR_PORT_ID;

    return rtl8367b_getAsicRegBit(RTL8367B_REG_HIGHPRI_INDICATOR, port, pIndicator);
}

/*
@func rtk_int32 | rtl8367b_setAsicGreenEthernet | Set green ethernet function.
@parm rtk_uint32 | green | Green feature function usage 1:enable 0:disable.
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@comm
 	The API can set Green Ethernet function to reduce power consumption. While green feature is enabled, ASIC will automatic
 detect the cable length and then select different power mode for best performance with minimums power consumption. Link down
 ports will enter power savining mode in 10 seconds after the cable disconnected if power saving function is enabled.
*/
ret_t rtl8367b_setAsicGreenEthernet(rtk_uint32 green)
{
    rtk_uint32 port;
    rtk_uint32 data;
    rtk_uint32 checkCounter;
    rtk_uint32 regData;
    rtk_uint32 phy_status;
    ret_t retVal;

    if (green > 1)
        return RT_ERR_INPUT;

    if((retVal = rtl8367b_setAsicReg(0x13C2, 0x0249)) != RT_ERR_OK)
        return retVal;

    if((retVal = rtl8367b_getAsicReg(0x1300, &data)) != RT_ERR_OK)
        return retVal;

    if( (data == 0x0276) || (data == 0x0597) || (data == 0x6367) )
    {
        for(port = 0; port <= RTL8367B_PHY_INTERNALNOMAX; port++)
        {
            /* 0xa420[2:0] */
            if((retVal = rtl8367b_getAsicPHYOCPReg(port, 0xA420, &regData)) != RT_ERR_OK)
                return retVal;

            phy_status = (regData & 0x0007);

            if(phy_status == 3)
            {
                /* 0xb820[4] = 1 */
                if((retVal = rtl8367b_getAsicPHYOCPReg(port, 0xB820, &regData)) != RT_ERR_OK)
                    return retVal;

                regData |= (0x0001 << 4);

                if((retVal = rtl8367b_setAsicPHYOCPReg(port, 0xB820, regData)) != RT_ERR_OK)
                    return retVal;

                /* wait 0xb800[6] = 1 */
                checkCounter = 100;
                while(checkCounter)
                {
                    retVal = rtl8367b_getAsicPHYOCPReg(port, 0xB800, &regData);
                    if( (retVal != RT_ERR_OK) || ((regData & 0x0040) != 0x0040) )
                    {
                        checkCounter --;
                        if(0 == checkCounter)
                        {
                             return RT_ERR_BUSYWAIT_TIMEOUT;
                        }
                    }
                    else
                        checkCounter = 0;
                }
            }

            /* 0xa436 = 0x8011 */
            if((retVal = rtl8367b_setAsicPHYOCPReg(port, 0xA436, 0x8011)) != RT_ERR_OK)
                return retVal;

            /* wr 0xa438[15] = 0: disable, 1: enable */
            if((retVal = rtl8367b_getAsicPHYOCPReg(port, 0xA438, &regData)) != RT_ERR_OK)
                return retVal;

            if(green)
                regData |= 0x8000;
            else
                regData &= 0x7FFF;

            if((retVal = rtl8367b_setAsicPHYOCPReg(port, 0xA438, regData)) != RT_ERR_OK)
                return retVal;

            if(phy_status == 3)
            {
                /* 0xb820[4] = 0  */
                if((retVal = rtl8367b_getAsicPHYOCPReg(port, 0xB820, &regData)) != RT_ERR_OK)
                    return retVal;

                regData &= ~(0x0001 << 4);

                if((retVal = rtl8367b_setAsicPHYOCPReg(port, 0xB820, regData)) != RT_ERR_OK)
                    return retVal;

                /* wait 0xb800[6] = 0 */
                checkCounter = 100;
                while(checkCounter)
                {
                    retVal = rtl8367b_getAsicPHYOCPReg(port, 0xB800, &regData);
                    if( (retVal != RT_ERR_OK) || ((regData & 0x0040) != 0x0000) )
                    {
                        checkCounter --;
                        if(0 == checkCounter)
                        {
                            return RT_ERR_BUSYWAIT_TIMEOUT;
                        }
                    }
                    else
                        checkCounter = 0;
                }
            }
        }
    }

    if((retVal = rtl8367b_setAsicRegBit(RTL8367B_REG_PHY_AD,RTL8367B_EN_PHY_GREEN_OFFSET,green)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/*
@func rtk_int32 | rtl8367b_getAsicGreenEthernet | Get green ethernet function.
@parm rtk_uint32 | *green | Green feature function usage 1:enable 0:disable.
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@comm
 	The API can set Green Ethernet function to reduce power consumption. While green feature is enabled, ASIC will automatic
 detect the cable length and then select different power mode for best performance with minimums power consumption. Link down
 ports will enter power savining mode in 10 seconds after the cable disconnected if power saving function is enabled.
*/
ret_t rtl8367b_getAsicGreenEthernet(rtk_uint32* green)
{
	return rtl8367b_getAsicRegBit(RTL8367B_REG_PHY_AD,RTL8367B_EN_PHY_GREEN_OFFSET,green);
}


/*
@func ret_t | rtl8367b_setAsicPowerSaving | Set power saving mode
@parm rtk_uint32 | phy | phy number
@parm rtk_uint32 | enable | enable power saving mode.
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_PORT_ID | Invalid port number.
@comm
    The API can set power saving mode per phy.
*/
ret_t rtl8367b_setAsicPowerSaving(rtk_uint32 phy, rtk_uint32 enable)
{
    rtk_api_ret_t retVal;
    rtk_uint32 phyData;
    rtk_uint32 phyReg;
    rtk_uint32 data;
    rtk_uint32 phy_status;
    rtk_uint32 regData;
    rtk_uint32 checkCounter;

    if(phy > RTL8367B_PHYIDMAX)
        return RT_ERR_PORT_ID;
    if (enable > 1)
        return RT_ERR_INPUT;

    if((retVal = rtl8367b_setAsicReg(0x13C2, 0x0249)) != RT_ERR_OK)
        return retVal;

    if((retVal = rtl8367b_getAsicReg(0x1300, &data)) != RT_ERR_OK)
        return retVal;

    if( (data == 0x0276) || (data == 0x0597) || (data == 0x6367) )
    {
        phyReg = PHY_POWERSAVING_REG + 3;

        /* 0xa420[2:0] */
        if((retVal = rtl8367b_getAsicPHYOCPReg(phy, 0xA420, &regData)) != RT_ERR_OK)
            return retVal;

        phy_status = (regData & 0x0007);

        if(phy_status == 3)
        {
            /* 0xb820[4] = 1 */
            if((retVal = rtl8367b_getAsicPHYOCPReg(phy, 0xB820, &regData)) != RT_ERR_OK)
                return retVal;

            regData |= (0x0001 << 4);

            if((retVal = rtl8367b_setAsicPHYOCPReg(phy, 0xB820, regData)) != RT_ERR_OK)
                return retVal;

            /* wait 0xb800[6] = 1 */
            checkCounter = 100;
            while(checkCounter)
            {
                retVal = rtl8367b_getAsicPHYOCPReg(phy, 0xB800, &regData);
                if( (retVal != RT_ERR_OK) || ((regData & 0x0040) != 0x0040) )
                {
                    checkCounter --;
                    if(0 == checkCounter)
                    {
                         return RT_ERR_BUSYWAIT_TIMEOUT;
                    }
                }
                else
                    checkCounter = 0;
            }
        }

        if ((retVal = rtl8367b_getAsicPHYReg(phy,phyReg,&phyData))!=RT_ERR_OK)
            return retVal;

        phyData = phyData & ~(0x0001 << 2);
        phyData = phyData | (enable << 2);

        if ((retVal = rtl8367b_setAsicPHYReg(phy,phyReg,phyData))!=RT_ERR_OK)
            return retVal;

        if(phy_status == 3)
        {
            /* 0xb820[4] = 0  */
            if((retVal = rtl8367b_getAsicPHYOCPReg(phy, 0xB820, &regData)) != RT_ERR_OK)
                return retVal;

            regData &= ~(0x0001 << 4);

            if((retVal = rtl8367b_setAsicPHYOCPReg(phy, 0xB820, regData)) != RT_ERR_OK)
                return retVal;

            /* wait 0xb800[6] = 0 */
            checkCounter = 100;
            while(checkCounter)
            {
                retVal = rtl8367b_getAsicPHYOCPReg(phy, 0xB800, &regData);
                if( (retVal != RT_ERR_OK) || ((regData & 0x0040) != 0x0000) )
                {
                    checkCounter --;
                    if(0 == checkCounter)
                    {
                        return RT_ERR_BUSYWAIT_TIMEOUT;
                    }
                }
                else
                    checkCounter = 0;
            }
        }
    }
    else
    {
        phyReg = PHY_POWERSAVING_REG;

        if ((retVal = rtl8367b_setAsicPHYReg(phy,RTL8367B_PHY_PAGE_ADDRESS,0))!=RT_ERR_OK)
            return retVal;

        if ((retVal = rtl8367b_getAsicPHYReg(phy,phyReg,&phyData))!=RT_ERR_OK)
            return retVal;

        phyData = (phyData & (~PHY_POWERSAVING_MASK)) | (enable<<PHY_POWERSAVING_OFFSET) ;

        if ((retVal = rtl8367b_setAsicPHYReg(phy,phyReg,phyData))!=RT_ERR_OK)
            return retVal;
    }

    return RT_ERR_OK;
}

/*
@func ret_t | rtl8367b_getAsicPowerSaving | Get power saving mode
@parm rtk_uint32 | port | The port number
@parm rtk_uint32* | enable | enable power saving mode.
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_PORT_ID | Invalid port number.
@comm
    The API can get power saving mode per phy.
*/
ret_t rtl8367b_getAsicPowerSaving(rtk_uint32 phy, rtk_uint32* enable)
{
    rtk_api_ret_t retVal;
    rtk_uint32 phyData;
    rtk_uint32 phyReg;
    rtk_uint32 data;

    if(phy > RTL8367B_PHYIDMAX)
        return RT_ERR_PORT_ID;

    if((retVal = rtl8367b_setAsicReg(0x13C2, 0x0249)) != RT_ERR_OK)
        return retVal;

    if((retVal = rtl8367b_getAsicReg(0x1300, &data)) != RT_ERR_OK)
        return retVal;

    if( (data == 0x0276) || (data == 0x0597) || (data == 0x6367) )
    {
        phyReg = PHY_POWERSAVING_REG + 3;

        if ((retVal = rtl8367b_getAsicPHYReg(phy,phyReg,&phyData))!=RT_ERR_OK)
            return retVal;

        if ((phyData & 0x0004) > 0)
            *enable = 1;
        else
            *enable = 0;
    }
    else
    {
        phyReg = PHY_POWERSAVING_REG;

        if ((retVal = rtl8367b_setAsicPHYReg(phy,RTL8367B_PHY_PAGE_ADDRESS,0))!=RT_ERR_OK)
            return retVal;

        if ((retVal = rtl8367b_getAsicPHYReg(phy,phyReg,&phyData))!=RT_ERR_OK)
            return retVal;

        if ((phyData & PHY_POWERSAVING_MASK) > 0)
            *enable = 1;
        else
            *enable = 0;
    }

    return RT_ERR_OK;
}

