#include <time.h>
#include <string.h>
#include "./sqlite3/sqlite3.h"
#include "error.c"

struct data_ctx 
{
	sqlite3 *db;
};

struct data_work {
	int id;
	int type_id;
	time_t start;
	time_t end;
};

struct data_type {
	int id;
	char *name;
	char *description; 
};

struct data_ctx
*data_init(char *filename)
{
	sqlite3 *db;
	int rc = sqlite3_open_v2(filename, &db, SQLITE_OPEN_READWRITE, NULL);
	if(ERROR) {
		return NULL;
	}
	struct data_ctx *context = malloc(sizeof(struct data_ctx));
	context->db = db;
	return context;
}

int
data_dispose(struct data_ctx *context)
{
	sqlite3_close_v2(context->db);
}

int
data_create_work(struct data_ctx *context, int typeid, time_t starttime)
{
	char *sql = "INSERT INTO WORK (type_id, start) VALUES (?1, ?2);";
	sqlite3_stmt *pstmt;
	sqlite3_prepare_v3(context->db, sql, -1, 0, &pstmt, NULL);
	if(ERROR) {
		THROW(ERR_SQLPREP)
		sqlite3_finalize(pstmt);
		return -1;
	}
	sqlite3_bind_int(pstmt, 1, typeid);
	sqlite3_bind_int64(pstmt, 2, starttime);
	if(ERROR) {
		THROW(ERR_DBOP)
		sqlite3_finalize(pstmt);
		return -1;
	}

	if (sqlite3_step(pstmt) != SQLITE_DONE) { 
		THROW(ERR_DBOP)
		sqlite3_finalize(pstmt);
		return -1;
	}

	sqlite3_finalize(pstmt);
	ERRRS;
	return 0;
}

int
data_create_type(struct data_ctx *context, char *name, char *description)
{
	char *sql = "INSERT INTO type (name, description) VALUES (?1, ?2);";
	sqlite3_stmt *pstmt;
	int res = sqlite3_prepare_v3(context->db, sql, -1, 0, &pstmt, NULL);
	if (res) {
		THROW(ERR_SQLPREP)
		sqlite3_finalize(pstmt);
		return -1;
	}

	sqlite3_bind_text(pstmt, 1, name, -1, NULL);
	sqlite3_bind_text(pstmt, 2, description, -1, NULL);
	if (sqlite3_step(pstmt) != SQLITE_DONE) { 
		THROW(ERR_DBOP)
		sqlite3_finalize(pstmt);
		return -1;
	}

	sqlite3_finalize(pstmt);
	ERRRS
	return 0;
}

int
data_stop_work(struct data_ctx *context, int workid, time_t time)
{
	char *sql = "UPDATE work SET end = ?2 WHERE id = ?1";
	sqlite3_stmt *pstmt;
	int res = sqlite3_prepare_v3(context->db, sql, -1, 0, &pstmt, NULL);
	if (res) {
		THROW(ERR_SQLPREP)
		sqlite3_finalize(pstmt);
		return -1;
	}

	sqlite3_bind_int(pstmt, 1, workid);
	sqlite3_bind_int64(pstmt, 2, time);
	if (sqlite3_step(pstmt) != SQLITE_DONE) { 
		THROW(ERR_DBOP)
		sqlite3_finalize(pstmt);
		return -1;
	}

	sqlite3_finalize(pstmt);
	ERRRS
	return 0;
}

struct data_work
*data_get_work_by_id(struct data_ctx *context, int id)
{
	char *sql = "SELECT id, type_id, start, end FROM work WHERE id = ?1;";
	sqlite3_stmt *pstmt;
	int res = sqlite3_prepare_v3(context->db, sql, -1, 0, &pstmt, NULL);
	if (res) {
		THROW(ERR_SQLPREP)
		sqlite3_finalize(pstmt);
		return NULL;
	}

	sqlite3_bind_int(pstmt, 1, id);
	int code = sqlite3_step(pstmt);
	if (code == SQLITE_DONE) { 
		THROW(ERR_WORKNOTFOUND)
		sqlite3_finalize(pstmt);
		return NULL;
	}
	if (code == SQLITE_ROW) {
		ERRRS
		struct data_work *work = malloc(sizeof(struct data_work));
		if(work == NULL) {
			sqlite3_finalize(pstmt);
			return NULL;
		}
		work->id = sqlite3_column_int(pstmt, 0);
		work->type_id = sqlite3_column_int(pstmt, 1);
		work->start = sqlite3_column_int64(pstmt, 2);
		work->end = sqlite3_column_int64(pstmt, 3);
		if (ERROR) {
			sqlite3_finalize(pstmt);
			return NULL;
		}

		sqlite3_finalize(pstmt);
		return work;
	}

	THROW(ERR_DBOP)
	sqlite3_finalize(pstmt);
	return NULL;
}

struct data_type
*data_get_type_by_id(struct data_ctx *context, int id)
{
	char *sql = "SELECT id, name, description FROM type WHERE id = ?1;";
	sqlite3_stmt *pstmt;
	int res = sqlite3_prepare_v3(context->db, sql, -1, 0, &pstmt, NULL);
	if (res) {
		THROW(ERR_SQLPREP)
		sqlite3_finalize(pstmt);
		return NULL;
	}

	sqlite3_bind_int(pstmt, 1, id);
	int code = sqlite3_step(pstmt);
	if (code == SQLITE_DONE) { 
		THROW(ERR_WORKNOTFOUND)
		sqlite3_finalize(pstmt);
		return NULL;
	}
	if (code == SQLITE_ROW) {
		ERRRS
		struct data_type *type = malloc(sizeof(struct data_type));
		if(type == NULL) {
			sqlite3_finalize(pstmt);
			return NULL;
		}

		ERRRS
		type->id = sqlite3_column_int(pstmt, 0);

		type->name = malloc(sqlite3_column_bytes(pstmt, 1));
		strcpy(type->name, (char *) sqlite3_column_text(pstmt, 1));
		
		type->description = malloc(sqlite3_column_bytes(pstmt, 2));
		strcpy(type->description, (char *) sqlite3_column_text(pstmt, 2));

		if (ERROR) {
			sqlite3_finalize(pstmt);
			return NULL;
		}

		sqlite3_finalize(pstmt);
		return type;
	}

	THROW(ERR_DBOP)
	sqlite3_finalize(pstmt);
	return NULL;
}

int
data_get_all_types(struct data_ctx *context, struct data_type* buf, int n, int skip)
{
	char *sql = "SELECT id, name, description FROM type LIMIT ?1 OFFSET ?2;";
	sqlite3_stmt *pstmt;
	int res = sqlite3_prepare_v3(context->db, sql, -1, 0, &pstmt, NULL);
	if (res) {
		THROW(ERR_SQLPREP)
		sqlite3_finalize(pstmt);
		return -1;
	}

	sqlite3_bind_int(pstmt, 1, n);
	sqlite3_bind_int(pstmt, 2, skip);

	int index = 0;
	int code;
	while (code = sqlite3_step(pstmt) == SQLITE_ROW)
	{
		struct data_type type;
		ERRRS
		type.id = sqlite3_column_int(pstmt, 0);

		type.name = malloc(sqlite3_column_bytes(pstmt, 1));
		strcpy(type.name, (char *) sqlite3_column_text(pstmt, 1));
		
		type.description = malloc(sqlite3_column_bytes(pstmt, 2));
		strcpy(type.description, (char *) sqlite3_column_text(pstmt, 2));

		if (ERROR) {
			sqlite3_finalize(pstmt);
			return -1;
		}

		buf[index] = type;
		index++;
	}

	if (code == SQLITE_DONE || code == SQLITE_OK) {
		sqlite3_finalize(pstmt);
		return index;
	}

	THROW(ERR_DBOP)
	sqlite3_finalize(pstmt);
	return -1;
}

int
data_get_type_by_name(struct data_ctx *context, char *name)
{
	char *sql = "SELECT id FROM type WHERE name = ?1;";
	sqlite3_stmt *pstmt;
	int res = sqlite3_prepare_v3(context->db, sql, -1, 0, &pstmt, NULL);
	if (res) {
		THROW(ERR_SQLPREP)
		sqlite3_finalize(pstmt);
		return -1;
	}

	sqlite3_bind_text(pstmt, 1, name, -1, NULL);
	int code = sqlite3_step(pstmt);
	if (code == SQLITE_DONE) { 
		THROW(ERR_TYPENOTFOUND)
		sqlite3_finalize(pstmt);
		return -1;
	}
	if (code == SQLITE_ROW) {
		ERRRS
		int id = sqlite3_column_int(pstmt, 0);
		sqlite3_finalize(pstmt);
		return id;
	}

	THROW(ERR_DBOP)
	sqlite3_finalize(pstmt);
	return -1;
}

int
data_get_open_work(struct data_ctx *context)
{
	char *sql = "SELECT id FROM work WHERE end IS NULL;";
	sqlite3_stmt *pstmt;
	int res = sqlite3_prepare_v3(context->db, sql, -1, 0, &pstmt, NULL);
	if (res) {
		THROW(ERR_SQLPREP)
		sqlite3_finalize(pstmt);
		return -1;
	}

	int code = sqlite3_step(pstmt);
	if (code == SQLITE_DONE) { 
		THROW(ERR_NOOPENWORK)
		sqlite3_finalize(pstmt);
		return -1;
	}
	if (code == SQLITE_ROW) {
		ERRRS
		int id = sqlite3_column_int(pstmt, 0);
		sqlite3_finalize(pstmt);
		return id;
	}

	THROW(ERR_DBOP)
	sqlite3_finalize(pstmt);
	return -1;
}