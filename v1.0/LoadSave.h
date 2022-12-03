/**************************
 *    LoadSave.h          *
 *    Colin Frayn         *
 *    May 2008            *
 **************************/

/*
  Contains the specific defines for LoadSave.cpp
 */

#ifndef LOADSAVE_H
#define LOADSAVE_H

bool LoadGame(void);
void SaveGame(void);
bool GetFileName(char *,bool);

#endif  // LOADSAVE_H
