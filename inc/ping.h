#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>

#include "literal.h"
#include "function.h"

#define PACKET_SIZE 64
