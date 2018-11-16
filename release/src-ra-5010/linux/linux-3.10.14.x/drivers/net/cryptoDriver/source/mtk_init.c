
#include "mtk_baseDefs.h"
#include "mtk_hwAccess.h"
#include "mtk_AdapterInternal.h"
#include "mtk_hwDmaAccess.h"

#include <linux/init.h>
#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <asm/mach-ralink/surfboardint.h>
#include "mtk_pecApi.h"
#include <net/mtk_esp.h>
#include <linux/proc_fs.h>

static struct proc_dir_entry *entry;

extern void 
mtk_ipsec_init(
	void
);

static bool Adapter_IsInitialized = false;


static bool
Adapter_Init(
	void
)
{
    if (Adapter_IsInitialized != false)
    {
        printk("Adapter_Init: Already initialized\n");
        return true;
    }


    if (!HWPAL_DMAResource_Init(1024))
    {
		printk("HWPAL_DMAResource_Init failed\n");
       return false;
    }

    if (!Adapter_EIP93_Init())
    {
        printk("Adapter_EIP93_Init failed\n");
		return false;
    }

#ifdef ADAPTER_EIP93PE_INTERRUPTS_ENABLE
    Adapter_Interrupts_Init(SURFBOARDINT_CRYPTO);
#endif

    Adapter_IsInitialized = true;

    return true;
}


static void
Adapter_UnInit(
	void
)
{
    if (!Adapter_IsInitialized)
    {
        printk("Adapter_UnInit: Adapter is not initialized\n");
        return;
    }

    Adapter_IsInitialized = false;



    Adapter_EIP93_UnInit();

#ifdef ADAPTER_EIP93PE_INTERRUPTS_ENABLE
    Adapter_Interrupts_UnInit();
#endif

    HWPAL_DMAResource_UnInit();
}
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,10,14)
static int mcrypto_proc_read(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
    int len, i;
    if (off > 0)
    {
        return 0;
    }
    for (i = 0; i < 10; i++)
		len = sprintf(buf, "ipicpu[%d] : %d\n", i,mcrypto_proc.ipicpu[i]);
    len = sprintf(buf, "expand : %d\n", mcrypto_proc.copy_expand_count);
    len += sprintf(buf + len, "nolinear packet : %d\n", mcrypto_proc.nolinear_count);
    len += sprintf(buf + len, "oom putpacket : %d\n", mcrypto_proc.oom_in_put);
    for (i = 0; i < 16; i++)
    	len += sprintf(buf + len, "skbq[%d] : %d\n", i, mcrypto_proc.qlen[i]);
    for (i = 0; i < 16; i++)
    	len += sprintf(buf + len, "dbgpt[%d] : %d\n", i, mcrypto_proc.dbg_pt[i]);	
    return len;
}
#endif

int
VDriver_Init(
	void
)
{
    int i;
    if (!Adapter_Init())
    {
		printk("\n !Adapter_Init failed! \n");
        return -1;
    }

	if (PEC_Init(NULL) == PEC_ERROR_BAD_USE_ORDER)
	{
		printk("\n !PEC is initialized already! \n");
		return -1;
	}
#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,36)
	entry = create_proc_entry(PROCNAME, 0666, NULL);
	if (entry == NULL)
	{
		printk("HW Crypto : unable to create /proc entry\n");
		return -1;
	}
	entry->read_proc = mcrypto_proc_read;
	entry->write_proc = NULL;
#endif	
	memset(&mcrypto_proc, 0, sizeof(mcrypto_proc_type));
	for (i = 0 ; i < 10 ; i++)
  		mcrypto_proc.ipicpu[i] = -1;
    
	mtk_ipsec_init();
    
    return 0;   // success
}



void
VDriver_Exit(
	void
)
{
    Adapter_UnInit();
	
	PEC_UnInit();
#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,36)
	remove_proc_entry(PROCNAME, entry);
#endif	
}

MODULE_LICENSE("Proprietary");

module_init(VDriver_Init);
module_exit(VDriver_Exit);
