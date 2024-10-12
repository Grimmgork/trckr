#include <sqlite3.h>

#define TRCKR_ERR -1
#define TRCKR_ERR_SQL -2
#define TRCKR_ERR_NO_OPEN_WORK -3
#define TRCKR_ERR_ALLOC -4
#define TRCKR_ERR_OPEN_WORK -5
#define TRCKR_NOT_FOUND -6
#define TRCKR_ERR_INITIALIZED -7
#define TRCKR_ERR_NAME_TAKEN -8
#define TRCKR_ERR_INVALID_INPUT -9

struct trckr_ctx {
	sqlite3 *db;
};

struct data_work {
	int id;
	int topic_id;
	time_t start;
	time_t end;
	char description[256];
};

struct data_work_topic {
	int id;
	char name[16];
	char description[256];
};

// struct data_work_topic_alias {
// 	int id;
//	int work_type_id;
//	char[16] name;
// };

int trckr_begin(char* dbpath, struct trckr_ctx* out_context);
void trckr_end(struct trckr_ctx* context);
int trckr_init(char* path);

int trckr_get_topic_by_name(struct trckr_ctx* context, char* name, struct data_work_topic* out_topic);
int trckr_get_topic_by_id(struct trckr_ctx* context, int id, struct data_work_topic* out_topic);
// int trckr_get_open_work_id(struct trckr_ctx* context, int* out_id);
int trckr_get_open_work(struct trckr_ctx* context, struct data_work* out_work);
int trckr_get_work_by_id(struct trckr_ctx* context, int id, struct data_work* out_work);
int trckr_start_work(struct trckr_ctx *context, int topic_id, char* description, time_t time, int* out_id);
int trckr_stop_work(struct trckr_ctx *context, time_t time);
int trckr_switch_work(struct trckr_ctx *context, time_t time, int topic_id, char* description);
int trckr_create_topic(struct trckr_ctx *context, char* name, char* description);
int trckr_iterate_topics_by_name(struct trckr_ctx *context, char* name, int (*callback)(struct data_work_topic*));
int trckr_create_alias();
// int trckr_iterate_aliases(struct trckr_ctx *context, int topic_id, callback?);
// int trckr_print_report(struct trckr_ctx *context, FILE* handle, int from, int to);

// # work
// topic_id
// description
// start
// stop

// # topic
// id
// name
// description
// meta
