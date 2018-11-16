/****************************************************************************
 * Copyright(c) 2000-2013 Broadcom Corporation, all rights reserved
 *
 * Name: esx_ioctl.h
 *
 * Description: Define data structures and prototypes to access ioctls
 *              supported by driver in VMware ESXi system.
 *
 ****************************************************************************/

#ifndef BRCM_VMWARE_IOCTL_H
#define BRCM_VMWARE_IOCTL_H

#ifdef __cplusplus
extern "C" {
#endif

#define BRCM_VMWARE_CIM_IOCTL  0x89f0

#define BRCM_VMWARE_CIM_CMD_ENABLE_NIC          0x0001
#define BRCM_VMWARE_CIM_CMD_DISABLE_NIC         0x0002
#define BRCM_VMWARE_CIM_CMD_REG_READ            0x0003
#define BRCM_VMWARE_CIM_CMD_REG_WRITE           0x0004
#define BRCM_VMWARE_CIM_CMD_GET_NIC_PARAM       0x0005
#define BRCM_VMWARE_CIM_CMD_GET_NIC_STATUS      0x0006
#define BRCM_VMWARE_CIM_CMD_CFG_REG_READ        0x0007
#define BRCM_VMWARE_CIM_CMD_CFG_REG_WRITE       0x0008

// Access type for Register Read/Write Ioctl
#define BRCM_VMWARE_REG_ACCESS_DIRECT           0x0000
#define BRCM_VMWARE_REG_ACCESS_PCI_CFG          0x0001
#define BRCM_VMWARE_REG_ACCESS_APE_REG          0x0002

struct brcm_vmware_ioctl_reg_read_req
{
    u32 reg_offset;
    u32 reg_value;
    u32 reg_access_type;
} __attribute__((packed));

struct brcm_vmware_ioctl_reg_write_req
{
    u32 reg_offset;
    u32 reg_value;
    u32 reg_access_type;
} __attribute__((packed));

#define BRCM_VMWARE_GET_NIC_PARAM_VERSION   1
struct brcm_vmware_ioctl_get_nic_param_req
{
    u32 version;
    u32 mtu;
    u8  current_mac_addr[8];
} __attribute__((packed));

#define BRCM_VMWARE_INVALID_NIC_STATUS  0xffffffff
struct brcm_vmware_ioctl_get_nic_status_req
{
    u32 nic_status; // 1: Up, 0: Down
} __attribute__((packed));

struct brcm_vmware_ioctl_req
{
    u32 cmd;
    union {
        // no struct for reset_nic command
        struct brcm_vmware_ioctl_reg_read_req reg_read_req;
        struct brcm_vmware_ioctl_reg_write_req reg_write_req;
        struct brcm_vmware_ioctl_get_nic_param_req get_nic_param_req;
        struct brcm_vmware_ioctl_get_nic_status_req get_nic_status_req;
    } cmd_req;
} __attribute__((packed));

#ifdef __cplusplus
};
#endif

#endif
