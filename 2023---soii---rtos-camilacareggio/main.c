/*
 * FreeRTOS V202212.01
 * Copyright (C) 2020 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * https://www.FreeRTOS.org
 * https://github.com/FreeRTOS
 *
 */


/*
 * This project contains an application demonstrating the use of the
 * FreeRTOS.org mini real time scheduler on the Luminary Micro LM3S811 Eval
 * board.  See http://www.FreeRTOS.org for more information.
 */

/* Environment includes. */
#include "DriverLib.h"

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

/* Demo app includes. */
#include "integer.h"
#include "PollQ.h"
#include "semtest.h"
#include "BlockQ.h"

#include <string.h>

#define mainCHECK_DELAY				    ( ( TickType_t ) 100 / portTICK_PERIOD_MS ) // 0.1 segundos -> Frecuencia 10Hz (1/10)
#define mainCHECK_TASK_PRIORITY			( tskIDLE_PRIORITY + 3 )
#define mainQUEUE_SIZE					( 10 )
#define mainTOP_DELAY             		( ( TickType_t ) 3000 / portTICK_PERIOD_MS )

/* UART configuration*/
#define mainBAUD_RATE		          	( 19200 )
#define mainFIFO_SET					( 0x10 )
#define timerINTERRUPT_FREQUENCY		( 20000UL )

/* Global variables */
#define N_ARRAY                   		20
#define MAX_TEMP                		30
#define MIN_TEMP                		10
#define DISPLAY_COLUMNS            		96

/* Tasks */
static void vSensor(void *);
static void vFilter(void *);
static void vDisplay(void *);
static void vTop(void *);

/* Hardware functions */
void prvSetupHardware(void);
void setupTimer0(void);
void Timer0IntHandler(void);
unsigned long getTimerTicks(void);

/* Specific functions */
void updateArray(int[], int, int);
char* bitMapping(int);
void sendUART0(const char *);

/* Getters and Setters */
int getN(void);
void setN(int new_N);

/* Utility functions*/
void reverse(char str[], int length);
char* itoa(int num, char* str, int base);

/* Queues */
QueueHandle_t xSensorQueue;
QueueHandle_t xFilterQueue;

/* Variables */
static int temperature_value;
static int N_filter;
static volatile char *pcNextChar;
unsigned long ulHighFrequencyTimerTicks;
TaskStatus_t *pxTaskStatusArray;

int main(void) {
  	prvSetupHardware();

	/* Create queues */
	xSensorQueue = xQueueCreate(mainQUEUE_SIZE, sizeof(int));
	xFilterQueue = xQueueCreate(mainQUEUE_SIZE, sizeof(int));

  	temperature_value = 20;
  	N_filter = 15;

	/* Init display */
  	OSRAMInit(false);

	/* Create Tasks */
	xTaskCreate(vSensor, "Sensor", configSENSOR_STACK_SIZE, NULL, mainCHECK_TASK_PRIORITY - 2, NULL);
  	xTaskCreate(vFilter, "Filter", configFILTER_STACK_SIZE, NULL, mainCHECK_TASK_PRIORITY - 2, NULL);
  	xTaskCreate(vDisplay, "Display", configDISPLAY_STACK_SIZE, NULL, mainCHECK_TASK_PRIORITY - 2, NULL);
  	xTaskCreate(vTop, "Top", configTOP_STACK_SIZE, NULL, mainCHECK_TASK_PRIORITY - 1, NULL);

	/* Start the scheduler. */
	vTaskStartScheduler();

	/* Will only get here if there was insufficient heap to start the
	scheduler. */

	return 0;
}

static void vSensor(void *pvParameters) {
	const TickType_t xDelaySensor = mainCHECK_DELAY;
	
	while (true) {
		/* Sensor gets temperature value */
		if(temperature_value <= MAX_TEMP) {
			temperature_value++;
		} else {
			temperature_value = MIN_TEMP;
		}
		xQueueSend(xSensorQueue, &temperature_value, portMAX_DELAY);

		vTaskDelay(xDelaySensor); // waits mainCHECK_DELAY milliseconds for 10Hz frequency
	}
}

static void vFilter(void *pvParameters) {
  	int new_temp_value; // value read from sensor
  	int avg_temp;

	/* create array to store latest temperature values read */
  	int temperature_array[N_ARRAY] = {};

  	while (true) {
    	xQueueReceive(xSensorQueue, &new_temp_value, portMAX_DELAY);

		/* add new temperature value to array */
    	updateArray(temperature_array, N_ARRAY, new_temp_value);

		/* Calculate average */
		int accum = 0;
		for (int i = 0; i < getN(); i++) {
			accum += temperature_array[i];
		}
		avg_temp = accum / getN();

		xQueueSend(xFilterQueue, &avg_temp, portMAX_DELAY);
  	}
}

static void vDisplay(void *pvParameters) {
	/* create array with latest filtered values */
  	int filtered_array[DISPLAY_COLUMNS] = {};

	/* initialize array with minimum temperature value */
  	for (int i = 0 ; i < DISPLAY_COLUMNS; i++) {
    	filtered_array[i] = MIN_TEMP;
  	}

	int new_filtered_value;

	while (true) {
		xQueueReceive(xFilterQueue, &new_filtered_value, portMAX_DELAY);

		/* stores the conversion int to char */
		char str[2] = {};

		/* update array with new filtered value */
		updateArray(filtered_array, DISPLAY_COLUMNS, new_filtered_value);

		/* clear display */
		OSRAMClear();

		for (int i = DISPLAY_COLUMNS - 1; i > 0; i--) {
			/* draw N of filter value */
			OSRAMStringDraw(itoa(filtered_array[i], str, 10), 0, 1);

			/* draw temperature filtered value */
			OSRAMStringDraw(itoa(getN(), str, 10), 0, 0);

			/* draw graphic on the display */
			int bit_map_half = filtered_array[DISPLAY_COLUMNS - i] >= 20 ? 0 : 1;
			OSRAMImageDraw(bitMapping(filtered_array[DISPLAY_COLUMNS - i]), i+10, bit_map_half , 1, 1);
		}
	}
}

static void vTop(void *pvParameters) {
	/* Take a snapshot of the number of tasks in case it changes while this
   	function is executing. */
	UBaseType_t uxArraySize = uxTaskGetNumberOfTasks();

	/* Allocate a TaskStatus_t structure for each task.  An array could be
   	allocated statically at compile time. */
	pxTaskStatusArray = pvPortMalloc(uxArraySize * sizeof(TaskStatus_t));

	while (true) {
		volatile UBaseType_t uxArraySize, x;
		unsigned long ulTotalRunTime, ulStatsAsPercentage;

		if (pxTaskStatusArray != NULL) {
			/* Generate raw status information about each task. */
			uxArraySize = uxTaskGetSystemState( pxTaskStatusArray, uxArraySize, &ulTotalRunTime );

			/* For percentage calculations. */
			ulTotalRunTime /= 100UL;

			/* Send via UART. */
			sendUART0("\r");

			/* Avoid divide by zero errors. */
			if (ulTotalRunTime > 0) {
				char* current_N[2] = {};
				sendUART0("\r\n\r\n----------- TASK STATS -----------\r\n\r\n");
				sendUART0("Filter N: ");
				sendUART0(itoa(getN(), current_N, 10));
				sendUART0("\r\n\r\n");
				sendUART0("Task\tTime\tCPU%\tStack Free\r\n");

				/* For each populated position in the pxTaskStatusArray array,
         		format the raw data as human readable ASCII data. */
				for (x = 0; x < uxArraySize; x++) {
					/* What percentage of the total run time has the task used?
					This will always be rounded down to the nearest integer.
					ulTotalRunTimeDiv100 has already been divided by 100. */
					ulStatsAsPercentage = pxTaskStatusArray[x].ulRunTimeCounter / ulTotalRunTime;

					char buffer[8];

					/* Send Tasks Names */
					sendUART0(pxTaskStatusArray[x].pcTaskName);
					sendUART0("\t");

					/* Send Tasks Ticks */
					itoa(pxTaskStatusArray[x].ulRunTimeCounter, buffer, 10);
					sendUART0(buffer);
					sendUART0("\t");

					/* Send Tasks CPU% usage */
					if (ulStatsAsPercentage > 0UL) {
						itoa(ulStatsAsPercentage, buffer, 10);
						sendUART0(buffer);
						sendUART0("%\t");
					} else {
						/* If the percentage is zero here then the task has
               			consumed less than 1% of the total run time. */
						sendUART0("<1%\t");
					}

					/* Send Tasks Stack Free */
					/* Closer to 0, closer to overflow */
					itoa(uxTaskGetStackHighWaterMark(pxTaskStatusArray[x].xHandle), buffer, 10);
					sendUART0(buffer);
					sendUART0(" words\r\n");
				}
			}
		}

		vTaskDelay(mainTOP_DELAY);
	}
}

/* Hardware Set up */
void prvSetupHardware(void) {
	/* Enable the UART.  */
	SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);

	/* Configure the UART for 8-N-1 operation. 
		8 bit word lenght, one stop bit, no parity*/
	UARTConfigSet(UART0_BASE, mainBAUD_RATE, (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));

	/* Enable the UART interrupt. */
  	UARTIntEnable(UART0_BASE, UART_INT_RX);
  	UARTIntEnable(UART0_BASE, UART_INT_TX);

	/* Set priority for UART interrupt. */
	HWREG( UART0_BASE + UART_O_IM ) |= UART_INT_TX;
	HWREG( UART0_BASE + UART_O_IM ) |= UART_INT_RX;
	IntPrioritySet(INT_UART0, configKERNEL_INTERRUPT_PRIORITY);
	IntEnable(INT_UART0);
}

/* Timer0 set up and configuration */
void setupTimer0(void) {
	/* Enable Timer0 */
  	SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
  	TimerConfigure(TIMER0_BASE, TIMER_CFG_32_BIT_TIMER);

	/* Ensure interrupts do not start until the scheduler is running. */
	portDISABLE_INTERRUPTS();

	/* Allow global interrupt */
	IntMasterEnable();
  	
	/* Set Timer0 interrupt */
	unsigned long ulFrequency = configCPU_CLOCK_HZ / timerINTERRUPT_FREQUENCY;
  	TimerLoadSet(TIMER0_BASE, TIMER_A, ulFrequency);
  	TimerIntRegister(TIMER0_BASE, TIMER_A, Timer0IntHandler);
	TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

	/* Enable Timer0 */
  	TimerEnable(TIMER0_BASE, TIMER_A);
}

/* Timer0 Interrupt Handler */
void Timer0IntHandler(void) {
	TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
	ulHighFrequencyTimerTicks++;
}

/* Get timer Ticks */
unsigned long getTimerTicks(void) {
  	return ulHighFrequencyTimerTicks;
}

/* Update array with new value and dismiss the oldest value */
void updateArray(int array[], int size, int new_value) {
	for(int i = size - 1; i > 0; i--) {
		array[i] = array[i-1];
	}
	array[0] = new_value;
}

/* Map each temperature value with a pixel in the display */
char* bitMapping(int valor) {
	if ((valor <= 13) || (valor == 20)) {
		return "@";
	}

	if ((valor == 14) || (valor == 21)) {
		return " ";
	}

	if ((valor == 15) || (valor == 22)) {
		return "";
	}

	if ((valor == 16) || (valor == 23)){
		return "";
	}

	if ((valor == 17) || (valor == 24)){
		return "";
	}

	if ((valor == 18) || (valor == 25)) {
		return "";
	}

	if ((valor == 19) || (valor == 26)) {
		return "";
	}
}

void vUART_ISR(void){
	unsigned long ulStatus;

	/* What caused the interrupt. */
	ulStatus = UARTIntStatus( UART0_BASE, pdTRUE );

	/* Clear the interrupt. */
	UARTIntClear( UART0_BASE, ulStatus );

	/* Was a Tx interrupt pending? */
	if( ulStatus & UART_INT_TX )
	{
		/* Send the next character in the string.  We are not using the FIFO. */
		if( *pcNextChar != 0 )
		{
			if( !( HWREG( UART0_BASE + UART_O_FR ) & UART_FR_TXFF ) )
			{
				HWREG( UART0_BASE + UART_O_DR ) = *pcNextChar;
			}
			pcNextChar++;
		}
	}
	/* Was a Rx interrupt pending? */
    else if (ulStatus & UART_INT_RX) {
        signed char input;
        portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;

        /* A character was received.  Place it in the queue of received characters. */
        input = UARTCharGet(UART0_BASE);
        if (input == '+' && getN() < N_ARRAY) {
            setN(getN() + 1);
			sendUART0("\r\nN incremented\r\n");
        }
		else if(input == '-' && getN() > 1){
			setN(getN() - 1);
			sendUART0("\r\nN decremented\r\n");
		}
		else {
			sendUART0("\r\nCan't permform that operation on N, limit reached \r\n");
		}
        portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
    }
}

/* Send and print string through UART0 */
void sendUART0(const char *string) {
	while (*string != '\0') {
		UARTCharPut(UART0_BASE, *string);
		string++;
	}
}

/* Get N of filter */
int getN(void) {
	return N_filter;
}

/* Set N of filter */
void setN(int new_N) {
	N_filter = new_N;
}

// A utility function to reverse a string
void reverse(char str[], int length){
    int start = 0;
    int end = length - 1;
    while (start < end) {
        char temp = str[start];
        str[start] = str[end];
        str[end] = temp;
        end--;
        start++;
    }
}

/* Convert integer to string: implementation of stdio itoa()*/
char* itoa(int num, char* str, int base){
    int i = 0;
    int isNegative = 0;
 
    /* Handle 0 explicitly, otherwise empty string is
     * printed for 0 */
    if (num == 0) {
        str[i++] = '0';
        str[i] = '\0';
        return str;
    }
 
    // In standard itoa(), negative numbers are handled
    // only with base 10. Otherwise numbers are
    // considered unsigned.
    if (num < 0 && base == 10) {
        isNegative = 1;
        num = -num;
    }
 
    // Process individual digits
    while (num != 0) {
        int rem = num % base;
        str[i++] = (rem > 9) ? (rem - 10) + 'a' : rem + '0';
        num = num / base;
    }
 
    // If number is negative, append '-'
    if (isNegative)
        str[i++] = '-';
 
    str[i] = '\0'; // Append string terminator
 
    // Reverse the string
    reverse(str, i);
 
    return str;
}