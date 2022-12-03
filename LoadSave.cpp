// Extras.cpp : Additional features (File I/O, printing etc)

#include "stdafx.h"
#include "common.h"
#include "LoadSave.h"
#include "Bit.h"
#include "Moves.h"

// Global Variables:
extern HWND hWndMain;
extern HMENU g_hMenu;
extern HINSTANCE g_hInstance;
extern int iHistoryPos, iHistoryMax, History[64];
extern BITBOARD Undo[64];
extern BoardType Board;

// Load in a SuDoku game setup
bool LoadGame(void) {
  int sq;
  char strFile[256],strErr[128];
  FILE *fp;

  // Get a suitable target file to load
  if (!GetFileName(strFile,true)) return false;

  // Try to open the specified file
  if ((fp = fopen(strFile,"r")) == NULL) { 
    sprintf(strErr,"Unable to open file %s for loading!",strFile);
    MessageBox(hWndMain,strErr,"File Error!",MB_OK);
    return false;
  }

  // Load the game
  iHistoryPos = 0;
  ResetBoard(&Board);
  while (!feof(fp)) {
    fscanf(fp,"%d\n",&sq);
    Undo[iHistoryPos] = DoMove(&Board,sq);
    History[iHistoryPos++] = sq;
  }
  iHistoryMax = iHistoryPos;

  // Close file handle
  fclose(fp);
  return true;
}

// Save a game
void SaveGame(void) {
  char strFile[256], strErr[128];
  FILE *fp;

  // Get a suitable target file to load
  if (!GetFileName(strFile,false)) return;

  // Try to open the specified file
  if ((fp = fopen(strFile,"w")) == NULL) {
    sprintf(strErr,"Unable to open file %s for saving!",strFile);
    MessageBox(hWndMain,strErr,"File Error!",MB_OK);
    return;
  }

  // Save the game
  for (int n=0; n<iHistoryPos; n++) {
    fprintf(fp,"%d\n",History[n]);
  }

  // Close file handle
  fclose(fp);
}


// Select a data file for loading or saving
bool GetFileName(char *szFile, bool iOpen) {
  OPENFILENAME ofn;
  char szTemp[256]="";
  int  ErrorCode;

  strcpy(szFile,"");
  ZeroMemory(&ofn, sizeof(OPENFILENAME));
  ofn.lStructSize = sizeof(OPENFILENAME);
  ofn.hwndOwner = hWndMain;
  ofn.lpstrFile = szTemp;
  ofn.nMaxFile = sizeof(szTemp);
  ofn.lpstrFilter = "All\0*.*\0SuDoku Data Files\0*.dat\0";
  ofn.lpstrCustomFilter = NULL;
  ofn.nFilterIndex = 1;
  ofn.lpstrFileTitle = NULL;
  ofn.nMaxFileTitle = 0;
  ofn.lpstrInitialDir = NULL;
  if (iOpen) ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
  else ofn.Flags = OFN_PATHMUSTEXIST;
  
  if (iOpen) {
    if (GetOpenFileName(&ofn) == FALSE) {
      ErrorCode = CommDlgExtendedError();
      return false;
    }
  }
  else {
    if (GetSaveFileName(&ofn) == FALSE) {
      ErrorCode = CommDlgExtendedError();
      return false;
    }
  }
  strcpy(szFile, szTemp);
  return true;
}
