#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>

// Define ICMP header structure since we're creating it by hand
struct icmp_header {
    uint8_t  type;        // ICMP message type
    uint8_t  code;        // ICMP message code
    uint16_t checksum;    // Checksum of ICMP header and data
    union {
        struct {
            uint16_t id;      // Identifier
            uint16_t sequence;// Sequence number
        } echo;
        uint32_t gateway;     // Gateway address
        struct {
            uint16_t unused;  // Unused
            uint16_t mtu;     // MTU
        } frag;
    } un;
};

// Define IP header structure for parsing the response
struct ip_header {
    uint8_t  ihl:4;       // Internet header length (4 bits)
    uint8_t  version:4;   // Version (4 bits)
    uint8_t  tos;         // Type of service
    uint16_t tot_len;     // Total length
    uint16_t id;          // Identification
    uint16_t frag_off;    // Fragment offset
    uint8_t  ttl;         // Time to live
    uint8_t  protocol;    // Protocol
    uint16_t check;       // Header checksum
    uint32_t saddr;       // Source address
    uint32_t daddr;       // Destination address
};

#define ICMP_ECHO 8
#define ICMP_ECHO_REPLY 0
#define PACKET_SIZE 64  // Size of ICMP packet (without IP header)

// Calculate ICMP checksum
uint16_t compute_checksum(uint16_t *addr, int len) {
    int nleft = len;
    uint32_t sum = 0;
    uint16_t *w = addr;
    uint16_t answer = 0;

    // Adding up 16-bit words
    while (nleft > 1) {
        sum += *w++;
        nleft -= 2;
    }

    // Handle odd byte if necessary
    if (nleft == 1) {
        sum += *(uint8_t *)w;
    }

    // Add back carry outs from top 16 bits to low 16 bits
    sum = (sum >> 16) + (sum & 0xffff);  // Add high 16 to low 16
    sum += (sum >> 16);                   // Add carry
    answer = ~sum;                        // Take one's complement

    return answer;
}

int main() {
    int sockfd;
    struct sockaddr_in dest_addr;
    char packet[PACKET_SIZE];
    struct icmp_header *icmp;
    
    // Create raw socket for ICMP
    sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sockfd < 0) {
        perror("socket creation failed");
        return 1;
    }

    // Set destination address
    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_addr.s_addr = inet_addr("1.1.1.1");

    // Craft ICMP packet manually
    memset(packet, 0, PACKET_SIZE);
    
    // Set up the ICMP header manually
    icmp = (struct icmp_header *)packet;
    icmp->type = ICMP_ECHO;       // Echo Request
    icmp->code = 0;               // Must be 0 for Echo Request
    icmp->un.echo.id = htons(getpid() & 0xFFFF);  // Use process ID as identifier
    icmp->un.echo.sequence = htons(1);            // Sequence number
    
    // Fill in some data for the packet payload
    char *data_ptr = packet + sizeof(struct icmp_header);
    int data_size = PACKET_SIZE - sizeof(struct icmp_header);
    
    // Fill the payload with a simple pattern
    for (int i = 0; i < data_size; i++) {
        data_ptr[i] = 'a' + (i % 26);
    }
    
    // Calculate checksum (must be done after filling the packet)
    icmp->checksum = 0;  // Must be 0 before calculating checksum
    icmp->checksum = compute_checksum((uint16_t *)packet, PACKET_SIZE);

    // Send the packet
    ssize_t bytes_sent = sendto(sockfd, packet, PACKET_SIZE, 0, 
                                (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    if (bytes_sent < 0) {
        perror("sendto failed");
        close(sockfd);
        return 1;
    }

    printf("ICMP Echo Request sent to 1.1.1.1 (%zd bytes)\n", bytes_sent);

    // Buffer to receive the response
    char recv_buffer[PACKET_SIZE + 20];  // +20 for IP header
    struct sockaddr_in sender_addr;
    socklen_t sender_addr_len = sizeof(sender_addr);

    // Wait for response
    ssize_t bytes_received = recvfrom(sockfd, recv_buffer, sizeof(recv_buffer), 0,
                                     (struct sockaddr *)&sender_addr, &sender_addr_len);
    if (bytes_received < 0) {
        perror("recvfrom failed");
        close(sockfd);
        return 1;
    }

    printf("Received %zd bytes from %s\n", bytes_received, inet_ntoa(sender_addr.sin_addr));

    // Parse the response (skip IP header to get to ICMP header)
    struct ip_header *ip = (struct ip_header *)recv_buffer;
    int ip_header_len = (ip->ihl) * 4;  // IP header length in bytes
    
    // Access ICMP header in the response
    struct icmp_header *icmp_reply = (struct icmp_header *)(recv_buffer + ip_header_len);

    // Check if it's an ICMP Echo Reply
    if (icmp_reply->type == ICMP_ECHO_REPLY && icmp_reply->code == 0) {
        printf("Received ICMP Echo Reply from %s!\n", inet_ntoa(sender_addr.sin_addr));
        printf("ICMP ID: %d, Sequence: %d\n", 
               ntohs(icmp_reply->un.echo.id), ntohs(icmp_reply->un.echo.sequence));
    } else {
        printf("Received ICMP packet but not an Echo Reply. Type: %d, Code: %d\n",
               icmp_reply->type, icmp_reply->code);
    }

    close(sockfd);
    return 0;
}