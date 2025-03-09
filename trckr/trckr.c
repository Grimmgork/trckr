#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <trckr.h>
#include <time.h>
#include "stack.c"
#include "query.c"
#include "utils.c"

int
trckr_parse_text(const char* str, trckr_text buffer)
{
	assert(str != NULL);
	assert(buffer != NULL);

	int length = strlen(str);
	if (length + 1 > sizeof(trckr_text))
	{
		return TRCKR_ERR_TEXT_TOO_LONG;
	}

	strcpy(buffer, str);
	return 0;
}

int
trckr_parse_text_small(const char* str, trckr_text_small buffer)
{
	assert(str != NULL);
	assert(buffer != NULL);

	int length = strlen(str);
	if (length + 1 > sizeof(trckr_text_small))
	{
		return TRCKR_ERR_TEXT_TOO_LONG;
	}

	strcpy(buffer, str);
	return 0;
}

int
trckr_begin(char *path, struct trckr_ctx* out_context)
{
	sqlite3 *db;
	int result;
	result = sqlite3_open_v2(path, &db, SQLITE_OPEN_READWRITE, NULL);
	if (result != SQLITE_OK) {
		return TRCKR_ERR_SQL;
	}

	out_context->db = db;
	out_context->transaction_depth = 0;
	result = query_transaction(out_context);
	if (result != 0) {
		sqlite3_close(db);
		return result;
	}

	result = query_load_context(out_context);
	if (result != 0) {
		query_rollback(out_context);
		sqlite3_close(db);
		return result;
	}

	return 0;
}

int
trckr_end(struct trckr_ctx *context)
{
	int result = query_write_context(context);
	if (result != 0) {
		return result;
	}

	result = query_commit(context);
	if (result != 0) {
		return result;
	}
	
	sqlite3_close(context->db);
	return 0;
}

void
trckr_end_rollback(struct trckr_ctx *context)
{
	query_rollback(context);
	sqlite3_close(context->db);
}

int
trckr_initialize(char* path)
{
	// check if file exists
	FILE *ptr;
	ptr = fopen(path, "r"); 
	if (ptr != NULL) {
		fclose(ptr);
		return TRCKR_ERR_INITIALIZED;
	}
	
	// open database
	sqlite3 *db;
	int result = sqlite3_open_v2(path, &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE , NULL);
	if(result != SQLITE_OK) {
		return TRCKR_ERR_SQL;
	}

	// run query
	result = query_create_schema(db);
	sqlite3_close(db);
	return result;
}

int
trckr_get_open_work(struct trckr_ctx *context, struct data_work* out_work)
{
	return query_get_open_work(context, out_work);
}

int
trckr_get_topic_by_name(struct trckr_ctx *context, trckr_text_small name, struct data_work_topic* out_topic)
{
	return query_get_topic_by_name(context, name, out_topic);
}

int
trckr_get_topic_by_id(struct trckr_ctx *context, int id, struct data_work_topic* out_topic)
{
	return query_get_topic_by_id(context, id, out_topic);
}

int
trckr_start_work(struct trckr_ctx *context, int topic_id, trckr_text description, time_t time, int* out_id)
{
	// TODO
	// query selected work
	// read starttime + duration
	// create new work with start at end of first work
	// run align start times 

	int result;
	result = query_transaction(context);
	if (result != 0) {
		return result;
	}

	struct data_work work;
	result = query_get_open_work(context, &work);
	if (result == 0) {
		// there is open work
		query_rollback(context);
		return TRCKR_ERR_OPEN_WORK;
	}

	if (result != TRCKR_NOT_FOUND) {
		// some error occured
		query_rollback(context);
		return result;
	}

	if (description == NULL || strlen(description) > sizeof(work.description)) {
		query_rollback(context);
		return TRCKR_ERR_INVALID_INPUT;
	}

	result = query_start_work(context, 0, time, topic_id, description, out_id);
	if (result != 0) {
		query_rollback(context);
		return result;
	}
	
	return query_commit(context);
}

int
trckr_stop_work(struct trckr_ctx *context, time_t time)
{
	int result;
	result = query_transaction(context);
	if (result != 0) {
		return result;
	}

	struct data_work work;
	result = query_get_open_work(context, &work);
	if (result == TRCKR_NOT_FOUND) {
		// there is no open work
		query_rollback(context);
		return TRCKR_ERR_NO_OPEN_WORK;
	}

	if (result != 0) {
		// error occured
		query_rollback(context);
		return result;
	}

	int duration = time - work.start;
	result = query_stop_work(context, work.id, duration);
	if (result != 0) {
		query_rollback(context);
		return result;
	}

	return query_commit(context);
}

int
trckr_create_topic(struct trckr_ctx *context, trckr_text_small name, trckr_text description)
{
	int result;
	if (name == NULL) {
		return TRCKR_ERR_INVALID_INPUT;
	}

	result = query_transaction(context);
	if (result != 0) {
		return result;
	}

	struct data_work_topic topic;
	result = query_get_topic_by_name(context, name, &topic);
	if (result == 0) {
		query_rollback(context);
		return TRCKR_ERR_NAME_TAKEN;
	}
	if (result != TRCKR_NOT_FOUND) {
		query_rollback(context);
		return result;
	}

	result = query_create_topic(context, name, description);
	if (result != 0) {
		query_rollback(context);
		return result;
	}

	return query_commit(context);
}

int
trckr_iterate_topics_by_name(struct trckr_ctx *context, trckr_text_small name, struct data_work_topic *topic, int(*callback)())
{
	return query_iterate_topics_by_name(context, name, topic, callback);
}

int
sub_shift_start_times(struct trckr_ctx *context, int shift, int skip)
{
	if (shift == 0) {
		return 0;
	}

	int result;
	struct trckr_stack* stack = trckr_stack_init();
	if (stack == NULL) {
		return TRCKR_ERR_ALLOC;
	}

	struct data_work work;
	int query_iterate_callback()
	{
		struct data_work *alloc = trckr_stack_push(stack, sizeof(struct data_work));
		if (alloc == NULL) {
			return TRCKR_ERR_ALLOC;
		}

		alloc->id = work.id;
		alloc->topic_id = work.id;
		alloc->start = work.start;
		alloc->duration = work.duration;

		int result = trckr_parse_text(work.description, alloc->description);
		if (result != 0) {
			return result;
		}

		return 0;
	}

	result = query_iterate_work_day(context, skip, &work, query_iterate_callback);
	if (result != 0) {
		trckr_stack_free(stack);
		return result;
	}

	struct data_work *item = trckr_stack_iterate_reset(stack);
	item = trckr_stack_iterate_next(stack);
	while (item != NULL) {
		result = query_update_work_time(context, item->id, item->start + shift, item->duration);
		if (result != 0) {
			trckr_stack_free(stack);
			return result;
		}

		item = trckr_stack_iterate_next(stack);
	}

	trckr_stack_free(stack);
	return result;
}

int
trckr_push_work(struct trckr_ctx *context, int work_type_id, char* description, int duration)
{
	// query selected work
	// read starttime + duration
	// create new work with start at end of selected work
	// run align start times 
	return 0;
}

int
trckr_cursor_select_today()
{
	// select today
	// move cursor to top of stack
	// write status
	return 0;
}

int
trckr_cursor_select_day()
{
	// select specified day
	// move cursor to top of stack
	// write status
	return 0;
}

int
trckr_cursor_select_first()
{
	return 0;
}

int
trckr_cursor_select_last()
{
	
}

int
trckr_cursor_select()
{

}
