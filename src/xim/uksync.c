// -*- coding:unix; mode:c++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-
/* Unikey Vietnamese Input Method
 * Copyright (C) 2004 Pham Kim Long
 * Contact:
 *   longcz@yahoo.com
 *   UniKey project: http://unikey.sf.net
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

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include "../gui/xvnkb.h"
#include "uksync.h"
#include "keycons.h"
#include "vnconv.h"

extern Display *display;
extern Window RootWindow;

typedef struct {
  int from;
  int to;
} SyncMap;

SyncMap UkToSyncCharsetList[] ={
  {CONV_CHARSET_XUTF8, VKC_UTF8},
  {CONV_CHARSET_VIQR, VKC_VIQR},
  {CONV_CHARSET_TCVN3, VKC_TCVN},
  {CONV_CHARSET_VNIWIN, VKC_VNI},
  {CONV_CHARSET_BKHCM2, VKC_BKHCM2}
};

SyncMap SyncToUkCharsetList[] = {
  {VKC_UTF8, CONV_CHARSET_XUTF8},
  {VKC_VIQR, CONV_CHARSET_VIQR},
  {VKC_TCVN, CONV_CHARSET_TCVN3},
  {VKC_VNI, CONV_CHARSET_VNIWIN},
  {VKC_BKHCM2, CONV_CHARSET_BKHCM2}
};

SyncMap UkToSyncMethodList[] ={
  {UkTelex, VKM_TELEX},
  {UkVni, VKM_VNI},
  {UkViqr, VKM_VIQR},
  {UkUsrIM, VKM_USER}
};

SyncMap SyncToUkMethodList[] ={
  {VKM_OFF, UkTelex},
  {VKM_TELEX, UkTelex},
  {VKM_VNI, UkVni},
  {VKM_VIQR, UkViqr},
  {VKM_USER, UkUsrIM}
};


//--------------------------------------------
int SyncTranslate(int from, SyncMap *list, int count, int fallback)
{
  int i;
  for (i = 0; i<count && list[i].from != from; i++);
  if (i==count)
    return fallback;
  return list[i].to;
}


//--------------------------------------------
int UnikeyToSyncCharset(int uk)
{
  return SyncTranslate(uk, 
		       UkToSyncCharsetList, 
		       sizeof(UkToSyncCharsetList)/sizeof(SyncMap),
		       VKC_UTF8);
}


//--------------------------------------------
int SyncToUnikeyCharset(int sync)
{
  return SyncTranslate(sync,
		       SyncToUkCharsetList,
		       sizeof(SyncToUkCharsetList)/sizeof(SyncMap),
		       CONV_CHARSET_XUTF8);
}

//--------------------------------------------
int UnikeyToSyncMethod(int uk)
{
  return SyncTranslate(uk, 
		       UkToSyncMethodList, 
		       sizeof(UkToSyncMethodList)/sizeof(SyncMap),
		       VKM_TELEX);
}

//--------------------------------------------
int SyncToUnikeyMethod(int sync)
{
  return SyncTranslate(sync,
		       SyncToUkMethodList,
		       sizeof(SyncToUkMethodList)/sizeof(SyncMap),
		       UkTelex);
}


//-------------------------------------------------
// Pclouds' contributed code
// modified from VKGetValue (Dao Hai Lam's xvnkb)
//-------------------------------------------------
long atom_get_value(Atom key)
{
  Atom at;
  int af;
  long *s;
  unsigned long ni, br;

  if (XGetWindowProperty(
	display,	RootWindow, key,
	0, //long offset
	1, //long length
	False, //no delete
	XA_CARDINAL, //request type
	&at, //actual type
	&af, //actual format
	&ni, //items returned
	&br, //bytes remaining
	(unsigned char **)&s) == Success) {
    if (s != NULL) {
      long v = *s;
      XFree(s);
      return v;
    }
  }
  return -1;
}


//-------------------------------------------------
void UkSetPropValue(Atom atom, long value)
{
  XChangeProperty(display, RootWindow, atom, XA_CARDINAL, 32,
		  PropModeReplace, 
		  (unsigned char *)&value, 1);
}

//-------------------------------------------------
long UkGetPropValue(Atom atom, long defValue)
{
  long v = atom_get_value(atom);
  if (v == -1) {
    v = defValue;
    //    UkSetPropValue(atom, v);
  }
  return v;
}
