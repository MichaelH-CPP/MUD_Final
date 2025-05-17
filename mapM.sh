#!/bin/bash

broker=$1

rows=3
cols=6
declare -a matrix

#first row
i=0
j=0
matrix[$((i*cols+j))]="S_It's really cold over here"
i=1
j=0
matrix[$((i*cols+j))]="R_Just more walking..."
i=2
j=0
matrix[$((i*cols+j))]="R_Almost tripped, that's embarrising"

#second row
i=0
j=1
matrix[$((i*cols+j))]="W_That is a wall"
i=1
j=1
matrix[$((i*cols+j))]="W_Path is blocked this way"
i=2
j=1
matrix[$((i*cols+j))]="R_Hopefully this is the right way"

#third row
i=0
j=2
matrix[$((i*cols+j))]="W_Path is blocked this way"
i=1
j=2
matrix[$((i*cols+j))]="W_There isn't a door that way"
i=2
j=2
matrix[$((i*cols+j))]="R_It's really quiet over here"

#fourth row
i=0
j=3
matrix[$((i*cols+j))]="R_Feels like you've been here before"
i=1
j=3
matrix[$((i*cols+j))]="R_Seems like something else passed through here"
i=2
j=3
matrix[$((i*cols+j))]="R_This room is very still"

#fifth row
i=0
j=4
matrix[$((i*cols+j))]="R_Darkness fills the room"
i=1
j=4
matrix[$((i*cols+j))]="W_There isn't a door that way"
i=2
j=4
matrix[$((i*cols+j))]="I_You found the glowing artifact!"

#sixth row
i=0
j=5
matrix[$((i*cols+j))]="C_This seems different than the other rooms"
i=1
j=5
matrix[$((i*cols+j))]="W_That is a wall"
i=2
j=5
matrix[$((i*cols+j))]="W_Path is blocked this way"

mosquitto_pub -h "$broker" -t "mudClient" -m "$rows"
mosquitto_pub -h "$broker" -t "mudClient" -m "$cols"

for ((i = 0; i < rows; i++)); do
    for ((j = 0; j < cols; j++)); do
        msg="${matrix[$((i*cols+j))]}"
        mosquitto_pub -h "$broker" -t "mudClient" -m "$msg"
    done
done
