// -*- coding:unix -*-
/* GTK Unikey Vietnamese Input Method
 * Copyright (C) 2004 Pham Kim Long
 * Contact:
 *   longcz@yahoo.com
 *   UniKey project: http://unikey.sf.net
 * Contributor(s): Nguyen Thai Ngoc Duy (pclouds)
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

Display *display;
Window RootWindow;

typedef struct {
  int from;
  int to;
} SyncMap;

SyncMap UkToSyncCharsetList[] ={
  {UNICODE_CHARSET, VKC_UTF8},
  {VIQR_CHARSET, VKC_VIQR},
  {TCVN3_CHARSET, VKC_TCVN},
  {VNI_CHARSET, VKC_VNI}
};

SyncMap SyncToUkCharsetList[] = {
  {VKC_UTF8, UNICODE_CHARSET},
  {VKC_VIQR, VIQR_CHARSET},
  {VKC_TCVN, TCVN3_CHARSET},
  {VKC_VNI, VNI_CHARSET}};

SyncMap UkToSyncMethodList[] ={
  {TELEX_INPUT, VKM_TELEX},
  {VNI_INPUT, VKM_VNI},
  {VIQR_INPUT, VKM_VIQR},
  {VIQR_STAR_INPUT, VKM_VIQR}
};

SyncMap SyncToUkMethodList[] ={
  {VKM_OFF, TELEX_INPUT},
  {VKM_TELEX, TELEX_INPUT},
  {VKM_VNI, VNI_INPUT},
  {VKM_VIQR, VIQR_INPUT}};


void UkInitSync(Display *dpy, Window root)
{
  display = dpy;
  RootWindow = root;
}

//--------------------------------------------
static int SyncTranslate(int from, SyncMap *list, int count, int fallback)
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
		       UNICODE_CHARSET);
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
		       TELEX_INPUT);
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
    UkSetPropValue(atom, v);
  }
  return v;
}
