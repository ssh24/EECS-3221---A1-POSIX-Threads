/*
 * alarm_mutex.c
 *
 * This is an enhancement to the alarm_thread.c program, which
 * created an "alarm thread" for each alarm command. This new
 * version uses a single alarm thread, which reads the next
 * entry in a list. The main thread places new requests onto the
 * list, in order of absolute expiration time. The list is
 * protected by a mutex, and the alarm thread sleeps for at
 * least 1 second, each iteration, to ensure that the main
 * thread can lock the mutex to add new work to the list.
 */
#include <pthread.h>
#include <time.h>
#include "errors.h"
#include <limits.h>

/*
 * The "alarm" structure now contains the time_t (time since the
 * Epoch, in seconds) for each alarm, so that they can be
 * sorted. Storing the requested number of seconds would not be
 * enough, since the "alarm thread" cannot tell how long it has
 * been on the list.
 */
typedef struct alarm_tag {
    struct alarm_tag    *link;
    int                 seconds;
    time_t              time;   /* seconds from EPOCH */
    char                message[64];
} alarm_t;

pthread_mutex_t alarm_mutex = PTHREAD_MUTEX_INITIALIZER;
alarm_t *alarm_list = NULL;


void *alarm_thread_spawn (void *arg)
{
	alarm_t *alarm = (alarm_t*)arg;
	int status;
	int timeR;
	time_t now;
	struct tm *local_time;
	char buffer[80];
	
	timeR = alarm->seconds;
	
	status = pthread_detach (pthread_self ());
	if (status != 0)
		err_abort (status, "Detach thread");
	
	while ((alarm->time) > 0)
	{ 
		for(; timeR>0; timeR--)
		{
			
            printf("\"Alarm:>: %d %s\"\n",alarm->seconds, alarm->message, time(NULL));
			sleep(1);
		
		}
		sleep (timeR);
		now = time(NULL);

		printf ("\"Alarm expired at %d: %d %s\"\n", time(NULL), alarm->seconds, alarm->message);
		alarm->time = 0;
	}
	return NULL; 
}




/*
 * The alarm thread's start routine.
 */ 
 

void *alarm_thread (void *arg)
{
    alarm_t *alarm;
    int sleep_time;
    time_t now;
    int status;
    pthread_t thread;

    /*
     * Loop forever, processing commands. The alarm thread will
     * be disintegrated when the process exits.
     */

    while (1) {
        status = pthread_mutex_lock (&alarm_mutex);
        if (status != 0)
            err_abort (status, "Lock mutex");
        alarm = alarm_list;

        /*
         * If the alarm list is empty, wait for one second. This
         * allows the main thread to run, and read another
        
 * command. If the list is not empty, remove the first
         * item. Compute the number of seconds to wait -- if the
         * result is less than 0 (the time has passed), then set
         * the sleep_time to 0.
         */
        if (alarm == NULL){
        	
            //sleep_time = 1;
        }
        else {
            alarm_list = alarm->link;
            now = time (NULL);
            status = pthread_create (&thread, NULL, alarm_thread_spawn, alarm);
            if (alarm->time <= now)
                sleep_time = 0;
            else
                sleep_time = alarm->time - now;
        
                printf ("\"Alarm Retrieved at %d: %d %s\"\n", now, alarm->seconds, alarm->message);

			//printf("\"Alarm:>:%d\"\n",alarm->message);			
            
            }

        /*
         * Unlock the mutex before waiting, so that the main
         * thread can lock it to insert a new alarm request. If
         * the sleep_time is 0, then call sched_yield, giving
         * the main thread a chance to run if it has been
         * readied by user input, without delaying the message
         * if there's no input.
         */
        status = pthread_mutex_unlock (&alarm_mutex);
        if (status != 0)
            err_abort (status, "Unlock mutex");
        if (sleep_time > 0){
            //int i = 0;
            //for(; i < alarm->seconds; i++)
        //{
            //printf("\"Alarm:>%d....%d\"\n",alarm->message, time(NULL));
           // sleep (sleep_time);
        }
            
        //}

        else
            sched_yield ();

        /*
         * If a timer expired, print the message and free the
         * structure.
         */
        if (alarm != NULL) {
            //fprintf(stdout, "%u\n", (unsigned)time(NULL));
            // printf ("\"Alarm Expired at %d: %s\"", alarm->seconds, alarm->message);
            // printf("%s", "hello");
            // free (alarm);
            //printf ("\"Alarm expired at %d: %s\"\n", time(NULL), alarm->message);
 //           free (alarm);
        }
    }
}

int main (int argc, char *argv[])
{
    int status;
    char line[128];
    alarm_t *alarm, **last, *next;
    pthread_t thread;

    status = pthread_create (
        &thread, NULL, alarm_thread, NULL);
    
    if (status != 0)
        err_abort (status, "Create alarm thread");
    while (1) {
        printf ("alarm> ");
        if (fgets (line, sizeof (line), stdin) == NULL) exit (0);
        if (strlen (line) <= 1) continue;
        alarm = (alarm_t*)malloc (sizeof (alarm_t));
        if (alarm == NULL)
            errno_abort ("Allocate alarm");

        /*
         * Parse input line into seconds (%d) and a message
         * (%64[^\n]), consisting of up to 64 characters
         * separated from the seconds by whitespace.
         */
         
         if (sscanf (line, "%f %64[^\n]", 
            &alarm->seconds, alarm->message) < 2 ) {
         
         		fprintf (stderr, "Please enter an integer for time -> %s\n", line);
            free (alarm);
         
         
            }
         
        if (sscanf (line, "%d %64[^\n]", 
            &alarm->seconds, alarm->message) < 2) {
        
            fprintf (stderr, "Bad command -> %s\n", line);
            free (alarm);
            }
        
        	else if (alarm->seconds <= 0 || alarm->seconds > INT_MAX){
        		fprintf(stderr,"Please enter time which is greater than 0 seconds and less than %d\n", INT_MAX);
        		free (alarm);
        	}
        	
        	
             else {
            status = pthread_mutex_lock (&alarm_mutex);
            if (status != 0)
                err_abort (status, "Lock mutex");
            
            alarm->time = time (NULL) + alarm->seconds;
            
            printf ("\"Alarm Recieved at %d: %d %s\"\n", time(NULL), alarm -> seconds, alarm->message);

            /*
             * Insert the new alarm into the list of alarms,
             * sorted by expiration time.
             */

            last = &alarm_list;
            next = *last;
            while (next != NULL) {
                if (next->time >= alarm->time) {
                    alarm->link = next;
                    *last = alarm;
                    break;
                }
                last = &next->link;
                next = next->link;
            }
            /*
             * If we reached the end of the list, insert the new
             * alarm there. ("next" is NULL, and "last" points
             * to the link field of the last item, or to the
             * list header).
             */
            if (next == NULL) {
                *last = alarm;
                alarm->link = NULL;
            }
#ifdef DEBUG
            printf ("[list: ");
            for (next = alarm_list; next != NULL; next = next->link)
                printf ("%d(%d)[\"%s\"] ", next->time,
                    next->time - time (NULL), next->message);
            printf ("]\n");
#endif
            status = pthread_mutex_unlock (&alarm_mutex);
            if (status != 0)
                err_abort (status, "Unlock mutex");
            
            /*status = pthread_cond_signal (&alarm_cond); 
			if (status != 0)
				err_abort (status, "Signal cond");*/
        }
    }
}