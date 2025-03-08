#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <arpa/inet.h>


#define FULL_PACKET_SIZE 84
#define IP_HEADER_SIZE 20
#define ICMP_HEADER_SIZE 64

uint16_t checksum(uint16_t *data, size_t len) {
	uint32_t sum = 0;
	while (len > 1) {
		sum += *data++;
		len -= 2;
	}
	if (len == 1) {
		sum += *(uint8_t *)data;
	}
	while (sum >> 16) {
		sum = (sum & 0xffff) + (sum >> 16);
	}
	return (uint16_t)~sum;
}

int main() {
	int sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if (sockfd < 0) {
		perror("socket");
		return 1;
	}

	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = 0;
	addr.sin_addr.s_addr = inet_addr("1.1.1.1");

	uint16_t packet[FULL_PACKET_SIZE / 2];
	memset(packet, 0, FULL_PACKET_SIZE);

	// IP header
	packet[0] = 0x4500;
	packet[1] = 0x0054;
	packet[2] = 0xfadc;
	packet[3] = 0x4000;
	packet[4] = 0x4001;
	packet[5] = 0x0000; // Placeholder for checksum
	packet[6] = 0x0a00; // Source IP
	packet[7] = 0x020f; // 10.0.2.15 (Change if needed)
	packet[8] = 0x0101; // Destination IP
	packet[9] = 0x0101; // 1.1.1.1
	packet[5] = checksum(packet, IP_HEADER_SIZE);

	// ICMP header (Echo Request)
	packet[10] = 0x0800; // Type 8 (Echo Request)
	packet[11] = 0x0000; // Placeholder for checksum
	packet[12] = 0x0d22; // Identifier
	packet[13] = 0x0001; // Sequence Number
	for (int i = 14; i < FULL_PACKET_SIZE / 2; i++) {
		packet[i] = 0x0000; // Payload (Zero padding)
	}
	packet[11] = checksum(packet + 10, FULL_PACKET_SIZE - 20);

	// Send the packet
	ssize_t sent_bytes = sendto(sockfd, packet, FULL_PACKET_SIZE, 0, 
								(struct sockaddr *)&addr, sizeof(addr));
	if (sent_bytes < 0) {
		perror("sendto");
		close(sockfd);
		return 1;
	}

	printf("ICMP Echo Request sent to 1.1.1.1\n");

	// Buffer to receive response
	uint8_t recv_buffer[1024];
	struct sockaddr_in sender_addr;
	socklen_t sender_len = sizeof(sender_addr);

	// Wait for response (blocking)
	ssize_t recv_bytes = recvfrom(sockfd, recv_buffer, sizeof(recv_buffer), 0,
								  (struct sockaddr *)&sender_addr, &sender_len);
	if (recv_bytes < 0) {
		perror("recvfrom");
		close(sockfd);
		return 1;
	}

	printf("Received %zd bytes from %s\n", recv_bytes, inet_ntoa(sender_addr.sin_addr));

	// Extract ICMP response
	struct iphdr *ip_resp = (struct iphdr *)recv_buffer;
	struct icmphdr *icmp_resp = (struct icmphdr *)(recv_buffer + (ip_resp->ihl * 4));

	if (icmp_resp->type == 0 && icmp_resp->code == 0) {
		printf("Received ICMP Echo Reply from %s!\n", inet_ntoa(sender_addr.sin_addr));
	} else {
		printf("Received ICMP packet but not an Echo Reply. Type: %d, Code: %d\n",
			   icmp_resp->type, icmp_resp->code);
	}

	close(sockfd);
	return 0;
}
