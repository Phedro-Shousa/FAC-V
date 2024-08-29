/**
 * @ingroup     thread-metric
 * @{
 *
 * @file
 * @brief       Implementation of main entry point for thread-metric benchmark suite
 *
 * @author      mrgomes
 *
 * @}
 */

#include "tm_api.h"
#include "stdio.h"
#include "udp/udp-server.h"

#include "net/gnrc/netif.h"
#include "net/dhcpv6/client.h"
#include "net/sock.h"
#include "xtimer.h"
#include "sema.h"

#include "periph/gpio.h"

/* Define a queue for main in case of some lost data  */
#define MAIN_QUEUE_SIZE     (32)
static msg_t _main_msg_queue[MAIN_QUEUE_SIZE];

extern int _gnrc_netif_config(int argc, char **argv);

/* Define main entry point.  */
int main(void)
{	 
   /*Initialize an UDP SERVER*/
   /* we need a message queue for the thread running the shell in order to
    * receive potentially fast incoming networking packets */
    msg_init_queue(_main_msg_queue, MAIN_QUEUE_SIZE);
    puts("\nStarting RIOT client UDP application...");
        
    /* print network addresses */
    //puts("Configured network interfaces:");
    //_gnrc_netif_config(0, NULL);

    /*Start the UDP Server*/
    
    
    /* Initialize the test.  */
    puts("\nStarting Thread_Metric Benchmark Suite...");
    printf("  > Starting TM "TEST" with TM_TEST_DURATION = %d and TM_TEST_CYCLES = %d!\n", 
           TM_TEST_DURATION, TM_TEST_CYCLES);

        
#if THREADMETRIC == 1  
    tm_basic_processing_initialize();
#elif THREADMETRIC == 2  
    tm_cooperative_scheduling_initialize();
#elif THREADMETRIC == 3  
    tm_preemptive_scheduling_initialize();
#elif THREADMETRIC == 4  
    tm_interrupt_processing_initialize();
#elif THREADMETRIC == 5  
    tm_interrupt_preemption_processing_initialize();
#elif THREADMETRIC == 6  
    tm_message_processing_initialize();
#elif THREADMETRIC == 7  
    tm_synchronization_processing_initialize();
#elif THREADMETRIC == 8  
    tm_memory_allocation_initialize();
#elif THREADMETRIC == 9
    //start_server(SERVER_PORT);
#else
#warning Please select a test! 
#endif
    start_server(SERVER_PORT);

    /*since main is a thread we suspend it after initialization*/
    tm_thread_suspend();
    
    //this should be never reached
    return 0;
}

