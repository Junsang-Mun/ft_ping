#include "../inc/ping.h"

// ICMP 체크섬 계산 함수
unsigned short checksum(void *b, int len) {
    unsigned short *buf = b;
    unsigned int sum = 0;
    unsigned short result;

    for (sum = 0; len > 1; len -= 2)
        sum += *buf++;
    if (len == 1)
        sum += *(unsigned char *)buf;

    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    result = ~sum;
    return result;
}

// RAW 소켓 생성
int create_socket() {
    int sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sockfd < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }
    return sockfd;
}

// ICMP 패킷 생성
void build_icmp_packet(struct icmp *icmp_hdr, int seq) {
    memset(icmp_hdr, 0, sizeof(struct icmp));

    icmp_hdr->icmp_type = ICMP_ECHO;  // Echo Request
    icmp_hdr->icmp_code = 0;
    icmp_hdr->icmp_id = 0x4242 & 0xFFFF;  // Process ID
    icmp_hdr->icmp_seq = seq;
    memset(icmp_hdr->icmp_data, 0xA5, 56);  // 데이터 패딩 (일반적으로 56바이트)

    icmp_hdr->icmp_cksum = 0;
    icmp_hdr->icmp_cksum = checksum(icmp_hdr, sizeof(struct icmp) + 56);
}

// 메인 함수
int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <IP Address>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *target_ip = argv[1];
    struct sockaddr_in dest_addr;

    // 소켓 생성
    int sockfd = create_socket();

    // 대상 주소 설정
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_addr.s_addr = inet_addr(target_ip);

    // ICMP 요청 및 응답 처리
    send_icmp_request(sockfd, &dest_addr);
    receive_icmp_reply(sockfd);

    // 소켓 종료
    close(sockfd);
    return EXIT_SUCCESS;
}
