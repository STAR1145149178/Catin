#include "types.h"
typedef unsigned long ulong_t;

void *memset(void *s, int c, ulong_t n){
    unsigned char *p = (unsigned char*)s;
    while (n--) *p++ = (unsigned char)c;
    return s;
}
void *memcpy(void *d, const void *s, ulong_t n){
    unsigned char *a = (unsigned char*)d;
    const unsigned char *b = (const unsigned char*)s;
    while (n--) *a++ = *b++;
    return d;
}
int memcmp(const void *a, const void *b, ulong_t n){
    const unsigned char *x = (const unsigned char*)a;
    const unsigned char *y = (const unsigned char*)b;
    while (n--) { if (*x != *y) return (int)*x - (int)*y; x++; y++; }
    return 0;
}
void raise(int sig){ (void)sig; }
long __aeabi_ldiv0(long a, long b){ (void)b; return a; }
int  __aeabi_idiv0(int a, int b){ (void)b; return a; }