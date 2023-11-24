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
trckr_print_status(struct trckr_ctx *context)
{

}

int
trcker_print_report(struct trckr_ctx *context, time_t start, time_t end)
{

}