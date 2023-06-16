#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#define FCG_assert(expr) if (!(expr)) { fprintf(stderr, "Assert failed, %s:%i.\n%s\n", __FILENAME__, __LINE__, #expr); exit(1); }