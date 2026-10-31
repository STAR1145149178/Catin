#ifndef _SYSCALL_H
#define _SYSCALL_H
#define SYS_exit 0
#define SYS_fork 1
#define SYS_exec 2
#define SYS_wait 3
#define SYS_getpid 4
#define SYS_yield 5
#define SYS_sleep 6
#define SYS_write 7
#define SYS_read 8
#define SYS_open 9
#define SYS_close 10
#define SYS_lseek 11
#define SYS_mkdir 12
#define SYS_unlink 13
#define SYS_readdir 14
#define SYS_brk 15
#define SYS_uptime 16
#define SYS_getchar 17
#define SYS_putchar 18
#define SYS_ps 19
#define SYS_max 20
void syscall_init(void);
u64 syscall_dispatch(u64 nr, u64 a0, u64 a1, u64 a2, u64 a3, u64 a4, u64 a5);
#endif
