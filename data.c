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