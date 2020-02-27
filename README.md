# HTR
How to use the basic HTR solving mode: 

Corners are numbered 0-7, top layer corners are 0-3 and numbered clockwise with 0 at UFL.

Bottom layer corners are numbered 4-7 starting at DFR and going anti clockwise.

For edges only non M-Slice edges are considered (DR is with R and L as possible moves while U and F are not allowed).

Also, only orientation is considered, 1 is unoriented, 0 is oriented. 

Order is UL UR DL DR BL FL BR FR.

For example with R U2 R' as scramble the encoding would be: 10000001-34201567

and you would call the programm with ./HTR -s 10000001-34201567
