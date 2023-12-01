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
int cmd_add(int offs, int argc, char *argv[]);
int cmd_stop(int offs, int argc, char *argv[]);
int cmd_get_types(int offs, int argc, char *argv[]);
int cmd_report(int offs, int argc, char *argv[]);

struct trckr_ctx *g_trckr;

void printerror() {
	if(errno > 0) {
		perror("ERROR");
		return;
	}
	// Custom error
	fprintf(stderr, "ERROR: %i\n", errno);
}

int
main(int argc, char *argv[])
{
	int offs = 1;
	if(offs >= argc) {
		THROW(22)
		printerror();
		return errno;
	}

	g_trckr = trckr_init("data.db");
	if (ERROR) {
		printerror();
		return errno;
	}

	char *command = argv[offs];
	offs++;

	int res = -1;
	// status
	if (!strcmp(command, "status")) {
		res = cmd_status(offs, argc, argv);
	}
	else
	// start [type]
	if (!strcmp(command, "start")) {
		res = cmd_start(offs, argc, argv);
	}
	else
	// started [type] [time]
	if (!strcmp(command, "started")) {
		res = cmd_started(offs, argc, argv);
	}
	else
	// add type [name] [description]
	if (!strcmp(command, "add")) {
		res = cmd_add(offs, argc, argv);
	}
	// add stop
	else
	if (!strcmp(command, "stop")) {
		res = cmd_stop(offs, argc, argv);
	}
	// types
	else
	if (!strcmp(command, "types")) {
		res = cmd_get_types(offs, argc, argv);
	}
	// report [from] [to]
	else
	if (!strcmp(command, "report")) {
		res = cmd_report(offs, argc, argv);
	}
	else { // no command matched
		THROW(22);
	}

	trckr_dispose(g_trckr);
	
	if (res != 0) {
		printerror();
	}
	return errno;
}

int
cmd_status(int offs, int argc, char *argv[])
{
	return trckr_print_status(g_trckr, stdout);
}

int
cmd_report(int offs, int argc, char *argv[])
{
	return trckr_print_report(g_trckr, stdout, 0, 0);
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
		THROW(-10)
		return -1;
	}

	int result = trckr_start(g_trckr, typeid);
	if (result < 0) {
		return -1;
	}
	
	return 0;
}

int
cmd_stop(int offs, int argc, char *argv[])
{
	return trckr_stop(g_trckr);
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
		THROW(-10)
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
cmd_add(int offs, int argc, char *argv[])
{
	// check for enough arguments
	if (offs+2 >= argc) {
		THROW(22)
		return -1;
	}
	char *obj = argv[offs];
	if (!strcmp(obj, "type")) {
		trckr_create_type(g_trckr, argv[offs+1], argv[offs+2]);
		return 0;
	}

	THROW(22)
	return -1;
}

int
cmd_get_types(int offs, int argc, char *argv[])
{
	return trckr_print_types(g_trckr, stdout);
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

