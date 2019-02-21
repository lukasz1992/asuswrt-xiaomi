/* Copyright (C) 2010 - 2013 Broadcom Corporation.
 * Portions Copyright (C) VMware, Inc. 2007-2011. All Rights Reserved.
 */

struct tg3;

#define TG3_MSG_HW                    0x0010000 /* was: NETIF_MSG_HW */
#define TG3_MSG_NETQ                  0x0400000
#define TG3_MSG_NETQ_VERBOSE          0x0800000
/* regular debug print for VMware specific code */
#define DP(__mask, fmt, ...)                                    \
do {                                                            \
	if (unlikely(tp->msg_enable & (__mask)))         \
		netdev_info(tp->dev, "[%s:%d]" fmt,             \
			  __func__, __LINE__,                   \
			  ##__VA_ARGS__);                       \
} while (0)

/* for errors (never masked) */
#define DP_ERR(fmt, ...)                                        \
do {                                                            \
        netdev_err(tp->dev, "[%s:%d]" fmt,                      \
               __func__, __LINE__,                              \
               ##__VA_ARGS__);                                  \
} while (0)

/*
 *  On ESX the wmb() instruction is defined to only a compiler barrier.
 *  The macro wmb() needs to be overridden to properly synchronize memory.
 */
#if defined(__VMKLNX__)
#undef wmb
#define wmb()   asm volatile("sfence" ::: "memory")
#endif

static int psod_on_tx_timeout = 0;

module_param(psod_on_tx_timeout, int, 0);
MODULE_PARM_DESC(psod_on_tx_timeout, "For debugging purposes, crash the system "
                                     " when a tx timeout occurs");
#define TG3_MAX_NIC		32

#ifndef TG3_VMWARE_NETQ_DISABLE
#define TG3_VMWARE_NETQ_ENABLE

#define TG3_OPTION_UNSET	-1

static unsigned int __devinitdata tg3_netq_index;
static int __devinitdata tg3_netq_force[TG3_MAX_NIC+1] =
			 { [0 ... TG3_MAX_NIC] = TG3_OPTION_UNSET };

module_param_array_named(force_netq, tg3_netq_force, int, NULL, 0);
MODULE_PARM_DESC(force_netq,
"Force the maximum number of NetQueues available per port (NetQueue capable devices only)");

static const struct {
	const char string[ETH_GSTRING_LEN];
} tg3_vmware_ethtool_stats_keys[] = {
	{ "[0]: rx_packets (sw)" },
	{ "[0]: rx_packets (hw)" },
	{ "[0]: rx_bytes (sw)" },
	{ "[0]: rx_bytes (hw)" },
	{ "[0]: rx_errors (sw)" },
	{ "[0]: rx_errors (hw)" },
	{ "[0]: rx_crc_errors" },
	{ "[0]: rx_frame_errors" },
	{ "[0]: tx_bytes" },
	{ "[0]: tx_ucast_packets" },
	{ "[0]: tx_mcast_packets" },
	{ "[0]: tx_bcast_packets" },
};

/*
 * Pack this structure so that we don't get an extra 8 bytes
 * should this driver be built for a 128-bit CPU. :)
 */
struct tg3_netq_stats {
	u64	rx_packets_sw;
	u64	rx_packets_hw;
	u64	rx_bytes_sw;
	u64	rx_bytes_hw;
	u64	rx_errors_sw;
	u64	rx_errors_hw;
	u64	rx_crc_errors;
	u64	rx_frame_errors;
	u64	tx_bytes;
	u64	tx_ucast_packets;
	u64	tx_mcast_packets;
	u64	tx_bcast_packets;
} __attribute__((packed));

#define TG3_NETQ_NUM_STATS	(sizeof(struct tg3_netq_stats)/sizeof(u64))

struct tg3_netq_napi {
	u32			flags;
	#define TG3_NETQ_TXQ_ALLOCATED		0x0001
	#define TG3_NETQ_RXQ_ALLOCATED		0x0002
	#define TG3_NETQ_RXQ_ENABLED		0x0008
	#define TG3_NETQ_TXQ_FREE_STATE		0x0010
	#define TG3_NETQ_RXQ_FREE_STATE		0x0020

	struct tg3_netq_stats	stats;
	struct net_device_stats	net_stats;
};

struct tg3_vmware_netq {
	u16			n_tx_queues_allocated;
	u16			n_rx_queues_allocated;

	u32			index;
};

static void tg3_vmware_fetch_stats(struct tg3 *tp);
static void tg3_disable_prod_rcbs(struct tg3 *tp, u32 ring);
static void tg3_setup_prod_mboxes(struct tg3 *tp, u32 ring);
static void tg3_netq_init(struct tg3 *tp);
static void tg3_netq_free_all_qs(struct tg3 *tp);
static void tg3_netq_invalidate_state(struct tg3 *tp);
static void tg3_netq_restore(struct tg3 *tp);
static void tg3_netq_limit_dflt_queue_counts(struct tg3 *tp);
static u32  tg3_netq_tune_vector_count(struct tg3 *tp);
static int  tg3_netq_stats_size(struct tg3 *tp);
static void tg3_netq_stats_get_strings(struct tg3 *tp, u8 *buf);
static void tg3_netq_stats_get(struct tg3 *tp, u64 *tmp_stats);
static void tg3_netq_stats_clear(struct tg3 *tp);
#endif /* TG3_VMWARE_NETQ_ENABLE */

#if (defined(__VMKLNX__) && VMWARE_ESX_DDK_VERSION >= 55000)
#define FWDMP_REG_DUMP				0
#define FWDMP_APE_DUMP				1
#define FWDMP_SCRATCHPAD_DUMP			2
#define FWDMP_PHY_DUMP				3

#define TG3_FWDMP_SIZE		(170 * 1024)
#define STR2MARKER(x)		((x[3]<<24)+(x[2]<<16)+(x[1]<<8)+(x[0]))
#define FWDMP_MARKER		STR2MARKER("TG3")
#define FWDMP_END_MARKER	STR2MARKER("END")

struct fw_dmp_hdr {
	u32	ver;
	u32	len;
#define NIC_NAME_SIZE		(sizeof(((struct net_device *)0)->name))
	char	name[NIC_NAME_SIZE];
	void	*tp;
	u32	chip_id;
	u32	dmp_size;  /*actual firmware/chip dump size */
	u32	flags;
	u32	reserved1;
};

struct chip_core_dmp_t {
	struct fw_dmp_hdr fw_hdr;
	u32 fw_dmp_buf[(TG3_FWDMP_SIZE -
		sizeof(struct fw_dmp_hdr) + 10)/4];
		/* add few words for temporary overflow */
} chip_core_dmp_t;

#include <vmklinux_9/vmklinux_dump.h>
#define TG3_DUMPNAME "tg3_fwdmp"
static vmklnx_DumpFileHandle tg3_fwdmp_dh;
static 	void	*tg3_fwdmp_va_ptr;
static struct tg3 *fwdmp_tp_ptr[TG3_MAX_NIC+1] =
		{ [0 ... TG3_MAX_NIC] = NULL };
static VMK_ReturnStatus tg3_fwdmp_callback(void *cookie, vmk_Bool liveDump);

#endif /* defined(__VMKLNX__) && (VMWARE_ESX_DDK_VERSION >= 55000) */

struct tg3_vmware {
	u32 rx_mode_reset_counter;

#ifdef TG3_VMWARE_NETQ_ENABLE
	struct tg3_vmware_netq	netq;
#endif
	struct mutex netq_lock;
};

#if !defined(TG3_VMWARE_BMAPILNX_DISABLE)

#include "esx_ioctl.h"

static int
tg3_vmware_ioctl_cim(struct net_device *dev, struct ifreq *ifr);

#endif  /* !TG3_VMWARE_BMAPILNX_DISABLED */

static void tg3_vmware_timer(struct tg3 *tp);
static netdev_features_t tg3_vmware_tune_tso(struct tg3 *tp,
					     netdev_features_t features);
