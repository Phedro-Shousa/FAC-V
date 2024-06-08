/**************************************************************************/ 
/*                                                                        */ 
/*            Copyright (c) 1996-2016 by Express Logic Inc.               */ 
/*                                                                        */ 
/*  This Original Work may be modified, distributed, or otherwise used in */ 
/*  any manner with no obligations other than the following:              */ 
/*                                                                        */ 
/*    1. This legend must be retained in its entirety in any source code  */ 
/*       copies of this Work.                                             */ 
/*                                                                        */ 
/*    2. This software may not be used in the development of an operating */
/*       system product.                                                  */ 
/*                                                                        */  
/*  This Original Work is hereby provided on an "AS IS" BASIS and WITHOUT */ 
/*  WARRANTY, either express or implied, including, without limitation,   */ 
/*  the warranties of NON-INFRINGEMENT, MERCHANTABILITY or FITNESS FOR A  */ 
/*  PARTICULAR PURPOSE. THE ENTIRE RISK AS TO THE QUALITY OF this         */ 
/*  ORIGINAL WORK IS WITH the user.                                       */ 
/*                                                                        */ 
/*  Express Logic, Inc. reserves the right to modify this software        */ 
/*  without notice.                                                       */ 
/*                                                                        */ 
/*  Express Logic, Inc.                     info@expresslogic.com         */
/*  11423 West Bernardo Court               http://www.expresslogic.com   */
/*  San Diego, CA  92127                                                  */
/*                                                                        */
/**************************************************************************/

/**************************************************************************/
/**************************************************************************/
/**                                                                       */ 
/** Thread-Metric Component                                               */
/**                                                                       */
/**   Message Processing Test                                             */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#include "tm_api.h"
#include "stdio.h"
#include "udp/udp-server.h"
#include "ps.h"

#if (THREADMETRIC == 6)

/* Define the counters used in the demo application...  */
volatile uint32_t tm_message_processing_counter = 0;
uint16_t tm_message_sent[4];
uint16_t tm_message_received[4];

/* Define the test thread prototypes.  */
void * tm_message_processing_thread_0_entry(void * arg);

/* Define the reporting thread prototype.  */
void* tm_message_processing_thread_report(void * arg);

#define RCV_QUEUE_SIZE  (8)

/* Define the stacks for each thread.  */
static char stack_thread_0[THREAD_STACKSIZE_DEFAULT];
static char stack_thread_report[THREAD_STACKSIZE_MAIN];
static msg_t rcv_queue[RCV_QUEUE_SIZE];

/*Defenition of thread id's*/
kernel_pid_t pid_thread_0 = NULL;
kernel_pid_t pid_thread_report = NULL;


/* Define the message processing test initialization.  */
void tm_message_processing_initialize(void)
{
    /* Create thread 0 at priority 10.  */
    pid_thread_0 = tm_thread_create(0, 8, tm_message_processing_thread_0_entry, 
                                   stack_thread_0, "tm_thread_0");

    /* Create a queue for the message passing.  */
    //tm_queue_create( rcv_queue, 4);

    /* Create the reporting thread. It will preempt the other 
       threads and print out the test results.  */
    pid_thread_report = tm_thread_create(1, 7, tm_message_processing_thread_report, 
                                          stack_thread_report, "tm_thread_report");
    tm_thread_resume(pid_thread_report);
    
    /* Resume thread 0.  */
    tm_thread_resume(pid_thread_0);
}

/* Define the message processing thread.  */
void * tm_message_processing_thread_0_entry(void * arg)
{ 
    /*For a thread send to himself needs a private queue*/
    msg_init_queue(rcv_queue, RCV_QUEUE_SIZE);
    
    /*Declaration of the messages structs*/
    msg_t message_send;
    msg_t message_received; 
    
    /*Initialization of the send message struct*/
    message_send.content.value = 0xBABE;
    
    while(1)
    {  
        /* Send a message to the queue.  */
        tm_queue_send(NULL, &message_send);
       
        /* Receive a message from the queue.  */
        tm_queue_receive(NULL, &message_received);
                       
        /* Check for invalid message.  */
        if (message_send.content.value != message_received.content.value)
            break;

        /* Increment the last word of the 16-byte message.  */
        tm_message_sent[3]++;

        /* Increment the number of messages sent and received.  */
        tm_message_processing_counter++;
    }
    return 0;
}


/* Define the message test reporting thread.  */
void*  tm_message_processing_thread_report(void * arg)
{ 
  uint32_t   last_counter = 0;
  uint32_t   counter[TM_TEST_CYCLES] = {0};
  uint32_t   counter_index = 0;

    while(1)
    {
        /* Sleep to allow the test to run.  */
        tm_thread_sleep(TM_TEST_DURATION);
        
        /* See if there are any errors.  */
        if (tm_message_processing_counter == last_counter)
        {
          //abort test
           printf("ERROR: Invalid counter value(s). Error allocating/deallocating memory!\n");
           tm_thread_suspend();
        }
       
       //end of test, print values and exit
       if(counter_index == TM_TEST_CYCLES)
       {
            printf("\n");
            for(int i = 0; i < counter_index; i++)
            //printf("test %d:  %lu\n", i, counter[i]);
            printf("%lu\n", counter[i]);
         
            print_packets();
            printf("End of Test!\n\n");
			
			ps();
			
            tm_thread_suspend();
       }

        /* Save the last counter.  */
        counter[counter_index++] = tm_message_processing_counter - last_counter;
        last_counter =  tm_message_processing_counter;
                
        /* Show the time period total.  */
    }
}

#endif