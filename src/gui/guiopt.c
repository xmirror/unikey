// -*- coding:unix; mode:c++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-
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

#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include "../xim/optparse.h"
#include "guiopt.h"

static char PosXCmt[] =
"# XPos: X Position of unikey window\n"
"# set a minus value to let unikey use the default position\n";

static char PosYCmt[] =
"# YPos: Y Position of unikey window\n"
"# set a minus value to let unikey use the default position\n";

OptItem UkGuiOptList[] = {
  {"PosX", PosXCmt, offsetof(UkGuiOpt, posX), LongOpt, 0},
  {"PosY", PosYCmt, offsetof(UkGuiOpt, posY), LongOpt, 0}
};

//----------------------------------------------------
int UkGuiParseOptFile(const char *fileName,  UkGuiOpt *options)
{
  return ParseOptFile(fileName, options, UkGuiOptList, sizeof(UkGuiOptList)/sizeof(OptItem));
}

//------------------------------------------------------
char *UkGuiGetDefConfFileName()
{
  static char buf[128];
  strcpy(buf, getenv("HOME"));
  //  strcat(buf, "/.unikeyrc");
  strcat(buf, "/.unikey/options");
  return buf;
}

//------------------------------------------------------
void UkGuiSetDefOptions(UkGuiOpt *options)
{
  memset(options, 0, sizeof(UkGuiOpt));
  options->posX = -1;
  options->posY = -1;
}
