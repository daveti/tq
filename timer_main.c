/*
 * timer_main.c
 * A demonstration file used to create a timer control
 * for multiple objects with single one thread using pthread
 * Sep 24, 2013
 * root@davejingtian.org
 * http://davejingtian.org
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>
#include "timer_queue.h"
#include "timer_thread.h"

#define TIMER_MAIN_LOOP_NUM	100

static pthread_t timer_tid;
extern pthread_mutex_t timer_queue_mutex;

static void timer_signal_term(int signal)
{
	printf("Killing the timer thread...\n");
	pthread_kill(timer_tid, SIGTERM);
	tq_destroy_queue_all();
	exit(0);
}

static int signals_init(void)
{
        int rc;
        sigset_t sigmask;
        struct sigaction sa;

        sigemptyset(&sigmask);
        if ((rc = sigaddset(&sigmask, SIGTERM))) {
                printf("sigaddset: %s\n", strerror(errno));
		return -1;
        }

        sa.sa_flags = 0;
        sigemptyset(&sa.sa_mask);
        sa.sa_handler = timer_signal_term;
        if ((rc = sigaction(SIGTERM, &sa, NULL))) {
                printf("signal SIGTERM not registered: %s\n", strerror(errno));
                return -1;
        }

	return 0;
}


int main(void)
{
	int rtn;
	int i;
	timer_queue_msg tqm;

	/* Set the signals */
	rtn = signals_init();
	if (rtn != 0)
	{
		printf("setting up signals failed\n");
		return -1;
	}

	/* Init the timer queue */
	tq_init_queue_all();

	/* Create the timer thread */
	rtn = pthread_create(&timer_tid, NULL, timer_thread_main, NULL);
	if (rtn != 0)
	{
		printf("creating a new thread failed: %s\n", strerror(errno));
		return -1;
	}

	/* Do the main job */
	for (i = 0; i < TIMER_MAIN_LOOP_NUM; i++)
	{
		printf("main thread: %d [%lu]\n", i, time(NULL));

		/* Lock */
		pthread_mutex_lock(&timer_queue_mutex);
		printf("main thread: got the timer queue lock [%lu]\n", time(NULL));

		if (i % 2 == 0)
		{
			/* Add entry into MAC black list */
			tqm.type = TIMER_QUEUE_MSG_TYPE_MAC;
			snprintf(tqm.mac, TIMER_QUEUE_MAC_STRING_LEN, "%s%02x", "ff:ff:ff:ff:ff:", i);
			time(&(tqm.timer));
			tqm.timer += (time_t)TIMER_THREAD_BLACKLIST_BLOCK_TIME;
			if (tq_add_msg(&tqm, TIMER_THREAD_BLACKLIST_MAC) == -1)
				printf("adding MAC msg [%d] into MAC black list failed\n", i);
		}
		else
		{
			/* Add entry into IPv4 black list */
                        tqm.type = TIMER_QUEUE_MSG_TYPE_IPV4;
                        snprintf(tqm.mac, TIMER_QUEUE_IPV4_STRING_LEN, "%s%d", "255.255.255.", i);
                        time(&(tqm.timer));
			tqm.timer += (time_t)TIMER_THREAD_BLACKLIST_BLOCK_TIME;
                        if (tq_add_msg(&tqm, TIMER_THREAD_BLACKLIST_IPV4) == -1)
                                printf("adding IPv4 msg [%d] into IPv4 black list failed\n", i);
		}

		/* Unlock */
		pthread_mutex_unlock(&timer_queue_mutex);
		printf("main thread: released the timer queue lock [%lu]\n", time(NULL));

		tq_display_msg(&tqm);
		sleep(2);
	}

	printf("main thread: done - wait for the timer thread [%lu]\n", time(NULL));
	/* Lock */
	pthread_mutex_lock(&timer_queue_mutex);
	printf("==============================\n");
	tq_display_queue_all();
	printf("==============================\n");
	/* Unlock */
	pthread_mutex_unlock(&timer_queue_mutex);

	pthread_join(timer_tid, NULL);

	return 0;
}
