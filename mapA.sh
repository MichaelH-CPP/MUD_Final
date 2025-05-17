#!/bin/bash

broker=$1

rows=3
cols=5
declare -a matrix

#first row
i=0
j=0
matrix[$((i*cols+j))]="S_Woah it's dark in here"
i=1
j=0
matrix[$((i*cols+j))]="R_Nothing here, we should keep looking"
i=2
j=0
matrix[$((i*cols+j))]="R_It's just cold and empty here"

#second row
i=0
j=1
matrix[$((i*cols+j))]="W_That is a wall"
i=1
j=1
matrix[$((i*cols+j))]="R_Nothing of use here, better keep looking"
i=2
j=1
matrix[$((i*cols+j))]="R_There isn't a door that way"

#third row
i=0
j=2
matrix[$((i*cols+j))]="W_Path is blocked this way"
i=1
j=2
matrix[$((i*cols+j))]="R_Another empty stone chamber"
i=2
j=2
matrix[$((i*cols+j))]="R_this room is abandoned"

#fourth row
i=0
j=3
matrix[$((i*cols+j))]="R_Still nothing, keep searching"
i=1
j=3
matrix[$((i*cols+j))]="R_where could it be? Keep looking"
i=2
j=3
matrix[$((i*cols+j))]="I_You found the glowing artifact!"

#fifth row
i=0
j=4
matrix[$((i*cols+j))]="C_Smells horrible over here..."
i=1
j=4
matrix[$((i*cols+j))]="W_There isn't a door that way"
i=2
j=4
matrix[$((i*cols+j))]="W_That is a wall"

mosquitto_pub -h "$broker" -t "mudClient" -m "$rows"
mosquitto_pub -h "$broker" -t "mudClient" 0m "$cols"

for ((i=0; i<rows; i++)); do
  for ((j=0; j<cols; j++)); do
    msg="${matrix[$((i*cols+j))]}"
    mosquitto_pub -h "$broker" -t "mudClient" -m "$msg"
  done
done
