/*
 * Copyright (C) 2015 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     examples
 * @{
 *
 * @file
 * @brief       Application that creates a udp client and send data
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 *
 * @}
 */

#include <stdio.h>
#include "msg.h"
#include "net/gnrc.h"
#include "net/gnrc/ipv6.h"
#include "net/gnrc/netif.h"
#include "net/gnrc/netif/hdr.h"
#include "net/gnrc/udp.h"
#include "net/gnrc/pktdump.h"
#include "timex.h"
#include "utlist.h"
#include "xtimer.h"

#include "cc2520.h"
#include "cc2520_netdev.h"
#include "cc2520_internal.h"

#define DATA_SIZE 32

#define PACKET_HANDLER_MSG_QUEUE_SIZE      (32U)

#define MAIN_QUEUE_SIZE     (2)
static msg_t _main_msg_queue[MAIN_QUEUE_SIZE];

/*Function prototypes*/
static void send(char *addr_str, char *port_str, char *data, char *size, unsigned int num, unsigned int delay);

static gnrc_netreg_entry_t server = GNRC_NETREG_ENTRY_INIT_PID(GNRC_NETREG_DEMUX_CTX_ALL,
                                                               KERNEL_PID_UNDEF);

int main(void)
{
    /* we need a message queue for the thread running the shell in order to
     * receive potentially fast incoming networking packets */
    msg_init_queue(_main_msg_queue, MAIN_QUEUE_SIZE);
    puts("\nRIOT client udp application!\n");
    
    /*Init server info and dummy info*/
    char addr_good[]="fe80::212:4b00:0000:0002";
    char port_good[]="8808";
    char port_str[]="9909";
    
      uint16_t port;

    port = atoi(port_good);
    /* start server (which means registering pktdump for the chosen port) */
    server.target.pid = gnrc_pktdump_pid;
    server.demux_ctx = (uint32_t)port;
    gnrc_netreg_register(GNRC_NETTYPE_UDP, &server);
    printf("Success: started UDP server on port %" PRIu16 "\n", port);

  //  msg_t msg;
        
    /*Queue to receive the messages from the UDP SERVER*/
    msg_t msg_queue[PACKET_HANDLER_MSG_QUEUE_SIZE ];

    /* setup the message queue */
    msg_init_queue(msg_queue, PACKET_HANDLER_MSG_QUEUE_SIZE );
    
    //printf("  > Packet handler configured.\n");
    
    /*Init the payload*/
    char data[DATA_SIZE+1];
    for(int i = 0; i < DATA_SIZE; i++)
      data[i] = 0x41 + i;
		
    while(1){
                
	            send(addr_good, port_str, data, data, 1, 0);
           
    }
    
    /* should be never reached */
    return 0;
}

static void send(char *addr_str, char *port_str, char *data, char *size, unsigned int num, unsigned int delay)
{
    gnrc_netif_t *netif = NULL;
    char *iface;
    uint16_t port;
    ipv6_addr_t addr;

    /* get interface, if available */
    iface = ipv6_addr_split_iface(addr_str);
    if ((!iface) && (gnrc_netif_numof() == 1)) {
        netif = gnrc_netif_iter(NULL);
    }
    else if (iface){
        netif = gnrc_netif_get_by_pid(atoi(iface));
    }
    /* parse destination address */
    if (ipv6_addr_from_str(&addr, addr_str) == NULL) {
        puts("Error: unable to parse destination address");
        return;
    }
    /* parse port */
    port = atoi(port_str);
    if (port == 0) {
        puts("Error: unable to parse destination port");
        return;
    }

    for (unsigned int i = 0; i < num; i++) {
        gnrc_pktsnip_t *payload, *udp, *ip;
        /* allocate payload */
        payload = gnrc_pktbuf_add(NULL, data, strlen(size), GNRC_NETTYPE_UNDEF);
        if (payload == NULL) {
            puts("Error: unable to copy data to packet buffer");
            return;
        }
        /* allocate UDP header, set source port := destination port */
        udp = gnrc_udp_hdr_build(payload, port, port);
        if (udp == NULL) {
            puts("Error: unable to allocate UDP header");
            gnrc_pktbuf_release(payload);
            return;
        }
        /* allocate IPv6 header */
        ip = gnrc_ipv6_hdr_build(udp, NULL, &addr);
        if (ip == NULL) {
            puts("Error: unable to allocate IPv6 header");
            gnrc_pktbuf_release(udp);
            return;
        }
        /* add netif header, if interface was given */
        if (netif != NULL) {
            gnrc_pktsnip_t *netif_hdr = gnrc_netif_hdr_build(NULL, 0, NULL, 0);

            gnrc_netif_hdr_set_netif(netif_hdr->data, netif);
            ip = gnrc_pkt_prepend(ip, netif_hdr);
        }
        /* send packet */
        if (!gnrc_netapi_dispatch_send(GNRC_NETTYPE_UDP, GNRC_NETREG_DEMUX_CTX_ALL, ip)) {
            puts("Error: unable to locate UDP thread");
            gnrc_pktbuf_release(ip);
            return;
        }
        /* access to `payload` was implicitly given up with the send operation above
         * => use temporary variable for output */
        xtimer_usleep(delay);
    }
}

