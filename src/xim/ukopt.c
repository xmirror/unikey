#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
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
"# Input methods: TELEX|VNI|VIQR|VIQR*\n";

static char CharsetCmt[] = 
"# Output charsets: UNICODE|TCVN|VNI|VIQR\n";

static char FreeStyleCmt[] = 
"# FreeStyle: Yes|No\n"
"# \"Yes\" means you can type hook, breve marks at the end of words\n"
"# TELEX users should use \"No\", VNI users should use \"Yes\"\n";

static char ToneManualCmt[] =
"# ToneManual: Yes|No\n"
"# You should set this to \"No\" to let Unikey determine position for tone marks.\n";

static char ModernStyleCmt[] = 
"# ModernStyle: Yes|No\n"
"# \"Yes\" means \"hoa', khoe?\" style, \"No\" means \"ho'a, kho?e\" style\n";

static char MacroFileCmt[] =
"# Macrofile: path to macro file\n"
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
"# Before changing this option, make sure unikey is not loaded in memory.\n";


static char PosXCmt[] =
"# PosX: X Position of unikey window\n"
"# set a minus value to let unikey use the default position\n";

static char PosYCmt[] =
"# PosY: Y Position of unikey window\n"
"# set a minus value to let unikey use the default position\n";

static OptMap InputLookup[] = {
  {"TELEX", TELEX_INPUT},
  {"VNI", VNI_INPUT},
  {"VIQR", VIQR_INPUT},
  {"VIQR*", VIQR_STAR_INPUT},
  {0, 0}
};

static OptMap CharsetLookup[] = {
  {"UNICODE", UNICODE_CHARSET},
  {"UTF8", UNICODE_CHARSET},
  {"UTF-8", UNICODE_CHARSET},
  {"TCVN", TCVN3_CHARSET},
  {"VNI", VNI_CHARSET},
  {"VIQR", VIQR_CHARSET},
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
  {"ToneManual", ToneManualCmt, offsetof(UkXimOpt, uk.toneNextToVowel), BoolOpt, 0},
  {"ModernStyle", ModernStyleCmt, offsetof(UkXimOpt, uk.modernStyle), BoolOpt, 0},
  {"XvnkbSync", XvnkbSyncCmt, offsetof(UkXimOpt, xvnkbSync), BoolOpt, 0},
  {"Bell", BellCmt, offsetof(UkXimOpt, bellNotify), BoolOpt, 0},
  {"CommitMethod", CommitMethodCmt, offsetof(UkXimOpt, commitMethod), LookupOpt, CommitLookup},
  {"XimFlow", XimFlowCmt, offsetof(UkXimOpt, ximFlow), LookupOpt, XimFlowLookup},
  {"MacroFile", MacroFileCmt, offsetof(UkXimOpt, macroFile), StrOpt, 0},
  {"PosX", PosXCmt, offsetof(UkXimOpt, posX), LongOpt, 0},
  {"PosY", PosYCmt, offsetof(UkXimOpt, posY), LongOpt, 0}
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
  ret = ParseOptFile(fileName, options, UkXimOptList, sizeof(UkXimOptList)/sizeof(OptItem));
  char *expName;
  if (ParseExpandFileName(options->macroFile, &expName)) {
    free(options->macroFile);
    options->macroFile = expName;
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
  strcat(buf, "/.unikeyrc");
  return buf;
}

//------------------------------------------------------
void UkSetDefOptions(UkXimOpt *options)
{
  memset(options, 0, sizeof(UkXimOpt));

  CreateDefaultUnikeyOptions(&options->uk);

  options->inputMethod = TELEX_INPUT;
  options->charset = UNICODE_CHARSET;
  options->enabled = 1;
  options->xvnkbSync = 0;
  options->bellNotify = 1;
  options->commitMethod = UkSendCommit;
  options->ximFlow = UkXimStatic;
  options->autoSave = 1;
  options->posX = -1;
  options->posY = -1;
  options->macroFile = 0;
}

//------------------------------------------------------
// test if default config file exists, if not then create it
//------------------------------------------------------
void UkTestDefConfFile()
{
  FILE *f;
  char *name = UkGetDefConfFileName();
  f = fopen(name, "r");
  if (!f) {
    UkXimOpt opt;
    UkSetDefOptions(&opt);
    UkWriteOptFile(name, &opt);
  }
  else
    fclose(f);
}
