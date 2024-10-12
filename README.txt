# trckr

trckr type 			
trckr type add
trckr type remove
trckr type update [type] [prop] [value]

trckr status

trckr start [type]			// start work now
trckr start [type] [time]	// start running work

trckr switch [type]			// stop open work, start another work

trckr stop				// stop open work now
trckr stopped [type] [time]	// stop open work at time


trckr report today
trckr report yesterday
trckr report month
trckr report last month
trckr report since last month
trckr report week
trckr report last week
trckr report since last week
trckr report year
trckr report last year
trckr report since last year

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

# time
now			// now
[hh:mm]		// time

# date
yest			// yesterday
today		// today
[dd]			// day, this month
[mm-dd]		// day-month, this year
[yyyy-mm-dd]	// specific datetime