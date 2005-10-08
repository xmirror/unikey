// -*- coding:unix; mode:c++ -*-
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


#include <iostream>
using namespace std;

#include <ctype.h>

#include "usrkeymap.h"

#define OPT_COMMENT_CHAR ';'

struct UkEventLabelPair
{
  char label[32];
  int ev;
};

UkEventLabelPair UkEvLabelList[] = {
  {"Roof-All", vneRoofAll},
  {"Roof-A", vneRoof_a},
  {"Roof-E", vneRoof_e}, 
  {"Roof-O", vneRoof_o},
  {"Hook-Bowl", vneHookAll},
  {"Hook-UO", vneHook_uo},
  {"Hook-U", vneHook_u},
  {"Hook-O", vneHook_o},
  {"Bowl", vneBowl},
  {"D-Mark", vneDd},
  {"Tone0", vneTone0},
  {"Tone1", vneTone1},
  {"Tone2", vneTone2},
  {"Tone3", vneTone3},
  {"Tone4", vneTone4},
  {"Tone5", vneTone5},
  {"Telex-W", vne_telex_w},
  {"A^", vneCount + vnl_Ar},
  {"a^", vneCount + vnl_ar},
  {"A(", vneCount + vnl_Ab},
  {"a(", vneCount + vnl_ab},
  {"E^", vneCount + vnl_Er},
  {"e^", vneCount + vnl_er},
  {"O^", vneCount + vnl_Or},
  {"o^", vneCount + vnl_or},
  {"O+", vneCount + vnl_Oh},
  {"o+", vneCount + vnl_oh},
  {"U+", vneCount + vnl_Uh},
  {"u+", vneCount + vnl_uh},
  {"DD", vneCount + vnl_DD},
  {"dd", vneCount + vnl_dd}
};

const int UkEvLabelCount = sizeof(UkEvLabelList)/sizeof(UkEventLabelPair);

//--------------------------------------------------
static int parseNameValue(char *line, char **name, char **value)
{
  char *p, *mark;
  char ch;

  if (line == 0)
    return 0;

  // get rid of comment
  p = strchr(line, OPT_COMMENT_CHAR);
  if (p)
    *p = 0;

  //get option name
  for (p=line; *p == ' '; p++);
  if (*p == 0)
    return 0;

  *name = p;
  mark = 0; //mark the last non-space character
  while ((ch=*p) != '=' && ch!=0) {
    if (ch != ' ')
      mark = p;
    p++;
  }

  if (ch == 0 && mark == 0)
    return 0;
  *(mark+1) = 0; //terminate name with a null character

  //get option value
  p++;
  while (*p == ' ') p++;
  if (*p == 0)
    return 0;

  *value = p;
  mark = p;
  while (*p) { //strip trailing spaces
    if (*p != ' ')
      mark = p;
    p++;
  }
  *++mark = 0;
  return 1;
}

//-----------------------------------------------------
int UkLoadKeyMap(const char *fileName, UkKeyEvName keyMap[256])
{
  FILE *f;
  char *buf;
  char *name, *value;
  int bufSize, len, i, lineCount;
  unsigned char c;

  f = fopen(fileName, "r");
  if (f == 0) {
    cerr << "Failed to open file: " << fileName << endl;
    return 0;
  }

  UkResetKeyMap(keyMap);
  bufSize = 256;
  buf = new char[bufSize];

  lineCount = 0;
  while (!feof(f)) {
    if (fgets((char *)buf, bufSize, f) == 0)
      break;
    lineCount++;
    len = strlen(buf);
    if (len == 0)
      break;

    if (buf[len-1] == '\n')
      buf[len-1] = 0;
    if (parseNameValue(buf, (char **)&name, (char **)&value)) {
      if (strlen(name) == 1) {
	for (i=0; i < UkEvLabelCount; i++) {
	  if (strcmp(UkEvLabelList[i].label, value) == 0) {
	    c = (unsigned char)name[0];
	    //cout << "key: " << c << " value: " << UkEvLabelList[i].ev << endl; //DEBUG
	    keyMap[c] = (UkKeyEvName)UkEvLabelList[i].ev;
	    if (keyMap[c] < vneCount) {
	      if (islower(c))
		keyMap[toupper(c)] = keyMap[c];
	      else if (isupper(c))
		keyMap[tolower(c)] = keyMap[c];
	    }
	    break;
	  }
	}
	if (i == UkEvLabelCount) {
	  cerr << "Error in user key layout, line " << lineCount << ": command not found" << endl;
	}
      }
      else {
	cerr << "Error in user key layout, line " << lineCount 
	     << ": key name is not a single character" << endl;	
      }
    }
  }
  delete buf;
  fclose(f);
  return 1;
}
