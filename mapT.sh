#!/bin/bash

broker=$1

rows=4
cols=4
declare -a matrix

# Row 1
i=0
j=0
matrix[$((i*cols+j))]="S_You wake up on a cold stone floor"
i=1
j=0
matrix[$((i*cols+j))]="W_That is a wall"
i=2
j=0
matrix[$((i*cols+j))]="R_Nothing here, keep looking"
i=3
j=0
matrix[$((i*cols+j))]="C_Just more stone walls"

# Row 2
i=0
j=1
matrix[$((i*cols+j))]="R_You hear water dripping through the bricks"
i=1
j=1
matrix[$((i*cols+j))]="W_There isn't a door that way"
i=2
j=1
matrix[$((i*cols+j))]="R_Nothing of note here"
i=3
j=1
matrix[$((i*cols+j))]="W_The path is blocked that way"

# Row 3
i=0
j=2
matrix[$((i*cols+j))]="R_Your footsteps echo through the room"
i=1
j=2
matrix[$((i*cols+j))]="R_The door creaks as you open it"
i=2
j=2
matrix[$((i*cols+j))]="R_The faint smell of mold fills the air"
i=3
j=2
matrix[$((i*cols+j))]="W_That's just a wall"

# Row 4
i=0
j=3
matrix[$((i*cols+j))]="R_The room feels cold and empty"
i=1
j=3
matrix[$((i*cols+j))]="W_There isn't a door that way"
i=2
j=3
matrix[$((i*cols+j))]="I_You found the glowing artifact!"
i=3
j=3
matrix[$((i*cols+j))]="W_The path is blocked that way"

mosquitto_pub -h "$broker" -t "mudClient" -m "$rows"
mosquitto_pub -h "$broker" -t "mudClient" -m "$cols"

for ((i=0; i<rows; i++)); do
  for ((j=0; j<cols; j++)); do
    msg="${matrix[$((i*cols+j))]}"
    mosquitto_pub -h "$broker" -t "mudClient" -m "$msg"
  done
done
