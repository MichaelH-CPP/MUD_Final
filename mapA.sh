#!/bin/bash

rows=3
cols=5
declare -a matrix

i=0
j=0
matrix[$((i*cols+j))]="S_Woah it's dark in here"
i=1
j=0
matrix[$((i*cols+j))]="W_Nothing here, we should keep looking"
i=2
j=0
matrix[$((i*cols+j))]="W_It's just cold and empty here"
i=1
j=1
matrix[$((i*cols+j))]="W_Nothing of use here, better keep looking"
i=2
j=1
matrix[$((i*cols+j))]="W_Another empty stone chamber"
i=2
j=2
matrix[$((i*cols+j))]="W_this room is abandoned"
i=3
j=0
matrix[$((i*cols+j))]="W_Still nothing, keep searching"
i=3
j=1
matrix[$((i*cols+j))]="W_where could it be? Keep looking"
i=3
j=2
matrix[$((i*cols+j))]="I_You found the ___!"
i=4
j=0
matrix[$((i*cols+j))]="C_Smells horrible over here..."
