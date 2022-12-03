/**********************************
 *    defs.h                      *
 *    Colin Frayn                 *
 *    March 2001                  *
 **********************************/

/*
  This file includes all the definitions for various board mappings.
 */

#ifndef DEFS_H
#define DEFS_H

/* Global variables defined and used in main.c.  But these aren't the nasty kind of
 * global variables that you were warned against.  These are *nice* global variables. */
BITBOARD Mask[64],InvMask[64],FileMask[8],RankMask[8],DiagMaska1h8[64],DiagMaska8h1[64];
BITBOARD Targets[64][256], TargetsR90[64][256], TargetsL45[64][256], TargetsR45[64][256];
BITBOARD FullBoard, FPMask1, FPMask2, Neighbours[64], FlipMask[64][64], FlipMaskR45[64][64], FlipMaskL45[64][64];
BITBOARD CornerMask, CornerNextMask;
int DiagonalMask_a1h8[64],DiagonalMask_a8h1[64], Flip[64], Distance[64][64], RotateR45[64], RotateL45[64];
KeyType RandomTable[64][13];

/* square -> square mappings for a rotation of 90 degrees clockwise.
 * Such that RotateR90[sq] is sq rotated 90 degrees right. */
int RotateR90[64] = {
  h8,h7,h6,h5,h4,h3,h2,h1,
  g8,g7,g6,g5,g4,g3,g2,g1,
  f8,f7,f6,f5,f4,f3,f2,f1,
  e8,e7,e6,e5,e4,e3,e2,e1,
  d8,d7,d6,d5,d4,d3,d2,d1,
  c8,c7,c6,c5,c4,c3,c2,c1,
  b8,b7,b6,b5,b4,b3,b2,b1,
  a8,a7,a6,a5,a4,a3,a2,a1,
};

/* square -> square mappings for a rotation of 90 degrees anti-clockwise
 * Such that RotateL90[sq] is sq rotated 90 degrees left. */
int RotateL90[64] = {
  a1,a2,a3,a4,a5,a6,a7,a8,
  b1,b2,b3,b4,b5,b6,b7,b8,
  c1,c2,c3,c4,c5,c6,c7,c8,
  d1,d2,d3,d4,d5,d6,d7,d8,
  e1,e2,e3,e4,e5,e6,e7,e8,
  f1,f2,f3,f4,f5,f6,f7,f8,
  g1,g2,g3,g4,g5,g6,g7,g8,
  h1,h2,h3,h4,h5,h6,h7,h8
};

/* Formatting the rotation board like this makes it much easier to see
 * what is really going on.  These are just mappings effectively, mapping
 * each square in the source bitboard onto a new square in the rotated bitboard
 * such that diagonals are now lined up horizontally.*/

/* This board is used for working out leading diagonals (a1h8 sense).  It 
 * represents the square-square mappings for a rotation of 45 degrees clockwise.
 * such that InvRotateR45[sq] is the square that, when rotated right 45 degrees, is mapped to sq. */
int InvRotateR45[64] = {
a8,
a7,b8,
a6,b7,c8,
a5,b6,c7,d8,
a4,b5,c6,d7,e8,
a3,b4,c5,d6,e7,f8,
a2,b3,c4,d5,e6,f7,g8,
a1,b2,c3,d4,e5,f6,g7,h8,
b1,c2,d3,e4,f5,g6,h7,
c1,d2,e3,f4,g5,h6,
d1,e2,f3,g4,h5,
e1,f2,g3,h4,
f1,g2,h3,
g1,h2,
h1
};

/* This board is used for working out trailing diagonals (a8h1 sense). Effectively
 * it represents the square-square mappings for a rotation of 45 degrees anti-clockwise,
 * such that InvRotateL45[sq] is the square that, when rotated left 45 degrees, is mapped to sq. */
int InvRotateL45[64] = {
h8,
g8,h7,
f8,g7,h6,
e8,f7,g6,h5,
d8,e7,f6,g5,h4,
c8,d7,e6,f5,g4,h3,
b8,c7,d6,e5,f4,g3,h2,
a8,b7,c6,d5,e4,f3,g2,h1,
a7,b6,c5,d4,e3,f2,g1,
a6,b5,c4,d3,e2,f1,
a5,b4,c3,d2,e1,
a4,b3,c2,d1,
a3,b2,c1,
a2,b1,
a1   
};

/* These simply store the length of the diagonal in the required sense */
int DiagonalLength_a1h8[64] = {
  1,2,3,4,5,6,7,8,
  2,3,4,5,6,7,8,7,
  3,4,5,6,7,8,7,6,
  4,5,6,7,8,7,6,5,
  5,6,7,8,7,6,5,4,
  6,7,8,7,6,5,4,3,
  7,8,7,6,5,4,3,2,
  8,7,6,5,4,3,2,1
};

int DiagonalLength_a8h1[64] = {
  8,7,6,5,4,3,2,1,
  7,8,7,6,5,4,3,2,
  6,7,8,7,6,5,4,3,
  5,6,7,8,7,6,5,4,
  4,5,6,7,8,7,6,5,
  3,4,5,6,7,8,7,6,
  2,3,4,5,6,7,8,7,
  1,2,3,4,5,6,7,8 
};

/* These store the number of bits rotated bitboards need to be shifted to extract
 * the required row.  For example, if I wanted the diagonal from a6-c8, which is in the
 * a1h8 sense, I take the correct bitboard, namely the R45 one, and shift it right
 * by the stated amount (3) and then and it with (2^l)-1 where l is the diagonal
 * length. */
int DiagShifts_a1h8[64] = {
   0, 1, 3, 6,10,15,21,28,
   1, 3, 6,10,15,21,28,36,
   3, 6,10,15,21,28,36,43,
   6,10,15,21,28,36,43,49,
  10,15,21,28,36,43,49,54,
  15,21,28,36,43,49,54,58,
  21,28,36,43,49,54,58,61,
  28,36,43,49,54,58,61,63
};

int DiagShifts_a8h1[64] = {
  28,21,15,10, 6, 3, 1, 0,
  36,28,21,15,10, 6, 3, 1,
  43,36,28,21,15,10, 6, 3,
  49,43,36,28,21,15,10, 6,
  54,49,43,36,28,21,15,10,
  58,54,49,43,36,28,21,15,
  61,58,54,49,43,36,28,21,
  63,61,58,54,49,43,36,28
};
#endif // DEFS_H