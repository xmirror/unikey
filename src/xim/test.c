#include <stdio.h>
#include <locale.h>

int main()
{
  char *lc_ctype, *lc_all;
  lc_all = setlocale(LC_ALL, "enm_US.UTF-8");
  lc_ctype = setlocale(LC_CTYPE, "en_US.UTF-8");
  printf("Current LC_CTYPE: %s\n", lc_ctype);
  printf("Current LC_ALL: %s\n", lc_all);
  return 0;
}
