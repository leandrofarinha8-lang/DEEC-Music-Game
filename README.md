# DEEC-Music-Game

## A small video of the porject
https://drive.google.com/file/d/1O8VeOooAz9D24pQz-aa92cZrcmdjpqDR/view?usp=sharing

## Data
This contains all the data that should be stored on the SD for a basic build of the game. If you want to create maps add a folder with the map name to /MAPS and follow the structure described bellow.

IMPORTANT NOTE: SD.h allows a max of 8 char for file name and 3 for a file extension.

## Maps File format

On the folder you just created for your map store this on a file named MAP

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
7000
4
L,1000,2000
U,1500,3000 
D,3000,7000
R,4000,5000
```
## Songs File format

This should be stored on a file named SONG

```
frequency1 duration1
frequency2 duration2 
...
```


Note: a frenquency of 0 is equal to a pause.

## Backgrond image of a Map

The TFT screen we're using uses the BGR565 color format, the orientation of the pictures is upside down. So to add backgrounds you'll need to change the color channels and the roation of the image.
On each map folder there should be two bmp images. One named D.bmp which will be the one used during day (when the LDR detects high luminosity) and other named N.bmp which will be used at night.

<img width="3000" height="4000" alt="image" src="https://github.com/user-attachments/assets/da26dde6-09c6-48a0-b6dd-b3be624f7fd6" />



