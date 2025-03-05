#pragma once

void send_icmp_request(int sockfd, struct sockaddr_in *dest_addr);
void receive_icmp_reply(int sockfd);
void build_icmp_packet(struct icmp *icmp_hdr, int seq);
