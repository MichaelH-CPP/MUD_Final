#!/bin/bash

broker=$1

rows=3
cols=5
declare -a matrix

#row 1
i=0
j=0
matrix[$((i*cols+j))]="S_You're' in a strange stone chamber"

i=1
j=0
matrix[$((i*cols+j))]="R_Damp walls and silence surround you"

i=2
j=0
matrix[$((i*cols+j))]="R_A faint breeze passes through"

i=3
j=0
matrix[$((i*cols+j))]="W_That is a wall"

i=4
j=0
matrix[$((i*cols+j))]="I_You found the glowing artifact!"

#row 2
i=0
j=1
matrix[$((i*cols+j))]="W_There isn's a door that way"

i=1
j=1
matrix[$((i*cols+j))]="W_The path is blocked that way"

i=2
j=1
matrix[$((i*cols+j))]="R_Just cracked stone and silence"

i=3
j=1
matrix[$((i*cols+j))]="R_The walls seem to close in slightly"

i=4
j=1
matrix[$((i*cols+j))]="R_You hear a distant dripping sound"

#row 3

i=0
j=2
matrix[$((i*cols+j))]="C_A chilling whisper echoes around you..."

i=1
j=2
matrix[$((i*cols+j))]="R_There's a broken lantern on the ground"

i=2
j=2
matrix[$((i*cols+j))]="R_You hear your footsteps echo here"

i=3
j=2
matrix[$((i*cols+j))]="W_That is a wall"

i=4
j=2
matrix[$((i*cols+j))]="W_There isn't a door that way"


mosquitto_pub -h "$broker" -t "mudClient" -m "$rows"
mosquitto_pub -h "$broker" -t "mudClient" -m "$cols"

for ((i=0; i<rows; i++)); do
  for ((j=0; j<cols; j++)); do
    msg="${matrix[$((i*cols+j))]}"
    mosquitto_pub -h "$broker" -t "mudClient" -m "$msg"
  done
done
