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
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <syslog.h>
#include "pifacedigital.h"
#include "doorcontrold.h"

#define DAEMONIZE 1
#define PIN_DOOR_RELAY 0
#define PIN_STANDBY 7
#define PIN_DOOR_FREE 6
#define PIN_SWITCH_PRESSED 5
#define IN_PIN_SWITCH 0
#define IN_PIN_ALARM 3

#define SLEEP 200
#define TIME_OPEN_RELAY 10000

static volatile int exit_flag = 0;
static volatile int alarm_flag = 0;
static volatile int switch_pressed = 0;
static volatile int hw_addr = 0;

static volatile unsigned int alarm_raised_ts = 0;
static volatile unsigned int standby_ts = 0;


int main (int argc, char *argv[])
{
	if(DAEMONIZE == 1)
	{
		md_daemonize();
	}


	md_init();

	while(!exit_flag)
	{
		if(!alarm_flag) {
			md_standby ();
		}
		else if(switch_pressed) {
			md_open_door ();
		}
		else {
			md_alarm_raised();
		}

		md_msleep(SLEEP);
	}

	md_destroy();


	return EXIT_SUCCESS;
}

void md_open_door()
{
	md_log("Switch pressed. Opening door");

	pifacedigital_digital_write(PIN_DOOR_FREE, 0);
	pifacedigital_digital_write(PIN_SWITCH_PRESSED, 1);	
	md_blink(PIN_DOOR_RELAY, TIME_OPEN_RELAY);
	pifacedigital_digital_write(PIN_SWITCH_PRESSED, 0);

	md_log("Door opened. Alarm cleared");
	md_clear_alarm();
}

void md_init()
{
	md_log("Opening piface digital connection at location %d\n", hw_addr);
	pifacedigital_open(hw_addr);

	pifacedigital_write_reg(0x00, OUTPUT, hw_addr);
}

void md_destroy()
{
	pifacedigital_write_reg(0x00, OUTPUT, hw_addr);

	md_log("Closing piface digital connection at location %d\n", hw_addr);
    	pifacedigital_close(hw_addr);
}

void md_alarm_raised()
{

	uint8_t switch_state;

	if(alarm_raised_ts == 0)
	{
		alarm_raised_ts = (unsigned) time(NULL);
		md_log("alarm raised at %u\n", alarm_raised_ts);

		pifacedigital_digital_write(PIN_STANDBY, 0);
		pifacedigital_digital_write(PIN_DOOR_FREE, 1);
	}

	switch_state = pifacedigital_digital_read(IN_PIN_SWITCH);
	if(switch_state == 0) {
		switch_pressed = 1;		
	}
}

void md_standby()
{
	uint8_t switch_state;

	if(standby_ts == 0)
	{
		standby_ts = (unsigned) time(NULL);
		md_log("standby at %u\n", standby_ts);

		pifacedigital_digital_write(PIN_DOOR_FREE, 0);
	}

	switch_state = pifacedigital_digital_read(IN_PIN_ALARM);
	if(switch_state == 0) {
		alarm_flag = 1;		
	}

	md_blink (PIN_STANDBY, 200);
}

void md_blink(int i, int ms)
{
        pifacedigital_digital_write(i, 1);
        md_msleep(ms);
        pifacedigital_digital_write(i, 0);
}

void md_clear_alarm()
{
	alarm_flag = 0;
	switch_pressed = 0;

	alarm_raised_ts = 0;
	standby_ts = 0;
}

void md_signal_handler (int sig) {

	switch(sig)
	{
		case SIGTERM:
			md_log("SIGTERM caught. Exiting.");
			exit_flag = 1;
			break;
		case SIGUSR1:
			md_log("SIGUSR1 caught. Alarm raised.");
			alarm_flag = 1;
			break;
		case SIGUSR2:
			md_clear_alarm();
			md_log("SIGUSR2 caught. Alarm cleared.");
			break;
		default:
			md_log("Signal %i caught.", sig);
			break;
	}

}

void md_log(const char* s, ...)
{

	va_list ap;
	va_start (ap, s);

	if(DAEMONIZE)
	{
		syslog(LOG_NOTICE, s, ap);
	}
	else
	{
		printf(s, ap);
	}

	va_end(ap);
}

void md_daemonize()
{
	pid_t pid;
	pid = fork();

	if (pid < 0)
		exit(EXIT_FAILURE);

	if (pid > 0)
		exit(EXIT_SUCCESS);

	if (setsid() < 0)
		exit(EXIT_FAILURE);

	/* Catch, ignore and handle signals */
	signal(SIGCHLD, SIG_IGN);
	signal(SIGHUP, SIG_IGN);
	signal(SIGUSR1, md_signal_handler);
	signal(SIGUSR2, md_signal_handler);
	signal(SIGTERM, md_signal_handler);


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

void md_msleep(int millis)
{
	struct timespec ts;
	ts.tv_sec = millis / 1000;
	ts.tv_nsec = (millis % 1000) * 1000000;
	nanosleep(&ts, NULL);
}
