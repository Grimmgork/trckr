#include <stdio.h>
#include <regex.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include "trckr.c"
#include "error.c"

time_t unixtime_from_args(char *argv[], int i, int l);

int cmd_start(int offs, int argc, char *argv[]);
int cmd_started(int offs, int argc, char *argv[]);
int cmd_status(int offs, int argc, char *argv[]);

struct trckr_ctx *g_trckr;

int
main(int argc, char *argv[])
{
	int offs = 1;
	if(offs >= argc) {
		THROW(22)
		return errno;
	}

	g_trckr = trckr_init("/data.db");
	if (ERROR) {
		return errno;
	}

	char *command = argv[offs];
	offs++;

	// status
	if (!strcmp(command, "status")) {
		cmd_status(offs, argc, argv);
	}
	else
	// start [type]
	if (!strcmp(command, "start")) {
		cmd_start(offs, argc, argv);
	}
	else
	// started [type] [time]
	if (!strcmp(command, "started")) {
		cmd_started(offs, argc, argv);
	}

	trckr_dispose(g_trckr);

	if(ERROR){
		perror("An error occured: ");
	}

	return errno;
}

int
cmd_status(int offs, int argc, char *argv[])
{
	printf("Status:\n");
	return 0;
}

int
cmd_start(int offs, int argc, char *argv[])
{
	// check for enough arguments
	if (offs >= argc) {
		THROW(22)
		return -1;
	}

	// resolve type ref by its name
	int typeid = trckr_get_type_by_name(g_trckr, argv[offs]);
	if (ERROR) {
		THROW(22)
		return -1;
	}

	return trckr_start(g_trckr, typeid);
}

int
cmd_started(int offs, int argc, char *argv[])
{
	// check for enough arguments
	if (offs+1 >= argc) {
		THROW(22)
		return -1;
	}

	// get type by name
	int typeid = trckr_get_type_by_name(g_trckr, argv[offs]);
	if (ERROR) {
		return -1;
	}

	// parse time input
	time_t time = unixtime_from_args(argv, offs+1, argc);
	if (ERROR) {
		return -1;
	}

	return trckr_started(g_trckr, typeid, time);
}


int
parse_time(char *str, struct tm *datetime) 
{
	int hours, minutes;
	int status = sscanf(str, "%u:%u", &hours, &minutes);
	if (status==2) {
		datetime->tm_min = minutes;
		datetime->tm_hour = hours;
		return 0;
	}
	if (!strcmp(str, "now")) {
		return 0;
	}
	THROW(22)
	return -1; // error
}

int
parse_date(char *str, struct tm *datetime) 
{
	unsigned int y, m, d = 0;
	int status = sscanf(str, "%u-%u-%u", &y, &m, &d);
	if(status == 3) {
		datetime->tm_year = y-1900;
		datetime->tm_mon = m-1;
		datetime->tm_mday = d;
		return 0;
	}
	status = sscanf(str, "%u-%u", &m, &d);
	if(status == 2) {
		datetime->tm_mon = m-1;
		datetime->tm_mday = d;
		return 0;
	}
	status = sscanf(str, "%u", &d);
	if(status == 1) {
		datetime->tm_mday = d;
		return 0;
	}
	if (!strcmp(str, "today")) {
		return 0;
	}
	if (!strcmp(str, "yest")) {
		datetime->tm_mday -= 1; // TODO maybe wrong :(
		return 0;
	}
	THROW(22)
	return -1; // error
}

time_t
unixtime_from_args(char *argv[], int i, int l) 
{
	if (i > l-1) {
		THROW(22)
		return 0;
	}
	time_t now = time(NULL);
	struct tm datetime = *localtime(&now);
	datetime.tm_isdst = -1; // load DST from system
	datetime.tm_sec = 0;
	if (!parse_time(argv[i], &datetime)) {
		return mktime(&datetime);
	}
	if (i+2 > l) {
		THROW(22)
		return 0;
	}
	if (parse_time(argv[i+1], &datetime)) {
		THROW(22)
		return 0; // failed
	}
	if (parse_date(argv[i], &datetime)) {
		THROW(22)
		return 0; // failed
	}
	return mktime(&datetime);
}

