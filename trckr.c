#include <string.h>
#include <stdio.h>
#include <time.h>
#include "data.c"
#include "error.c"

int
trckr_init()
{
	
}

int
trckr_get_type_by_name(char *arg)
{
	return data_get_type_by_name(arg);
}

int
trckr_get_open_work()
{
	int id = data_get_open_work();
	if (ERROR) {
		return -1;
	}
	return id;
}

int
trckr_start(int typeid)
{
	data_get_open_work();
	if(ERROR && !ISERR(ERR_NOOPENWORK)) {
		return -1;
	}
	ERRRS
	data_create_work(typeid, time(NULL));
	if(ERROR) {
		return -1;
	}
	return 0;
}

int
trckr_stop()
{
	int id = data_get_open_work();
	if (ERROR) {
		return -1;
	}
	data_stop_work(id, time(NULL));
	if (ERROR) {
		return -1;
	}
	return 0;
}

int
trckr_switch(int typeid)
{
	time_t now = time(NULL);
	data_stop_work(data_get_open_work(), now);
	if (ERROR) {
		return -1;
	}
	int id = data_create_work(typeid, now);
	if (ERROR) {
		return -1;
	}
	return 0;
}

int
trckr_started(int typeid, time_t time)
{
	data_get_open_work();
	if (NOERR) {
		THROW(ERR_OPENWORK)
		return -1;
	}
	if (ERROR && !ISERR(ERR_NOOPENWORK)) {
		return -1;
	}
	ERRRS
	data_create_work(typeid, time);
	if (ERROR) {
		return -1;
	}
	return 0;
}

int
trckr_stopped(time_t time)
{
	int workid = data_get_open_work();
	if (ERROR) {
		return -1;
	}
	data_stop_work(workid, time);
	if(ERROR) {
		return -1;
	}
}

int
trckr_print_status()
{

}

int
trcker_print_report(time_t start, time_t end)
{

}