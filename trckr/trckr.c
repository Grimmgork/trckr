#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <trckr.h>
#include <time.h>
#include "list.c"
#include "query.c"
#include "utils.c"

int
trckr_parse_text(const char* str, trckr_text buffer)
{
	assert(buffer != NULL);
	if (str == NULL) {
		str = "";
	}

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
	assert(buffer != NULL);
	if (str == NULL) {
		str = "";
	}

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
trckr_end_commit(struct trckr_ctx *context)
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
trckr_start_work(struct trckr_ctx *context, int topic_id, trckr_text description, int *out_id)
{
	int result;
	result = query_transaction(context);
	if (result != 0) {
		return result;
	}

	struct data_work work;
	result = query_get_open_work(context, &work);
	if (result == 0) {
		query_rollback(context);
		return TRCKR_ERR_OPEN_WORK;
	}

	if (result != TRCKR_NOT_FOUND) {
		query_rollback(context);
		return result;
	}

	if (context->work_id == 0) {
		query_rollback(context);
		return TRCKR_ERR_NO_SELECTION;
	}

	int out;
	result = query_is_last_work_of_stack(context, context->work_id, &out);
	if (result != 0) {
		query_rollback(context);
		return result;
	}

	if (!out) {
		query_rollback(context);
		return TRCKR_ERR_INVALID_OPERATION;
	}

	result = query_get_work_by_id(context, context->work_id, &work);
	if (result != TRCKR_NOT_FOUND) {
		query_rollback(context);
		return result;
	}

	int start = work.start + work.duration;
	result = query_start_work(context, work.stack_id, start, topic_id, description, out_id);
	if (result != 0) {
		query_rollback(context);
		return result;
	}

	return query_commit(context);
}

int
trckr_start_stack(struct trckr_ctx *context, time_t time, int topic_id, trckr_text description, int *out_id)
{
	
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

	printf("asd\n");

	return query_commit(context);
}

int
trckr_iterate_topics_by_name(struct trckr_ctx *context, trckr_text_small name, struct data_work_topic *topic, int(*callback)())
{
	return query_iterate_topics_by_name(context, name, topic, callback);
}

int
sub_shift_start_times(struct trckr_ctx *context, int stack_id, int shift, int skip)
{
	if (shift == 0) {
		return 0;
	}

	int result;
	struct trckr_list* list = trckr_list_init();
	if (list == NULL) {
		return TRCKR_ERR_ALLOC;
	}

	struct data_work work;
	int callback()
	{
		struct data_work *alloc = trckr_list_push(list, sizeof(struct data_work));
		if (alloc == NULL) {
			return TRCKR_ERR_ALLOC;
		}

		alloc->id = work.id;
		alloc->topic_id = work.id;
		alloc->start = work.start;
		alloc->duration = work.duration;

		return trckr_parse_text(work.description, alloc->description);
	}

	result = query_iterate_work_by_stack(context, stack_id, skip, &work, callback);
	if (result != 0) {
		trckr_list_free(list);
		return result;
	}

	struct data_work *item = trckr_list_iterate_reset(list);
	item = trckr_list_iterate_next(list);
	while (item != NULL) {
		result = query_update_work_time(context, item->id, item->start + shift, item->duration);
		if (result != 0) {
			trckr_list_free(list);
			return result;
		}

		item = trckr_list_iterate_next(list);
	}

	trckr_list_free(list);
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
