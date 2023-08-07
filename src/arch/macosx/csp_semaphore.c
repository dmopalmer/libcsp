

#include "../../csp_semaphore.h"

#include <sys/time.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include <csp/csp_debug.h>
#include "../posix/pthread_queue.h"

void csp_bin_sem_init(csp_bin_sem_t * sem) {
	//csp_print("Mutex init: %p\n", sem);
	*sem = pthread_queue_create(1, sizeof(int));
	if (*sem) {
		int dummy = 0;
		pthread_queue_enqueue(*sem, &dummy, 0);
	} else {
		// We don't expect to be unable to allocate this unless
		// things are bad enough that we may as well crash.
		csp_print("Failed to allocate a pthread_queue with errno %d\n", errno);
		exit(errno);
	}
}


int csp_bin_sem_wait(csp_bin_sem_t * sem, uint32_t timeout) {
	
	//csp_print("Wait: %p timeout %" PRIu32"\n", sem, timeout);

	int dummy = 0;
	if (pthread_queue_dequeue(*sem, &dummy, timeout) == PTHREAD_QUEUE_OK) {
		return CSP_SEMAPHORE_OK;
	}

	return CSP_SEMAPHORE_ERROR;
}

int csp_bin_sem_post(csp_bin_sem_t * sem) {
	int dummy = 0;
	if (pthread_queue_enqueue(*sem, &dummy, 0) == PTHREAD_QUEUE_OK) {
		return CSP_SEMAPHORE_OK;
	}
	return CSP_SEMAPHORE_ERROR;

}
