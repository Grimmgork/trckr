#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <trckr.h>
#include <assert.h>

#include <arena.h>

#define ERR_INVALID_ARGS 1
#define ERR_CMD_NOT_FOUND 2
#define ERR_BAD_PROGRAMMING 3
#define ERR_MALLOC 4
#define ERR_MISSING_ARGS 5
#define ERR_INPUT_ABORTED 6
#define ERR_INVALID_INPUT 7

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
		case ERR_MISSING_ARGS: return "Missing arguments.";
		case ERR_INVALID_INPUT: return "Invalid input.";
		case ERR_INPUT_ABORTED: return "Input aborted.";

		case TRCKR_ERR: return "An error occured.";
		case TRCKR_ERR_SQL: return "SQL error occured.";
		case TRCKR_ERR_NO_OPEN_WORK: return "No open work.";
		case TRCKR_ERR_ALLOC: return "Memory allocation failed.";
		case TRCKR_ERR_OPEN_WORK: return "There is open work.";
		case TRCKR_NOT_FOUND: return "Not found.";
		case TRCKR_ERR_INITIALIZED: return "Already initialized.";
		case TRCKR_ERR_NAME_TAKEN: return "Name is taken.";
		case TRCKR_ERR_INVALID_INPUT: return "Invalid input.";
		case TRCKR_ERR_TEXT_TOO_LONG: return "Text is too long.";
		default: return "Unknown error occured.";
	}
}

void
print_error_message(int error) {
	char* message = get_error_message(error);
	fprintf(stderr, "ERROR [%d] %s\n", error, message);
}

char*
str_cat_dyn(struct arena* arena, char* a, char* b)
{
	int al = strlen(a);
	int bl = strlen(b);
	char* result = arena_push(arena, al + bl + 1);
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
get_db_path(struct arena* arena)
{
	#if __linux__
		char* variable = "HOME";
	#elif _WIN32
		char* variable = "USERPROFILE";
	#endif
	char* name = "/.trckr.db";
	char* dir = getenv(variable);
	return str_cat_dyn(arena, dir, name);
}

int
prompt_line(char *str, int n)
{
	assert(str != NULL);
	assert(n != 0);

	if (n == 1) {
		str[0] = 0;
		return 0;
	}

	int i;
	char c;
	for (i = 0; i < n-1; i++) {
		c = getc(stdin);
		if (c == '\n' || c == EOF)
		{
			if (ferror(stdin)) {
				return ERR_INPUT_ABORTED;
			}
			break;
		}
		str[i] = c;
	}
	str[i] = 0;

	int result = 0;
	// clear the buffer / read until newline
	while(!(c == '\n' || c == EOF)) {
		result = ERR_INVALID_INPUT;
		c = getc(stdin);
	}

	return result;
}

int
main(int argc, char *argv[])
{
	int result;
	shiftarg(&argc, &argv);
	char* command = shiftarg(&argc, &argv);

	struct arena* arena = arena_init();
	if (arena == NULL) {
		return ERR_MALLOC;
	}

	char* dbpath = get_db_path(arena);
	if (dbpath == NULL) {
		arena_free(arena);
		return ERR_MALLOC;
	}
	
	if (command != NULL && !strcmp(command, "init")) {
		result = trckr_initialize(dbpath);
		arena_free(arena);
		if (result != 0) print_error_message(result);
		return result;
	}

	struct trckr_ctx context;
	result = trckr_begin(dbpath, &context);
	if (result != 0) {
		arena_free(arena);
		if (result != 0) print_error_message(result);
		return result;
	}

	if (command == NULL) {
		result = cmd_status(&context, argc, argv);
	}
	else
	{
		result = cmd_route(&context, command, argc, argv, 12,
			"status", cmd_status,
			"start", cmd_start,
			"add", cmd_add,
			"stop", cmd_stop,
			"topic", cmd_topic,
			"report", cmd_report
		);
	}

	if (result == 0) {
		result = trckr_end(&context);
	}
	
	if (result != 0)
	{
		print_error_message(result);
		trckr_end_rollback(&context);
	}
	
	arena_free(arena);
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
	for (i=0; i < iterations; i++)
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
	printf("asdf");
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
	char *arg = shiftarg(&argc, &argv);
	int result;

	if (arg == NULL) {
		return ERR_MISSING_ARGS;
	}

	trckr_text_small name;
	result = trckr_parse_text_small(arg, name);
	if (result != 0) {
		return ERR_INVALID_ARGS;
	}

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

	arg = shiftarg(&argc, &argv);
	trckr_text description;
	if (arg == NULL) {
		printf("Description:\n");
		result = prompt_line(description, sizeof(description));
		if (result != 0) {
			return result;
		}
	}
	
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
	char* arg;
	int result;

	arg = shiftarg(&argc, &argv);
	if (arg == NULL) {
		return ERR_MISSING_ARGS;
	}

	trckr_text_small name;
	result = trckr_parse_text_small(arg, name);
	if (result != 0) {
		return result;
	}

	arg = shiftarg(&argc, &argv);
	if (arg == NULL) {
		return ERR_MISSING_ARGS;
	}

	trckr_text description;
	result = trckr_parse_text(arg, description);
	if (result != 0) {
		return result;
	}

	return trckr_create_topic(context, name, description);
}

int
cmd_topic(struct trckr_ctx* context, int argc, char *argv[])
{
	int result;
	char* arg;
	arg = shiftarg(&argc, &argv);

	trckr_text_small search;
	result = trckr_parse_text_small(arg, search);
	if (result != 0) {
		return ERR_INVALID_ARGS;
	}

	struct data_work_topic topic;
	int callback()
	{
		printf("%s - %s\n", topic.name, topic.description);
		return 0;
	}

	return trckr_iterate_topics_by_name(context, search, &topic, callback);
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
		*out_time = mktime(&datetime);
		return 0;
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
