# DEEC-Music-Game

## Tests
~On the *tests* folder the files should be able to run by themselves (or have the least amount of dependencies possible), this will make testing easier. On the src *folder* the code should be more compartimentated and reusable (ex: have a graphics library, pins definitions, etc).~

If no more errors are found tests are done.

## Data
This contains all the data that should be stored on the SD for a basic build of the game. If you want to create maps add a folder with the map name to /MAPS and follow the structure described bellow.

IMPORTANT NOTE: SD.h allows a max of 8 char for file name and 3 for a file extension.

## Maps File format

```
map_name
arrow_num
map_duration (ms)
type1, startTime1, endTime1
type2, startTime2, endTime2
...
```

Ex:

```
CrazyFrog
20000
4
L,1000,2000
U,1500,3000 
D,3000,7000
R,4000,5000
```
## Songs File format

```
frequency1 duration1
frequency2 duration2 
...
```


Note: a frenquency of 0 is equal to a pause.
