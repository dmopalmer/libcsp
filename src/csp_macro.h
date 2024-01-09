#pragma once

#include "csp/autoconfig.h"

#if (CSP_ZEPHYR)
#include <zephyr/kernel.h>
#elif (CSP_MACOSX)
#define __noinit __attribute__((section(NOINIT)))
#define __packed __attribute__((__packed__))
#define __unused __attribute__((__unused__))
#undef __weak
#define __weak __attribute__((weak))

#else
#define __noinit __attribute__((section(NOINIT)))
#define __packed __attribute__((__packed__))
#define __unused __attribute__((__unused__))
#define __weak   __attribute__((__weak__))
#endif
