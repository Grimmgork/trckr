#include <string.h>
#include <stdio.h>
#include <stdlib.h> 
#include <time.h>
#include "data.c"
#include "error.c"

struct trckr_ctx
{
	struct data_ctx *data;
};

struct trckr_ctx
*trckr_init(char *dbpath)
{
	struct trckr_ctx *context = malloc(sizeof(struct trckr_ctx));
	if(context == NULL) {
		return NULL;
	}
	context->data = data_init(dbpath);
	if(ERROR) {
		return NULL;
	}
	return context;
}

int
trckr_dispose(struct trckr_ctx *context)
{
	data_dispose(context->data);
	if(ERROR) {
		return -1;
	}
	return 0;
}

int
trckr_get_type_by_name(struct trckr_ctx *context, char *arg)
{
	return data_get_type_by_name(context->data, arg);
}

int
trckr_get_open_work(struct trckr_ctx *context)
{
	int id = data_get_open_work(context->data);
	if (id < 0) {
		return -1;
	}
	return id;
}

int
trckr_start(struct trckr_ctx *context, int typeid)
{
	int id = data_get_open_work(context->data);
	if (id >= 0) {
		THROW(ERR_OPENWORK);
		return -1;
	}
	if(id < 0 && !ISERR(ERR_NOOPENWORK)) {
		return -1;
	}
	ERRRS
	data_create_work(context->data, typeid, time(NULL));
	if(ERROR) {
		return -1;
	}
	return 0;
}

int
trckr_stop(struct trckr_ctx *context)
{
	int id = data_get_open_work(context->data);
	if (id < 0) {
		return -1;
	}
	int res = data_stop_work(context->data, id, time(NULL));
	if (res != 0) {
		return -1;
	}
	return 0;
}

int
trckr_switch(struct trckr_ctx *context, int typeid)
{
	time_t now = time(NULL);
	data_stop_work(context->data, data_get_open_work(context->data), now);
	if (ERROR) {
		return -1;
	}
	int id = data_create_work(context->data, typeid, now);
	if (ERROR) {
		return -1;
	}
	return 0;
}

int
trckr_started(struct trckr_ctx *context, int typeid, time_t time)
{
	data_get_open_work(context->data);
	if (NOERR) {
		THROW(ERR_OPENWORK)
		return -1;
	}
	if (ERROR && !ISERR(ERR_NOOPENWORK)) {
		return -1;
	}
	ERRRS
	data_create_work(context->data, typeid, time);
	if (ERROR) {
		return -1;
	}
	return 0;
}

int
trckr_stopped(struct trckr_ctx *context, time_t time)
{
	int workid = data_get_open_work(context->data);
	if (ERROR) {
		return -1;
	}
	data_stop_work(context->data, workid, time);
	if (ERROR) {
		return -1;
	}
	return 0;
}

int
trckr_create_type(struct trckr_ctx * context, char *name, char *description)
{
	int res = data_get_type_by_name(context->data, name);
	if (res < 0) {
		if (!ISERR(ERR_TYPENOTFOUND)) {
			return -1;
		}
	}
	else {
		THROW(ERR_TYPEEXISTS)
		return -1;
	}

	ERRRS
	data_create_type(context->data, name, description);
	if (ERROR) {
		return -1;
	}
	return 0;
}

int
strftspan(char *buf, int seconds)
{
	int index;
	if (seconds < 60) {
		// seconds
		sprintf(buf, "%i seconds", seconds);
		return 0;
	}
	if (seconds < 3600) {
		// minutes
		int minutes = seconds / 60;
		sprintf(buf, "%i minutes", minutes);
		return 0;
	}
	if (seconds < 86400) {
		// hours
		int hours = seconds / 3600;
		sprintf(buf, "%i hours", hours);
		return 0;
	}
	// days
	int days = seconds / 86400;
	sprintf(buf, "%i days", days);
	return 0;
}

int
trckr_print_status(struct trckr_ctx *context, FILE *fd)
{
	int id = data_get_open_work(context->data);
	if (id < 0) {
		if (ISERR(ERR_NOOPENWORK)) {
			fprintf(fd, "NONE\n");
			return 0;
		}
		else	{
			return -1;
		}
	}
	struct data_work *work = data_get_work_by_id(context->data, id);
	if (work == NULL) {
		return -1;
	}

	struct data_type *type = data_get_type_by_id(context->data, work->type_id);
	if(type == NULL) {
		return -1;
	}

	fprintf(fd, "work:\t%s\n", type->name);

	struct tm* info = localtime(&work->start);
	char buff[26];
	strftime(buff, 26, "%Y-%m-%d %H:%M", info);
	fprintf(fd, "start:\t%s\n", buff);

	int timespan = time(NULL) - work->start;
	strftspan(buff, timespan);
	fprintf(fd, "dur.:\t%s\n", buff);

	free(type->name);
	free(type->description);
	free(type);
	free(work);
	return 0;
}

int
trckr_print_types(struct trckr_ctx *context, FILE *fd)
{
	struct data_type buf[10];
	int skip = 0;
	int count;
	while (count = data_get_all_types(context->data, buf, 10, skip))
	{
		if (count < 0) {
			return -1;
		}

		for (int i = 0; i < count; i++) {
			fprintf(fd, "#%-4i %s\n", buf[i].id, buf[i].name);
		}

		skip += count;
	}
	return 0;
}

int
trckr_print_report(struct trckr_ctx *context, FILE *fd, time_t start, time_t end)
{
	return 0;
}