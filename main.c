#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <trckr.h>

#include <arena.h>

#define ERR_INVALID_ARGS 1
#define ERR_CMD_NOT_FOUND 2
#define ERR_BAD_PROGRAMMING 3
#define ERR_MALLOC 4

int unixtime_from_args(int *argc, char **argv[], time_t* out_time);

int cmd_start(struct trckr_ctx* context, int argc, char *argv[]);
int cmd_status(struct trckr_ctx* context, int argc, char *argv[]);
int cmd_add(struct trckr_ctx* context, int argc, char *argv[]);
int cmd_add_topic(struct trckr_ctx* context, int argc, char *argv[]);
int cmd_stop(struct trckr_ctx* context, int argc, char *argv[]);
int cmd_topic(struct trckr_ctx* context, int argc, char *argv[]);
int cmd_report(struct trckr_ctx* context, int argc, char *argv[]);
int cmd_work(struct trckr_ctx* context, int argc, char *argv[]);
int cmd_route(struct trckr_ctx*, char* name, int argc, char *argv[], int count, ...);

char*
get_error_message(int error) {
	switch (error) {
		case 0: return "No error occured.";
		case ERR_INVALID_ARGS: return "Invalid arguments.";
		case ERR_CMD_NOT_FOUND: return "Command not found.";
		case ERR_BAD_PROGRAMMING: return "Bad programming.";
		case ERR_MALLOC: return "Allocation failed.";

		case TRCKR_ERR: return "An error occured.";
		case TRCKR_ERR_SQL: return "SQL error occured.";
		case TRCKR_ERR_NO_OPEN_WORK: return "No open work.";
		case TRCKR_ERR_ALLOC: return "Memory allocation failed.";
		case TRCKR_ERR_OPEN_WORK: return "There is open work.";
		case TRCKR_NOT_FOUND: return "Not found.";
		case TRCKR_ERR_INITIALIZED: return "Already initialized.";
		case TRCKR_ERR_NAME_TAKEN: return "Name is taken.";
		case TRCKR_ERR_INVALID_INPUT: return "Invalid input.";
		default: return "An error occured.";
	}
}

void
print_error_message(int error) {
	char* message = get_error_message(error);
	fprintf(stderr, "ERROR [%d] %s\n", error, message);
}

char*
str_cat_dyn(char* a, char* b)
{
	int al = strlen(a);
	int bl = strlen(b);
	char* result = malloc(al + bl + 1);
	strcpy(result, a);
	strcpy(result + al, b);
	return result;
}

char*
shiftarg(int *argc, char **argv[])
{
	if (*argc <= 0) {
		return NULL;
	}

	char* ptr = *argv[0];
	--*argc;
	++*argv;
	return ptr;
}

char*
get_db_path()
{
	#if __linux__
		char* variable = "HOME";
	#elif _WIN32
		char* variable = "USERPROFILE";
	#endif
	char* name = "/.trckr.db";
	char* dir = getenv(variable);
	return str_cat_dyn(dir, name);
}

int
main(int argc, char *argv[])
{
	struct arena* arena = arena_init();
	if (arena == NULL) {
		return 1;
	}

	char* buffer = arena_push(arena, sizeof());
	if (buffer == NULL) {
		return 1;
	}

	arena_free(arena);
	return 0;

	int result;
	shiftarg(&argc, &argv);
	char* command = shiftarg(&argc, &argv);

	char* dbpath = get_db_path();
	if (dbpath == NULL) {
		return ERR_MALLOC;
	}
	
	if (command != NULL && !strcmp(command, "init")) {
		result = trckr_init(dbpath);
		free(dbpath);
		if (result != 0) print_error_message(result);
		return result;
	}

	struct trckr_ctx context;
	result = trckr_begin(dbpath, &context);
	if (result != 0) {
		free(dbpath);
		if (result != 0) print_error_message(result);
		return result;
	}

	if (command == NULL) {
		result = cmd_status(&context, argc, argv);
		trckr_end(&context);
		free(dbpath);
		if (result != 0) print_error_message(result);
		return result;
	}

	result = cmd_route(&context, command, argc, argv, 12,
		"status", cmd_status,
		"start", cmd_start,
		"add", cmd_add,
		"stop", cmd_stop,
		"topic", cmd_topic,
		"report", cmd_report
	);

	trckr_end(&context);
	free(dbpath);
	if (result != 0) print_error_message(result);
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

	char buffer[10];
	struct tm t = *localtime(&work.start);
	strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M", &t);

	printf("# open work:\n");
	printf("topic:\t%s\n", topic.name);
	printf("desc.:\t%s\n", topic.description);
	printf("start:\t%s\n", buffer);
	printf("for:\t%dm\n", (time(NULL) - work.start) / 60);
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
	time_t timestamp;

	if (argc > 0)
	{
		result = unixtime_from_args(&argc, &argv, &timestamp);
		if (result != 0) {
			return result;
		}
	}
	else
	{
		timestamp = time(NULL);
	}

	struct data_work_topic topic;
	result = trckr_get_topic_by_name(context, name, &topic);
	if (result != 0) {
		return result;
	}

	char* description = shiftarg(&argc, &argv);
	int id;
	return trckr_start_work(context, topic.id, description, timestamp, &id);
}

int
cmd_stop(struct trckr_ctx* context, int argc, char *argv[])
{
	int result;
	time_t timestamp;
	if (argc > 0)
	{
		result = unixtime_from_args(&argc, &argv, &timestamp);
		if (result != 0) {
			return result;
		}
	}
	else
	{
		timestamp = time(NULL);
	}

	return trckr_stop_work(context, timestamp);
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
	return 0;
}
