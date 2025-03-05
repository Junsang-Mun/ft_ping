// #include "../inc/ping.h"

// static int checksum(unsigned short *buf, int len) {
// 	unsigned long sum = 0;
// 	while (len > 1) {
// 		sum += *buf++;
// 		len -= 2;
// 	}
// 	if (len == 1) {
// 		sum += *(unsigned char *)buf;
// 	}
// 	sum = (sum >> 16) + (sum & 0xFFFF);
// 	sum += (sum >> 16);
// 	return (unsigned short)(~sum);
// }

// struct icmp createPacket() {
// 	struct icmp packet;
// 	packet.icmp_type = ICMP_ECHO;
// 	packet.icmp_code = 0;
// 	packet.icmp_id = getpid();
// 	packet.icmp_seq = 0;
// 	packet.icmp_cksum = 0;
// 	for (int i = 0; i < PACKET_SIZE; i++) {
// 		packet.icmp_data[i] = i;
// 	}
// 	packet.icmp_cksum = checksum((unsigned short *)&packet, sizeof(packet));
// 	return packet;
// }