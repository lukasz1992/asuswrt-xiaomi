#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <signal.h>
#include <unistd.h>
#include <shared.h>
#include <rc.h>
#include <netinet/ether.h>
#include <roamast.h>
#include <qca.h>

struct get_stainfo_priv_s {
	int bssidx;
	int vifidx;
	char *wlif_name;
};

/* Helper of get_stainfo()
 * @src:	pointer to WLANCONFIG_LIST
 * @arg:
 * @return:
 * 	0:	success
 *  otherwise:	error
 */
static int handle_get_stainfo(const WLANCONFIG_LIST *src, void *arg)
{
	struct get_stainfo_priv_s *priv = arg;
	int cur_txrx_bytes = 0;
	FILE *fp;
	char line_buf[300], *ptr;
	char cmd[sizeof("80211stats -i XXX XX:XX:XX:XX:XX:XX") + IFNAMSIZ];
	rast_sta_info_t *sta = NULL;

	if (!src || !arg)
		return -1;

	RAST_DBG("[%s][%u][%u][%s][%s][%u][%s][%s]\n",
		src->addr, src->aid, src->chan, src->txrate,
		src->rxrate, src->rssi, src->conn_time, src->mode);

	/* add to assoclist */
	sta = rast_add_to_assoclist(priv->bssidx, priv->vifidx, ether_aton(src->addr));
	sta->rssi = src->rssi;
	sta->active = uptime();
	//sta->tx_rate = safe_atoi(src->txrate) * 1000; /* Kbps */
	//sta->rx_rate = safe_atoi(src->rxrate) * 1000; /* Kbps */

	/* get Tx/Rx bytes of station */
	snprintf(cmd, sizeof(cmd), "80211stats -i %s %s", priv->wlif_name, src->addr);
	if (!(fp = popen(cmd, "r")))
		return 0;

	while (fgets(line_buf, sizeof(line_buf), fp)) {
		if ((ptr = strstr(line_buf, "rx_bytes ")) || (ptr = strstr(line_buf, "tx_bytes "))) {
			ptr += strlen("rx_bytes ");
			cur_txrx_bytes += safe_atoi(ptr);
		}
	}
	sta->datarate = (float)((cur_txrx_bytes - sta->last_txrx_bytes) >> 7/* bytes to Kbits*/) / RAST_POLL_INTV_NORMAL/* Kbps */;
	sta->last_txrx_bytes = cur_txrx_bytes;
	pclose(fp);

	return 0;
}

void get_stainfo(int bssidx, int vifidx)
{
	char wlif_name[IFNAMSIZ];
	struct get_stainfo_priv_s priv = { .bssidx = bssidx, .vifidx = vifidx, .wlif_name = wlif_name };

	__get_wlifname(bssidx, vifidx, wlif_name);

	/* get MAC and RSSI of station */
	__get_qca_sta_info_by_ifname(wlif_name, 0, handle_get_stainfo, &priv);
}
