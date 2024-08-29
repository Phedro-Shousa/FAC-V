#include "udp/udp-server.h"
#include "od.h"
#include "net/sixlowpan.h"

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <limits.h>


typedef unsigned char                uint8_t;

#define DATA_SIZE 10


uint32_t firstPacket = 0;
char addr_[]="fe80::212:4b00:0000:0001";
char port_[]="8808";

/*Init the payload*/
char data[DATA_SIZE+1];
    
/**
 * @brief   PID of the thread to handle packets
 */
kernel_pid_t packet_handler_pid = KERNEL_PID_UNDEF;

/*Declaration of the server interface*/
static gnrc_netreg_entry_t server = GNRC_NETREG_ENTRY_INIT_PID(GNRC_NETREG_DEMUX_CTX_ALL,
                                                               KERNEL_PID_UNDEF);


/**
 * @brief   Stack for the packet handle thread
 */
static char packet_handler_stack[GNRC_PKTDUMP_STACKSIZE];

/* for the echoooooing */
static void send(char *addr_str, char *port_str, char *data, char *size, unsigned int num,
				 unsigned int delay);

    char addr_echo[]="fe80::212:4b00:ffff:ffff";
    char port_echo[]="1234";
	
	
void start_server(char *port_str)
{
    uint16_t port;
 
     /* check if server is already running */
    if (server.target.pid != KERNEL_PID_UNDEF) {
        printf("Error: server already running on port %" PRIu32 "\n",
               server.demux_ctx);
        return;
    }
    /* parse port */
    port = atoi(port_str);
    if (port == 0) {
        puts("Error: invalid port specified");
        return;
    }
    
    /*Creates a thread to receive the incoming data*/
    if (packet_handler_pid == KERNEL_PID_UNDEF) {
        packet_handler_pid = thread_create(packet_handler_stack, sizeof(packet_handler_stack), PACKET_HANDLE_PRIO,
                             THREAD_CREATE_STACKTEST,
                             packet_handler, NULL, "packet_handler");
    }

    /* start server (which means registering packet_handle for the chosen port) */
    server.target.pid =  packet_handler_pid ;//;//   gnrc_pktdump_pid
    server.demux_ctx = (uint32_t)port;
    gnrc_netreg_register(GNRC_NETTYPE_UDP, &server);
    printf("  > Success: started UDP server on port %" PRIu16 ".\n", port);
    
    
    for(int i = 0; i < DATA_SIZE; i++)
      data[i] = 0x41 + i;

    while(1){
      //send(addr_, port_, data,data , 1, 1);
      
      xtimer_usleep(1000*100);
    }
    
}


void *packet_handler(void * arg){
    
    (void) arg;
    msg_t msg;
        
    /*Queue to receive the messages from the UDP SERVER*/
    msg_t msg_queue[PACKET_HANDLER_MSG_QUEUE_SIZE ];

    /* setup the message queue */
    msg_init_queue(msg_queue, PACKET_HANDLER_MSG_QUEUE_SIZE );
    
    printf("  > Packet handler configured.\n");
    
	
	 /*Init the payload*/
    char data[DATA_SIZE+1];
    for(int i = 0; i < DATA_SIZE; i++)
      data[i] = 0x41 + i;
	(void)data;
	
    while(1){
        /*wait for the message*/
        msg_receive(&msg);
        firstPacket++;
        if (firstPacket == 1) {
          printf("packet %ld received!\n", firstPacket);
        }
        switch (msg.type) {
            case GNRC_NETAPI_MSG_TYPE_RCV:

				/* echoooing packet through the network */
                //pt_send(msg.content.ptr);
                printf("dara received \n");
				send(addr_echo, port_echo, data, data, 1, 1);
				//without a dump the queue will be full
                gnrc_pktbuf_release(msg.content.ptr); 
				

                break;
            default:
              break;
      }
    }
}

void print_packets(void)
{
  printf(" Total packets received: %ld\n\n", firstPacket);
}

static void send(char *addr_str, char *port_str, char *data, char *size, unsigned int num,
                 unsigned int delay)
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


void pt_send(gnrc_pktsnip_t *pkt)
{
    uint8_t *pt;
    pt = (uint8_t *)pkt->data;
    size_t length = strlen((char *)pt);
    uint8_t result[length];
    uint8_t *pr;    
    pr = result;

/*  
for(int i=0; i<10; i++){
    printf(" %c", *pt);
    pt=pt+1;
}
*/	
send(addr_echo, port_echo, (char *)pr, (char *)pt, 1, 0);
    
}