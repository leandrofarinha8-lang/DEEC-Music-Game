# DEEC-Music-Game

## Tests
On the *tests* folder the files should be able to run by themselves (or have the least amount of dependencies possible), this will make testing easier. On the src *folder* the code should be more compartimentated and reusable (ex: have a graphics library, pins definitions, etc).

## Data
At some point this should be the things you need to store on the SD.

IMPORTANT NOTE: SD.h allows a max of 8 char for file name and 3 for a file extension.

## Maps File format

map_name <br>
arrow_num <br>
map_duration (ms)<br>
type1, startTime1, endTime1 <br>
type2, startTime2, endTime2 <br>
...

Ex:

CrazyFrog
20000
4
L,1000,2000 -> left arrow that is on the start position on 1000 ms gametime and is at the hit-position at 2000 ms gametime. <br>
U,1500,3000  <br>
D,3000,7000  <br>
R,4000,5000  <br>

## Songs File format

frequency1 duration1  <br>
frequency2 duration2  <br>
...

Note: a frenquency o 0 is equal to a pause.
