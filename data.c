#include <time.h>
#include "./sqlite3/sqlite3.h"
#include "error.c"

struct data_ctx 
{
	sqlite3 *db;
};

struct data_ctx
*data_init(char *filename)
{
	sqlite3 *db;
	int rc = sqlite3_open_v2(filename, &db, SQLITE_OPEN_READWRITE, NULL);
	if(ERROR) {
		THROW(errno)
		return NULL;
	}
	struct data_ctx *context = malloc(sizeof(struct data_ctx));
	context->db = db;
	return context;
}

int
data_dispose(struct data_ctx *context)
{
	
}

int
data_create_work(struct data_ctx *context, int typeid, time_t time)
{

}

int
data_stop_work(struct data_ctx *context, int workid, time_t time)
{
	
}

int
data_get_type_by_name(struct data_ctx *context, char *name)
{
	THROW(ERR_TYPENOTFOUND)
	return 0;
}

int
data_get_open_work(struct data_ctx *context)
{
	
}