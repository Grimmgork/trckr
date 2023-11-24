#include <errno.h>

#define NOERR errno == 0
#define ERROR errno != 0
#define ISERR(code) errno == code
#define ERRRS errno = 0;
#define THROW(code) errno = code;

#define ERR_TYPENOTFOUND -10
#define ERR_PARSETYPE -11
#define ERR_PARSETIME -12
#define ERR_NOOPENWORK -13
#define ERR_OPENWORK -14
#define ERR_DBOP -15
#define ERR_TYPEEXISTS -16
#define ERR_SQLPREP -17
#define ERR_NOTIMPLEMENTED -18