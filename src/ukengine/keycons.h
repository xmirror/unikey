// -*- coding:unix -*-
/*------------------------------------------------------------------------------
UniKey - Open-source Vietnamese Keyboard
Copyright (C) 1998-2004 Pham Kim Long
Contact:
  unikey@gmail.com
  http://unikey.org

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
#ifndef __KEY_CONS_H
#define __KEY_CONS_H

// macro table constants
#define MAX_MACRO_KEY_LEN 16
//#define MAX_MACRO_TEXT_LEN 256
#define MAX_MACRO_TEXT_LEN 1024
#define MAX_MACRO_ITEMS 1024
#define MAX_MACRO_LINE (MAX_MACRO_TEXT_LEN + MAX_MACRO_KEY_LEN)

#define MACRO_MEM_SIZE (1024*128) //128 KB

#define CP_US_ANSI 1252

typedef enum {UkTelex, UkVni, UkViqr, UkUsrIM} UkInputMethod;
  typedef struct _UnikeyOptions UnikeyOptions;

  struct _UnikeyOptions
  {
    int freeMarking;
    int toneNextToVowel;
    int modernStyle;
    int macroEnabled;
    int useUnicodeClipboard;
    int alwaysMacro;
    int strictSpellCheck;
    int useIME; //for Win32 only
  };

#endif
