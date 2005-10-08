// -*- coding:unix; mode:c++-*-
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
#ifndef __UK_INPUT_PROCESSOR_H
#define __UK_INPUT_PROCESSOR_H

#include "keycons.h"
#include "vnlexi.h"

enum UkKeyEvName {
  vneRoofAll, vneRoof_a, vneRoof_e, vneRoof_o, 
  vneHookAll, vneHook_uo, vneHook_u, vneHook_o, vneBowl, 
  vneDd, 
  vneTone0, vneTone1, vneTone2, vneTone3, vneTone4, vneTone5,
  vne_telex_w, //special for telex
  vneMapChar, //e.g. [ -> u+ , ] -> o+
  vneNormal, //does not belong to any of the above categories
  vneCount //just to count how many event types there are
};

enum UkCharType {
  ukcVn,
  ukcWordBreak, 
  ukcNonVn, 
  ukcReset
};

struct UkKeyEvent {
  UkKeyEvName evType;
  UkCharType chType;
  VnLexiName vnSym; //meaningful only when chType==ukcVn
  unsigned int keyCode;
  int tone; //meaningful only when this is a vowel
};

///////////////////////////////////////////
class UkInputProcessor {

public:
  UkInputProcessor();
  UkInputProcessor(int map[256]);

  UkInputMethod getIM()
  {
    return m_im;
  }

  void keyCodeToEvent(unsigned int keyCode, UkKeyEvent & ev);
  int setIM(UkInputMethod im);
  int setIM(UkKeyEvName map[256]);
  UkCharType getCharType(unsigned int keyCode);

protected:
  static bool m_classInit;

  UkInputMethod m_im;
  UkKeyEvName m_keyMap[256];

  void buildTelex();
  void buildVni();
  void buildViqr();
  //  void resetKeyMap();

};

void UkResetKeyMap(UkKeyEvName keyMap[256]);
VnLexiName IsoToVnLexi(int keyCode);

#endif
