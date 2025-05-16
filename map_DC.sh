#!/bin/bash

rows=5
cols=3
declare -a matrix

i=0
j=1
matrix[$((i*cols+j))]="S_You awaken in a strange stone chamber"

i=0
j=0
matrix[$((i*cols+j))]="W_Damp walls and silence surround you"

i=0
j=2
matrix[$((i*cols+j))]="W_A faint breeze passes through"

i=1
j=1
matrix[$((i*cols+j))]="W_Something scurried past your feet"

i=1
j=2
matrix[$((i*cols+j))]="W_Dust covers the old floor"

i=2
j=1
matrix[$((i*cols+j))]="W_No signs of life here"

i=2
j=2
matrix[$((i*cols+j))]="I_You found the glowing artifact!"

i=3
j=0
matrix[$((i*cols+j))]="W_Just cracked stone and silence"

i=3
j=2
matrix[$((i*cols+j))]="W_The walls seem to close in slightly"

i=4
j=1
matrix[$((i*cols+j))]="C_A chilling whisper echoes around you..."