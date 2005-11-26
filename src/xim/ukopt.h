// -*- coding:unix; mode:c++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-
#ifndef __UNIKEY_OPT_H
#define __UNIKEY_OPT_H

#include "unikey.h"

typedef struct _UkXimOpt {
  UnikeyOptions uk;
  long inputMethod;
  long charset;
  long enabled;
  long xvnkbSync;
  long bellNotify;
  long commitMethod;
  long ximFlow; //dynamic or static
  long autoSave;
  long posX;
  long posY;
  char *macroFile;
  char *ximLocales;
  long gtkImAlone;
  char *usrKeyMapFile;
} UkXimOpt;

enum {
  UkSendCommit,
  UkForwardCommit
};

enum {
  UkXimStatic,
  UkXimDynamic
};

int UkParseOptFile(const char *fileName, UkXimOpt *options);
int UkWriteOptFile(const char *fileName, UkXimOpt *options);
char *UkGetDefConfFileName();
void UkSetDefOptions(UkXimOpt *options);
void UkTestDefConfFile();

#endif
