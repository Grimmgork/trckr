# trckr

trckr type
trckr add type [name] [description]
trckr remove type [name]
trckr set type [name] [prop] [value]

trckr set [type] [identifier] [prop] [value]

trckr start [type]			// start work now
trckr start [type] [time]	// start running work

trckr switch [type]			// stop open work, start another work

trckr stop				// stop open work now

trckr work

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

trckr start 101 "asdad ad awd awdawd"
trckr stop
