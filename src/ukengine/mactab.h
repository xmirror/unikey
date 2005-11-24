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

#ifndef __MACRO_TABLE_H
#define __MACRO_TABLE_H

#include "keycons.h"
#include "charset.h"

/*
struct MacroDef
{
  StdVnChar *key;
  StdVnChar *text;
};
*/

struct MacroDef
{
  int keyOffset;
  int textOffset;
};

#if !defined(WIN32)
typedef char TCHAR;
#endif

class CMacroTable
{
public:
  void init();
  int loadFromFile(const TCHAR *fname);
  int writeToFile(const TCHAR *fname);

  const StdVnChar *lookup(StdVnChar *key);

protected:
  MacroDef m_table[MAX_MACRO_ITEMS];
  char m_macroMem[MACRO_MEM_SIZE];

  int m_count;
  int m_memSize, m_occupied;

  void resetContent();
  int addItem(const char *item);
  int addItem(const char *key, const char *text);
};

#endif
