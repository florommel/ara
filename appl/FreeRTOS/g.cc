/*
	FreeRTOS.org V5.0.4 - Copyright (C) 2003-2008 Richard Barry.

	This file is part of the FreeRTOS.org distribution.

	FreeRTOS.org is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	FreeRTOS.org is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with FreeRTOS.org; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

	A special exception to the GPL can be applied should you wish to distribute
	a combined work that includes FreeRTOS.org, without being obliged to provide
	the source code for any proprietary components.  See the licensing section 
	of http://www.FreeRTOS.org for full details of how and when the exception
	can be applied.

    ***************************************************************************
    ***************************************************************************
    *                                                                         *
    * SAVE TIME AND MONEY!  We can port FreeRTOS.org to your own hardware,    *
    * and even write all or part of your application on your behalf.          *
    * See http://www.OpenRTOS.com for details of the services we provide to   *
    * expedite your project.                                                  *
    *                                                                         *
    ***************************************************************************
    ***************************************************************************

	Please ensure to read the configuration and relevant port sections of the
	online documentation.

	http://www.FreeRTOS.org - Documentation, latest information, license and 
	contact details.

	http://www.SafeRTOS.com - A version that is certified for use in safety 
	critical systems.

	http://www.OpenRTOS.com - Commercial support, development, porting, 
	licensing and training services.
*/

/* FreeRTOS.org includes. */
#include "source/include/FreeRTOS.h"
#include "source/include/task.h"
#include "source/include/queue.h"
#include "source/include/semphr.h"
#include <stdint.h>
void vPrintString( const char * string );

void vPrintStringAndNumber( const char *string , int32_t number );


/* Declare two variables of type QueueHandle_t. Both queues are added to the same queue set. */

static QueueHandle_t xQueue1 = NULL, xQueue2 = NULL;



/* The handle of the queue from which character pointers are received. */

QueueHandle_t xCharPointerQueue;

/* The handle of the queue from which uint32_t values are received.*/

QueueHandle_t xUint32tQueue;

/* The handle of the binary semaphore. */

SemaphoreHandle_t xBinarySemaphore;

/* The queue set to which the two queues and the binary semaphore belong. */

QueueSetHandle_t xQueueSet;



void vSenderTask1( void *pvParameters )
{

    const TickType_t xBlockTime = pdMS_TO_TICKS( 100 );

    const char * const pcMessage = "Message from vSenderTask1\r\n";

    /* As per most tasks, this task is implemented within an infinite loop. */

    for( ;; )

    {
        /* Block for 100ms. */

        vTaskDelay( xBlockTime );

        /* Send this task's string to xQueue1. It is not necessary to use a block time, even though the queue can only hold one item. This is because the priority of the task that reads from the queue is higher than the priority of this task. As soon as this task writes to the queue, it will be preempted by the task that reads from the queue, so the queue will already be empty  again by the time the call to xQueueSend() returns. The block time is set to 0. */

        xQueueSend( xQueue1, &pcMessage, 0 );

    }
}

/*-----------------------------------------------------------*/

void vSenderTask2( void *pvParameters )
{

    const TickType_t xBlockTime = pdMS_TO_TICKS( 200 );

    const char * const pcMessage = "Message from vSenderTask2\r\n";

    /* As per most tasks, this task is implemented within an infinite loop. */

    for( ;; )

    {

        /* Block for 200ms. */

        vTaskDelay( xBlockTime );

        /* Send this task's string to xQueue2. It is not necessary to use a block time, even though the queue can only hold one item. This is because the priority of the task that reads from the queue is higher than the priority of this task. As soon as this task writes to the queue, it will be preempted by the task that reads from the queue, so the queue will already be empty again by the time the call to xQueueSend() returns. The block time is set to 0. */

        xQueueSend( xQueue2, &pcMessage, 0 );

    }

}




void vAMoreRealisticReceiverTask( void *pvParameters )
{

    QueueSetMemberHandle_t xHandle;

            char *pcReceivedString;

    uint32_t ulRecievedValue;

    const TickType_t xDelay100ms = pdMS_TO_TICKS( 100 );

    for( ;; )

    {

        /* Block on the queue set for a maximum of 100ms to wait for one of the members of the set to contain data. */

        xHandle = xQueueSelectFromSet( xQueueSet, xDelay100ms );

        /* Test the value returned from xQueueSelectFromSet(). If the returned value is NULL, then the call to xQueueSelectFromSet() timed out. If the returned value is not NULL, then the returned value will be the handle of one of the set's members. The QueueSetMemberHandle_t value can be cast to either a QueueHandle_t or a SemaphoreHandle_t. Whether an explicit cast is required depends on the compiler. */

        if( xHandle == NULL )

        {

            /* The call to xQueueSelectFromSet() timed out. */

        }

        else if( xHandle == ( QueueSetMemberHandle_t ) xCharPointerQueue )

        {

            /* The call to xQueueSelectFromSet() returned the handle of the queue that receives character pointers. Read from the queue. The queue is known to contain data, so a block time of 0 is used. */

            xQueueReceive( xCharPointerQueue, &pcReceivedString, 0 );

            /* The received character pointer can be processed here... */

        }

        else if( xHandle == ( QueueSetMemberHandle_t ) xUint32tQueue )

        {

            /* The call to xQueueSelectFromSet() returned the handle of the queue that receives uint32_t types. Read from the queue. The queue is known to contain data, so a block time of 0 is used. */

            xQueueReceive(xUint32tQueue, &ulRecievedValue, 0 );

            /* The received value can be processed here... */

         }

        else if( xHandle == ( QueueSetMemberHandle_t ) xBinarySemaphore )

        {

            /* The call to xQueueSelectFromSet() returned the handle of the binary semaphore. Take the semaphore now. The semaphore is known to be available, so a block time of 0 is used. */

            xSemaphoreTake( xBinarySemaphore, 0 );

            /* Whatever processing is necessary when the semaphore is taken can be performed here... */

        }

    }

}

int main( void )

{

    /* Create the two queues, both of which send character pointers. The priority of the receiving task is above the priority of the sending tasks, so the queues will never have more than one item in them at any one time*/

    xQueue1 = xQueueCreate( 1, sizeof( char * ) );

    xQueue2 = xQueueCreate( 1, sizeof( char * ) );

    /* Create the queue set. Two queues will be added to the set, each of which can contain 1 item, so the maximum number of queue handles the queue set will ever have to hold at one time is 2 (2 queues multiplied by 1 item per queue). */

    xQueueSet = xQueueCreateSet( 1 * 2 );

    /* Add the two queues to the set. */

    xQueueAddToSet( xQueue1, xQueueSet );

    xQueueAddToSet( xQueue2, xQueueSet );

    /* Create the tasks that send to the queues. */

    xTaskCreate( vSenderTask1, "Sender1", 1000, NULL, 1, NULL );

    xTaskCreate( vSenderTask2, "Sender2", 1000, NULL, 1, NULL );

    /* Create the task that reads from the queue set to determine which of the two queues contain data. */

    xTaskCreate( vAMoreRealisticReceiverTask, "Receiver", 1000, NULL, 2, NULL );

    /* Start the scheduler so the created tasks start executing. */

    vTaskStartScheduler();

    /* As normal, vTaskStartScheduler() should not return, so the following lines  will never execute. */

    for( ;; );

    return 0;

}
