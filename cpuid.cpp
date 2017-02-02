#include <iostream>
 
int main(int argc, char **argv) {
      int b;
      for (int a = 0; a < 5; a++) {
         asm ( "mov %1, %%eax; " // a into eax
               "cpuid;"
               "mov %%eax, %0;" // eeax into b
               :"=r"(b) /* output */
               :"r"(a) /* input */
               :"%eax" /* clobbered register */
             );
         std::cout << "The code " << a << " gives " << b << std::endl;
      }
      return 0;
}

