/**********************************
 *    comp.h                      *
 *    Colin Frayn                 *
 *    June 2008                   *
 **********************************/

/*
  Contains the specific defines for comp.cpp
 */

#ifndef COMP_H
#define COMP_H

void Comp(void *);
int  Search(BoardType *,int,int,int,int,int,bool);
int  Score(BoardType *);
void SetStartTime(void);
int  GetElapsedTime(void);

#endif // COMP_H
