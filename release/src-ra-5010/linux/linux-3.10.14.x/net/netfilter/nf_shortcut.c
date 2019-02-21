#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/types.h>

#include <linux/inetdevice.h>
#include <linux/tcp.h>
#include <linux/ip.h>
#include <net/tcp.h>
#include <net/ip.h>

extern int (*smb_nf_local_in_hook)(struct sk_buff *skb);
extern int (*smb_nf_pre_routing_hook)(struct sk_buff *skb);
extern int (*smb_nf_local_out_hook)(struct sk_buff *skb);
extern int (*smb_nf_post_routing_hook)(struct sk_buff *skb);

struct net_device *lan_int = NULL;
struct in_ifaddr *lan_ifa = NULL;


int mtk_smb_nf_local_in_hook(struct sk_buff *skb)
{
	struct iphdr *iph = ip_hdr(skb);

	if (skb->protocol == htons(ETH_P_IP)) {
		struct iphdr *iph = ip_hdr(skb);
			
		if (iph->protocol == IPPROTO_TCP) {
			struct tcphdr *th = tcp_hdr(skb);
			unsigned short sport, dport;

			th = tcp_hdr(skb);
			th = (struct tcphdr *)(((unsigned char *)iph) + iph->ihl*4);

			if ((iph->daddr == lan_ifa->ifa_local) 
				&& ((th->dest == 0xbd01) || (th->dest == 0x8900) 
				|| (th->dest == 0x8a00) || (th->dest == 0x8b00)))
				return 1;
			else
				return 0;
		}

	}
	
	return 0;
}

int mtk_smb_nf_pre_routing_hook(struct sk_buff *skb)
{
	struct iphdr *iph = ip_hdr(skb);

	if (skb->protocol == htons(ETH_P_IP)) {
		struct iphdr *iph = ip_hdr(skb);
			
		if (iph->protocol == IPPROTO_TCP) {
			struct tcphdr *th = tcp_hdr(skb);
			unsigned short sport, dport;

			th = tcp_hdr(skb);
			th = (struct tcphdr *)(((unsigned char *)iph) + iph->ihl*4);
			if ((iph->daddr == lan_ifa->ifa_local) 
				&& ((th->dest == 0xbd01) || (th->dest == 0x8900) 
				|| (th->dest == 0x8a00) || (th->dest == 0x8b00)))
				return 1;
			else
				return 0;
		}

	}	

	return 0;
}

int mtk_smb_nf_local_out_hook(struct sk_buff *skb)
{
	struct iphdr *iph = ip_hdr(skb);

	if (iph->protocol == IPPROTO_TCP) {
		struct tcphdr *th = tcp_hdr(skb);

		th = tcp_hdr(skb);
		th = (struct tcphdr *)(((unsigned char *)iph) + iph->ihl*4);

		if ((iph->saddr == lan_ifa->ifa_local)
			&& ((th->source == 0xbd01) || (th->source == 0x8900) 
			|| (th->source == 0x8a00) || (th->source == 0x8b00)))
			return 1;
		else
			return 0;
	}

	return 0;
}

int mtk_smb_nf_post_routing_hook(struct sk_buff *skb)
{
	struct iphdr *iph = ip_hdr(skb);

	if (skb->protocol == htons(ETH_P_IP)) {
		struct iphdr *iph = ip_hdr(skb);
			
		if (iph->protocol == IPPROTO_TCP) {
			struct tcphdr *th = tcp_hdr(skb);

			th = tcp_hdr(skb);
			th = (struct tcphdr *)(((unsigned char *)iph) + iph->ihl*4);

			if ((iph->saddr == lan_ifa->ifa_local)
				&& ((th->source == 0xbd01) || (th->source == 0x8900) 
				|| (th->source == 0x8a00) || (th->source == 0x8b00)))
				return 1;
			else
				return 0;
		}

	}	

	return 0;
}

int __init mtk_smb_hook_init(void)
{
	struct in_device *in_dev;
	struct in_ifaddr **ifap = NULL;
	struct in_ifaddr *ifa = NULL;

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,35)
	lan_int = dev_get_by_name_rcu(&init_net, "br0");
#else
	lan_int = __dev_get_by_name("br0");
#endif
	if (lan_int)
		in_dev = __in_dev_get_rtnl(lan_int);
	else
		return 0;

	if (in_dev) {
		for (ifap = &in_dev->ifa_list; (ifa = *ifap) != NULL;
		     ifap = &ifa->ifa_next) {
			if (!strcmp("br0", ifa->ifa_label))
			{
				lan_ifa = ifa;
				break; /* found */
			}
		}
	}
	else
		return 0;

	if (lan_ifa) {
		smb_nf_local_in_hook = mtk_smb_nf_local_in_hook;
		smb_nf_pre_routing_hook = mtk_smb_nf_pre_routing_hook;
		smb_nf_local_out_hook = mtk_smb_nf_local_out_hook;
		smb_nf_post_routing_hook = mtk_smb_nf_post_routing_hook;
	}

	//printk("Samba Netfilter Hook Enabled\n");

	return 0;
}

void mtk_smb_hook_cleanup(void)
{
	lan_int = NULL;
	lan_ifa = NULL;
	smb_nf_local_in_hook = NULL;
	smb_nf_pre_routing_hook = NULL;
	smb_nf_local_out_hook = NULL;
	smb_nf_post_routing_hook = NULL;

	return;
}

module_init(mtk_smb_hook_init);
module_exit(mtk_smb_hook_cleanup);

MODULE_LICENSE("GPL");
