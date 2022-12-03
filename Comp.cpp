/**********************************
 *    comp.cpp                    *
 *    Colin Frayn                 *
 *    June 2008                   *
 **********************************/

/*
  This file contains all the algorithms for the computer engine.
*/


#include "stdafx.h"
#include "Common.h"
#include "Comp.h"
#include "Moves.h"
#include "Bit.h"
#include "Othello.h"
#include "resource.h"

extern BITBOARD PossibleMoves, Undo[64], CornerMask, CornerNextMask;
extern int iHistoryPos, iHistoryMax;
extern int History[64], MaxDepth, SearchTime;
extern bool bCalculating;
extern HWND hWndMain;
extern HMENU g_hMenu;
extern BoardType Board;
extern float flicker;

// Globals required for the computer engine
int g_BestMove = -1, g_ThinkingMove = -1, g_Depth = -1, g_Score;
struct timeb start;       // Used for the timer
bool bAbortFlag = false;


// The main entry point for the calculation thread
void Comp(void *Ignore) {
  int depth=2;
  bool bLastPass = false;

  bCalculating = true;
  bAbortFlag = false;
  CheckMenuItem(g_hMenu,IDM_OPTIONS_FORCE,MF_ENABLED);

  // Set up the board. Use a copy because we don't want to
  // alter the original and have that displayed during the thinking
  BoardType BoardCopy = Board;
  BoardType *B = &BoardCopy;
  if (iHistoryPos>0 && History[iHistoryPos-1] == -1) bLastPass = true;
  SetStartTime();
  g_Score = 0;

  // -- Iterative deepening loop
  do {
    // Search for the best move
    g_Depth = depth;
    g_Score = Search(B,depth,0,-(WIN-100),WIN-100,Count(B->white|B->black),bLastPass);

    // If this was a win then don't search further
    if (g_Score < -(WIN-100) || g_Score > (WIN-100)) bAbortFlag = true;

    // Loop until target depth reached and we've run out of time
    // Don't loop if there's only one legal move, obviously! ;)
    // Only loop if we haven't saturated the board
  } while (!bAbortFlag && Count(PossibleMoves) > 1 && (Count(B->white|B->black)+depth)<64 && (depth++ < MaxDepth || GetElapsedTime() < (SearchTime * 75)));

  // Do the best move
  B = &Board;
  Undo[iHistoryPos] = DoMove(B,g_BestMove);
  History[iHistoryPos++] = g_BestMove;
  iHistoryMax = iHistoryPos;
  flicker = 0.0f;
  EnableMenuItem(g_hMenu,IDM_EDIT_UNDO,MF_ENABLED);
  EnableMenuItem(g_hMenu,IDM_EDIT_REDO,MF_GRAYED);
  CheckMenuItem(g_hMenu,IDM_OPTIONS_FORCE,MF_GRAYED);
  g_BestMove = g_ThinkingMove = g_Depth = -1;

  // Generate the new possible move list
  PossibleMoves = GenerateMoves(B);

  // ** THREAD ENDS **
  bCalculating = false;
}

// Start the search
int Search(BoardType *B, int depth, int ply, int alpha, int beta, int counters, bool bLastPass) {
  int iBestMove = -1, score;
  BITBOARD Moves = GenerateMoves(B);
  BoardType Backup;

  // Allow a pass move
  if (Moves == EMPTY) {
    // Game over
    if (bLastPass) {
      score = Count(B->black) - Count(B->white);
      if (B->side * score < 0) return WIN-ply;
      if (B->side * score > 0) return -(WIN-ply);
      return DRAW;
    }
    // Play a pass move
    B->side = -B->side;
    // Leaf node?
    if (depth <= 1) score = Score(B);
    // Recurse
    else score = -Search(B,depth-1,ply+1,-beta,-alpha,counters,true);
    // Return the score
    if (score > beta) return beta;
    else return max(alpha,score);
  }

  // Loop through all the moves
  while (Moves) {
    int sq = FirstPiece(Moves);
    if (ply == 0) g_ThinkingMove = sq;
    // Do the move and recurse
    Backup = *B;
    (void)DoMove(B,sq);
    // Game won?
    if (!B->white) score = B->side*(ply-WIN);
    else if (!B->black) score = B->side*(WIN-ply);
    else if (counters == 63) { // Board is full
      if (Count(B->white) > Count(B->black)) score = B->side*(WIN-ply);
      else if (Count(B->white) < Count(B->black)) score = B->side*(ply-WIN);
      else score = 0;
    }
    // Leaf node?
    else if (depth <= 1) score = -Score(B);
    // Recurse
    else score = -Search(B,depth-1,ply+1,-beta,-alpha,counters+1,false);
    // Undo the move
    *B = Backup;
    if (bAbortFlag) break;

    // Have we beaten the best score?
    if (score > alpha) {
      alpha = score;
      iBestMove = sq;
      if (ply==0) { g_BestMove = iBestMove; g_Score = score; }
      if (score >= beta) {
        return beta;
      }
    }

    // Remove this move from the list
    RemoveFirst(Moves);
    // Out of time?
    if (ply<5 && GetElapsedTime() > SearchTime*99) { bAbortFlag = true; break; } 
  }
  return alpha;
}

// Score the board for the current player
int Score(BoardType *B) {
  int score = Pts(B,B->side) * SCORE_SCALE;
  score += (Count(B->white|CornerMask) - Count(B->black|CornerMask)) * B->side * SCORE_CORNER;
  score += (Count(B->white|CornerNextMask) - Count(B->black|CornerNextMask)) * B->side * SCORE_CORNER_NEXT;
  return score;
}

// Set the start of the timer
void SetStartTime(void) {
  (void)ftime(&start);
}

// Get the elapsed time in centiseconds so far
int GetElapsedTime(void) {
  struct timeb end;
  int TimeTaken;

  (void)ftime(&end);
  TimeTaken  = int(end.time - start.time)*100;
  TimeTaken += int((end.millitm - start.millitm)/10);

  return TimeTaken;
}