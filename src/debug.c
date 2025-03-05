#include "../inc/ping.h"

// 패킷을 16진수(Hex)로 출력하는 함수
void print_packet(const char *label, const unsigned char *packet, int length) {
    printf("%s:\n", label);
    for (int i = 0; i < length; i++) {
        printf("%02X ", packet[i]);
        if ((i + 1) % 16 == 0)  // 16바이트마다 줄 바꿈
            printf("\n");
    }
    printf("\n\n");
}

// ICMP 패킷 전송
void send_icmp_request(int sockfd, struct sockaddr_in *dest_addr) {
    char packet[64];  // ICMP 패킷 버퍼
    struct icmp *icmp_hdr = (struct icmp *)packet;

    build_icmp_packet(icmp_hdr, 1);

    print_packet("Sent ICMP Packet", (unsigned char *)packet, sizeof(packet));

    if (sendto(sockfd, packet, sizeof(packet), 0, (struct sockaddr *)dest_addr, sizeof(*dest_addr)) <= 0) {
        perror("Send failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    printf("ICMP Echo Request sent to %s\n", inet_ntoa(dest_addr->sin_addr));
}

// ICMP 응답 수신
void receive_icmp_reply(int sockfd) {
    char recv_buf[128];
    struct sockaddr_in recv_addr;
    socklen_t addr_len = sizeof(recv_addr);

    int recv_len = recvfrom(sockfd, recv_buf, sizeof(recv_buf), 0, (struct sockaddr *)&recv_addr, &addr_len);
    if (recv_len > 0) {
        struct ip *ip_hdr = (struct ip *)recv_buf;
        struct icmp *icmp_hdr_reply = (struct icmp *)(recv_buf + (ip_hdr->ip_hl << 2));

        printf("Received ICMP Packet from %s\n", inet_ntoa(recv_addr.sin_addr));
        print_packet("Received ICMP Packet", (unsigned char *)recv_buf, recv_len);

        if (icmp_hdr_reply->icmp_type == ICMP_ECHOREPLY) {
            printf("ICMP Echo Reply received from %s\n", inet_ntoa(recv_addr.sin_addr));
        } else {
            printf("Received ICMP packet, but not an Echo Reply.\n");
        }
    } else {
        perror("Receive failed");
    }
}
