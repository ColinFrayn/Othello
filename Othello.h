/**************************
 *    Othello.h           *
 *    Colin Frayn         *
 *    May 2008            *
 **************************/

/*
  Contains the specific defines for Othello.cpp
 */

#ifndef OTHELLO_H
#define OTHELLO_H

INT_PTR CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void SetupMainWindow(void);
void SetupViewport(HWND);
HGLRC Graphics_SetupWindow(HWND,HDC);
bool Graphics_SetupFonts(HDC,HGLRC,int);
void DisplayBoard(BoardType *,int,int);
void Graphics_PrintString(char[], float, float);
void SetupLighting(void);
void MouseClick(BoardType *,int,int);
void GameOver(void);

#endif  // OTHELLO_H
