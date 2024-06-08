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

#include "periph/gpio.h"


#include "tm_api.h"
#include "stdio.h"
#include "udp/udp-server.h"

#include "periph/gpio.h"

/* Define a queue for main in case of some lost data  */
#define MAIN_QUEUE_SIZE     (8)
msg_t _main_msg_queue[MAIN_QUEUE_SIZE];

/* Define main entry point.  */
int main(void)
{	 
   /*Initialize an UDP SERVER*/
   /* we need a message queue for the thread running the shell in order to
    * receive potentially fast incoming networking packets */
    msg_init_queue(_main_msg_queue, MAIN_QUEUE_SIZE);
    puts("\nStarting RIOT client UDP application...");
        
    /*Start the UDP Server*/
    start_server(SERVER_PORT);
    
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
#else
#endif

    /*since main is a thread we suspend it after initialization*/
    tm_thread_suspend();
    
    //this should be never reached
    return 0;
}

