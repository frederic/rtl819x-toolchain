/* Force shared libraries to know about the correct library loader */
#include <features.h>
const char __dl_ldso__[] attribute_hidden __attribute__ ((weak)) __attribute__ ((section  (".interp"))) ="//ld-uClibc.so.0";
