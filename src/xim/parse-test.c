#include <stdio.h>
#include "optparse.h"

//----------------------------------------------------
void test(UkXimOpt *options)
{
  printf("Input method: %ld\n", options->inputMethod);
  printf("Charset: %ld\n", options->charset);
  printf("FreeStyle: %s\n", options->uk.freeMarking? "Yes":"No");
  printf("ToneManual: %s\n", options->uk.toneNextToVowel? "Yes":"No");
  printf("ModernStyle: %s\n", options->uk.modernStyle? "Yes":"No");
  printf("MacroFile: %s\n", options->macroFile? options->macroFile: "Unspecified");
}

//----------------------------------------------------
int main()
{
  UkXimOpt options;
  memset(&options, 0, sizeof(UkXimOpt));

  printf("Parsing...\n");
  //  parseFile("cfg", NULL, 0);
  ParseUnikeyOptFile("cfg", &options);
  test(&options);
  printf("Done.\n");
  printf("Writing default file...\n");
  WriteDefConfigFile("def.cfg");
  printf("Finished.\n");
  return 0;
}
