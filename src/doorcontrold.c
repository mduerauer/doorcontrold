/**
 * doorcontrold.c
 * --------------
 *
 * author: Markus Dürauer
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <syslog.h>
#include "pifacedigital.h"

static void skeleton_daemon()
{
    pid_t pid;

    /* Fork off the parent process */
    pid = fork();

    /* An error occurred */
    if (pid < 0)
        exit(EXIT_FAILURE);

    /* Success: Let the parent terminate */
    if (pid > 0)
        exit(EXIT_SUCCESS);

    /* On success: The child process becomes session leader */
    if (setsid() < 0)
        exit(EXIT_FAILURE);

    /* Catch, ignore and handle signals */
    //TODO: Implement a working signal handler */
    signal(SIGCHLD, SIG_IGN);
    signal(SIGHUP, SIG_IGN);

    /* Fork off for the second time*/
    pid = fork();

    /* An error occurred */
    if (pid < 0)
        exit(EXIT_FAILURE);

    /* Success: Let the parent terminate */
    if (pid > 0)
        exit(EXIT_SUCCESS);

    /* Set new file permissions */
    umask(0);

    /* Change the working directory to the root directory */
    /* or another appropriated directory */
    chdir("/");

    /* Close all open file descriptors */
    int x;
    for (x = sysconf(_SC_OPEN_MAX); x>0; x--)
    {
        close (x);
    }

    /* Open the log file */
    openlog ("doorcontrold", LOG_PID, LOG_DAEMON);
}

int main (int argc, char *argv[])
{
	int hw_addr = 0;

	skeleton_daemon();

	while(1)
	{

		syslog(LOG_NOTICE, "Opening piface digital connection at location %d\n", hw_addr);
		pifacedigital_open(hw_addr);

		syslog(LOG_NOTICE, "Closing piface digital connection at location %d\n", hw_addr);
	    	pifacedigital_close(hw_addr);

		sleep(20);
		break;	
	}

	return EXIT_SUCCESS;
}
