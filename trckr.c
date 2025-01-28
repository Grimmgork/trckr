#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <trckr.h>
#include <time.h>

int
query_create_topic(struct trckr_ctx *context, char* name, char* description)
{
	const char *sql = "INSERT INTO topic (name, description) VALUES (?1, ?2);";
	int result;
	sqlite3_stmt *pstmt;
	result = sqlite3_prepare_v3(context->db, sql, -1, 0, &pstmt, NULL);
	if (result != SQLITE_OK) {
		sqlite3_finalize(pstmt);
		return TRCKR_ERR_SQL;
	}

	sqlite3_bind_text(pstmt, 1, name, -1, NULL);
	sqlite3_bind_text(pstmt, 2, description, -1, NULL);
	result = sqlite3_step(pstmt);

	if (result != SQLITE_DONE) {
		sqlite3_finalize(pstmt);
		return TRCKR_ERR_SQL;
	}

	sqlite3_finalize(pstmt);
	return 0;
}

int
query_get_open_work_id(struct trckr_ctx *context, int* out_id)
{
	char *sql = "SELECT id FROM work WHERE end IS NULL;";
	sqlite3_stmt *pstmt;
	int result;
	result = sqlite3_prepare_v3(context->db, sql, -1, 0, &pstmt, NULL);
	if (result != SQLITE_OK) {
		sqlite3_finalize(pstmt);
		return TRCKR_ERR_SQL;
	}

	result = sqlite3_step(pstmt);
	if (result == SQLITE_DONE) {
		sqlite3_finalize(pstmt);
		return TRCKR_NOT_FOUND;
	}

	if (result != SQLITE_ROW) {
		sqlite3_finalize(pstmt);
		return TRCKR_ERR_SQL;
	}

	*out_id = sqlite3_column_int(pstmt, 0);
	sqlite3_finalize(pstmt);
	return 0;
}

int
query_get_open_work(struct trckr_ctx *context, struct data_work* out_work)
{
	char *sql = "SELECT id, topic_id, start, duration, description FROM work WHERE duration IS NULL;";
	sqlite3_stmt *pstmt;
	int result;
	result = sqlite3_prepare_v3(context->db, sql, -1, 0, &pstmt, NULL);

	if (result != SQLITE_OK) {
		sqlite3_finalize(pstmt);
		return TRCKR_ERR_SQL;
	}

	result = sqlite3_step(pstmt);
	if (result == SQLITE_DONE) {
		sqlite3_finalize(pstmt);
		return TRCKR_NOT_FOUND;
	}

	if (result != SQLITE_ROW) {
		sqlite3_finalize(pstmt);
		return TRCKR_ERR_SQL;
	}

	out_work->id = sqlite3_column_int(pstmt, 0);
	out_work->topic_id = sqlite3_column_int(pstmt, 1);
	out_work->start = sqlite3_column_int(pstmt, 2);
	out_work->duration = sqlite3_column_int(pstmt, 3);
	snprintf(out_work->description, sizeof(out_work->description), "%s", (char*)sqlite3_column_text(pstmt, 3));
	sqlite3_finalize(pstmt);
	return 0;
}

int
query_create_work(struct trckr_ctx *context, time_t start, int topic_id, char* description, int* out_id)
{
	const char *sql = "INSERT INTO work (topic_id, start, description) VALUES (?1, ?2, ?3);";
	int result;
	sqlite3_stmt *pstmt;
	result = sqlite3_prepare_v3(context->db, sql, -1, 0, &pstmt, NULL);
	if (result != SQLITE_OK) {
		sqlite3_finalize(pstmt);
		return TRCKR_ERR_SQL;
	}

	sqlite3_bind_int(pstmt, 1, topic_id);
	sqlite3_bind_int(pstmt, 2, start);
	sqlite3_bind_text(pstmt, 3, description, -1, NULL);
	result = sqlite3_step(pstmt);

	if (result != SQLITE_DONE) {
		sqlite3_finalize(pstmt);
		return TRCKR_ERR_SQL;
	}

	sqlite3_finalize(pstmt);
	return 0;
}

int
query_stop_work(struct trckr_ctx *context, int work_id, time_t time)
{
	return 0;
}

int
query_iterate_last_work(struct trckr_ctx *context, int count, int (*callback)(struct data_work*))
{
	int result;
	const char *sql = "SELECT id, topic_id, start, duration, description FROM work ORDER BY start";
	sqlite3_stmt *pstmt;
	result = sqlite3_prepare_v3(context->db, sql, -1, 0, &pstmt, NULL);
	if (result != SQLITE_OK) {
		sqlite3_finalize(pstmt);
		return TRCKR_ERR_SQL;
	}

	struct data_work work;
	int i;
	for (i=0; i<count; i++) {
		result = sqlite3_step(pstmt);
		if (result == SQLITE_DONE) {
			break;
		}

		if (result != SQLITE_ROW) {
			sqlite3_finalize(pstmt);
			return TRCKR_ERR_SQL;
		}

		work.id = sqlite3_column_int(pstmt, 0);
		work.topic_id = sqlite3_column_int(pstmt, 1);
		work.start = sqlite3_column_int(pstmt, 2);
		work.duration = sqlite3_column_int(pstmt, 3);
		snprintf(work.description, sizeof(work.description), "%s", (char*) sqlite3_column_text(pstmt, 4));

		result = callback(&work);
		if (result == TRCKR_ITERATION_DONE) {
			break;
		}

		if (result != 0) {
			// error occured
			sqlite3_finalize(pstmt);
			return result;
		}
	}
	
	sqlite3_finalize(pstmt);
	return 0;
}

int
query_iterate_topics_by_name(struct trckr_ctx *context, char* name, int (*callback)(struct data_work_topic*))
{
	int result;
	const char *sql = "SELECT id, name, description FROM topic WHERE name LIKE CONCAT('%%', ?1, '%%');";
	sqlite3_stmt *pstmt;
	result = sqlite3_prepare_v3(context->db, sql, -1, 0, &pstmt, NULL);
	if (result != SQLITE_OK) {
		sqlite3_finalize(pstmt);
		return TRCKR_ERR_SQL;
	}

	sqlite3_bind_text(pstmt, 1, name, -1, NULL);

	struct data_work_topic topic;
	while(1) {
		result = sqlite3_step(pstmt);
		if (result == SQLITE_DONE) {
			break;
		}

		if (result != SQLITE_ROW) {
			sqlite3_finalize(pstmt);
			return TRCKR_ERR_SQL;
		}

		topic.id = sqlite3_column_int(pstmt, 0);
		snprintf(topic.name, sizeof(topic.name), "%s", (char*) sqlite3_column_text(pstmt, 1));
		snprintf(topic.description, sizeof(topic.description), "%s", (char*) sqlite3_column_text(pstmt, 2));
		result = callback(&topic);
		if (result == TRCKR_ITERATION_DONE) {
			break;
		}

		if (result != 0) {
			// error occured
			sqlite3_finalize(pstmt);
			return result;
		}
	}
	
	sqlite3_finalize(pstmt);
	return 0;
}

int
query_get_topic_by_id(struct trckr_ctx *context, int id, struct data_work_topic* out_topic)
{
	int result;
	const char *sql = "SELECT id, name, description FROM topic WHERE id = ?1;";
	sqlite3_stmt *pstmt;
	result = sqlite3_prepare_v3(context->db, sql, -1, 0, &pstmt, NULL);
	if (result != SQLITE_OK) {
		sqlite3_finalize(pstmt);
		return TRCKR_ERR_SQL;
	}

	sqlite3_bind_int(pstmt, 1, id);
	result = sqlite3_step(pstmt);

	if (result != SQLITE_ROW) {
		sqlite3_finalize(pstmt);
		return TRCKR_NOT_FOUND;
	}

	out_topic->id = sqlite3_column_int(pstmt, 0);
	snprintf(out_topic->name, sizeof(out_topic->name), "%s", (char*)sqlite3_column_text(pstmt, 1));
	snprintf(out_topic->description, sizeof(out_topic->description), "%s", (char*)sqlite3_column_text(pstmt, 2));

	sqlite3_finalize(pstmt);
	return 0;
}

int
query_get_topic_id_by_name(struct trckr_ctx *context, char* name, int* out_id)
{
	int result;
	const char *sql = "SELECT id FROM topic WHERE name = ?1;";
	sqlite3_stmt *pstmt;
	result = sqlite3_prepare_v3(context->db, sql, -1, 0, &pstmt, NULL);
	if (result != SQLITE_OK) {
		sqlite3_finalize(pstmt);
		return TRCKR_ERR_SQL;
	}

	sqlite3_bind_text(pstmt, 1, name, -1, NULL);
	result = sqlite3_step(pstmt);

	if (result == SQLITE_DONE)
	{
		sqlite3_finalize(pstmt);
		return TRCKR_NOT_FOUND;
	}

	if (result != SQLITE_ROW) {
		sqlite3_finalize(pstmt);
		return TRCKR_ERR_SQL;
	}

	*out_id = sqlite3_column_int(pstmt, 0);
	sqlite3_finalize(pstmt);
	return 0;
}

int
query_get_topic_by_name(struct trckr_ctx *context, char* name, struct data_work_topic* out_topic)
{
	int result;
	const char *sql = "SELECT id, name, description FROM topic WHERE name = ?1;";
	sqlite3_stmt *pstmt;
	result = sqlite3_prepare_v3(context->db, sql, -1, 0, &pstmt, NULL);
	if (result != SQLITE_OK) {
		sqlite3_finalize(pstmt);
		return TRCKR_ERR_SQL;
	}

	sqlite3_bind_text(pstmt, 1, name, -1, NULL);
	result = sqlite3_step(pstmt);

	if (result != SQLITE_ROW) {
		sqlite3_finalize(pstmt);
		return TRCKR_NOT_FOUND;
	}

	out_topic->id = sqlite3_column_int(pstmt, 0);
	snprintf(out_topic->name, sizeof(out_topic->name), "%s", (char*)sqlite3_column_text(pstmt, 1));
	snprintf(out_topic->description, sizeof(out_topic->description), "%s", (char*)sqlite3_column_text(pstmt, 2));

	sqlite3_finalize(pstmt);
	return 0;
}

int
query_transaction(struct trckr_ctx* context)
{
	int result;
	const char *sql = "BEGIN TRANSACTION IMMEDIATE;";
	sqlite3_stmt *pstmt;
	result = sqlite3_prepare_v3(context->db, sql, -1, 0, &pstmt, NULL);
	if (result != SQLITE_OK) {
		sqlite3_finalize(pstmt);
		return TRCKR_ERR_SQL;
	}

	result = sqlite3_step(pstmt);
	if (result != SQLITE_DONE) {
		sqlite3_finalize(pstmt);
		return TRCKR_ERR_SQL;
	}

	sqlite3_finalize(pstmt);
	return 0;
}

int
query_rollback(struct trckr_ctx* context)
{
	int result;
	const char *sql = "ROLLBACK TRANSACTION;";
	sqlite3_stmt *pstmt;
	result = sqlite3_prepare_v3(context->db, sql, -1, 0, &pstmt, NULL);
	if (result != SQLITE_OK) {
		sqlite3_finalize(pstmt);
		return TRCKR_ERR_SQL;
	}

	result = sqlite3_step(pstmt);
	if (result != SQLITE_DONE) {
		sqlite3_finalize(pstmt);
		return TRCKR_ERR_SQL;
	}

	sqlite3_finalize(pstmt);
	return 0;
}

int
query_commit(struct trckr_ctx* context)
{
	int result;
	const char *sql = "COMMIT TRANSACTION;";
	sqlite3_stmt *pstmt;
	result = sqlite3_prepare_v3(context->db, sql, -1, 0, &pstmt, NULL);
	if (result != SQLITE_OK) {
		sqlite3_finalize(pstmt);
		return TRCKR_ERR_SQL;
	}

	result = sqlite3_step(pstmt);
	if (result != SQLITE_DONE) {
		sqlite3_finalize(pstmt);
		return TRCKR_ERR_SQL;
	}

	sqlite3_finalize(pstmt);
	return 0;
}

// begin implementation of trckr.h

int
trckr_begin(char *path, struct trckr_ctx* out_context)
{
	sqlite3 *db;
	int result;
	result = sqlite3_open_v2(path, &db, SQLITE_OPEN_READWRITE, NULL);
	if(result != SQLITE_OK) {
		return TRCKR_ERR_SQL;
	}

	out_context->db = db;
	return 0;
}

void
trckr_end(struct trckr_ctx *context)
{
	sqlite3_close(context->db);
}

int
trckr_init(char* path)
{
	// check if file exists
	FILE *ptr;
	ptr = fopen(path, "r"); 
	if (ptr != NULL) {
		return TRCKR_ERR_INITIALIZED;
	}
	fclose(ptr);

	sqlite3 *db;
	int result;
	result = sqlite3_open_v2(path, &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE , NULL);
	if(result != SQLITE_OK) {
		return TRCKR_ERR_SQL;
	}

	// create shema
	const char *sql = "CREATE TABLE IF NOT EXISTS topic(id INTEGER PRIMARY KEY, name TEXT UNIQUE, description TEXT); \
	                   CREATE TABLE IF NOT EXISTS work(id INTEGER PRIMARY KEY, topic_id INTEGER, start INTEGER, duration INTEGER, description TEXT);";
	result = sqlite3_exec(db, sql, 0, 0, NULL);
	if (result != SQLITE_OK) {
		sqlite3_close(db);
		return TRCKR_ERR_SQL;
	}

	sqlite3_close(db);
	return 0;
}

int
trckr_get_open_work(struct trckr_ctx *context, struct data_work* out_work)
{
	return query_get_open_work(context, out_work);
}

int
trckr_get_topic_by_name(struct trckr_ctx *context, char *name, struct data_work_topic* out_topic)
{
	return query_get_topic_by_name(context, name, out_topic);
}

int
trckr_get_topic_by_id(struct trckr_ctx *context, int id, struct data_work_topic* out_topic)
{
	return query_get_topic_by_id(context, id, out_topic);
}

int
trckr_start_work(struct trckr_ctx *context, int topic_id, char* description, time_t time, int* out_id)
{
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

	result = query_create_work(context, time, topic_id, description, out_id);
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

	result = query_stop_work(context, work.id, time);
	if (result != 0) {
		query_rollback(context);
		return result;
	}

	return query_commit(context);
}

int
trckr_create_topic(struct trckr_ctx *context, char* name, char* description)
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
trckr_iterate_topics_by_name(struct trckr_ctx *context, char* name, int (*callback)(struct data_work_topic*))
{
	return query_iterate_topics_by_name(context, name, callback);
}

int
trckr_switch_work(struct trckr_ctx *context, time_t time, int topic_id, char* description)
{
	int result;
	result = query_transaction(context);
	if (result != 0) {
		return result;
	}

	struct data_work work;
	result = query_get_open_work(context, &work);
	if (result == 0) {
		// there is open work
		result = query_stop_work(context, work.id, time);
		if (result != 0) {
			query_rollback(context);
			return result;
		}
	}
	else
	{
		if (result != TRCKR_NOT_FOUND) {
			// error occured
			query_rollback(context);
			return result;
		}
	}
	
	int id;
	result = query_create_work(context, time, topic_id, description, &id);
	if (result != 0) {
		query_rollback(context);
		return result;
	}

	return query_commit(context);
}
