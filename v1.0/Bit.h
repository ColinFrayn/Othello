/**********************************
 *    bit.h                       *
 *    Colin Frayn                 *
 *    May 2008                    *
 **********************************/

/*
  Contains the specific defines for bit.c
 */

#ifndef BIT_H
#define BIT_H

BITBOARD RotateBoard_R90(const BITBOARD);
BITBOARD RotateBoard_L90(const BITBOARD);
BITBOARD RotateBoard_R45(const BITBOARD);
BITBOARD RotateBoard_L45(const BITBOARD);
int Count(const BITBOARD);
int FirstPiece(const BITBOARD);
void ResetBoard(BoardType *);
int Pts(BoardType *, const int);
void SetupPrecalculatedData(void);
longlong Rand64(void);

#endif  // BIT_H