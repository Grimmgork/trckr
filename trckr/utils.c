#include <time.h>

int
trckr_get_day_from_unix(time_t time, time_t *out_start, time_t *out_end)
{
	struct tm* local;
	local = localtime(&time);
	if (local == NULL) {
		return 1;
	}

	local->tm_sec = 0;
	local->tm_min = 0;
	local->tm_hour = 0;
	*out_start = mktime(local);

	local->tm_sec = 59;
	local->tm_min = 59;
	local->tm_hour = 23;
	*out_end = mktime(local);
	return 0;
}