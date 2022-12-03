/**********************************
 *    moves.cpp                   *
 *    Colin Frayn                 *
 *    June 2008                   *
 **********************************/

/*
  This file contains all the algorithms for generating and playing moves.
*/


#include "stdafx.h"
#include "Common.h"
#include "Moves.h"
#include "Bit.h"
#include "Comp.h"

extern int RotateR90[64], RotateR45[64], RotateL45[64];
extern int DiagShifts_a1h8[64], DiagonalMask_a1h8[64];
extern int DiagShifts_a8h1[64], DiagonalMask_a8h1[64], MaxDepth;
extern BITBOARD Mask[64], Neighbours[64], FlipMask[64][64], FlipMaskR45[64][64], FlipMaskL45[64][64];
extern BITBOARD Targets[64][256], TargetsR90[64][256], TargetsL45[64][256], TargetsR45[64][256];
extern bool bCalculating;
extern HWND hWndMain;

// Do a move on the board and return the undo information
BITBOARD DoMove(BoardType *B, int sq) {
  int x = sq&7, y = sq/8, mask;
  BITBOARD Flip = EMPTY, FlipR45 = EMPTY, FlipL45 = EMPTY, Opponent, undo;

  // Skip a move
  if (sq == -1) { B->side = -B->side; return EMPTY; }

  // Play the counter
  if (B->side == BLACK) { B->black |= Mask[sq]; B->BR90 |= Mask[RotateR90[sq]]; B->BL45 |= Mask[RotateL45[sq]]; B->BR45 |= Mask[RotateR45[sq]]; } 
  else { B->white |= Mask[sq]; B->WR90 |= Mask[RotateR90[sq]]; B->WL45 |= Mask[RotateL45[sq]]; B->WR45 |= Mask[RotateR45[sq]]; } 

  // ---- Update the bitboards by flipping counters
  // -- First get all target squares between this square and which we need to flip counters
  if (B->side == WHITE) Opponent = B->black;
  else Opponent = B->white;
  mask = (int)((Opponent >> (Rank(sq)<<3)) & FullRank);
  Flip |= Targets[sq][mask];

  // -- Now get a list of all target squares
  // Capture vertically
  if (B->side == WHITE) Opponent = B->BR90;
  else Opponent = B->WR90;
  mask = (int)((Opponent >> (File(sq)<<3)) & FullRank);
  Flip |= TargetsR90[sq][mask];

  // Capture diagonally a1-h8 sense
  if (B->side == WHITE) Opponent = B->BR45;
  else Opponent = B->WR45;
  mask = (int)((Opponent >> DiagShifts_a1h8[sq]) & DiagonalMask_a1h8[sq]);
  Flip |= TargetsR45[sq][mask];

  // Capture diagonally a8-h1 sense
  if (B->side == WHITE) Opponent = B->BL45;
  else Opponent = B->WL45;
  mask = (int)((Opponent >> DiagShifts_a8h1[sq]) & DiagonalMask_a8h1[sq]);
  Flip |= TargetsL45[sq][mask];

  // Get all target squares that actually contain friendly counters
  if (B->side == BLACK) Flip &= B->black;
  else Flip &= B->white;
  undo = Flip;

  // -- Loop through all friendly targets and flip the counters in lines from the new counter to each target counter
  if (B->side == WHITE) {
    while (Flip) {
      int to = FirstPiece(Flip);
      RemoveFirst(Flip);
      B->white |= FlipMask[sq][to];
      B->black ^= FlipMask[sq][to];
      B->WR90 |= FlipMask[RotateR90[sq]][RotateR90[to]];
      B->BR90 ^= FlipMask[RotateR90[sq]][RotateR90[to]];
      B->WR45 |= FlipMaskR45[sq][to];
      B->BR45 ^= FlipMaskR45[sq][to];
      B->WL45 |= FlipMaskL45[sq][to];
      B->BL45 ^= FlipMaskL45[sq][to];
    }
  }
  else {
    while (Flip) {
      int to = FirstPiece(Flip);
      RemoveFirst(Flip);
      B->black |= FlipMask[sq][to];
      B->white ^= FlipMask[sq][to];
      B->BR90 |= FlipMask[RotateR90[sq]][RotateR90[to]];
      B->WR90 ^= FlipMask[RotateR90[sq]][RotateR90[to]];
      B->BR45 |= FlipMaskR45[sq][to];
      B->WR45 ^= FlipMaskR45[sq][to];
      B->BL45 |= FlipMaskL45[sq][to];
      B->WL45 ^= FlipMaskL45[sq][to];
    }
  }

  // Flip the side
  B->side = -B->side;

  // Return the flip mask as undo information
  return undo;
}

// Generate a list of legal moves for the current player
// Do it the slow way at first
BITBOARD GenerateMoves(BoardType *B) {
  BITBOARD Moves = EMPTY, Candidates = EMPTY;
  BITBOARD temp, mask, All = B->white|B->black, Self, Opponent, target;

  // Get all potential empty squares that are next to an opponent piece
  if (B->side == WHITE) {temp = B->black; Self = B->white; }
  else { temp = B->white; Self = B->black; }
  while (temp) {
    int foo = FirstPiece(temp);
    Candidates |= Neighbours[FirstPiece(temp)];
    RemoveFirst(temp);
  }
  Candidates &= ~(B->white|B->black);

  // Loop through potential squares and see if they are legal moves
  while (Candidates) {
    int sq = FirstPiece(Candidates);
    RemoveFirst(Candidates);

    // ---- Is this a legal move?

    // -- Capture horizontally
    if (B->side == WHITE) Opponent = B->black;
    else Opponent = B->white;
    mask = (int)((Opponent >> (Rank(sq)<<3)) & FullRank);
    target = Targets[sq][mask];
    if (target&Self) { Moves |= Mask[sq]; continue; }

    // -- Capture vertically
    if (B->side == WHITE) Opponent = B->BR90;
    else Opponent = B->WR90;
    mask = (int)((Opponent >> (File(sq)<<3)) & FullRank);
    target = TargetsR90[sq][mask];
    if (target&Self) { Moves |= Mask[sq]; continue; }

    // Capture diagonally a1-h8 sense
    if (B->side == WHITE) Opponent = B->BR45;
    else Opponent = B->WR45;
    mask = (int)((Opponent >> DiagShifts_a1h8[sq]) & DiagonalMask_a1h8[sq]);
    target = TargetsR45[sq][mask];
    if (target&Self) { Moves |= Mask[sq]; continue; }

    // Capture diagonally a8-h1 sense
    if (B->side == WHITE) Opponent = B->BL45;
    else Opponent = B->WL45;
    mask = (int)((Opponent >> DiagShifts_a8h1[sq]) & DiagonalMask_a8h1[sq]);
    target = TargetsL45[sq][mask];
    if (target&Self) { Moves |= Mask[sq]; continue; }
  }

  return Moves;
}

// Undo a move on the board
void UndoMove(BoardType *B, int sq, BITBOARD target) {
  // Skipped a move
  if (sq == -1) { B->side = -B->side; return; }

  // Play the counter
  if (B->side == WHITE) B->black ^= Mask[sq];
  else B->white ^= Mask[sq];

  // -- Unflip counters
  if (B->side == BLACK) {
    while (target) {
      int to = FirstPiece(target);
      RemoveFirst(target);
      B->white ^= FlipMask[sq][to];
      B->black |= FlipMask[sq][to];
    }
  }
  else {
    while (target) {
      int to = FirstPiece(target);
      RemoveFirst(target);
      B->black ^= FlipMask[sq][to];
      B->white |= FlipMask[sq][to];
    }
  }

  // Setup the rotated bitboards
  // This algorithm is only used in low-speed situations,
  // so there's no need to optimise this part
  B->WR90 = RotateBoard_R90(B->white);
  B->BR90 = RotateBoard_R90(B->black);
  B->WR45 = RotateBoard_R45(B->white);
  B->BR45 = RotateBoard_R45(B->black);
  B->WL45 = RotateBoard_L45(B->white);
  B->BL45 = RotateBoard_L45(B->black);

  // Flip the side
  B->side = -B->side;
}


// The main entry point for the calculation thread
void Perft(void *InitialBoard) {
  int count;

  bCalculating = true;

  // Set up the board as the correct type
  BoardType *B = (BoardType *)InitialBoard;
  SetStartTime();
  count = PerftIter(B,MaxDepth);
  int t = GetElapsedTime();
  bCalculating = false;
  char strTemp[128];
  sprintf(strTemp,"Depth %d completed\nTime = %.2f seconds\nNode count = %d",MaxDepth,(float)t/100.0f,count);
  MessageBox(hWndMain,strTemp,"Movegen Test completed",MB_OK);
}

// Iteration step for the perft component
int PerftIter(BoardType *B, int depth) {
  int count = 0, sq;
  BITBOARD Moves = GenerateMoves(B);
  BoardType Backup;

  if (depth == 0) return Count(Moves);

  // Loop through all the moves
  while (Moves) {
    sq = FirstPiece(Moves);
    Backup = *B;

    // Do the move and recurse
    (void)DoMove(B,sq);
    count += PerftIter(B,depth-1);

    // Undo the move
    *B = Backup;

    // Remove this move from the list
    RemoveFirst(Moves);
  }
  return count;
}