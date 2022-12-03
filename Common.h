/*------------------- 
 * Project : Othello
 * File    : Common.h
 *-------------------*/

/* This file contains the universal defines.
 */

#ifndef COMMON_H
#define COMMON_H

using namespace std;

// Random defines
#define Random(a)   ((a) == (0) ? (0) : (int)((double)rand() / ((double)RAND_MAX + 1) * (a)))
#define Randomise() (srand((unsigned int)time(NULL)))

// GUI definitions
#define WINDOW_WIDTH   (400)
#define WINDOW_HEIGHT  (400)

// Colours
#define C_BLACK    RGB(0,0,0)
#define C_RED      RGB(255,0,0)
#define C_GREEN    RGB(0,255,0)
#define C_SMALL_COLOUR RGB(0,0,255)  // Colour of the small numbers for the permissions.
#define C_BLUE     RGB(0,0,255)
#define C_WHITE    RGB(255,255,255)
#define C_GREY     RGB(170,170,170)
#define C_HIGHLIGHT RGB(200,200,200)

// GUI parameters
#define BORDERX      (10)
#define BORDERY      (10)
#define FRAME_BORDER (10)

// Sides
#define WHITE ( 1)
#define BLACK (-1)

// Scores
#define SCORE_SCALE       (10)  // Points for one counter
#define SCORE_CORNER      (20)  // Extra bonus points for a corner counter
#define SCORE_CORNER_NEXT (-5)  // Bonus points (penalty) for being next to a corner

// Datatypes
#define longlong __int64
typedef unsigned longlong BITBOARD;
typedef unsigned longlong u64;
typedef unsigned char u8;
typedef unsigned longlong KeyType;

 /* Removes the indicated piece from the board. 
  * The XOR is a little naughty, and fails if the piece doesn't exist! */
#define Remove(a,b)  ((a) = (a^(UNIT<<b)))
 /* Remove the first piece */
#define RemoveFirst(a)   ((a) = ((a) & ((a)-1)))
  /* Miscellaneous Board defines */
#define UNIT   ((BITBOARD)1)
#define EMPTY  ((BITBOARD)0)
#define FullRank     (255)
#define TwoFullRanks (65535)
#define PIF       (3.14159265f)
#define INFINITY (1000000) // Limit for scoring
#define WIN      (10000)   // A won game score
#define DRAW     (0)       // A drawn game score
#define File(x)           (x & 7)
#define Rank(x)           (x >> 3)
#define Not(x)            (((x)==(TRUE)) ? (FALSE) : (TRUE))
#define Clip(x)           (((x) < 0) ? (0) : (x))

// Board type
typedef struct BoardType {
  BITBOARD white, black; // All counters in play
  BITBOARD WR90,WR45,WL45;  // Rotated bitboards
  BITBOARD BR90,BR45,BL45;  // Rotated bitboards
  int side;              // 1=white, -1=black
} BoardType;

// --- Piece position enums

// Enumerated Ranks and Files
#define FileA   (0)
#define FileB   (1)
#define FileC   (2)
#define FileD   (3)
#define FileE   (4)
#define FileF   (5)
#define FileG   (6)
#define FileH   (7)
#define Rank1   (7)
#define Rank2   (6)
#define Rank3   (5)
#define Rank4   (4)
#define Rank5   (3)
#define Rank6   (2)
#define Rank7   (1)
#define Rank8   (0)

// Enumerated board squares
#define a8 (0)
#define b8 (1)
#define c8 (2)
#define d8 (3)
#define e8 (4)
#define f8 (5)
#define g8 (6)
#define h8 (7)
#define a7 (8)
#define b7 (9)
#define c7 (10)
#define d7 (11)
#define e7 (12)
#define f7 (13)
#define g7 (14)
#define h7 (15)
#define a6 (16)
#define b6 (17)
#define c6 (18)
#define d6 (19)
#define e6 (20)
#define f6 (21)
#define g6 (22)
#define h6 (23)
#define a5 (24)
#define b5 (25)
#define c5 (26)
#define d5 (27)
#define e5 (28)
#define f5 (29)
#define g5 (30)
#define h5 (31)
#define a4 (32)
#define b4 (33)
#define c4 (34)
#define d4 (35)
#define e4 (36)
#define f4 (37)
#define g4 (38)
#define h4 (39)
#define a3 (40)
#define b3 (41)
#define c3 (42)
#define d3 (43)
#define e3 (44)
#define f3 (45)
#define g3 (46)
#define h3 (47)
#define a2 (48)
#define b2 (49)
#define c2 (50)
#define d2 (51)
#define e2 (52)
#define f2 (53)
#define g2 (54)
#define h2 (55)
#define a1 (56)
#define b1 (57)
#define c1 (58)
#define d1 (59)
#define e1 (60)
#define f1 (61)
#define g1 (62)
#define h1 (63)



#endif // COMMON_H
