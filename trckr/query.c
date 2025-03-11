#include <trckr.h>
#include <assert.h>

int
query_create_schema(sqlite3 *db)
{
	const char *sql = "CREATE TABLE IF NOT EXISTS topic(id INTEGER PRIMARY KEY, name TEXT UNIQUE, description TEXT); \
	                   CREATE TABLE IF NOT EXISTS work(id INTEGER PRIMARY KEY, stack_id INTEGER, topic_id INTEGER, start INTEGER, duration INTEGER, description TEXT); \
					   CREATE Table IF NOT EXISTS stack(id INTEGER PRIMARY KEY); \
					   CREATE TABLE IF NOT EXISTS context(id INTEGER PRIMARY KEY, selected_work_id INTEGER);";
	int result = sqlite3_exec(db, sql, 0, 0, NULL);
	if (result != SQLITE_OK) {
		return TRCKR_ERR_SQL;
	}

	return 0;
}

int
query_create_topic(struct trckr_ctx *context, trckr_text_small name, trckr_text description)
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
	snprintf(out_work->description, sizeof(out_work->description), "%s", sqlite3_column_text(pstmt, 4));

	sqlite3_finalize(pstmt);
	return 0;
}

int
query_get_work_by_id(struct trckr_ctx *context, int id, struct data_work* out_work)
{
	char *sql = "SELECT id, topic_id, start, duration, stack_id, description FROM work WHERE id IS ?1;";
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
	out_work->stack_id = sqlite3_column_int(pstmt, 4);
	result = trckr_parse_text(sqlite3_column_text(pstmt, 5), out_work->description);
	if (result != 0) {
		sqlite3_finalize(pstmt);
		return result;
	}

	sqlite3_finalize(pstmt);
	return 0;
}

int
query_start_work(struct trckr_ctx *context, int stack_id, time_t start, int topic_id, trckr_text description, int* out_id)
{
	const char *sql = "INSERT INTO work (topic_id, stack_id, start, description) VALUES (?1, ?2, ?3, ?4);";
	
	sqlite3_stmt *pstmt;
	int result = sqlite3_prepare_v3(context->db, sql, -1, 0, &pstmt, NULL);
	if (result != SQLITE_OK) {
		sqlite3_finalize(pstmt);
		return TRCKR_ERR_SQL;
	}

	sqlite3_bind_int(pstmt, 1, topic_id);
	sqlite3_bind_int(pstmt, 2, stack_id);
	sqlite3_bind_int(pstmt, 3, start);
	sqlite3_bind_text(pstmt, 4, description, -1, NULL);
	result = sqlite3_step(pstmt);

	if (result != SQLITE_DONE) {
		sqlite3_finalize(pstmt);
		return TRCKR_ERR_SQL;
	}

	*out_id = sqlite3_last_insert_rowid(context->db);

	sqlite3_finalize(pstmt);
	return 0;
}

int
query_create_stack(struct trckr_ctx *context, int *out_id)
{
	const char *sql = "INSERT INTO stack (id) VALUES(NULL);";
	int result;
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

	*out_id = sqlite3_last_insert_rowid(context->db);

	sqlite3_finalize(pstmt);
	return 0;
}

int
query_stop_work(struct trckr_ctx *context, int work_id, int duration)
{
	const char *sql = "UPDATE work SET duration=?1 WHERE id=?2;";
	int result;
	sqlite3_stmt *pstmt;
	result = sqlite3_prepare_v3(context->db, sql, -1, 0, &pstmt, NULL);
	if (result != SQLITE_OK) {
		sqlite3_finalize(pstmt);
		return TRCKR_ERR_SQL;
	}

	sqlite3_bind_int(pstmt, 1, duration);
	sqlite3_bind_int(pstmt, 2, work_id);
	result = sqlite3_step(pstmt);

	if (result != SQLITE_DONE) {
		sqlite3_finalize(pstmt);
		return TRCKR_ERR_SQL;
	}

	sqlite3_finalize(pstmt);
	return 0;
}

int
query_iterate_topics_by_name(struct trckr_ctx *context, trckr_text_small name, struct data_work_topic *topic, int(*callback)())
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

	while (1) {
		result = sqlite3_step(pstmt);
		if (result == SQLITE_DONE) {
			break;
		}

		if (result != SQLITE_ROW) {
			sqlite3_finalize(pstmt);
			return TRCKR_ERR_SQL;
		}

		if (topic != NULL)
		{
			topic->id = sqlite3_column_int(pstmt, 0);
			snprintf(topic->name, sizeof(topic->name), "%s", sqlite3_column_text(pstmt, 1));
			snprintf(topic->description, sizeof(topic->description), "%s", sqlite3_column_text(pstmt, 2));
		}
		
		result = callback();
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
	snprintf(out_topic->name, sizeof(out_topic->name), "%s", sqlite3_column_text(pstmt, 1));
	snprintf(out_topic->description, sizeof(out_topic->description), "%s", sqlite3_column_text(pstmt, 2));

	sqlite3_finalize(pstmt);
	return 0;
}

int
query_get_topic_by_name(struct trckr_ctx *context, trckr_text_small name, struct data_work_topic* out_topic)
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
	snprintf(out_topic->name, sizeof(out_topic->name), "%s", sqlite3_column_text(pstmt, 1));
	snprintf(out_topic->description, sizeof(out_topic->description), "%s", sqlite3_column_text(pstmt, 2));

	sqlite3_finalize(pstmt);
	return 0;
}

int
query_savepoint(struct trckr_ctx* context)
{
	assert(context->transaction_depth > 0);

	if (context->transaction_depth == 255) {
		return TRCKR_ERR_TRANSACTION_DEPTH;
	}

	int result;
	char sql[16];
	char depth = context->transaction_depth + 1;
	snprintf(sql, sizeof(sql), "SAVEPOINT SP%02x;", depth);

	sqlite3_stmt *pstmt;
	result = sqlite3_prepare_v3(context->db, (const char*)sql, -1, 0, &pstmt, NULL);
	if (result != SQLITE_OK) {
		sqlite3_finalize(pstmt);
		return TRCKR_ERR_SQL;
	}

	result = sqlite3_step(pstmt);
	if (result != SQLITE_DONE) {
		sqlite3_finalize(pstmt);
		return TRCKR_ERR_SQL;
	}

	context->transaction_depth++;
	sqlite3_finalize(pstmt);
	return 0;
}

int
query_savepoint_commit(struct trckr_ctx* context)
{
	int result;

	assert(context->transaction_depth > 1);

	char sql[24];
	char depth = context->transaction_depth;
	snprintf(sql, sizeof(sql), "RELEASE SAVEPOINT SP%02x;", depth);

	sqlite3_stmt *pstmt;
	result = sqlite3_prepare_v3(context->db, (const char *)sql, -1, 0, &pstmt, NULL);
	if (result != SQLITE_OK) {
		sqlite3_finalize(pstmt);
		return TRCKR_ERR_SQL;
	}

	result = sqlite3_step(pstmt);
	if (result != SQLITE_DONE) {
		sqlite3_finalize(pstmt);
		return TRCKR_ERR_SQL;
	}

	context->transaction_depth--;
	sqlite3_finalize(pstmt);
	return 0;
}

int
query_savepoint_rollback(struct trckr_ctx* context)
{
	int result;

	assert(context->transaction_depth > 1);

	char sql[30];
	char depth = context->transaction_depth;
	snprintf(sql, sizeof(sql), "ROLLBACK TRANSACTION TO SP%02x;", depth);

	sqlite3_stmt *pstmt;
	result = sqlite3_prepare_v3(context->db, (const char *)sql, -1, 0, &pstmt, NULL);
	if (result != SQLITE_OK) {
		sqlite3_finalize(pstmt);
		return TRCKR_ERR_SQL;
	}

	result = sqlite3_step(pstmt);
	if (result != SQLITE_DONE) {
		sqlite3_finalize(pstmt);
		return TRCKR_ERR_SQL;
	}

	context->transaction_depth--;
	sqlite3_finalize(pstmt);
	return 0;
}

int
query_transaction(struct trckr_ctx* context)
{
	if (context->transaction_depth > 0) {
		return query_savepoint(context);
	}

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

	context->transaction_depth++;
	sqlite3_finalize(pstmt);
	return 0;
}

int
query_rollback(struct trckr_ctx* context)
{
	if (context->transaction_depth > 1) {
		return query_savepoint_rollback(context);
	}

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

	context->transaction_depth--;
	sqlite3_finalize(pstmt);
	return 0;
}

int
query_commit(struct trckr_ctx* context)
{
	if (context->transaction_depth > 1) {
		return query_savepoint_commit(context);
	}

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

	context->transaction_depth--;
	sqlite3_finalize(pstmt);
	return 0;
}

int
query_is_last_work_of_stack(struct trckr_ctx* context, int stack_id, int work_id, int *out_result)
{
	// TODO
}

int
query_has_work_overlap(struct trckr_ctx* context, time_t start, int duration, int *out_result)
{
	// TODO
}

int
query_iterate_stack(struct trckr_ctx* context, int stack_id, struct data_work* work, int(*callback)())
{
	// TODO
}



int
query_update_work_time(struct trckr_ctx* context, int id, int start, int duration)
{
	int result;
	const char *sql = "UPDATE work SET start=?1, duration=?2 WHERE id=?3;";
	sqlite3_stmt *pstmt;
	result = sqlite3_prepare_v3(context->db, sql, -1, 0, &pstmt, NULL);
	if (result != SQLITE_OK) {
		sqlite3_finalize(pstmt);
		return TRCKR_ERR_SQL;
	}

	sqlite3_bind_int(pstmt, 1, start);
	sqlite3_bind_int(pstmt, 2, duration);
	sqlite3_bind_int(pstmt, 3, id);

	result = sqlite3_step(pstmt);
	if (result != SQLITE_DONE) {
		sqlite3_finalize(pstmt);
		return TRCKR_ERR_SQL;
	}

	sqlite3_finalize(pstmt);
	return 0;
}

int
query_iterate_work_day(struct trckr_ctx* context, int skip, struct data_work *work, int(*callback)())
{
	assert(work != NULL);
	assert(callback != NULL);
	assert(skip >= 0);

	int result;
	const char *sql = "SELECT id, topic_id, start, duration, description FROM work WHERE start > ?1 AND start < ?2 ORDER BY start SKIP ?3;";
	sqlite3_stmt *pstmt;
	result = sqlite3_prepare_v3(context->db, sql, -1, 0, &pstmt, NULL);
	if (result != SQLITE_OK) {
		sqlite3_finalize(pstmt);
		return TRCKR_ERR_SQL;
	}

	int start = 0;// context->day * 86400;
	int end = 0;// (context->day + 1) * 86400;

	sqlite3_bind_int(pstmt, 1, start);
	sqlite3_bind_int(pstmt, 2, end);
	sqlite3_bind_int(pstmt, 3, skip);

	while (1)
	{
		result = sqlite3_step(pstmt);
		if (result == SQLITE_DONE) {
			break;
		}

		if (result != SQLITE_ROW) {
			sqlite3_finalize(pstmt);
			return TRCKR_ERR_SQL;
		}

		work->id = sqlite3_column_int(pstmt, 0);
		work->topic_id = sqlite3_column_int(pstmt, 1);
		work->start = sqlite3_column_int(pstmt, 2);
		work->duration = sqlite3_column_int(pstmt, 3);
		result = trckr_parse_text(sqlite3_column_text(pstmt, 4), work->description);
		if (result != 0) {
			sqlite3_finalize(pstmt);
			return result;
		}

		result = callback();
		if (result == TRCKR_ITERATION_DONE) {
			sqlite3_finalize(pstmt);
			return 0;
		}

		if (result != 0) {
			// error occured
			sqlite3_finalize(pstmt);
			return result;
		}
	}
}

int
query_load_context(struct trckr_ctx* context)
{
	const char *sql = "SELECT selected_work_id FROM context WHERE rowid = 1;";
	int result;
	sqlite3_stmt *pstmt;
	result = sqlite3_prepare_v3(context->db, sql, -1, 0, &pstmt, NULL);
	if (result != SQLITE_OK) {
		sqlite3_finalize(pstmt);
		return TRCKR_ERR_SQL;
	}

	result = sqlite3_step(pstmt);
	if (result == SQLITE_DONE) 
	{
		context->work_id = 0;
	}
	else if (result == SQLITE_ROW)
	{
		context->work_id = sqlite3_column_int(pstmt, 0);
	}
	else
	{
		sqlite3_finalize(pstmt);
		return TRCKR_ERR_SQL;
	}

	sqlite3_finalize(pstmt);
	return 0;
}

int
query_write_context(struct trckr_ctx* context)
{
	const char *sql = "INSERT INTO context(id, selected_work_id) VALUES(1, ?1) \
	                   ON CONFLICT(id) DO UPDATE SET selected_work_id=?1 WHERE id = 1;";
	int result;
	sqlite3_stmt *pstmt;
	result = sqlite3_prepare_v3(context->db, sql, -1, 0, &pstmt, NULL);
	if (result != SQLITE_OK) {
		sqlite3_finalize(pstmt);
		return TRCKR_ERR_SQL;
	}

	sqlite3_bind_int(pstmt, 1, context->work_id);

	result = sqlite3_step(pstmt);
	if (result != SQLITE_DONE) {
		sqlite3_finalize(pstmt);
		return TRCKR_ERR_SQL;
	}

	sqlite3_finalize(pstmt);
	return 0;
}