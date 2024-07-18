/*
 * Rick Mesta
 * 07/18/2024
 *
 * University of Colorado at Boulder
 * ECEN 5713: Advanced Embedded Linux Development
 * Assignment 4 (Part 1)
 */
#include "threading.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

// Optional: use these functions to add debug or error prints to your application
// #define DEBUG_LOG(msg,...)
#define DEBUG_LOG(msg,...) printf("threading: " msg "\n" , ##__VA_ARGS__)
#define ERROR_LOG(msg,...) printf("threading ERROR: " msg "\n" , ##__VA_ARGS__)

void    *
threadfunc(void *thread_param)
{
    struct thread_data  *tfa = (struct thread_data *)thread_param;
    int                  terr = 0;

    // TODO: wait, obtain mutex, wait, release mutex as described by thread_data structure
    // hint: use a cast like the one below to obtain thread arguments from your parameter
    //struct thread_data* thread_func_args = (struct thread_data *) thread_param;

    errno = 0;
    if (usleep(tfa->ms_wait * 1000) < 0) {
        ERROR_LOG("%s: Could not wait BEFORE lock: %s", __func__, strerror(errno));
        tfa->thread_complete_success = false;
        return thread_param;
    }

    terr = 0;
    if ((terr = pthread_mutex_lock(tfa->mtx)) != 0) {
        ERROR_LOG("%s: Could not acquire lock: %s", __func__, strerror(terr));
        tfa->thread_complete_success = false;
        return thread_param;
    }

    errno = 0;
    if (usleep(tfa->ms_rele * 1000) < 0) {
        ERROR_LOG("%s: Could not wait AFTER lock: %s", __func__, strerror(errno));
        tfa->thread_complete_success = false;
        return thread_param;
    }

    // Updates done while holding mutex
    tfa->thread_complete_success = pthread_equal(pthread_self(), *tfa->tp);

    terr = 0;
    if ((terr = pthread_mutex_unlock(tfa->mtx)) != 0) {
        ERROR_LOG("%s: Could not release lock: %s", __func__, strerror(terr));
        tfa->thread_complete_success = false;
        return thread_param;
    }

    return thread_param;
}


bool
start_thread_obtaining_mutex(pthread_t *thread, pthread_mutex_t *mutex, int wait_to_obtain_ms, int wait_to_release_ms)
{
    /**
     * TODO: allocate memory for thread_data, setup mutex and wait arguments, pass thread_data to created thread
     * using threadfunc() as entry point.
     *
     * return true if successful.
     *
     * See implementation details in threading.h file comment block
     */
    struct thread_data  *tdp = NULL;
    int                  terr = 0;

    errno = 0;
    tdp = (struct thread_data *)malloc(sizeof(struct thread_data));
    if (tdp == NULL) {
        ERROR_LOG("Could not allocate memory for thread data: %s", strerror(errno));
        return false;
    }

    bzero(tdp, sizeof(struct thread_data));
    tdp->tp = thread;
    tdp->mtx = mutex;
    tdp->ms_wait = wait_to_obtain_ms;
    tdp->ms_rele = wait_to_release_ms;

    /*
     * NB: passed in mutex is already initialized
     * https://github.com/cu-ecen-aeld/assignment-autotest/blob/master/test/assignment4/Test_threading.c#L91
     */

    terr = 0;
    if ((terr = pthread_create(thread, NULL, threadfunc, tdp)) != 0) {
        ERROR_LOG("Could not create thread: %s", strerror(terr));
        free(tdp);
        return false;
    }

    /*
     * NB: Caller free()'s all memory allocation
     */
    return true;
}

