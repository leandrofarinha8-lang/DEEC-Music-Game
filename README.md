# DEEC-Music-Game

## Tests
On the *tests* folder the files should be able to run by themselves (or have the least amount of dependencies possible), this will make testing easier. On the src *folder* the code should be more compartimentated and reusable (ex: have a graphics library, pins definitions, etc).

## Data
At some point this should be the things you need to store on the SD.

## Maps File format
Probably a single file with the format:

Map name <br />
duration (ms) <br />
ArrowNum <br />
ArrowsArray <br />
(format: type, startTime, endTime) type should be the constant value defined on the code for each direction (or maybe a char like 'L' , 'U', 'D' or 'R') <br />
