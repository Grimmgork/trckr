# trckr

trckr topic
trckr add topic [name] [description]
trckr remove topic [name]
trckr update topic [name] [prop] [value]

trckr update work [prop] [value]
trckr update work [id] [prop] [value]

trckr start [topic] [time]	// start work at specific time
trckr start [topic]         // start work after last closed work

trckr stop		     // stop open work now
trckr stop [time]    // stop open work at time

trckr work
trckr work today
trckr work yesterday
trckr work week
trckr work month


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

# work
id
type
start
duration

# topic
id
name
description


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
