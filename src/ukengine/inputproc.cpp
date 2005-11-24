// -*- coding:unix -*-
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
#include "stdafx.h"
#include <iostream>
#include "inputproc.h"

using namespace std;

unsigned char WordBreakSyms[] = {
	',', ';', ':', '.', '\"', '\'', '!', '?', ' ',
	'<', '>', '=', '+', '-', '*', '/', '\\',
	'_', '~', '`', '@', '#', '$', '%', '^', '&', '(', ')', '{', '}', '[', ']'};

VnLexiName AZLexiUpper[] = 
  {vnl_A, vnl_B, vnl_C, vnl_D, vnl_E, vnl_F, vnl_G, vnl_H, vnl_I, vnl_J,
   vnl_K, vnl_L, vnl_M, vnl_N, vnl_O, vnl_P, vnl_Q, vnl_R, vnl_S, vnl_T,
   vnl_U, vnl_V, vnl_W, vnl_X, vnl_Y, vnl_Z};

VnLexiName AZLexiLower[] =
  {vnl_a, vnl_b, vnl_c, vnl_d, vnl_e, vnl_f, vnl_g, vnl_h, vnl_i, vnl_j,
   vnl_k, vnl_l, vnl_m, vnl_n, vnl_o, vnl_p, vnl_q, vnl_r, vnl_s, vnl_t,
   vnl_u, vnl_v, vnl_w, vnl_x, vnl_y, vnl_z};

UkCharType UkcMap[256];

//bool UkInputProcessor::m_classInit = false;
bool ClassifierTableInitialized = false;

//-------------------------------------------
void SetupInputClassifierTable()
{
  unsigned int c;
  for (c=0; c<256; c++)
    UkcMap[c] = ukcReset;

  for (c = 'a'; c <= 'z'; c++)
    UkcMap[c] = ukcVn;
  for (c = 'A'; c <= 'Z'; c++)
    UkcMap[c] = ukcVn;

  for (c = '0'; c <= '9'; c++)
    UkcMap[c] = ukcNonVn;

  UkcMap[(unsigned char)'j'] = ukcNonVn;
  UkcMap[(unsigned char)'J'] = ukcNonVn;
  UkcMap[(unsigned char)'f'] = ukcNonVn;
  UkcMap[(unsigned char)'F'] = ukcNonVn;
  UkcMap[(unsigned char)'w'] = ukcNonVn;
  UkcMap[(unsigned char)'W'] = ukcNonVn;

  int count = sizeof(WordBreakSyms)/sizeof(unsigned char);
  for (int i = 0; i < count; i++)
    UkcMap[WordBreakSyms[i]] = ukcWordBreak;
}

//-------------------------------------------
void UkInputProcessor::init()
{
  if (!ClassifierTableInitialized) {
    SetupInputClassifierTable();
    ClassifierTableInitialized = true;
  }
  setIM(UkTelex);
}

//-------------------------------------------
int UkInputProcessor::setIM(UkInputMethod im)
{
  m_im = im;
  switch (im) {
  case UkTelex:
    buildTelex();
    break;
  case UkVni:
    buildVni();
    break;
  case UkViqr:
    buildViqr();
    break;
  default:
    m_im = UkTelex;
    buildTelex();
  }
  return 1;
}

//-------------------------------------------
int UkInputProcessor::setIM(UkKeyEvName map[256])
{
  int i;
  m_im = UkUsrIM;
  for (i=0; i<256; i++)
    m_keyMap[i] = map[i];
  return 1;
}
  

//-------------------------------------------
void UkResetKeyMap(UkKeyEvName keyMap[256])
{
  unsigned int c;
  for (c=0; c<256; c++)
    keyMap[c] = vneNormal;
}

//-------------------------------------------
void UkInputProcessor::buildTelex()
{
  //cout << "Building TELEX mode\n"; //DEBUG
  UkResetKeyMap(m_keyMap);

  m_keyMap[(unsigned char)'z'] = m_keyMap[(unsigned char)'Z'] = vneTone0;
  m_keyMap[(unsigned char)'s'] = m_keyMap[(unsigned char)'S'] = vneTone1;
  m_keyMap[(unsigned char)'f'] = m_keyMap[(unsigned char)'F'] = vneTone2;
  m_keyMap[(unsigned char)'r'] = m_keyMap[(unsigned char)'R'] = vneTone3;
  m_keyMap[(unsigned char)'x'] = m_keyMap[(unsigned char)'X'] = vneTone4;
  m_keyMap[(unsigned char)'j'] = m_keyMap[(unsigned char)'J'] = vneTone5;

  m_keyMap[(unsigned char)'w'] = m_keyMap[(unsigned char)'W'] = vne_telex_w;
  m_keyMap[(unsigned char)'a'] = m_keyMap[(unsigned char)'A'] = vneRoof_a;
  m_keyMap[(unsigned char)'e'] = m_keyMap[(unsigned char)'E'] = vneRoof_e;
  m_keyMap[(unsigned char)'o'] = m_keyMap[(unsigned char)'O'] = vneRoof_o;
  m_keyMap[(unsigned char)'d'] = m_keyMap[(unsigned char)'D'] = vneDd;

  m_keyMap[(unsigned char)'['] = (UkKeyEvName)(vneCount + vnl_uh);
  m_keyMap[(unsigned char)']'] = (UkKeyEvName)(vneCount + vnl_oh);
  m_keyMap[(unsigned char)'{'] = (UkKeyEvName)(vneCount + vnl_Uh);
  m_keyMap[(unsigned char)'}'] = (UkKeyEvName)(vneCount + vnl_Oh);
}

//-------------------------------------------
void UkInputProcessor::buildVni()
{
  UkResetKeyMap(m_keyMap);

  m_keyMap[(unsigned char)'0'] = vneTone0;
  m_keyMap[(unsigned char)'1'] = vneTone1;
  m_keyMap[(unsigned char)'2'] = vneTone2;
  m_keyMap[(unsigned char)'3'] = vneTone3;
  m_keyMap[(unsigned char)'4'] = vneTone4;
  m_keyMap[(unsigned char)'5'] = vneTone5;

  m_keyMap[(unsigned char)'6'] = vneRoofAll;
  m_keyMap[(unsigned char)'7'] = vneHook_uo;
  m_keyMap[(unsigned char)'8'] = vneBowl;
  m_keyMap[(unsigned char)'9'] = vneDd;

}

//-------------------------------------------
void UkInputProcessor::buildViqr()
{
  UkResetKeyMap(m_keyMap);

  m_keyMap[(unsigned char)'0'] = vneTone0;
  m_keyMap[(unsigned char)'\''] = vneTone1;
  m_keyMap[(unsigned char)'`'] = vneTone2;
  m_keyMap[(unsigned char)'?'] = vneTone3;
  m_keyMap[(unsigned char)'~'] = vneTone4;
  m_keyMap[(unsigned char)'.'] = vneTone5;

  m_keyMap[(unsigned char)'^'] = vneRoofAll;
  m_keyMap[(unsigned char)'+'] = vneHook_uo;
  m_keyMap[(unsigned char)'*'] = vneHook_uo;
  m_keyMap[(unsigned char)'('] = vneBowl;
  m_keyMap[(unsigned char)'d'] = vneDd;

}

//-------------------------------------------
void UkInputProcessor::keyCodeToEvent(unsigned int keyCode, UkKeyEvent & ev)
{
  ev.keyCode = keyCode;

  if (keyCode > 255) {
    ev.evType = vneNormal;
    ev.vnSym = IsoToVnLexi(keyCode);
    ev.chType = (ev.vnSym == vnl_nonVnChar)? ukcNonVn : ukcVn;
  }
  else {
    ev.chType = UkcMap[keyCode];
    ev.evType = m_keyMap[keyCode];

    if (ev.evType >= vneTone0 && ev.evType <= vneTone5) {
      ev.tone = ev.evType - vneTone0;
    }

    if (ev.evType >= vneCount) {
      //cout << "Input Processor: shortcut. Key: " << (unsigned char)keyCode 
      //     << " EvType: " << ev.evType << endl; //DEBUG
      ev.chType = ukcVn;
      ev.vnSym = (VnLexiName)(ev.evType - vneCount);
      ev.evType = vneMapChar;
    }
    else {
      ev.vnSym = IsoToVnLexi(keyCode);
    }
  }
}

//-------------------------------------------
UkCharType UkInputProcessor::getCharType(unsigned int keyCode)
{
  if (keyCode > 255)
    return (IsoToVnLexi(keyCode) == vnl_nonVnChar) ? ukcNonVn : ukcVn;
  return UkcMap[keyCode];
}

//-------------------------------------------
//TODO:
// At the moment we considers character outside the ranges a-z, A-Z
// as non-Vietnamese
// We should handle characters outside those ranges
// to test if they are Vietnamese according to ISO8859-1 (western)
//-------------------------------------------
VnLexiName IsoToVnLexi(int keyCode)
{
  if (keyCode >= 'a' && keyCode <= 'z')
    return AZLexiLower[keyCode - 'a'];
  if (keyCode >= 'A' && keyCode <= 'Z')
    return AZLexiUpper[keyCode - 'A'];
  return vnl_nonVnChar;
}
