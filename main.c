#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <trckr.h>

#define ERR_INVALID_ARGS 1
#define ERR_CMD_NOT_FOUND 2
#define ERR_BAD_PROGRAMMING 3

int unixtime_from_args(int *argc, char **argv[], time_t* out_time);

int cmd_start(struct trckr_ctx* context, int argc, char *argv[]);
int cmd_initialize(int argc, char *argv[]);
int cmd_started(struct trckr_ctx* context, int argc, char *argv[]);
int cmd_status(struct trckr_ctx* context, int argc, char *argv[]);
int cmd_add(struct trckr_ctx* context, int argc, char *argv[]);
int cmd_add_topic(struct trckr_ctx* context, int argc, char *argv[]);
int cmd_stop(struct trckr_ctx* context, int argc, char *argv[]);
int cmd_topic(struct trckr_ctx* context, int argc, char *argv[]);
int cmd_report(struct trckr_ctx* context, int argc, char *argv[]);
int cmd_route(struct trckr_ctx*, char* name, int argc, char *argv[], int count, ...);

void printerror(int error) {
	if (error == 0) {
		return;
	}
	fprintf(stderr, "ERROR: %d\n", error);
}

char* shiftarg(int *argc, char **argv[])
{
	if (*argc <= 0) {
		return NULL;
	}

	char* ptr = *argv[0];
	--*argc;
	++*argv;
	return ptr;
}

int
main(int argc, char *argv[])
{
	int result;
	shiftarg(&argc, &argv);
	char* command = shiftarg(&argc, &argv);
	
	if (command != NULL && !strcmp(command, "init")) {
		result = cmd_initialize(argc, argv);
		printerror(result);
		return result;
	}

	struct trckr_ctx context;
	result = trckr_begin("trckr.db", &context);
	if (result != 0) {
		printerror(result);
		return result;
	}

	if (command == NULL) {
		result = cmd_status(&context, argc, argv);
		trckr_end(&context);
		printerror(result);
		return result;
	}

	result = cmd_route(&context, command, argc, argv, 14,
		"status", cmd_status,
		"start", cmd_start,
		"started", cmd_started,
		"add", cmd_add,
		"stop", cmd_stop,
		"topic", cmd_topic,
		"report", cmd_report
	);

	trckr_end(&context);
	printerror(result);
	return result;
}

int
cmd_route(struct trckr_ctx* context, char* name, int argc, char *argv[], int count, ...)
{
	if (name == NULL) {
		return ERR_CMD_NOT_FOUND;
	}

	va_list args;
	va_start(args, count);

	if (count % 2) {
		return ERR_BAD_PROGRAMMING;
	}

	int i;
	int iterations = count / 2;
	for (i=0; i<iterations; i++)
	{
		char* match = va_arg(args, char*);
		int (*handler)(struct trckr_ctx*, int, char*[]) = va_arg(args, int (*)(struct trckr_ctx*, int, char *[]));
		if (!strcmp(match, name)) {
			va_end(args);
			return handler(context, argc, argv);
		}
  	}

	va_end(args);
	return ERR_CMD_NOT_FOUND;
}

int
cmd_initialize(int argc, char *argv[])
{
	return trckr_init("trckr.db");
}

int
cmd_status(struct trckr_ctx* context, int argc, char *argv[])
{
	
	int result;
	struct data_work work;
	result = trckr_get_open_work(context, &work);
	if (result == TRCKR_NOT_FOUND) {
		printf("no open work.\n");
		return 0;
	}

	if (result != 0) {
		return result;
	}

	struct data_work_topic topic;
	result = trckr_get_topic_by_id(context, work.topic_id, &topic);
	if (result != 0) {
		return result;
	}

	printf("topic: %s\n", topic.name);
	printf("started: %d\n", work.start);
	return 0;
}

int
cmd_report(struct trckr_ctx* context, int argc, char *argv[])
{
	return 0;
}

int
cmd_start(struct trckr_ctx* context, int argc, char* argv[])
{
	char* name = shiftarg(&argc, &argv);
	if (name == NULL) {
		return ERR_INVALID_ARGS;
	}

	int result;
	struct data_work_topic topic;
	result = trckr_get_topic_by_name(context, name, &topic);
	if (result != 0) {
		return result;
	}

	char* description = shiftarg(&argc, &argv);
	int id;
	return trckr_start_work(context, topic.id, description, time(NULL), &id);
}

int
cmd_stop(struct trckr_ctx* context, int argc, char *argv[])
{
	return trckr_stop_work(context, time(NULL));
}

int
cmd_started(struct trckr_ctx* context, int argc, char *argv[])
{
	// check for enough arguments
	if (argc <= 0) {
		return ERR_INVALID_ARGS;
	}

	// get type by name
	int result;
	struct data_work_topic topic;
	result = trckr_get_topic_by_name(context, argv[0], &topic);
	if (result != 0) {
		return result;
	}

	// parse time input
	time_t time;
	result = unixtime_from_args(&argc, &argv, &time);
	if (result != 0) {
		return result;
	}

	// todo description
	int id;
	return trckr_start_work(context, topic.id, "", time, &id);
}

int
cmd_add(struct trckr_ctx* context, int argc, char *argv[])
{
	char *type = shiftarg(&argc, &argv);
	return cmd_route(context, type, argc, argv, 2,
		"topic", cmd_add_topic
	);
}

int
cmd_add_topic(struct trckr_ctx* context, int argc, char *argv[])
{
	char* name = shiftarg(&argc, &argv);
	char* description = shiftarg(&argc, &argv);
	return trckr_create_topic(context, name, description);
}

int
cmd_topic(struct trckr_ctx* context, int argc, char *argv[])
{
	char* search = shiftarg(&argc, &argv);
	int callback(struct data_work_topic* topic)
	{
		printf("%s\n", topic->name);
		return 0;
	}

	return trckr_iterate_topics_by_name(context, search, callback);
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
	return -1; // error
}

int
unixtime_from_args(int *argc, char **argv[], time_t* out_time) 
{
	char* a = shiftarg(argc, argv);
	if (a == NULL) {
		return ERR_INVALID_ARGS;
	}

	time_t now = time(NULL);
	struct tm datetime = *localtime(&now);
	datetime.tm_isdst = -1; // load DST from system
	datetime.tm_sec = 0;
	if (!parse_time(a, &datetime)) {
		return mktime(&datetime);
	}

	char* b = shiftarg(argc, argv);
	if (b == NULL) {
		return ERR_INVALID_ARGS;
	}

	if (parse_time(b, &datetime)) {
		return ERR_INVALID_ARGS; // failed
	}

	if (parse_date(a, &datetime)) {
		return ERR_INVALID_ARGS; // failed
	}

	*out_time = mktime(&datetime);
}
