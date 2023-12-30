


#include <csp/csp_debug.h>
#include <string.h>
#include <stdlib.h>
#include <csp/csp.h>
#include <pthread.h>

int csp_sys_tasklist(char * out) {

	strcpy(out, "Tasklist not available on OSX");
	return CSP_ERR_NONE;
}

int csp_sys_tasklist_size(void) {

	return 100;
}

uint32_t csp_memfree_hook(void) {

	return 0;  // not implemented
}

unsigned int csp_ps_hook(csp_packet_t * packet) {
	return 0;
}

void csp_reboot_hook(void) {
	exit(0);	 // not implemented
}

void csp_shutdown_hook(void) {
	exit(0);	// not implemented
}
