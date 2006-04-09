// -*- coding:unix; mode:c++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include "vnconv.h"
#include "optparse.h"
#include "ukopt.h"

static char ConfigHeaderCmt[] = 
"# Unikey XIM config file\n\n";

static char InitStateCmt[] = 
"# Initial state: On|Off\n";

static char AutoSaveCmt[] = 
"# AutoSave: Yes|No\n"
"# If AutoSave=Yes, unikey will save options on exit\n"
"# Be careful when editing this file while unikey is running with AutoSave enabled\n"
"#   Upon exit unikey will overwrite your settings. To avoid that, tell unikey\n"
"#   to reload config file when you finish editing. See unikey manual\n"
"# AutoSave is useful if you want unikey to remember its window position\n";

static char InputCmt[] = 
"# Input: TELEX|VNI|VIQR|USER\n"
"# Input method. If you specify input=USER, you must also specify UsrKeyMapFile\n";

static char CharsetCmt[] = 
"# Output charsets: UNICODE|TCVN|VNI|VIQR|BK2\n";

static char FreeStyleCmt[] = 
"# FreeStyle: Yes|No\n"
"#  \"Yes\" means you can type hook, breve marks anywhere after the base character\n"
"#  not necessarily right after the base.\n"
"#  VNI users should set to Yes. TELEX users: your choice\n"
"# Default: Yes\n";

/*
static char ToneManualCmt[] =
"# ToneManual: Yes|No\n"
"# You should set this to \"No\" to let Unikey determine position for tone marks.\n";
*/

static char ModernStyleCmt[] = 
"# ModernStyle: Yes|No\n"
"# \"Yes\" means \"hoa', khoe?\" style, \"No\" means \"ho'a, kho?e\" style\n";

static char GtkImAloneCmt[] = 
"# GtkImAlone: Yes|No. Default: No\n"
"# Set this to Yes if you want GTK unikey module enabled\n"
"# even when unikey GUI is not running\n";

static char MacroFileCmt[] =
"# Macrofile: path to macro file. Default: empty, macro is disabled\n"
"# To enable macro, specify the path to your macro file.\n"
"# For example: MacroFile = ~/ukmacro\n";

static char BellCmt[] = 
"# Bell: Yes|No\n"
"# Turn on/off bell notification when key shortcuts are pressed\n";

static char XvnkbSyncCmt[] = 
"# XvnkbSync: Yes|No\n"
"# Set to \"Yes\" to synchronize unikey-gtk with xvnkb GUI\n"
"# This applies only to unikey-gtk module, not XIM server (ukxim)\n"
"# To synchronize ukxim with xvnkb GUI, see doc/manual\n";

static char CommitMethodCmt[] = 
"# CommitMethod: Send|Forward|Mixed. Default: Send\n"
"#   Use XSendEvent or XIM forward key event to commit string\n"
"# Both methods will work with most applications.\n"
"# Some application with high-level security may\n"
"#   block XSendEvent, so CommitMethod must be set to \"Forward\"\n"
"# Some versions of xterm are known to work only if CommitMethod=Send\n"
"#   see doc/manual for information on xterm\n"
"# I recommend \"Send\" first. If it does not work, try \"Forward\"\n";

static char XimFlowCmt[] = 
"# XimFlow = Static|Dynamic. Default: Static\n"
"# You should always use Static. If something does not work,\n"
"#   then try Dynamic. Rxvt-unicode is known to work only with Dynamic mode.\n"
"# Before changing this option, make sure unikey is not loaded in memory.\n"
"# See manual, section unload/restart unikey.\n";

static char XimLocalesCmt[] = 
"# XimLocales = List of locales separated by commas.\n"
"# Default: C,en_US,vi_VN,fr_FR,fr_BE,fr_CA,de_DE,ja_JP,cs_CZ,ru_RU\n"
"# Theses are locales that XIM Server must advertise to applications\n" 
"# If your application runs in certain locale, make sure that locale is listed here.\n"
"# Before changing this option, make sure unikey is not loaded in memory.\n"
"# See manual, section unload/restart unikey.\n";

static char PosXCmt[] =
"# PosX: X Position of unikey window\n"
"# set a minus value to let unikey use the default position\n";

static char PosYCmt[] =
"# PosY: Y Position of unikey window\n"
"# set a minus value to let unikey use the default position\n";

static char UsrKeyMapFileCmt[] =
"# UsrKeyMapFile: path to user-defined input method file. Default: empty.\n"
"# To enable user-define input method, specify the path here\n"
"# For example: UsrKeyMapFile = ~/.unikey/my-telex\n";

static char EnableSpellCheckCmt[] = 
"# EnableSpellCheck: Yes|No\n"
"# Enable Vietnamese spell checking\n"
"# Default: Yes\n";

static char AutoRestoreNonVnCmt[] = 
"# AutoRestoreNonVn: Yes|No\n"
"# Enable auto-restore key strokes for non-Vietnamese word\n"
"# Default: No\n";

static OptMap InputLookup[] = {
  {"TELEX", UkTelex},
  {"VNI", UkVni},
  {"VIQR", UkViqr},
  {"VIQR*", UkViqr},
  {"USER", UkUsrIM},
  {0, 0}
};

static OptMap CharsetLookup[] = {
  {"UNICODE", CONV_CHARSET_XUTF8},
  {"UTF8", CONV_CHARSET_XUTF8},
  {"UTF-8", CONV_CHARSET_XUTF8},
  {"TCVN", CONV_CHARSET_TCVN3},
  {"VNI", CONV_CHARSET_VNIWIN},
  {"VIQR", CONV_CHARSET_VIQR},
  {"BK2", CONV_CHARSET_BKHCM2},
  {0, 0}
};

static OptMap StateLookup[] = {
  {"ON", 1},
  {"OFF", 0},
  {0, 0}
};

static OptMap CommitLookup[] = {
  {"Send", UkSendCommit},
  {"Forward", UkForwardCommit},
  {0, 0}
};

static OptMap XimFlowLookup[] = {
  {"Static", UkXimStatic},
  {"Dynamic", UkXimDynamic},
  {0, 0}
};

OptItem UkXimOptList[] = {
  {"AutoSave", AutoSaveCmt, offsetof(UkXimOpt, autoSave), BoolOpt, 0},
  {"InitState", InitStateCmt, offsetof(UkXimOpt, enabled), LookupOpt, StateLookup},  
  {"Input", InputCmt, offsetof(UkXimOpt, inputMethod), LookupOpt, InputLookup},
  {"Charset", CharsetCmt, offsetof(UkXimOpt, charset), LookupOpt, CharsetLookup},
  {"FreeStyle", FreeStyleCmt, offsetof(UkXimOpt, uk.freeMarking), BoolOpt, 0},
  {"ModernStyle", ModernStyleCmt, offsetof(UkXimOpt, uk.modernStyle), BoolOpt, 0},
  {"XvnkbSync", XvnkbSyncCmt, offsetof(UkXimOpt, xvnkbSync), BoolOpt, 0},
  {"Bell", BellCmt, offsetof(UkXimOpt, bellNotify), BoolOpt, 0},
  {"CommitMethod", CommitMethodCmt, offsetof(UkXimOpt, commitMethod), LookupOpt, CommitLookup},
  {"XimFlow", XimFlowCmt, offsetof(UkXimOpt, ximFlow), LookupOpt, XimFlowLookup},
  {"MacroFile", MacroFileCmt, offsetof(UkXimOpt, macroFile), StrOpt, 0},
  {"XimLocales", XimLocalesCmt, offsetof(UkXimOpt, ximLocales), StrOpt, 0},
  {"GtkImAlone", GtkImAloneCmt, offsetof(UkXimOpt, gtkImAlone), BoolOpt, 0},
  {"PosX", PosXCmt, offsetof(UkXimOpt, posX), LongOpt, 0},
  {"PosY", PosYCmt, offsetof(UkXimOpt, posY), LongOpt, 0},
  {"UsrKeyMapFile", UsrKeyMapFileCmt, offsetof(UkXimOpt, usrKeyMapFile), StrOpt, 0},
  {"EnableSpellCheck", EnableSpellCheckCmt, offsetof(UkXimOpt, uk.spellCheckEnabled), BoolOpt, 0},
  {"AutoRestoreNonVn", AutoRestoreNonVnCmt, offsetof(UkXimOpt, uk.autoNonVnRestore), BoolOpt, 0}
};

/*
//----------------------------------------------------
void testParse(UkXimOpt *options)
{
  printf("Enabled: %s\n", options->enabled ? "Yes":"No");
  printf("Input method: %ld\n", options->inputMethod);
  printf("Charset: %ld\n", options->charset);
  printf("FreeStyle: %s\n", options->uk.freeMarking? "Yes":"No");
  printf("ToneManual: %s\n", options->uk.toneNextToVowel? "Yes":"No");
  printf("ModernStyle: %s\n", options->uk.modernStyle? "Yes":"No");
  printf("MacroFile: %s\n", options->macroFile? options->macroFile: "Unspecified");
}
*/

//----------------------------------------------------
int UkParseOptFile(const char *fileName, UkXimOpt *options)
{
  int ret;
  char *expName;
  ret = ParseOptFile(fileName, options, UkXimOptList, sizeof(UkXimOptList)/sizeof(OptItem));
  if (ParseExpandFileName(options->macroFile, &expName)) {
    free(options->macroFile);
    options->macroFile = expName;
  }
  if (ParseExpandFileName(options->usrKeyMapFile, &expName)) {
    free(options->usrKeyMapFile);
    options->usrKeyMapFile = expName;
  }
  //  testParse(options);
  return ret;
}

//----------------------------------------------------
int UkWriteOptFile(const char *fileName, UkXimOpt *options)
{
  return ParseWriteOptFile(fileName, ConfigHeaderCmt, options,
		       UkXimOptList, sizeof(UkXimOptList)/sizeof(OptItem));
}

//------------------------------------------------------
char *UkGetDefConfFileName()
{
  static char buf[128];
  strcpy(buf, getenv("HOME"));
  //  strcat(buf, "/.unikeyrc");
  strcat(buf, "/.unikey/options");
  return buf;
}

//------------------------------------------------------
int createDefConfDir()
{
  int ret;
  char name[128];
  strcpy(name, getenv("HOME"));
  strcat(name, "/.unikey");
  ret = mkdir(name, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
  return (ret == 0 || errno == EEXIST);
}

//------------------------------------------------------
void UkSetDefOptions(UkXimOpt *options)
{
  memset(options, 0, sizeof(UkXimOpt));

  CreateDefaultUnikeyOptions(&options->uk);

  options->inputMethod = UkTelex;
  options->charset = CONV_CHARSET_XUTF8;
  options->enabled = 1;
  options->xvnkbSync = 0;
  options->bellNotify = 1;
  options->commitMethod = UkSendCommit;
  options->ximFlow = UkXimStatic;
  options->autoSave = 1;
  options->posX = -1;
  options->posY = -1;
  options->macroFile = 0;
  options->ximLocales = 0;
  options->gtkImAlone = 0;
  options->usrKeyMapFile = 0;
}

//------------------------------------------------------
// test if default config file and directory exist, if not then create them
//------------------------------------------------------
void UkTestDefConfFile()
{
  FILE *f;
  char *fname;
  createDefConfDir();
  fname = UkGetDefConfFileName();
  f = fopen(fname, "r");
  if (!f) {
    UkXimOpt opt;
    UkSetDefOptions(&opt);
    UkWriteOptFile(fname, &opt);
  }
  else
    fclose(f);
}
