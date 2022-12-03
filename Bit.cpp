/**********************************
 *    bit.cpp                     *
 *    Colin Frayn                 *
 *    May 2008                    *
 **********************************/

/*
  This file contains all the algorithms for messing
  about with bitboards.
*/


#include "stdafx.h"
#include "Common.h"
#include "Defs.h"
#include "Bit.h"


 /* Return a bitboard rotated through 90 degrees anti-clockwise */
BITBOARD RotateBoard_R90(const BITBOARD B) {
  BITBOARD R=0,b=UNIT,i;
  for (i=0;i<64;i++) {
    if (B & Mask[i]) R += (b<<RotateR90[i]); 
  }
  return R;
}

 /* Return a bitboard rotated through 90 degrees clockwise */
BITBOARD RotateBoard_L90(const BITBOARD B) {
  BITBOARD R=0,b=UNIT,i;
  for (i=0;i<64;i++) {
    if (B & Mask[i]) R += (b<<RotateL90[i]); 
  }
  return R;
}

 /* Return a bitboard in a1h8 diagonal form */
BITBOARD RotateBoard_R45(const BITBOARD B) {
  BITBOARD R=0,b=UNIT,i;
  for (i=0;i<64;i++) {
    if (B & Mask[i]) R += (b<<RotateR45[i]); 
  }
  return R;
}

 /* Return a bitboard in a8h1 diagonal form */
BITBOARD RotateBoard_L45(const BITBOARD B) {
  BITBOARD R=0,b=UNIT,i;
  for (i=0;i<64;i++) {
    if (B & Mask[i]) R += (b<<RotateL45[i]); 
  }
  return R;
}

/* A list of the number of bits in numbers from 0-255.  This is used in the
  * bit counting algorithm.  Thanks to Dann Corbit for this one. */
static int inbits[256] = {
  0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4,
  1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
  1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
  2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
  1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
  2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
  2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
  3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
  1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
  2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
  2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
  3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
  2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
  3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
  3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
  4, 5, 5, 6, 5, 6, 6, 7, 5, 6, 6, 7, 6, 7, 7, 8,
};

/* This algorithm thanks to Dann Corbit.  Apparently it's faster than
 * the standard one.  */
int Count(const BITBOARD B) {
  return inbits[(unsigned char) B] +
    inbits[(unsigned char) (B >> 8)] +
    inbits[(unsigned char) (B >> 16)] +
    inbits[(unsigned char) (B >> 24)] +
    inbits[(unsigned char) (B >> 32)] +
    inbits[(unsigned char) (B >> 40)] +
    inbits[(unsigned char) (B >> 48)] +
    inbits[(unsigned char) (B >> 56)];
}

static const int lsz64_tbl[64] = {
     0, 31,  4, 33, 60, 15, 12, 34,
    61, 25, 51, 10, 56, 20, 22, 35,
    62, 30,  3, 54, 52, 24, 42, 19,
    57, 29,  2, 44, 47, 28,  1, 36,
    63, 32, 59,  5,  6, 50, 55,  7,
    16, 53, 13, 41,  8, 43, 46, 17,
    26, 58, 49, 14, 11, 40,  9, 45,
    21, 48, 39, 23, 18, 38, 37, 27
};

//______________________________________________________________________________
/* FirstPiece():
  *
  *      Return square number (0 to 63) of the least significant set bit
  *      in bitboard 'bb'
  *
  *      source: Matt Taylor's "de Bruijn method" implementation
  *      Probably best if you don't even TRY to understand this one. I certainly don't...
  */
//______________________________________________________________________________
int FirstPiece(const BITBOARD bb) {
    const BITBOARD lsb = (bb & -bb) - 1;
    const unsigned int foldedLSB = ((unsigned int) lsb) ^ ((unsigned int)(lsb >> 32));
    return lsz64_tbl[foldedLSB * 0x78291ACF >> 26];
}


/* Reset the board to the start position */
void ResetBoard(BoardType *B) {
  // Setup the bitboards
  B->white = Mask[d5] | Mask[e4];
  B->black = Mask[d4] | Mask[e5];
  // Initialise rotated bitboards
  B->WR90 = RotateBoard_R90(B->white);
  B->BR90 = RotateBoard_R90(B->black);
  B->WR45 = RotateBoard_R45(B->white);
  B->BR45 = RotateBoard_R45(B->black);
  B->WL45 = RotateBoard_L45(B->white);
  B->BL45 = RotateBoard_L45(B->black);
  // Black to play
  B->side = -1;
}

/* Return (slowly) the count of all points on the board.  Doesn't trust any
 * of the bitboards to be set up correctly */
int Pts(BoardType *B,const int side) {
  return (Count(B->white) - Count(B->black))*side; // Negative values for black
}


/* Setup the precalculated bitboards that we're going to need
 * later on in the game */
void SetupPrecalculatedData(void) {
  BITBOARD rank=FullRank;
  int i,j;

  srand((int)time(NULL));

  /* Rank and File Masks */
  FullBoard=0;
  for (i=0;i<8;i++) {
    FileMask[i] = 0;
    for (j=i;j<64;j+=8) FileMask[i] += (UNIT<<j);
    RankMask[i] = (rank << (8*i));
    FullBoard += RankMask[i];
  }
   
   /* Masks for the FirstPiece algorithm */
  FPMask1 = (BITBOARD)TwoFullRanks << 16;
  FPMask2 = (BITBOARD)TwoFullRanks << 32;
   
  /* Square Masks */
  for (i=0;i<64;i++) {
    Mask[i] = (UNIT<<i);
    InvMask[i] = ~Mask[i];
  }
   
  /* Setup inverse transformation matrices */
  for (i=0;i<64;i++) {
    RotateL45[(InvRotateL45[i])]=i;
    RotateR45[(InvRotateR45[i])]=i;
  }

  /* Setup masks for diagonal sliding pieces */
  for (i=0;i<64;i++) {
    DiagonalMask_a1h8[i] = (1 << DiagonalLength_a1h8[i])-1;
    DiagonalMask_a8h1[i] = (1 << DiagonalLength_a8h1[i])-1;
  }

   /* Setup a random key table for the hash function.
    * The hash key is of type KeyType. */
  for (i=0;i<64;i++) {
    for (j=0;j<13;j++) RandomTable[i][j] = (KeyType)(Rand64());
  }

  /* Distances between squares */
  for (i=0;i<64;i++) {
    for (j=0;j<64;j++) {
      Distance[i][j] = max(abs(Rank(j)-Rank(i)),abs(File(j)-File(i)));
    }
  }

  // Neighbouring locations
  for (i=0;i<64;i++) {
    Neighbours[i] = EMPTY;
    if (Rank(i)>0) {
      if (File(i)>0) Neighbours[i] += (UNIT<<(i-9));
      if (File(i)<7) Neighbours[i] += (UNIT<<(i-7));
      Neighbours[i] += (UNIT<<(i-8));
    }
    if (Rank(i)<7) {
      if (File(i)>0) Neighbours[i] += (UNIT<<(i+7));
      if (File(i)<7) Neighbours[i] += (UNIT<<(i+9));
      Neighbours[i] += (UNIT<<(i+8));
    }
    if (File(i)>0) Neighbours[i] += (UNIT<<(i-1));
    if (File(i)<7) Neighbours[i] += (UNIT<<(i+1));
  }

  // Flip targets.  Given the square id and rank mask, return a bitboard with 1s in all squares
  // where we need to test for friendly counters in order to check if a move at this location would
  // be legal (i.e. would flip counters in this row)
  // Horizontal & vertical masks
  for (i=0;i<64;i++) {
    int x = File(i), y = Rank(i);
    int xx,yy;
    for (int mask=0; mask<256; mask++) {
      Targets[i][mask] = EMPTY;
      TargetsR90[i][mask] = EMPTY;
      // Horizontal
      if (!(mask&(1<<x))) {
        for (xx=x-1; xx>=0; xx--) {
          if (!(mask&(1<<xx))) break;
        }
        if (xx>=0 && x-xx>1) Targets[i][mask] |= Mask[i-(x-xx)];
        for (xx=x+1; xx<8; xx++) {
          if (!(mask&(1<<xx))) break;
        }
        if (xx<8 && xx-x>1) Targets[i][mask] |= Mask[i+(xx-x)];
      }
      // Vertical
      if (!(mask&(1<<(7-y)))) {
        for (yy=y-1; yy>=0; yy--) {
          if (!(mask&(1<<(7-yy)))) break;
        }
        if (yy>=0 && y-yy>1) TargetsR90[i][mask] |= Mask[i-((y-yy)*8)];
        for (yy=y+1; yy<8; yy++) {
          if (!(mask&(1<<(7-yy)))) break;
        }
        if (yy<8 && yy-y>1) TargetsR90[i][mask] |= Mask[i+((yy-y)*8)];
      }
    }
  }
  
  // Diagonals. Start with the A1H8 sense
  for (i=0;i<64;i++) {
    // Get the far left hand square on this diagonal
    int diagstart = 7*(min((File(i)),7-(Rank(i)))) + i;
    int dsfile = File(diagstart);
    int dl = DiagonalLength_a1h8[i];
    int fi = File(i);
     // Loop through all possible occupations of this diagonal line
    for (int mask=0 ; mask < (1 << dl) ; mask++) {
      int x = fi-dsfile;
      int xx;
      TargetsR45[i][mask] = EMPTY;
      // Calculate possible target squares
      for (xx=x-1;xx>=0;xx--) {
        if (!(mask&(1<<xx))) break;
      }
      if (xx>=0 && x-xx>1) TargetsR45[i][mask] |= Mask[diagstart-(xx*7)];
      for (xx=x+1;xx<dl;xx++) {
        if (!(mask&(1<<xx))) break;
      }
      if (xx<dl && xx-x>1) TargetsR45[i][mask] |= Mask[diagstart-(xx*7)];
    }
  }

  // Diagonals; A8H1 sense.
  for (i=0;i<64;i++) {
    // Get the far left hand square on this diagonal
    int diagstart = i - 9*(min((File(i)),(Rank(i))));
    int dsfile = File(diagstart);
    int dl = DiagonalLength_a8h1[i];
    int fi = File(i);
    // Loop through all possible occupations of this diagonal line
    for (int mask=0 ; mask < (1 << dl) ; mask++) {
      int x = fi-dsfile;
      int xx;
      TargetsL45[i][mask] = EMPTY;
      // Calculate possible target squares
      for (xx=x-1;xx>=0;xx--) {
        if (!(mask&(1<<xx))) break;
      }
      if (xx>=0 && x-xx>1) TargetsL45[i][mask] |= Mask[diagstart+(xx*9)];
      for (xx=x+1;xx<dl;xx++) {
        if (!(mask&(1<<xx))) break;
      }
      if (xx<dl && xx-x>1) TargetsL45[i][mask] |= Mask[diagstart+(xx*9)];
    }
  }


  // Flip mask:
  // Contains a bitboard of 1s in each square that must be flipped
  // if a direct horizontal, vertical or diagonal line exists between
  // the two given squares
  for (i=0; i<64; i++) {
    for (j=0; j<64; j++) {
      FlipMask[i][j] = EMPTY;
      if (i == j) continue;
      int fx = File(i), fy = Rank(i);
      int tx = File(j), ty = Rank(j);
      // Same horizontal
      if (fy == ty) {
        for (int sq = min(i,j)+1; sq < max(i,j); sq++) FlipMask[i][j] |= Mask[sq];
      }
      // Same vertical
      if (fx == tx) {
        for (int sq = min(i,j)+8; sq < max(i,j); sq+=8) FlipMask[i][j] |= Mask[sq];
      }
      // Same diagonal, a1h8 sense
      if ((tx-fx) == (fy-ty)) {
        for (int sq = min(i,j)+7; sq < max(i,j); sq+=7) FlipMask[i][j] |= Mask[sq];
      }
      // Same diagonal, a8h1 sense
      if ((tx-fx) == (ty-fy)) {
        for (int sq = min(i,j)+9; sq < max(i,j); sq+=9) FlipMask[i][j] |= Mask[sq];
      }
      // Rotated versions
      FlipMaskR45[i][j] = RotateBoard_R45(FlipMask[i][j]);
      FlipMaskL45[i][j] = RotateBoard_L45(FlipMask[i][j]);
    }
  }

  // Corner Masks
  CornerMask = Mask[a1] | Mask[a8] | Mask[h1] | Mask[h8];
  CornerNextMask = Neighbours[a1] | Neighbours[a8] | Neighbours[h1] | Neighbours[h8];

   /* Phew */
}

// Get a 64 bit random number. The easy way.
longlong Rand64(void) {
  return (longlong)rand() + ((longlong)rand()<<32);
}
