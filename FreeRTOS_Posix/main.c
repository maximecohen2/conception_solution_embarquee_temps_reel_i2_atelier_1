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

#define MAX_TASK (1 << 7)
#define ARRAY_SIZE (1 << 20)

void vSort(void *pvParameters);
void vPivot(int taskId, int first, int last);

int data[ARRAY_SIZE];

struct Parameter {
    int first, last;
};
static struct Parameter parameters[MAX_TASK] = {
    { 0, ARRAY_SIZE-1 } 
};

int main() {
	int i;

	vParTestInitialise();
	vPrintInitialise();

    srand(time(NULL));

    for (i = 0; i<ARRAY_SIZE; i++) {
        data[i] = rand();
    }
    TaskHandle_t taskHandle;

    xTaskCreate(&vSort, "", 20, (void*)0, 1, &taskHandle);

	vTaskStartScheduler();
}

void vSort(void *pvParameters) {
    int taskId = (int)pvParameters;
    struct Parameter params = parameters[taskId];
    vPivot(taskId, params.first, params.last);

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

void vPivot(int taskId, int first, int last) {
    portTickType start = xTaskGetTickCount();
    int pos = first;

    // AFAIRE : condition d'arrêt (autre que true) et Faire la répartition
	if (last - first < 2) {
		return;
	}

    // Répartition du tableau de gauche
    int leftTaskId  = taskId * 2 + 1;
    int rightTaskId;

    if (leftTaskId < MAX_TASK) {
        TaskHandle_t taskHandle;
        
        parameters[leftTaskId].first = first;
        parameters[leftTaskId].last  = pos-1;
        // AFAIRE Créer la tâche pour répartir le tableau de gauche
		rightTaskId = taskId * 2 + 2;
    }
    else {
        // AFAIRE Appel récursif pour répartir le tableau de gauche
		rightTaskId = taskId;
    }
	// Répartition du tableau de droite
	vPivot(rightTaskId, pos + 1, last);
}
