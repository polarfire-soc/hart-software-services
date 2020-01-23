/*
 * ptp_packets.h
 *
 *  Created on: Feb 14, 2019
 *      Author: Peter Mc Shane
 */

#ifndef PTP_PACKETS_H_
#define PTP_PACKETS_H_

/* Meta data about the packet */
#define PKT_INFO_MASK   0x00FFFFFFul
#define PKT_TYPE_MASK   0xFF000000ul

#define PKT_IS_PTPV1    0x00000001ul
#define PKT_IS_PTPV2    0x00000002ul
#define PKT_IS_ETH      0x00000004ul
#define PKT_IS_IPV4     0x00000008ul
#define PKT_IS_IPV6     0x00000010ul
#define PKT_IS_EVENT    0x00000020ul
#define PKT_IS_UCAST    0x00000040ul
#define PKT_IS_MCAST    0x00000080ul /* General network multicast - 01:1b:19:00:00:00 or IP Multicast */
#define PKT_IS_MCASTL   0x00000100ul /* Local network multicast   - 01:80:c2:00:00:0e */
#define PKT_IS_PDRQM    0x01000000ul /* Peer Delay Request Message */
#define PKT_IS_PDRSM    0x02000000ul /* Peer Delay Response Message */
#define PKT_IS_PDFUM    0x03000000ul /* Peer Delay Follow Up Message */
#define PKT_IS_SM       0x04000000ul /* Sync Message */
#define PKT_IS_AM       0x05000000ul /* Announce Message */
#define PKT_IS_DRQM     0x06000000ul /* Delay Request Message */
#define PKT_IS_DRSM     0x07000000ul /* Delay Response Message */
#define PKT_IS_FUM      0x08000000ul /* Follow Up Message */
#define PKT_IS_SIGM     0x09000000ul /* Signaling Message */

#define PKT_IS_MANM     0x0A000000ul /* Management Message */

typedef struct packet_info
{
	uint8_t const *data;
	uint32_t length;
	uint32_t correction_offset;
	uint32_t time_offset;
	uint32_t flags;
	char   *description;
} packet_info_t;

typedef struct ts_info
{
	mss_mac_tsu_time_t launch_time;    /* Local TSU time for launch as seen by software */

	/* Time stamp as taken from packet before tx */

	uint32_t          secs_msb;        /* Most significant bits of seconds count  */
    uint32_t          secs_lsb;        /* Least significant bits of seconds count  */
    uint32_t          nanoseconds;     /* Nanoseconds count  */

	/* Correction field as taken from packet before tx */

	uint32_t          nsecs_msb;       /* Most significant bits of seconds count  */
    uint32_t          nsecs_lsb;       /* Least significant bits of seconds count  */
    uint32_t          sub_nanoseconds; /* Nanoseconds count  */
} ts_info_t;

typedef struct mac_address
{
	uint8_t octets[6];
} mac_address_t;

extern packet_info_t ptp_packets[];
extern packet_info_t ptp_sync_packets[];

extern mac_address_t mac_address_list[];

extern volatile int g_vlan_tags0;
extern volatile int g_vlan_tags1;

extern volatile int g_address_swap;

void send_ptp_stream(mss_mac_instance_t *this_mac, packet_info_t *packets, int *index);
uint16_t get_ptp_packet_short_uint(uint8_t *source);
uint32_t get_ptp_packet_uint(uint8_t *source);
uint32_t calc_gem_hash_index(uint8_t *p_address);


#endif /* PTP_PACKETS_H_ */
