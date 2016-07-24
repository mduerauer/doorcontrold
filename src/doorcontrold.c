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
#include <string.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <syslog.h>
#include "pifacedigital.h"
#include "doorcontrold.h"

#define DAEMONIZE 0

static void daemonize()
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

static volatile int exit_flag = 0;

static void hdl (int sig) {

	if(sig == SIGTERM) {
		md_outpt("SIGTERM caught. Exiting.");
		exit_flag = 1;
	}

	md_outpt("Signal %i caught.", sig);
}

int main (int argc, char *argv[])
{
	int hw_addr = 0;
	struct sigaction act;

	if(DAEMONIZE == 1)
	{
		daemonize();
	}


	memset (&act, '\0', sizeof(act));
	act.sa_handler = &hdl;
	if (sigaction(SIGTERM, &act, NULL) > 0)
	{
		perror ("sigaction");
		return 1;
	}


	md_outpt("Opening piface digital connection at location %d\n", hw_addr);
	pifacedigital_open(hw_addr);

	while(!exit_flag)
	{

		toggle (5);

	}

	md_outpt("Closing piface digital connection at location %d\n", hw_addr);
    	pifacedigital_close(hw_addr);

	return EXIT_SUCCESS;
}

void toggle(int i)
{
        pifacedigital_digital_write(i, 1);
        sleep(1);

        pifacedigital_digital_write(i, 0);
        sleep(1);
}

void md_outpt(const char* s, ...)
{

	va_list ap;
	va_start (ap, s);

		if(DAEMONIZE == 1)
	{
		syslog(LOG_NOTICE, s, ap);
	}
	else
	{
		printf(s, ap);
	}

	va_end(ap);
}
