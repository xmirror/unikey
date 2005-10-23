// -*- coding:unix; mode:c++ -*-
/*------------------------------------------------------------------------------
UniKey - Open-source Vietnamese Keyboard
Copyright (C) 1998-2004 Pham Kim Long
Contact:
  longcz@yahoo.com
  http://unikey.sf.net

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
--------------------------------------------------------------------------------*/
#include <ctype.h>
#include <memory.h>
#include <stdio.h>
#include <iostream>
#include "unikey.h"
#include "ukengine.h"
#include "usrkeymap.h"

using namespace std;

//---- exported variables for use in UkEnginge class ----
UkSharedMem *pShMem = 0;

UkEngine MyKbEngine;

int UnikeyCapsLockOn = 0;
int UnikeyShiftPressed = 0;
//----------------------------------------------------

unsigned char UnikeyBuf[1024];
int UnikeyBackspaces;
int UnikeyBufChars;

//--------------------------------------------
void UnikeySetInputMethod(UkInputMethod im)
{
  if (im == UkTelex || im == UkVni || im == UkViqr) {
    pShMem->input.setIM(im);
    MyKbEngine.reset();
  }
  else if (im == UkUsrIM && pShMem->usrKeyMapLoaded) {
    //cout << "Switched to user mode\n"; //DEBUG
    pShMem->input.setIM(pShMem->usrKeyMap);
    MyKbEngine.reset();
  }

  //cout << "IM changed to: " << im << endl; //DEBUG
}


//--------------------------------------------
void UnikeySetCapsState(int shiftPressed, int CapsLockOn)
{
  //UnikeyCapsAll = (shiftPressed && !CapsLockOn) || (!shiftPressed && CapsLockOn);
  UnikeyCapsLockOn = CapsLockOn;
  UnikeyShiftPressed = shiftPressed;
}

//--------------------------------------------
int UnikeySetOutputCharset(int charset)
{
  VnCharset *pCharset = VnCharsetLibObj.getVnCharset(charset);
  if (pCharset) {
    //cout << "Charset changed to: " << charset << endl; //DEBUG
    pShMem->pCharset = pCharset;
    pShMem->charsetId = charset;
    MyKbEngine.reset();
    return 1;
  }
  return 0;
}

//--------------------------------------------
void UnikeySetOptions(UnikeyOptions *pOpt)
{
  pShMem->options.freeMarking = pOpt->freeMarking;
  pShMem->options.toneNextToVowel = pOpt->toneNextToVowel;
  pShMem->options.modernStyle = pOpt->modernStyle;
  pShMem->options.macroEnabled = pOpt->macroEnabled;
  pShMem->options.useUnicodeClipboard = pOpt->useUnicodeClipboard;
  pShMem->options.alwaysMacro = pOpt->alwaysMacro;
  pShMem->options.strictSpellCheck = pOpt->strictSpellCheck;
}

//--------------------------------------------
void UnikeyGetOptions(UnikeyOptions *pOpt)
{
  *pOpt = pShMem->options;
}

//--------------------------------------------
void CreateDefaultUnikeyOptions(UnikeyOptions *pOpt)
{
  pOpt->freeMarking = 1;
  pOpt->toneNextToVowel = 0;
  pOpt->modernStyle = 0;
  pOpt->macroEnabled = 0;
  pOpt->useUnicodeClipboard = 0;
  pOpt->alwaysMacro = 0;
  pOpt->strictSpellCheck = 1;

}

//--------------------------------------------
void UnikeyCheckKbCase(int *pShiftPressed, int *pCapsLockOn)
{
  *pShiftPressed = UnikeyShiftPressed;
  *pCapsLockOn = UnikeyCapsLockOn;
}

//--------------------------------------------
void UnikeySetup()
{
  pShMem = new UkSharedMem;
  pShMem->vietKey = 1;
  pShMem->usrKeyMapLoaded = 0;
  MyKbEngine.setCtrlInfo(pShMem);
  MyKbEngine.setCheckKbCaseFunc(&UnikeyCheckKbCase);
  UnikeySetInputMethod(UkTelex);
  UnikeySetOutputCharset(CONV_CHARSET_XUTF8);
  pShMem->initialized = 1;
  CreateDefaultUnikeyOptions(&pShMem->options);
}

//--------------------------------------------
void UnikeyCleanup()
{
  delete pShMem;
}

//--------------------------------------------
void UnikeyFilter(unsigned int ch)
{
  UnikeyBufChars = sizeof(UnikeyBuf);
  MyKbEngine.process(ch, UnikeyBackspaces, UnikeyBuf, UnikeyBufChars);
}

//--------------------------------------------
void UnikeyPutChar(unsigned int ch)
{
  MyKbEngine.pass(ch);
  UnikeyBufChars = 0;
  UnikeyBackspaces = 0;
}

//--------------------------------------------
void UnikeyResetBuf()
{
  MyKbEngine.reset();
}

//--------------------------------------------
void UnikeySetSingleMode()
{
  MyKbEngine.setSingleMode();
}

//--------------------------------------------
void UnikeyBackspacePress()
{
  UnikeyBufChars = sizeof(UnikeyBuf);
  MyKbEngine.processBackspace(UnikeyBackspaces, UnikeyBuf, UnikeyBufChars);
  //  printf("Backspaces: %d\n",UnikeyBackspaces);
}

//--------------------------------------------
int UnikeyLoadMacroTable(const char *fileName)
{
  return pShMem->macStore.loadFromFile(fileName);
}

//--------------------------------------------
int UnikeyLoadUserKeyMap(const char *fileName)
{
  if (UkLoadKeyMap(fileName, pShMem->usrKeyMap)) {
    //cout << "User key map loaded!\n"; //DEBUG
    pShMem->usrKeyMapLoaded = 1;
    return 1;
  }
  return 0;
}

//--------------------------------------------
void UnikeyRestoreKeyStrokes()
{
  UnikeyBufChars = sizeof(UnikeyBuf);
  MyKbEngine.restoreKeyStrokes(UnikeyBackspaces, UnikeyBuf, UnikeyBufChars);
}