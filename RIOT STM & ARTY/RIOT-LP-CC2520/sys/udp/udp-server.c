#include "udp/udp-server.h"
#include "od.h"
#include "net/sixlowpan.h"

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <limits.h>
#include "ps.h"

#include "crypto/aes.h"
#include "crypto/modes/ecb.h"
#include "crypto/ciphers.h"


typedef unsigned char                uint8_t;

#define DATA_SIZE 32

uint32_t firstPacket = 0;
char addr_[]="fe80::212:4b00:0000:0001";
char port_[]="8808";

/*Init the payload*/
char data[DATA_SIZE+1];
    

cipher_context_t ctx;


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

    char addr_echo[]="fe80::212:4b00:0000:0003";
    char port_echo[]="9908";
	
	
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

    uint8_t key[] = {
    0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7,
    0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7,
    0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7,
    0x8, 0x9, 0xA, 0xB, 0xC, 0xD, 0xE, 0xF
    };
    uint8_t key_len = 32;
    aes_init(&ctx, key, key_len);   
    printf("AES_INIT()\n");

	
    while(1){
      
      xtimer_usleep(1000*100);
    }
    
}


void *packet_handler(void * arg){
    (void)arg;
    
    msg_t msg;
        
    /*Queue to receive the messages from the UDP SERVER*/
    msg_t msg_queue[PACKET_HANDLER_MSG_QUEUE_SIZE ];

    /* setup the message queue */
    msg_init_queue(msg_queue, PACKET_HANDLER_MSG_QUEUE_SIZE );
    
    //printf("  > Packet handler configured.\n");
    
		
    while(1){
        /*wait for the message*/
        msg_receive(&msg);
        firstPacket++;
        switch (msg.type) {
            case GNRC_NETAPI_MSG_TYPE_RCV:

				//without a dump the queue will be full
                puts("Data received");
                encrypt_send(msg.content.ptr);
                
	            //send(addr_echo, port_echo, msg.content.ptr, msg.content.ptr, 1, 0);
                //gnrc_pktbuf_release(msg.content.ptr); 
				
				/* echoooing packet through the network */

                break;
            default:
              break;
      }
    }
}

void print_packets(void)
{
  printf(" Total packets received: %lu\n\n", firstPacket);
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




void encrypt_send(gnrc_pktsnip_t *pkt)
{
    uint8_t *pt;
    pt = (uint8_t *)pkt->data;
    size_t length = strlen((char *)pt);
    uint8_t result[length];
    uint8_t *pr;    
    pr = result;

/*    size_t offset = 0;
    uint8_t block_size = AES_BLOCK_SIZE;

    do {
        aes_encrypt(&ctx, pt + offset, pr + offset);  
        offset += block_size;
    } while (offset < length);
*/	
for(int i=0; i<10; i++){
    printf(" %c", *pt);
    pt=pt+1;
}
send(addr_echo, port_echo, (char *)pr, (char *)pt, 1, 0);
    gnrc_pktbuf_release(pkt);
    
}
/*
128key & 16B message:
Input
4142434445464748494a4b4c4d4e4f50
5152535455565758595a5b5c5d5e5f60

Key
000102030405060708090a0b0c0d0e0f

Enc
9cdd85de85b48bed892f02d8a5cbdacb
0134ba9b132797e9de4891f520d8abac


256key & 80B message:
Input
4142434445464748494a4b4c4d4e4f50
5152535455565758595a5b5c5d5e5f60
6162636465666768696a6b6c6d6e6f70
7172737475767778797a7b7c7d7e7f80
8182838485868788898a8b8c8d8e8f90

Key
00010203040506070001020304050607
000102030405060708090a0b0c0d0e0f

Enc
23ef7b1908b0b0da2d1dac5a369b5157
3612568547941e8c70f392b373f3327e
e0b4704e620bc1dcfaa7c6ca68986935
f85648ad0b42106a2d57bae08029fbac
4b01f4a823565a386167c0c68f16c8f3
*/