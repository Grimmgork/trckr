#include <time.h>
#include "./sqlite3/sqlite3.h"
#include "error.c"

struct data_ctx {
	int is_closed;
};

int
data_open(struct data_ctx context, char *filename)
{
	sqlite3 *db;
	int rc = sqlite3_open_v2(filename, &db, SQLITE_OPEN_READWRITE, NULL);
	if(rc) {
		THROW(-1)
		return -1;
	}
}

int
data_close(struct data_ctx context)
{
	
}

int
data_create_work(int typeid, time_t time)
{

}

int
data_stop_work(int workid, time_t time)
{
	
}

int
data_get_type_by_name(char *name)
{
	THROW(ERR_TYPENOTFOUND)
	return 0;
}

int
data_get_open_work()
{
	
}