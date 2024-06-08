#ifndef		UDP_SERVER_H
#define		UDP_SERVER_H

#include <stdio.h>
#include "msg.h"
#include "shell.h"
#include "net/gnrc.h"
#include "net/gnrc/ipv6.h"
#include "net/gnrc/netif.h"
#include "net/gnrc/netif/hdr.h"
#include "net/gnrc/udp.h"
#include "net/gnrc/pktdump.h"
#include "timex.h"
#include "utlist.h"
#include "xtimer.h"


/**
 * @brief   Priority of the pktdump thread
 */
#ifndef PACKET_HANDLE_PRIO
#define PACKET_HANDLE_PRIO               (THREAD_PRIORITY_MAIN - 1)
#endif

/**
 * @brief   Message queue size for the pktdump thread
 */
#ifndef PACKET_HANDLER_MSG_QUEUE_SIZE 
#define PACKET_HANDLER_MSG_QUEUE_SIZE      (32U)
#endif

#define SERVER_PORT "8808"

/*Declaration of a dummy thread to forward the packets*/
void *packet_handler(void * arg);

/*
 * @brief Starts an udp server an listen to incoming messages
 */
void start_server(char *port_str);

void print_packets(void);

void encrypt_send(gnrc_pktsnip_t *pkt);

#endif	
