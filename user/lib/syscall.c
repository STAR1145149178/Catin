#include "../include/catin.h"
#include "../include/syscall.h"
int  catin_fork(void){ return __sc0(SYS_fork); }
int  catin_wait(int*s){ return __sc1(SYS_wait,(long)s); }
void catin_exit(int c){ __sc1(SYS_exit,c); for(;;); }
int  catin_getpid(void){ return __sc0(SYS_getpid); }
void catin_sleep(unsigned int ms){ __sc1(SYS_sleep,ms); }
void catin_putchar(char c){ __sc1(SYS_putchar,c); }
void catin_puts(const char *s){ while(*s) catin_putchar(*s++); }
int  catin_getchar(void){ return __sc0(SYS_getchar); }
int  catin_open(const char*p,int f){ return __sc2(SYS_open,(long)p,f); }
int  catin_close(int fd){ return __sc1(SYS_close,fd); }
int  catin_read(int fd,void*b,unsigned n){ return __sc3(SYS_read,fd,(long)b,n); }
int  catin_write(int fd,const void*b,unsigned n){ return __sc3(SYS_write,fd,(long)b,n); }
