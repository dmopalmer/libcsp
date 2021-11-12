#define CSP_USE_ASSERT 1  // always enable CSP assert

#include <zephyr.h>
#include <csp/csp_debug.h>
#include <csp/arch/csp_clock.h>
#include <csp/arch/csp_time.h>
#include <csp/arch/csp_queue.h>
#include <csp/arch/csp_semaphore.h>

#include <stdlib.h>

static bool thread_executed = false;

void csp_assert_fail_action(const char *assertion, const char *file, int line) {
    printf("assertion: [%s], file: %s:%d\r\n", assertion, file, line);
    exit(1);
}

void thread_func(void * p1, void * p2, void * p3) {
    csp_log_info("Thread started");
    thread_executed = true;
    csp_sleep_ms(10000); // safty - ensure process terminates
    exit(1);
    return;
}

#define STACKSIZE 1024
K_THREAD_STACK_DEFINE(stack, STACKSIZE);

int main(int argc, char * argv[]) {

    // debug/log - enable all levels
    for (int i = 0; i <= CSP_LOCK; ++i) {
        csp_debug_set_level(i, true);
    }
    csp_log_error("csp_log_error(...), level: %d", CSP_ERROR);
    csp_log_warn("csp_log_warn(...), level: %d", CSP_WARN);
    csp_log_info("csp_log_info((...), level: %d", CSP_INFO);
    csp_log_buffer("csp_log_buffer(...), level: %d", CSP_BUFFER);
    csp_log_packet("csp_log_packet(...), level: %d", CSP_PACKET);
    csp_log_protocol("csp_log_protocol(...), level: %d", CSP_PROTOCOL);
    csp_log_lock("csp_log_lock(...), level: %d", CSP_LOCK);

    // create a thread - csp_thread doesn't support join
    k_tid_t tid;
    struct k_thread new_thread;

    tid = k_thread_create(&new_thread,
						  stack, K_THREAD_STACK_SIZEOF(stack),
						  thread_func, NULL, NULL, NULL,
						  0, 0, K_NO_WAIT);
    csp_assert(tid != NULL);

    // clock
    csp_timestamp_t csp_clock = {};
    csp_clock_get_time(&csp_clock);
    csp_assert(csp_clock.tv_sec != 0);
    csp_log_info("csp_clock_get_time(..) -> sec:nsec = %"PRIu32":%"PRIu32, csp_clock.tv_sec, csp_clock.tv_nsec);

    // relative time
    const uint32_t msec1 = csp_get_ms();
    const uint32_t msec2 = csp_get_ms_isr();
    const uint32_t sec1 = csp_get_s();
    const uint32_t sec2 = csp_get_s_isr();
    csp_sleep_ms(2000);

    csp_assert(csp_get_ms() >= (msec1 + 500));
    csp_assert(csp_get_ms_isr() >= (msec2 + 500));
    csp_assert(csp_get_s() >= (sec1 + 1));
    csp_assert(csp_get_s_isr() >= (sec2 + 1));

    // check thread actually executed
    csp_assert(thread_executed != false);

    // queue handling
    uint32_t value;
    csp_static_queue_t sq;
    csp_queue_handle_t q;
    char buf[3 * sizeof(value)];
    q = csp_queue_create_static(3, sizeof(value), buf, &sq);
    csp_assert(csp_queue_size(q) == 0);
    csp_assert(csp_queue_size_isr(q) == 0);
    csp_assert(csp_queue_dequeue(q, &value, 0) == CSP_QUEUE_ERROR);
    csp_assert(csp_queue_dequeue(q, &value, 200) == CSP_QUEUE_ERROR);
    csp_assert(csp_queue_dequeue_isr(q, &value, NULL) == CSP_QUEUE_ERROR);
    value = 1;
    csp_assert(csp_queue_enqueue(q, &value, 0) == CSP_QUEUE_OK);
    value = 2;
    csp_assert(csp_queue_enqueue(q, &value, 200) == CSP_QUEUE_OK);
    value = 3;
    csp_assert(csp_queue_enqueue_isr(q, &value, NULL) == CSP_QUEUE_OK);
    csp_assert(csp_queue_size(q) == 3);
    csp_assert(csp_queue_size_isr(q) == 3);
    value = 10;
    csp_assert(csp_queue_enqueue(q, &value, 0) == CSP_QUEUE_ERROR);
    value = 20;
    csp_assert(csp_queue_enqueue(q, &value, 200) == CSP_QUEUE_ERROR);
    value = 30;
    csp_assert(csp_queue_enqueue_isr(q, &value, NULL) == CSP_QUEUE_ERROR);
    value = 100;
    csp_assert(csp_queue_dequeue(q, &value, 0) == CSP_QUEUE_OK);
    csp_assert(value == 1);
    csp_assert(csp_queue_dequeue(q, &value, 200) == CSP_QUEUE_OK);
    csp_assert(value == 2);
    csp_assert(csp_queue_dequeue_isr(q, &value, NULL) == CSP_QUEUE_OK);
    csp_assert(value == 3);

    // mutex - the actual mutex lock can't be tested from a single thread
    csp_mutex_t m;
    csp_mutex_create_static(&m, NULL);
    csp_assert(csp_mutex_lock(&m, 0) == CSP_MUTEX_OK);
#if (CSP_WINDOWS || CSP_ZEPHYR) // implementations differ in return value if already locked
    csp_assert(csp_mutex_lock(&m, 200) == CSP_MUTEX_OK);
#else
    csp_assert(csp_mutex_lock(&m, 200) == CSP_MUTEX_ERROR);
#endif
    csp_assert(csp_mutex_unlock(&m) == CSP_MUTEX_OK);
    csp_assert(csp_mutex_lock(&m, 200) == CSP_MUTEX_OK);
    csp_assert(csp_mutex_unlock(&m) == CSP_MUTEX_OK);

    // semaphore
    csp_bin_sem_handle_t s;
    csp_bin_sem_create_static(&s, NULL);
    csp_assert(csp_bin_sem_wait(&s, 0) == CSP_SEMAPHORE_OK);
    csp_assert(csp_bin_sem_post(&s) == CSP_SEMAPHORE_OK);
#if (CSP_POSIX || CSP_ZEPHYR) // implementations differ in return value if already posted/signaled
    csp_assert(csp_bin_sem_post_isr(&s, NULL) == CSP_SEMAPHORE_OK);
#else
    csp_assert(csp_bin_sem_post_isr(&s, NULL) == CSP_SEMAPHORE_ERROR);
#endif
    csp_assert(csp_bin_sem_wait(&s, 200) == CSP_SEMAPHORE_OK);
    csp_assert(csp_bin_sem_wait(&s, 200) == CSP_SEMAPHORE_ERROR);

    return 0;
}
