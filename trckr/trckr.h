#pragma once
#include <sqlite3.h>
#include <time.h>

#define TRCKR_MINIMUM_INTERVAL_SECONDS 60

#define TRCKR_OFFSET 1000
#define TRCKR_ERR 1 + TRCKR_OFFSET
#define TRCKR_ERR_SQL 2 + TRCKR_OFFSET
#define TRCKR_ERR_NO_OPEN_WORK 3 + TRCKR_OFFSET
#define TRCKR_ERR_ALLOC 4 + TRCKR_OFFSET
#define TRCKR_ERR_OPEN_WORK 5 + TRCKR_OFFSET
#define TRCKR_NOT_FOUND 6 + TRCKR_OFFSET
#define TRCKR_ERR_INITIALIZED 7 + TRCKR_OFFSET
#define TRCKR_ERR_NAME_TAKEN 8 + TRCKR_OFFSET
#define TRCKR_ERR_INVALID_INPUT 9 + TRCKR_OFFSET
#define TRCKR_ITERATION_DONE 10 + TRCKR_OFFSET
#define TRCKR_ERR_TEXT_TOO_LONG 11 + TRCKR_OFFSET
#define TRCKR_ERR_TRANSACTION_DEPTH 12 + TRCKR_OFFSET

typedef char trckr_text[256];
typedef char trckr_text_small[16];

struct trckr_ctx {
	sqlite3 *db;
	char transaction_depth;
	int work_id;
};

struct data_work {
	int id;
	int topic_id;
	time_t start;
	time_t duration;
	int stack_id;
	trckr_text description;
};

struct data_work_topic {
	int id;
	trckr_text_small name;
	trckr_text description;
};

struct data_status {
	int selected_day;
	struct data_work work;
	struct data_work_topic topic;
};

// struct data_work_aggregate {
//	struct data_work work;
//	struct data_work_topic topic;
// };

// struct data_work_topic_alias {
//	int id;
//	int work_type_id;
//	char[16] name;
// };

int trckr_initialize(char* path);

int trckr_begin(char* path, struct trckr_ctx* out_context);
void trckr_end_rollback(struct trckr_ctx* context); // TODO implement nested transactions
int trckr_end(struct trckr_ctx* context);

int trckr_get_topic_by_name(struct trckr_ctx *context, trckr_text_small name, struct data_work_topic *out_topic);
int trckr_get_topic_by_id(struct trckr_ctx *context, int id, struct data_work_topic *out_topic);
int trckr_get_open_work(struct trckr_ctx *context, struct data_work *out_work);
int trckr_get_work_by_id(struct trckr_ctx *context, int id, struct data_work *out_work);

int trckr_start_work(struct trckr_ctx *context, int topic_id, trckr_text description, time_t time, int *out_id);
int trckr_stop_work(struct trckr_ctx *context, time_t time);
int trckr_create_topic(struct trckr_ctx *context, trckr_text_small name, trckr_text description);
int trckr_iterate_topics_by_name(struct trckr_ctx *context, trckr_text_small name, struct data_work_topic *topic, int (*callback)());
int trckr_iterate_last_work(struct trckr_ctx *context, struct data_work *work, int count, int (*callback)());

int trckr_parse_text(const char* str, trckr_text buffer);
int trckr_parse_text_small(const char* str, trckr_text_small buffer);
