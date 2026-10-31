#ifndef _USER_CATIN_H
#define _USER_CATIN_H
typedef unsigned long long u64;
typedef unsigned int u32;
int  catin_fork(void);
int  catin_wait(int *s);
void catin_exit(int code) __attribute__((noreturn));
int  catin_getpid(void);
void catin_sleep(unsigned int ms);
void catin_putchar(char c);
void catin_puts(const char *s);
int  catin_getchar(void);
int  catin_open(const char *p, int f);
int  catin_close(int fd);
int  catin_read(int fd, void *b, unsigned int n);
int  catin_write(int fd, const void *b, unsigned int n);
#endif
