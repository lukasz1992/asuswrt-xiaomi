#include <linux/version.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/skbuff.h>


int (*smb_nf_local_in_hook)(struct sk_buff *skb) = NULL;
int (*smb_nf_pre_routing_hook)(struct sk_buff *skb) = NULL;
int (*smb_nf_local_out_hook)(struct sk_buff *skb) = NULL;
int (*smb_nf_post_routing_hook)(struct sk_buff *skb) = NULL;
EXPORT_SYMBOL(smb_nf_local_in_hook);
EXPORT_SYMBOL(smb_nf_pre_routing_hook);
EXPORT_SYMBOL(smb_nf_local_out_hook);
EXPORT_SYMBOL(smb_nf_post_routing_hook);


