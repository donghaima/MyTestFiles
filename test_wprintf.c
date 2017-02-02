#include <stdio.h>
#include <wchar.h>
#include <stdlib.h>
#include <locale.h>

int main() {
  int r;
  wchar_t myChar1 = L'ù';
  wchar_t wChar[] = L"ùfdfdùfdfd";

  setlocale(LC_CTYPE, "");
  r = printf("char is %lc (%x), wChar=%ls\n", myChar1, myChar1, wChar);

  //r = wprintf(L"***char is %lc (%x), wChar=%ls\n", myChar1, myChar1, wChar);

}
