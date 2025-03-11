# trckr

// pushes work on top of the selected work
trckr push [topic] [duration]
trckr push [topic] // does only work on top of the stack with closed work

trckr stop [time]    // stop open work at time

trckr pop   // removes the work at the cursor

trckr create stack [timestamp]
trckr remove stack

trckr work [date]
trckr work week
trckr work month

trckr status

trckr select open // select open work
trckr select [date] // selects the last work at the first stack of the day
trckr select [n] // selects the nth work at the currently selected date

trckr create topic [name] [description]
trckr remove topic [name]

starttime and duration shall be used.
The minimum time interval shall be set down to 5 min.
This ensures, that the start and end times can be used to track start and end of work day.

work can be pushed on a stack
downtime shall be modeled by pushing work without a topic (topicid = 0) onto a stack.
a stack must be explicitly started by a "start" timestamp and ended by a "end" timestamp
these stacks must not overlap but can grow independently

each work has a stack id assigned

the cursor selects a work wich is pointing to a stack id

# work
id
type
start
duration
stack

# topic
id
name
description

# cursor
workid

# stack
id
start
end

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
      ...
   1  10:30 0.5h  102  "do something"
   2  10:30 10.5h 101  "do something"
   3  10:30 2h    100  "do something else"
   4  10:30 1h    000  "do somethi"
   5  10:30 1h    000  "do somethi"
   6  10:30 4h    000

   7  10:30 4h    000  "do something"
   8  10:30 4h    000  "do something"
   9  10:30 4h    000  "do something"
  10  10:30 4h    000  "do something"
> 11  10:30 ?     201  "do something"
      ...
