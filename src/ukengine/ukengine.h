// -*- coding:unix -*-
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
#ifndef __UKVIET_ENGINE_H
#define __UKVIET_ENGINE_H 1

#include "unikey.h"

#ifndef _WIN32
typedef unsigned long int DWORD;
typedef unsigned short WORD;
typedef unsigned char BYTE;
typedef short int SHORT;

#define max(x, y) ((x > y)? x : y)

#endif

#ifndef LOWORD
#define LOWORD(l)           ((WORD)(l))
#endif

#ifndef HIWORD
#define HIWORD(l)           ((WORD)(((DWORD)(l) >> 16) & 0xFFFF))
#endif

#ifndef MAKEWORD
#define MAKEWORD(a, b)      ((WORD)(((BYTE)(a)) | ((WORD)((BYTE)(b))) << 8))
#endif

#ifndef HIBYTE
#define HIBYTE(w) ((BYTE)(w>>8))
#endif

#ifndef LOBYTE
#define LOBYTE(w) ((BYTE)(w & 0x00ff))
#endif

struct CodeInfo {
	DWORD DT[256];
	unsigned char BD[12][6];
	unsigned char BK[8];
	unsigned char BW[6];
	unsigned char BT[6];
	WORD ToUniH[256];
	WORD ToUniL[256];
	int multiBytes; // 2 bytes charset (Unicode, VNI, ....)
	int encoding; //UCS-2, UTF-8, Reference
	int singleBackspace;
};

#define CTRL_SHIFT_SW 0
#define ALT_Z_SW 1

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


struct SharedMem {
	//states
	int initialized;
	int vietKey;
	int iconShown;
	int switchKey;

	UnikeyOptions options;

	WORD keyMode;
	int inMethod;
	CodeInfo codeTable;

#if defined(WIN32)
	// system
	HHOOK keyHook,mouseHook;
	HWND hMainDlg;
	UINT iconMsgId;
	HICON hVietIcon,hEnIcon;
	int unicodePlatform;
	DWORD winMajorVersion, winMinorVersion;
#endif
};


#define KEY_BUFSIZE 40
#define KEYS_MAINTAIN 20

class UkVietEngine {
private:
	int keys;
	unsigned char buf[KEY_BUFSIZE];
	int lowerCase[KEY_BUFSIZE];
	int lastWConverted; // used in TELEX, WConverted =1 when last single w was converted to u+
	int lastIsEscape;  // the last key was an escape key
	int tempVietOff;	// Vietnamese mode is temporarily turned off

	DWORD  *DT;
	unsigned char (*BD)[6];
	unsigned char *BK;
	unsigned char *BW;
	unsigned char *BT;
	WORD *ToUniH;
	WORD *ToUniL;

	int keyCategory(unsigned char c);
	void putBreveMark(unsigned char c);
	void doubleChar(unsigned char c);
	void putToneMark(unsigned char c);
	void shortKey(unsigned char c);
	void vniDoubleCharMark(unsigned char c);
	void throwBuf();  // used when buffer is full

	// helper functions
	void postProcess();
	void processBackspace();
	void encodeUnicode(int encoding);
	int checkMacro(unsigned char ch);
	int checkEscapeVIQR(unsigned char ch);
public:
	UkVietEngine();
	int keysPushed,backs;
	unsigned char ansiPush[1024];
	WORD uniPush[512];

	void clearBuf();
	void process(unsigned char ch);
	void setCodeTable(CodeInfo * pInfo);
	void putChar(unsigned char ch, int isLowerCase = 1); // puts char into buffer without processing it
};

#endif
