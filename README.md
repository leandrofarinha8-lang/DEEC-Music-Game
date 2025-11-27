# DEEC-Music-Game

## Tests
On the *tests* folder the files should be able to run by themselves (or have the least amount of dependencies possible), this will make testing easier. On the src *folder* the code should be more compartimentated and reusable (ex: have a graphics library, pins definitions, etc).

## Data
At some point this should be the things you need to store on the SD.

## Maps File format

type1, startTime1, endTime1
type2, startTime2, endTime2
...

Ex:
L,1000,2000 -> left arrow that is on the start position on 1000 ms gametime and is at the hit-position at 2000 ms gametime.
U,1500,3000
D,3000,7000
R,4000,5000
...

IMPORTANT NOTE: SD.h allows a max of 8 char for file name and 3 for a file extension.
## Songs File format

frequency1 duration1
frequency2 duration2
...

Note: a frenquency o 0 is equal to a pause.
