#ifndef __UNIKEY_GUI_OPT_H
#define __UNIKEY_GUI_OPT_H

typedef struct _UkGuiOpt {
  long posX;
  long posY;
} UkGuiOpt;

int UkGuiParseOptFile(const char *fileName, UkGuiOpt *options);
char *UkGuiGetDefConfFileName();
void UkGuiSetDefOptions(UkGuiOpt *options);

#endif
