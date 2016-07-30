/**
 * doorcontrold.h
 * --------------
 *
 * author: Markus Dürauer
 *
 */

void md_init();
void md_destroy();
void md_blink(int,int);
void md_clear_alarm();
void md_alarm_raised();
void md_open_door();
void md_standby();
void md_signal_handler(int);
void md_log(const char*, ...);
void md_daemonize();
void md_msleep(int);
