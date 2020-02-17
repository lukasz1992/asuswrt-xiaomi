/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
#include <stdio.h>
#include <fcntl.h>

#include <shutils.h>
#include <shared.h>
#include <qca.h>

/* Helper of __get_qca_sta_info_by_ifname()
 * @src:	pointer to WLANCONFIG_LIST
 * @arg:
 * @return:
 * 	0:	success
 *  otherwise:	error
 */
static int handler_qca_sta_info(const WLANCONFIG_LIST *src, void *arg)
{
	WIFI_STA_TABLE *sta_info = arg;
	WLANCONFIG_LIST *dst;

	if (!src || !arg)
		return -1;

	dst = &sta_info->Entry[sta_info->Num++];
	*dst = *src;
#if 0
	dbg("[%s][%u][%u][%s][%s][%d][%s][%s]\n", dst->addr, dst->aid, dst->chan,
		dst->txrate, dst->rxrate, dst->rssi, dst->mode, dst->conn_time);
#endif


	return 0;
}

#define MAX_NR_STA_ITEMS	(30)
struct sta_info_item_s {
	int idx;		/* < 0: doesn't exist; >= 0: v[] index */
	const char *key;
	const char *fmt;	/* format string that is used to convert v[idx] */
	void *var;		/* target address that is used to store convertion result. */
};

/* Helper of __get_qca_sta_info_by_ifname() that is used to initialize struct sta_info_item_s array,
 * according to header line of output of "wlanconfig athX list".
 * header line maybe truncated due to:
 * 1. ACAPS never has data.
 * 2. IEs maybe empty string, RSN, WME, or RSN WME.
 * @line:
 * @sta_info_items:
 * @return:
 * 	0:	success
 *  otherwise:	error
 */
static int init_sta_info_item(const char *line, struct sta_info_item_s *sta_info_items)
{
	const char f[] = "%s";
	int i, n;
	char fmt[MAX_NR_STA_ITEMS * sizeof(f)];
	char v[MAX_NR_STA_ITEMS][sizeof("MAXRATE(DOT11)XXXXX")];
	struct sta_info_item_s *p;

	if (!line || !sta_info_items)
		return -1;

	for (p = sta_info_items; p->key != NULL; ++p)
		p->idx = -1;

	for (i = 0, *fmt = '\0'; i < MAX_NR_STA_ITEMS; ++i)
		strlcat(fmt, f, sizeof(fmt));

	n = sscanf(line, fmt, v, v + 1, v + 2, v + 3, v + 4, v + 5, v + 6, v + 7, v + 8, v + 9,
		v + 10, v + 11, v + 12, v + 13, v + 14, v + 15, v + 16, v + 17, v + 18, v + 19,
		v + 20, v + 21, v + 22, v + 23, v + 24, v + 25, v + 26, v + 27, v + 28, v + 29);
	for (i = 0; i < n; ++i) {
		for (p = &sta_info_items[0]; p->key != NULL; ++p) {
			if (strcmp(v[i], p->key))
				continue;

			p->idx = i;
			break;
		}
	}

	return 0;
}

/* Helper of __get_qca_sta_info_by_ifname() that is used to fill data to WLANCONFIG_LIST,
 * according to header line of output of "wlanconfig athX list".
 * header line maybe truncated due to:
 * 1. ACAPS never has data.
 * 2. IEs maybe empty string, RSN, WME, or RSN WME.
 * @line:
 * @sta_info_items:
 * @return:
 * 	0:	success
 *     <0:	error
 *     >0:	number of items can't be parsed.
 */
static int fill_sta_info_item(const char *line, const struct sta_info_item_s *sta_info_items)
{
	const char f[] = "%s";
	int i, n, ret = 0;
	char fmt[MAX_NR_STA_ITEMS * sizeof(f)];
	char v[MAX_NR_STA_ITEMS][sizeof("IEEE80211_MODE_11AXA_HE40MINUSXXXXX")];
	const struct sta_info_item_s *p;

	if (!line || !sta_info_items)
		return -1;

	for (i = 0, *fmt = '\0'; i < MAX_NR_STA_ITEMS; ++i)
		strlcat(fmt, f, sizeof(fmt));

	n = sscanf(line, fmt, v, v + 1, v + 2, v + 3, v + 4, v + 5, v + 6, v + 7, v + 8, v + 9,
		v + 10, v + 11, v + 12, v + 13, v + 14, v + 15, v + 16, v + 17, v + 18, v + 19,
		v + 20, v + 21, v + 22, v + 23, v + 24, v + 25, v + 26, v + 27, v + 28, v + 29);
	for (p = sta_info_items; n > 0 && p->key != NULL; ++p) {
		if (p->idx < 0)
			continue;

		if (sscanf(v[p->idx], p->fmt, p->var) == 1)
			continue;

		ret++;
		dbg("%s: can't parse. argv[%d] = [%s] key [%s] fmt [%s] var [%p]\n",
			__func__, p->idx, v[p->idx]? : "<NULL>", p->key, p->fmt, p->var);
	}

	return ret;
}

/* Parsing "wlanconfig athX list" result, fill WLANCONFIG_LIST, and then pass it to @handler() with @arg which is provided by caller.
 * @ifname:	VAP interface name that is used to execute "wlanconfig @ifname list" command.
 * @subunit_id:	if non-zero, copied to WLANCONFIG_LIST.subunit
 * @handler:	handler function that will be execute for each client.
 * return:
 * 	0:	success
 *  otherwise:	error
 *
 * ILQ3.1 example:
 * wlanconfig ath1 list
 * ADDR               AID CHAN TXRATE RXRATE RSSI IDLE  TXSEQ  RXSEQ  CAPS        ACAPS     ERP    STATE MAXRATE(DOT11) HTCAPS ASSOCTIME    IEs   MODE PSMODE
 * 00:10:18:55:cc:08    1  149  55M   1299M   63    0      0   65535               0        807              0              Q 00:10:33 IEEE80211_MODE_11A  0
 * 08:60:6e:8f:1e:e6    2  149 159M    866M   44    0      0   65535     E         0          b              0           WPSM 00:13:32 WME IEEE80211_MODE_11AC_VHT80  0
 * 08:60:6e:8f:1e:e8    1  157 526M    526M   51 4320      0   65535    EP         0          b              0          AWPSM 00:00:10 RSN WME IEEE80211_MODE_11AC_VHT80 0
 *
 * SPF8 CSU2 QSDK example:
 * admin@RT-AX89U:/tmp/home/root# wlanconfig ath0 list
 * ADDR               AID CHAN TXRATE RXRATE RSSI MINRSSI MAXRSSI IDLE  TXSEQ  RXSEQ  CAPS        ACAPS     ERP    STATE MAXRATE(DOT11) HTCAPS ASSOCTIME    IEs   MODE                   PSMODE
 * 12:9d:92:4e:85:bc    1  104 2882M   3026M   73       0      74    0      0   65535   EPs         0          b              0           AWPSM 00:00:35     RSN WME IEEE80211_MODE_11AXA_HE80   0
 * 14:dd:a9:3d:68:65    2  104 433M    433M   69       0      79    0      0   65535    EP         0          b              0            AWPS 00:00:35     RSN WME IEEE80211_MODE_11AC_VHT80   1
 *
 * SPF10 ES QSDK example:
 * admin@GT-AXY16000:/tmp/home/root# wlanconfig ath0 list
 * ADDR               AID CHAN TXRATE RXRATE RSSI MINRSSI MAXRSSI IDLE  TXSEQ  RXSEQ  CAPS XCAPS        ACAPS     ERP    STATE MAXRATE(DOT11) HTCAPS   VHTCAPS ASSOCTIME    IEs   MODE RXNSS TXNSS                   PSMODE
 * 14:dd:a9:3d:68:65    1   60 433M      6M   36      22      40    0      0   65535    EP    OI         0          b              0            AWPS             gGR 00:00:09     RSN WME IEEE80211_MODE_11AC_VHT80  1 1   1
 *  Minimum Tx Power             : 0
 *  Maximum Tx Power             : 0
 *  HT Capability                        : Yes
 *  VHT Capability                       : Yes
 *  MU capable                   : No
 *  SNR                          : 36
 *  Operating band                       : 5GHz
 *  Current Operating class      : 0
 *  Supported Rates              : 12  18  24  36  48  72  96  108
 */
int __get_qca_sta_info_by_ifname(const char *ifname, char subunit_id, int (*handler)(const WLANCONFIG_LIST *rptr, void *arg), void *arg)
{
#if defined(RTCONFIG_SOC_IPQ8074)
	const int l2_offset = 91;
#elif defined(RTCONFIG_SOC_IPQ8064) || defined(RTCONFIG_)
	const int l2_offset = 85;
#else
	const int l2_offset = 79;
#endif
	FILE *fp;
	int channf, ret = 0, ax2he = 0;
	unsigned char tmac[6], *tm = &tmac[0];
	char cmd[sizeof("wlanconfig XXX list") + IFNAMSIZ];
	char *q, line_buf[300], *l2 = line_buf + l2_offset;
	WLANCONFIG_LIST result, *r = &result;
	struct sta_info_item_s part1_tbl[] = {
		/* Parse ADDR ~ XCAPS. */
		{ .key = "ADDR",	.fmt = "%s",	.var = &r->addr },
		{ .key = "AID",		.fmt = "%u",	.var = &r->aid },
		{ .key = "CHAN",	.fmt = "%u",	.var = &r->chan },
		{ .key = "TXRATE",	.fmt = "%s",	.var = &r->txrate },
		{ .key = "RXRATE",	.fmt = "%s",	.var = &r->rxrate },
		{ .key = "RSSI",	.fmt = "%d",	.var = &r->rssi },

		{ .key = NULL, .fmt = NULL, .var = NULL },
	}, part2_tbl[] = {
		/* Parse ACAPS (no data, omit) ~ IEs (maybe empty string, RSN, WME, or both). */
		{ .key = "ASSOCTIME",	.fmt = "%s",	.var = &r->conn_time },

		{ .key = NULL, .fmt = NULL, .var = NULL },
	}, part3_tbl[] = {
		/* Parse MODE ~ PSMODE */
		{ .key = "MODE",	.fmt = "IEEE80211_MODE_%s", .var = r->mode },

		{ .key = NULL, .fmt = NULL, .var = NULL },
	};

	if (!ifname || !handler)
		return -1;

	snprintf(cmd, sizeof(cmd), "wlanconfig %s list", ifname);
	if (!(fp = popen(cmd, "r")))
		return -2;

#if defined(RTCONFIG_WIFI_QCN5024_QCN5054)
	if (!find_word(nvram_safe_get("rc_support"), "11AX"))
		ax2he = 1;
#endif
	channf = QCA_DEFAULT_NOISE_FLOOR;

	/* Parsing header and initialize related data structure */
	if (!fgets(line_buf, sizeof(line_buf), fp))
		goto leave;

	if ((q = strstr(line_buf, "MODE")) != NULL) {
		*(q - 1) = '\0';
		init_sta_info_item(q, part3_tbl);
	}
	if ((q = strstr(line_buf, "ACAPS")) != NULL) {
		*(q - 1) = '\0';
		l2 = q;
		init_sta_info_item(q + strlen("ACAPS"), part2_tbl);	/* skip ACAPS due to it doesn't have data. */
	}
	init_sta_info_item(line_buf, part1_tbl);

	/* Parsing client list */
	while (fgets(line_buf, sizeof(line_buf), fp) != NULL) {
		if (sscanf(line_buf, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx %*[^\n]", tm, tm + 1, tm + 2, tm + 3, tm + 4, tm + 5) != 6)
			continue;

		memset(r, 0, sizeof(*r));
		/* Parsing part3, all data behind IEs (started from IEEE802...) */
		if ((q = strstr(line_buf, "IEEE80211_MODE_")) != NULL) {
			*(q - 1) = '\0';
			fill_sta_info_item(q, part3_tbl);
		}

		/* Parsing part2, ACAPS (omit) ~ IEs */
		*(l2 - 1) = '\0';
		fill_sta_info_item(l2, part2_tbl);

		/* Parsing part1, ADDR ~ IEs */
		fill_sta_info_item(line_buf, part1_tbl);

		/* Post adjustment */
		if (ax2he) {
			if ((q = strstr(r->mode, "11AXA")) != NULL)
				memcpy(q, "11AHE", 5);
			else if ((q = strstr(r->mode, "11AXG")) != NULL)
				memcpy(q, "11GHE", 5);
		}
		if (subunit_id)
			r->subunit_id = subunit_id;
		if (strlen(r->rxrate) >= 6)
			strcpy(r->rxrate, "0M");
		convert_mac_string(r->addr);
		r->rssi += channf;

		handler(r, arg);
	}
leave:
	pclose(fp);
	return ret;
}

/* Wrapper function of QCA Wireless client list parser.
 * @ifname:	VAP interface name
 * @subunit_id:
 * @sta_info:	pointer to WIFI_STA_TABLE
 * @return:
 * 	0:	success
 *  otherwise:	error
 */
int get_qca_sta_info_by_ifname(const char *ifname, char subunit_id, WIFI_STA_TABLE *sta_info)
{
	if (!ifname || !sta_info)
		return -1;

	return __get_qca_sta_info_by_ifname(ifname, subunit_id, handler_qca_sta_info, sta_info);
}
