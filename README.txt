# trckr

work type alias:
#mcs
#asd
#dis

trckr type 			
trckr type add
trckr type remove
trckr type update [type] [prop] [value]

trckr status

trckr start [type]			// start work now
trckr started [type] [time]	// start running work

trckr switch [type]			// stop open work, start another work

trckr stop				// stop open work now
trckr stopped [type] [time]	// stop open work at time

trckr report month-1
trckr report month
trckr report today
trckr report today-1
trckr report week
trckr report week-1
trckr report year
trckr report year-1
trckr report [time] [time]

# config
aliases:

# work
id
type
start
end

trckr switch 1

10:30
10 10:30
5-10 10:30
2023-05-10 10:30

try_parse_time
	return 
try_parse_date
try_parse_time

# time
now			// now
[hh:mm]		// time

dd:mm
-> TIME

# date
yest			// yesterday
today		// today
[dd]			// day, this month
[mm-dd]		// day-month, this year
[yyyy-mm-dd]	// specific datetime

# error handling
- return NULL or -1;
- set errno;


# config
key, value
format 1
alias_asd 1
alias_sdf 2
alias_cfd 3