# Conception d'une solution embarquée temps-réel - Atelier 1

## Groupe
- Lucas SOUMAILLES
- Maxime COHEN

## Réalisation
Le programme à été réaliser sur linux

main.c
```c
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <stdlib.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <errno.h>
#include <unistd.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "croutine.h"
#include "partest.h"

#define LINUX

#ifdef LINUX
typedef xTaskHandle TaskHandle_t;

#define portTICK_PERIOD_MS 1

static unsigned long uxQueueSendPassedCount = 0;

void vPrintStringAndNumber(const char *s, int num) {
	printf("%s %d\n", s, num);
}

void vMainQueueSendPassed( void )
{
	uxQueueSendPassedCount++;
}

void vApplicationIdleHook( void )
{
	/* The co-routines are executed in the idle task using the idle task hook. */
	vCoRoutineSchedule();	/* Comment this out if not using Co-routines. */

#ifdef __GCC_POSIX__
	struct timespec xTimeToSleep, xTimeSlept;
		/* Makes the process more agreeable when using the Posix simulator. */
		xTimeToSleep.tv_sec = 1;
		xTimeToSleep.tv_nsec = 0;
		nanosleep( &xTimeToSleep, &xTimeSlept );
#endif
}
#endif

#define MAX_TASK (1 << 3)
//#define ARRAY_SIZE (1 << 20)
#define ARRAY_SIZE (1 << 20)

void vSort(void *pvParameters);
void vPivot(int first, int last, int taskId, int lastId);

int data[ARRAY_SIZE];

struct Parameter {
    int first, last, lastId;
};
static struct Parameter parameters[MAX_TASK] = {
    { 0, ARRAY_SIZE-1, MAX_TASK - 1 }
};

void printData()
{
    int i = 0;
    while (i < ARRAY_SIZE)
    {
        printf("%d ", data[i]);
        ++i;
    }
    printf("\n");
    fflush(stdout);
}


int main() {
	int i;

	vParTestInitialise();
	vPrintInitialise();

    srand(time(NULL));

    for (i = 0; i < ARRAY_SIZE; i++) {
        data[i] = rand();
    }
    TaskHandle_t taskHandle;

    printData();
    xTaskCreate(&vSort, "", 20, (void*)0, 1, &taskHandle);

	vTaskStartScheduler();
}

void vSort(void *pvParameters) {
    int taskId = (int)pvParameters;
    struct Parameter params = parameters[taskId];

    vPivot(params.first, params.last, taskId, params.lastId);

    if (taskId == 0) {
		while(uxTaskGetNumberOfTasks()>1) {
			usleep(1);
		}
        // AFAIRE calculer et afficher la durée (et le nombre de tâches)
        // AFAIRE afficher le tableau
        vPrintStringAndNumber("Fin du tri : ", 0);
    }
	vTaskDelete(NULL);
}

void swap(int firstId, int lastId) {
    int temp;

    temp = data[firstId];
    data[firstId] = data[lastId];
    data[lastId] = temp;
}

void vPivot(int first, int last, int taskId, int lastId) {
    portTickType start = xTaskGetTickCount();
    int pos = first;

    // AFAIRE : condition d'arrêt (autre que true) et Faire la répartition
	if (first >= last) {
		return;
	}

	int i = first;
	int j = last;
	while (i < j)
    {
        while (data[i] <= data[pos] && i < last)
            ++i;
        while (data[j] > data[pos])
            --j;
        if (i < j)
            swap(i, j);
    }
    swap(pos, j);
	pos = i;

    // Répartition du tableau de gauche
    if (taskId != lastId) {
        TaskHandle_t taskHandle;
        int newTaskId = (taskId + lastId + 1) / 2;

        parameters[newTaskId].first = first;
        parameters[newTaskId].last  = pos - 1;
        parameters[newTaskId].lastId = lastId;
        lastId = newTaskId - 1;
        xTaskCreate(vSort, "", 50, (void*)newTaskId, 1, &taskHandle);

    }
    else {
        vPivot(first, pos - 1, taskId, lastId);
    }
	// Répartition du tableau de droite
    vPivot(pos + 1, last, taskId, lastId);
    printData();
}

```

