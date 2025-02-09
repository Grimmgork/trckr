# trckr

// pushes work on top of the selected work
trckr push [topic] [duration]
trckr push [topic] // does only work on top of the stack

trckr stop [time]    // stop open work at time

trckr pop   // removes the work at the cursor

trckr select day [date]
trckr select [work_item]

trckr status

trckr work
trckr work week
trckr work month

Starttime and duration shall be used.
The minimum time interval shall be set down to 5 min.
This ensures, that the start and end times can be used to track start and end of work day.

a day shall be a stack from wich can be pushed and popped work
downtime shall be modeled by pushing work without a topic (topicid = 0) onto the stack.

# work
id
type
start
duration

# topic
id
name
description

# cursor
day
index
work_id

# work_item
[n]
[last]
[first]

# duration
1m
10.3h

# time
now			// now
[hh:mm]		// time

# date
yest			// yesterday
today		// today
[dd]			// day, this month
[mm-dd]		// day-month, this year
[yyyy-mm-dd]	// specific datetime

# timestamp
[time]
[date] [time]

# relative day description
today
yesterday
[n] days ago
[n] weeks ago

~ trckr

 # 22-10-2025 (3 days ago)
  < 10:00 - now >

   1 10:30  0.5h 102  "do someing"
   2 10:30 10.5h 101  "do somethin"
   3 10:30  2h   100  "do something else"
   4 10:30  1h   000  "do somethi"
   5 10:30  4h   000  "do smething"
=> 6 10:30  ?    201  "do sothing"