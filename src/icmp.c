#include "../inc/ping.h"

// ICMP 패킷 전송
void send_icmp_request(int sockfd, struct sockaddr_in *dest_addr) {
    char packet[64];  // ICMP 패킷 버퍼
    struct icmp *icmp_hdr = (struct icmp *)packet;

    build_icmp_packet(icmp_hdr, 1);

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

    if (recvfrom(sockfd, recv_buf, sizeof(recv_buf), 0, (struct sockaddr *)&recv_addr, &addr_len) > 0) {
        struct ip *ip_hdr = (struct ip *)recv_buf;
        struct icmp *icmp_hdr_reply = (struct icmp *)(recv_buf + (ip_hdr->ip_hl << 2));

        if (icmp_hdr_reply->icmp_type == ICMP_ECHOREPLY) {
            printf("ICMP Echo Reply received from %s\n", inet_ntoa(recv_addr.sin_addr));
        } else {
            printf("Received ICMP packet, but not an Echo Reply.\n");
        }
    } else {
        perror("Receive failed");
    }
}
