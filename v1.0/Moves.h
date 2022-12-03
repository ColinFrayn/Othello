/**********************************
 *    moves.h                     *
 *    Colin Frayn                 *
 *    June 2008                   *
 **********************************/

/*
  Contains the specific defines for moves.cpp
 */

#ifndef MOVES_H
#define MOVES_H

BITBOARD DoMove(BoardType *,int);
BITBOARD GenerateMoves(BoardType *);
void UndoMove(BoardType *,int,BITBOARD);
void Perft(void *);
int  PerftIter(BoardType *,int);

#endif // MOVES_H