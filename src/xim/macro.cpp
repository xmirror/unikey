// -*- coding:unix -*-
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

#include <string.h>
#include "macro.h"
#include "ukengine.h"
#include "vnconv.h"
#include "mactab.h"

//---- exported variables from unikey.cpp ----
extern SharedMem *pShMem;
extern UkMacroInfo *pUkMacro;
//--------------------------------------------

static CMacroTable MacroStore;

//--------------------------------------------
int UkSetupMacro()
{
  pUkMacro = new UkMacroInfo;
  return 1;
}

//--------------------------------------------
void UkCleanupMacro()
{
  if (pUkMacro)
    delete pUkMacro;
  pUkMacro = 0;
}

//--------------------------------------------
int UkLoadMacroTable(const char *fileName)
{
  return MacroStore.loadFromFile(fileName);
}

//--------------------------------------------
static int engineToVnConvCharset(int engCharset)
{
  switch (engCharset) {
  case UNICODE_CHARSET:
    return CONV_CHARSET_UNICODE;
  case TCVN3_CHARSET:
    return CONV_CHARSET_TCVN3;
  case VNI_CHARSET:
    return CONV_CHARSET_VNIWIN;
  case VIQR_CHARSET:
    return CONV_CHARSET_VIQR;
  default:
    return CONV_CHARSET_UNICODE;
  }
}

/*
struct UkMacroInfo {
  HookMacroDef macroTable[MAX_MACRO_ITEMS];
  char macroMem[MACRO_MEM_SIZE];
  int macroCount;
};
*/

//--------------------------------------------
void UkUpdateMacroTable(int charsetInUse)
{
  int inLen, maxOutLen;
  int ret, i, k;
  int outCharset;

  if (pUkMacro == 0)
    return;

  outCharset = engineToVnConvCharset(charsetInUse);

  char *p = pUkMacro->macroMem;
  int offset = 0;
  int len;

  for (i=0, k=0; i<MacroStore.m_count && k < MAX_MACRO_ITEMS; i++) {
    len = strlen(MacroStore.m_table[i].key);
    if (offset+len+1 > MACRO_MEM_SIZE)
      break;
    strcpy(p, MacroStore.m_table[i].key);
    pUkMacro->macroTable[k].keyOffset = offset;
    p += len+1;
    offset += (len+1);

    inLen = -1;
    maxOutLen = MACRO_MEM_SIZE - offset;
    ret = VnConvert(CONV_CHARSET_VIQR, outCharset, 
		    (BYTE *)MacroStore.m_table[i].text,
		    (BYTE *)p,
		    &inLen, &maxOutLen);
    if (ret != 0) {
      p -= (len+1);
      offset -= (len+1);
      continue;
    }
    
    pUkMacro->macroTable[k].textOffset = offset;
    p += maxOutLen;
    offset += maxOutLen;
    k++;
  }
  pUkMacro->macroCount = k;
}

