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
  strcat(buf, "/.unikeyrc");
  return buf;
}

//------------------------------------------------------
void UkGuiSetDefOptions(UkGuiOpt *options)
{
  memset(options, 0, sizeof(UkGuiOpt));
  options->posX = -1;
  options->posY = -1;
}
