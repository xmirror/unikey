// -*-coding:unix; mode:c++-*-
/* Unikey Vietnamese Input Method
 * Copyright (C) 2000-2005 Pham Kim Long
 * Contact:
 *   unikey@gmail.com
 *   UniKey project: http://unikey.org
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef __UKENGINE_H
#define __UKENGINE_H

#include "unikey.h"
#include "charset.h"
#include "vnlexi.h"
#include "inputproc.h"
#include "mactab.h"

struct UkSharedMem {
  //states
  int initialized;
  int vietKey;
  int iconShown;
  int switchKey;


  UnikeyOptions options;
  UkInputProcessor input;
  int usrKeyMapLoaded;
  UkKeyEvName usrKeyMap[256];
  int charsetId;
  VnCharset *pCharset;
  int xutf8;

#if defined(WIN32)
  // system
  HHOOK keyHook,mouseHook;
  HWND hMainDlg;
  UINT iconMsgId;
  HICON hVietIcon,hEnIcon;
  int unicodePlatform;
  DWORD winMajorVersion, winMinorVersion;
#endif

  CMacroTable macStore;
};

#define CTRL_SHIFT_SW 0
#define ALT_Z_SW 1

/*
struct HookMacroDef
{
	int keyOffset;
	int textOffset;
};

struct UkMacroInfo {
  HookMacroDef macroTable[MAX_MACRO_ITEMS];
  char macroMem[MACRO_MEM_SIZE];
  int macroCount;
};
*/


#define MAX_UK_ENGINE 128

enum VnWordForm {vnw_nonVn, vnw_empty, vnw_c, vnw_v, vnw_cv, vnw_vc, vnw_cvc};

class UkEngine
{
public:
  UkEngine();
  void setCtrlInfo(UkSharedMem *p)
  {
    m_pCtrl = p;
  }

  int process(unsigned int keyCode, int & backs, unsigned char *outBuf, int & outSize);
  void pass(int keyCode); //just pass through without filtering
  void setSingleMode();

  int processBackspace(int & backs, unsigned char *outBuf, int & outSize);
  void reset();
  int restoreKeyStrokes(int & backs, unsigned char *outBuf, int & outSize);

  //following methods must be public just to enable the use of pointers to them
  //they should not be called from outside.
  int processTone(UkKeyEvent & ev);
  int processRoof(UkKeyEvent & ev);
  int processHook(UkKeyEvent & ev);
  int processAppend(UkKeyEvent & ev);
  int appendVowel(UkKeyEvent & ev);
  int appendConsonnant(UkKeyEvent & ev);
  int processDd(UkKeyEvent & ev);
  int processMapChar(UkKeyEvent & ev);
  int processTelexW(UkKeyEvent & ev);

protected:
  static bool m_classInit;
  UkSharedMem *m_pCtrl;

  int m_changePos;
  int m_backs;
  int m_bufSize;
  int m_current;
  int m_singleMode;

  int m_keyBufSize;
  unsigned int m_keyStrokes[MAX_UK_ENGINE];
  int m_keyCurrent;

  //varables valid in one session
  unsigned char *m_pOutBuf;
  int *m_pOutSize;
  bool m_outputWritten;
  
  struct WordInfo {
    //info for word ending at this position
    VnWordForm form;
    int c1Offset, vOffset, c2Offset;

    union {
      VowelSeq vseq;
      ConSeq cseq;
    };

    //info for current symbol
    int caps, tone;
    //canonical symbol, after caps, tone are removed
    //for non-Vn, vnSym == -1
    VnLexiName vnSym;
    int keyCode;
  };

  WordInfo m_buffer[MAX_UK_ENGINE];

  int processHookWithUO(UkKeyEvent & ev);
  int macroMatch(UkKeyEvent & ev);
  void markChange(int pos);
  void prepareBuffer(); //make sure we have a least 10 entries available
  int writeOutput(unsigned char *outBuf, int & outSize);
  int getSeqLength(int first, int last);
  int getTonePosition(VowelSeq vs, bool terminated);
  void resetKeyBuf();
  int checkEscapeVIQR(UkKeyEvent & ev);

};


#endif
