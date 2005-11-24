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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include "mactab.h"
#include "vnconv.h"

using namespace std;

//---------------------------------------------------------------
void CMacroTable::init()
{
  m_memSize = MACRO_MEM_SIZE;
  m_count = 0;
  m_occupied = 0;
}

//---------------------------------------------------------------
char *MacCompareStartMem;

int macCompare(const void *p1, const void *p2)
{
  StdVnChar *s1 = (StdVnChar *) ((char *)MacCompareStartMem + ((MacroDef *)p1)->keyOffset);
  StdVnChar *s2 = (StdVnChar *) ((char *)MacCompareStartMem + ((MacroDef *)p2)->keyOffset);

  int i;
  for (i=0; s1[i] != 0 && s2[i] != 0; i++) {
    if (s1[i] > s2[i])
      return 1;
    if (s1[i] < s2[i])
      return -1;
  }
  if (s1[i] == 0)
    return (s2[i] == 0)? 0 : -1;
  return 1;
}

//---------------------------------------------------------------
int macKeyCompare(const void *key, const void *ele)
{
  StdVnChar *s1 = (StdVnChar *)key;
  StdVnChar *s2 = (StdVnChar *) ((char *)MacCompareStartMem + ((MacroDef *)ele)->keyOffset);

  int i;
  for (i=0; s1[i] != 0 && s2[i] != 0; i++) {
    if (s1[i] > s2[i])
      return 1;
    if (s1[i] < s2[i])
      return -1;
  }
  if (s1[i] == 0)
    return (s2[i] == 0)? 0 : -1;
  return 1;
}

//---------------------------------------------------------------
const StdVnChar *CMacroTable::lookup(StdVnChar *key)
{
  MacCompareStartMem = m_macroMem;
  MacroDef *p = (MacroDef *)bsearch(key, m_table, m_count, sizeof(MacroDef), macKeyCompare);
  if (p)
    return (StdVnChar *)(m_macroMem + p->textOffset);
  return 0;
}

//---------------------------------------------------------------
int CMacroTable::loadFromFile(const TCHAR *fname)
{
  FILE *f;
#if defined(WIN32)
  f = _tfopen(fname, _TEXT("rt"));
#else
  f = fopen(fname, "r");
#endif

  if (f == NULL) 
    return 0;
  char line[MAX_MACRO_LINE];
  size_t len;

  resetContent();
  while (fgets(line, sizeof(line), f)) {
    len = strlen(line);
    if (len > 0 && line[len-1] == '\n')
      line[len-1] = 0;
    if (len > 1 && line[len-2] == '\r')
      line[len-2] = 0;
    addItem(line);
  }
  fclose(f);
  MacCompareStartMem = m_macroMem;
  qsort(m_table, m_count, sizeof(MacroDef), macCompare);
  //writeToFile("uktest");
  return 1;
}

//---------------------------------------------------------------
int strStdVnLen(StdVnChar *s)
{
  int len = 0;
  while (s[len] != 0) len++;
  return len;
}

//---------------------------------------------------------------
int CMacroTable::writeToFile(const TCHAR *fname)
{
  int ret;
  int inLen, maxOutLen;
  FILE *f;
#if defined(WIN32)
  f = _tfopen(fname, _TEXT("wt"));
#else
  f = fopen(fname, "w");
#endif

  if (f == NULL)
    return 0;

  char line[MAX_MACRO_LINE*3]; //1 VnChar may need 3 VIQR chars
  char key[MAX_MACRO_KEY_LEN*3];
  char text[MAX_MACRO_TEXT_LEN*3];

  UKBYTE *p;
  for (int i=0; i < m_count; i++) {
    p = (UKBYTE *)m_macroMem + m_table[i].keyOffset;
    inLen = (strStdVnLen((StdVnChar *)p)+1) * sizeof(StdVnChar);
    maxOutLen = sizeof(key);
    ret = VnConvert(CONV_CHARSET_VNSTANDARD, CONV_CHARSET_VIQR,
		    (UKBYTE *) p, (UKBYTE *)key,
		    &inLen, &maxOutLen);
    if (ret != 0)
      continue;

    p = (UKBYTE *)m_macroMem + m_table[i].textOffset;
    inLen = (strStdVnLen((StdVnChar *)p)+1) * sizeof(StdVnChar);
    maxOutLen = sizeof(text);
    ret = VnConvert(CONV_CHARSET_VNSTANDARD, CONV_CHARSET_VIQR,
		    p, (UKBYTE *)text,
		    &inLen, &maxOutLen);
    if (ret != 0)
      continue;
    if (i < m_count-1)
      sprintf(line, "%s:%s\n", key, text);
    else
      sprintf(line, "%s:%s", key, text);
    fputs(line, f);
  }

  fclose(f);
  return 1;
}

//---------------------------------------------------------------
int CMacroTable::addItem(const char *key, const char *text)
{
  int ret;
  int inLen, maxOutLen;
  int offset = m_occupied;
  char *p = m_macroMem + offset;

  if (m_count >= MAX_MACRO_ITEMS)
    return -1;
  
  m_table[m_count].keyOffset = offset;

  //convert macro key form VIQR to VN standard
  inLen = -1; //input is null-terminated
  maxOutLen = MAX_MACRO_KEY_LEN * sizeof(StdVnChar);
  if (maxOutLen + offset > m_memSize)
    maxOutLen = m_memSize - offset;
  ret = VnConvert(CONV_CHARSET_VIQR, CONV_CHARSET_VNSTANDARD, 
		  (UKBYTE *)key, (UKBYTE *)p,
		  &inLen, &maxOutLen);
  if (ret != 0)
    return -1;

  offset += maxOutLen;
  p += maxOutLen;

  //convert macro text from VIQR to VN standard
  m_table[m_count].textOffset = offset;
  inLen = -1; //input is null-terminated
  maxOutLen = MAX_MACRO_TEXT_LEN * sizeof(StdVnChar);
  if (maxOutLen + offset > m_memSize)
    maxOutLen = m_memSize - offset;
  ret = VnConvert(CONV_CHARSET_VIQR, CONV_CHARSET_VNSTANDARD, 
		  (UKBYTE *)text, (UKBYTE *)p,
		  &inLen, &maxOutLen);
  if (ret != 0)
    return -1;

  m_occupied = offset + maxOutLen;
  m_count++;
  return (m_count-1);
}

//---------------------------------------------------------------
// add a new macro into the sorted macro table
// item format: key:text (key and text are separated by a colon)
//---------------------------------------------------------------
int CMacroTable::addItem(const char *item)
{
  char key[MAX_MACRO_KEY_LEN];
  
  // Parse the input item
  char * pos = strchr(item, ':');
  if (pos == NULL)
    return -1;
  int keyLen = (int)(pos - item);
  if (keyLen > MAX_MACRO_KEY_LEN-1)
    keyLen = MAX_MACRO_KEY_LEN-1;
  strncpy(key, item, keyLen);
  key[keyLen] = '\0';
  //cout << "Adding key: " << key << endl; //DEBUG
  return addItem(key, ++pos);
}

//---------------------------------------------------------------
void CMacroTable::resetContent()
{
  m_occupied = 0;
  m_count = 0;
}
