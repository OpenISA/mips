#include "openisa_syscall.H"

// 'using namespace' statement to allow access to all
// openisa-specific datatypes
using namespace openisa_parms;
unsigned procNumber = 0;

void openisa_syscall::get_buffer(int argn, unsigned char* buf, unsigned int size)
{
  unsigned int addr = RB[4+argn];
  for (unsigned int i = 0; i<size; i++, addr++) {
    buf[i] = DATA_PORT->read_byte(addr);
  }
}

void openisa_syscall::guest2hostmemcpy(unsigned char *dst, uint32_t src,
                                    unsigned int size) {
  for (unsigned int i = 0; i < size; i++) {
    dst[i] = DATA_PORT->read_byte(src++);
  }
}

void openisa_syscall::set_buffer(int argn, unsigned char* buf, unsigned int size)
{
  unsigned int addr = RB[4+argn];

  for (unsigned int i = 0; i<size; i++, addr++) {
    DATA_PORT->write_byte(addr, buf[i]);
  }
}

void openisa_syscall::host2guestmemcpy(uint32_t dst, unsigned char *src,
                                    unsigned int size) {
  for (unsigned int i = 0; i < size; i++) {
    DATA_PORT->write_byte(dst++, src[i]);
  }
}

void openisa_syscall::set_buffer_noinvert(int argn, unsigned char* buf, unsigned int size)
{
  unsigned int addr = RB[4+argn];

  for (unsigned int i = 0; i<size; i+=4, addr+=4) {
    DATA_PORT->write(addr, *(unsigned int *) &buf[i]);
    //printf("\nDATA_PORT_no[%d]=%d", addr, buf[i]);
  }
}

int openisa_syscall::get_int(int argn)
{
  return RB[4+argn];
}

void openisa_syscall::set_int(int argn, int val)
{
  RB[2+argn] = val;
}

void openisa_syscall::return_from_syscall()
{
  ac_pc = RB[31];
}

void openisa_syscall::set_prog_args(int argc, char **argv)
{
  // ac_argstr holds argument strings to be stored into guest memory
  char ac_argstr[512];
  // guest_stack represents pre-allocated values into the guest stack
  // passing information from kernel to userland (program args, etc)
  //
  // Memory diagram:
  //   higher addresses  .. auxv
  //                        envp pointers
  //                        argv pointers
  //   lower addresses   .. argc
  uint32_t guest_stack_size = argc + 6;
  uint32_t *guest_stack = (uint32_t *) malloc(sizeof(uint32_t) *
                                              guest_stack_size);
  uint32_t i = 0, j = 0;
  uint32_t strtable = AC_RAM_END - 512 - procNumber * 64 * 1024;
  guest_stack[i++] = argc;
  for (uint32_t k = 0; k < argc; ++k) {
    uint32_t len = strlen(argv[k]) + 1;
    guest_stack[i++] = strtable + j;
    if (j + len > 512) {
      fprintf(stderr, "Fatal: argv strings bigger than 512 bytes.\n");
      exit(EXIT_FAILURE);
    }
    memcpy(&ac_argstr[j], argv[k], len);
    j += len;
  }
  // Set argv end
  guest_stack[i++] = 0;
  // Set envp
  guest_stack[i++] = 0;
  // Set auxv
  guest_stack[i++] = 6; // AT_PAGESZ -
                    // see http://articles.manugarg.com/aboutelfauxiliaryvectors
  guest_stack[i++] = 4096; // page size
  guest_stack[i++] = 0; // AT_NULL

  RB[4] = strtable;
  set_buffer(0, (unsigned char*) ac_argstr, 512);   //$25 = $29(sp) - 4 (set_buffer adds 4)

  RB[4] = strtable - guest_stack_size * 4;
  set_buffer_noinvert(0, (unsigned char*) guest_stack, guest_stack_size * 4);

  RB[29] = strtable - guest_stack_size * 4;

  // FIXME: Necessary?
  RB[4] = argc;
  RB[5] = strtable - guest_stack_size * 4 + 4;

  procNumber ++;
}

void openisa_syscall::set_pc(unsigned val) {
  ac_pc.write(val);
}

void openisa_syscall::set_return(unsigned val) {
  RB.write(31, val);
}

unsigned openisa_syscall::get_return() {
  return (unsigned) RB.read(31);
}

bool openisa_syscall::is_mmap_anonymous(uint32_t flags) {
  return flags & 0x800;
}

uint32_t openisa_syscall::convert_open_flags(uint32_t flags) {
  uint32_t dst = 0;
  dst |= (flags & 00000)? O_RDONLY : 0;
  dst |= (flags & 00001)? O_WRONLY : 0;
  dst |= (flags & 00002)? O_RDWR   : 0;
  dst |= (flags & 001000)? O_CREAT  : 0;
  dst |= (flags & 004000)? O_EXCL   : 0;
  dst |= (flags & 0100000)? O_NOCTTY   : 0;
  dst |= (flags & 02000)? O_TRUNC    : 0;
  dst |= (flags & 00010)? O_APPEND   : 0;
  dst |= (flags & 040000)? O_NONBLOCK : 0;
  dst |= (flags & 020000)? O_SYNC  : 0;
  // We don't know the mapping of these:
  //    dst |= (flags & 020000)? O_ASYNC   : 0;
  //    dst |= (flags & 0100000)? O_LARGEFILE  : 0;
  //    dst |= (flags & 0200000)? O_DIRECTORY  : 0;
  //    dst |= (flags & 0400000)? O_NOFOLLOW   : 0;
  //    dst |= (flags & 02000000)? O_CLOEXEC   : 0;
  //    dst |= (flags & 040000)? O_DIRECT      : 0;
  //    dst |= (flags & 01000000)? O_NOATIME   : 0;
  //    dst |= (flags & 010000000)? O_PATH     : 0;
  //    dst |= (flags & 010000)? O_DSYNC       : 0;
  return dst;
}


// OPENISA syscalls mapping for process simulators
//

#define OPENISA__NR_Linux			4000
#define OPENISA__NR_syscall			(OPENISA__NR_Linux +	0)
#define OPENISA__NR_exit			(OPENISA__NR_Linux +	1)
#define OPENISA__NR_fork			(OPENISA__NR_Linux +	2)
#define OPENISA__NR_read			(OPENISA__NR_Linux +	3)
#define OPENISA__NR_write			(OPENISA__NR_Linux +	4)
#define OPENISA__NR_open			(OPENISA__NR_Linux +	5)
#define OPENISA__NR_close			(OPENISA__NR_Linux +	6)
#define OPENISA__NR_waitpid			(OPENISA__NR_Linux +	7)
#define OPENISA__NR_creat			(OPENISA__NR_Linux +	8)
#define OPENISA__NR_link			(OPENISA__NR_Linux +	9)
#define OPENISA__NR_unlink			(OPENISA__NR_Linux +  10)
#define OPENISA__NR_execve			(OPENISA__NR_Linux +  11)
#define OPENISA__NR_chdir			(OPENISA__NR_Linux +  12)
#define OPENISA__NR_time			(OPENISA__NR_Linux +  13)
#define OPENISA__NR_mknod			(OPENISA__NR_Linux +  14)
#define OPENISA__NR_chmod			(OPENISA__NR_Linux +  15)
#define OPENISA__NR_lchown			(OPENISA__NR_Linux +  16)
#define OPENISA__NR_break			(OPENISA__NR_Linux +  17)
#define OPENISA__NR_unused18			(OPENISA__NR_Linux +  18)
#define OPENISA__NR_lseek			(OPENISA__NR_Linux +  19)
#define OPENISA__NR_getpid			(OPENISA__NR_Linux +  20)
#define OPENISA__NR_mount			(OPENISA__NR_Linux +  21)
#define OPENISA__NR_umount			(OPENISA__NR_Linux +  22)
#define OPENISA__NR_setuid			(OPENISA__NR_Linux +  23)
#define OPENISA__NR_getuid			(OPENISA__NR_Linux +  24)
#define OPENISA__NR_stime			(OPENISA__NR_Linux +  25)
#define OPENISA__NR_ptrace			(OPENISA__NR_Linux +  26)
#define OPENISA__NR_alarm			(OPENISA__NR_Linux +  27)
#define OPENISA__NR_unused28			(OPENISA__NR_Linux +  28)
#define OPENISA__NR_pause			(OPENISA__NR_Linux +  29)
#define OPENISA__NR_utime			(OPENISA__NR_Linux +  30)
#define OPENISA__NR_stty			(OPENISA__NR_Linux +  31)
#define OPENISA__NR_gtty			(OPENISA__NR_Linux +  32)
#define OPENISA__NR_access			(OPENISA__NR_Linux +  33)
#define OPENISA__NR_nice			(OPENISA__NR_Linux +  34)
#define OPENISA__NR_ftime			(OPENISA__NR_Linux +  35)
#define OPENISA__NR_sync			(OPENISA__NR_Linux +  36)
#define OPENISA__NR_kill			(OPENISA__NR_Linux +  37)
#define OPENISA__NR_rename			(OPENISA__NR_Linux +  38)
#define OPENISA__NR_mkdir			(OPENISA__NR_Linux +  39)
#define OPENISA__NR_rmdir			(OPENISA__NR_Linux +  40)
#define OPENISA__NR_dup			(OPENISA__NR_Linux +  41)
#define OPENISA__NR_pipe			(OPENISA__NR_Linux +  42)
#define OPENISA__NR_times			(OPENISA__NR_Linux +  43)
#define OPENISA__NR_prof			(OPENISA__NR_Linux +  44)
#define OPENISA__NR_brk			(OPENISA__NR_Linux +  45)
#define OPENISA__NR_setgid			(OPENISA__NR_Linux +  46)
#define OPENISA__NR_getgid			(OPENISA__NR_Linux +  47)
#define OPENISA__NR_signal			(OPENISA__NR_Linux +  48)
#define OPENISA__NR_geteuid			(OPENISA__NR_Linux +  49)
#define OPENISA__NR_getegid			(OPENISA__NR_Linux +  50)
#define OPENISA__NR_acct			(OPENISA__NR_Linux +  51)
#define OPENISA__NR_umount2			(OPENISA__NR_Linux +  52)
#define OPENISA__NR_lock			(OPENISA__NR_Linux +  53)
#define OPENISA__NR_ioctl			(OPENISA__NR_Linux +  54)
#define OPENISA__NR_fcntl			(OPENISA__NR_Linux +  55)
#define OPENISA__NR_mpx			(OPENISA__NR_Linux +  56)
#define OPENISA__NR_setpgid			(OPENISA__NR_Linux +  57)
#define OPENISA__NR_ulimit			(OPENISA__NR_Linux +  58)
#define OPENISA__NR_unused59			(OPENISA__NR_Linux +  59)
#define OPENISA__NR_umask			(OPENISA__NR_Linux +  60)
#define OPENISA__NR_chroot			(OPENISA__NR_Linux +  61)
#define OPENISA__NR_ustat			(OPENISA__NR_Linux +  62)
#define OPENISA__NR_dup2			(OPENISA__NR_Linux +  63)
#define OPENISA__NR_getppid			(OPENISA__NR_Linux +  64)
#define OPENISA__NR_getpgrp			(OPENISA__NR_Linux +  65)
#define OPENISA__NR_setsid			(OPENISA__NR_Linux +  66)
#define OPENISA__NR_sigaction			(OPENISA__NR_Linux +  67)
#define OPENISA__NR_sgetmask			(OPENISA__NR_Linux +  68)
#define OPENISA__NR_ssetmask			(OPENISA__NR_Linux +  69)
#define OPENISA__NR_setreuid			(OPENISA__NR_Linux +  70)
#define OPENISA__NR_setregid			(OPENISA__NR_Linux +  71)
#define OPENISA__NR_sigsuspend			(OPENISA__NR_Linux +  72)
#define OPENISA__NR_sigpending			(OPENISA__NR_Linux +  73)
#define OPENISA__NR_sethostname		(OPENISA__NR_Linux +  74)
#define OPENISA__NR_setrlimit			(OPENISA__NR_Linux +  75)
#define OPENISA__NR_getrlimit			(OPENISA__NR_Linux +  76)
#define OPENISA__NR_getrusage			(OPENISA__NR_Linux +  77)
#define OPENISA__NR_gettimeofday		(OPENISA__NR_Linux +  78)
#define OPENISA__NR_settimeofday		(OPENISA__NR_Linux +  79)
#define OPENISA__NR_getgroups			(OPENISA__NR_Linux +  80)
#define OPENISA__NR_setgroups			(OPENISA__NR_Linux +  81)
#define OPENISA__NR_reserved82			(OPENISA__NR_Linux +  82)
#define OPENISA__NR_symlink			(OPENISA__NR_Linux +  83)
#define OPENISA__NR_unused84			(OPENISA__NR_Linux +  84)
#define OPENISA__NR_readlink			(OPENISA__NR_Linux +  85)
#define OPENISA__NR_uselib			(OPENISA__NR_Linux +  86)
#define OPENISA__NR_swapon			(OPENISA__NR_Linux +  87)
#define OPENISA__NR_reboot			(OPENISA__NR_Linux +  88)
#define OPENISA__NR_readdir			(OPENISA__NR_Linux +  89)
#define OPENISA__NR_mmap			(OPENISA__NR_Linux +  90)
#define OPENISA__NR_munmap			(OPENISA__NR_Linux +  91)
#define OPENISA__NR_truncate			(OPENISA__NR_Linux +  92)
#define OPENISA__NR_ftruncate			(OPENISA__NR_Linux +  93)
#define OPENISA__NR_fchmod			(OPENISA__NR_Linux +  94)
#define OPENISA__NR_fchown			(OPENISA__NR_Linux +  95)
#define OPENISA__NR_getpriority		(OPENISA__NR_Linux +  96)
#define OPENISA__NR_setpriority		(OPENISA__NR_Linux +  97)
#define OPENISA__NR_profil			(OPENISA__NR_Linux +  98)
#define OPENISA__NR_statfs			(OPENISA__NR_Linux +  99)
#define OPENISA__NR_fstatfs			(OPENISA__NR_Linux + 100)
#define OPENISA__NR_ioperm			(OPENISA__NR_Linux + 101)
#define OPENISA__NR_socketcall			(OPENISA__NR_Linux + 102)
#define OPENISA__NR_syslog			(OPENISA__NR_Linux + 103)
#define OPENISA__NR_setitimer			(OPENISA__NR_Linux + 104)
#define OPENISA__NR_getitimer			(OPENISA__NR_Linux + 105)
#define OPENISA__NR_stat			(OPENISA__NR_Linux + 106)
#define OPENISA__NR_lstat			(OPENISA__NR_Linux + 107)
#define OPENISA__NR_fstat			(OPENISA__NR_Linux + 108)
#define OPENISA__NR_unused109			(OPENISA__NR_Linux + 109)
#define OPENISA__NR_iopl			(OPENISA__NR_Linux + 110)
#define OPENISA__NR_vhangup			(OPENISA__NR_Linux + 111)
#define OPENISA__NR_idle			(OPENISA__NR_Linux + 112)
#define OPENISA__NR_vm86			(OPENISA__NR_Linux + 113)
#define OPENISA__NR_wait4			(OPENISA__NR_Linux + 114)
#define OPENISA__NR_swapoff			(OPENISA__NR_Linux + 115)
#define OPENISA__NR_sysinfo			(OPENISA__NR_Linux + 116)
#define OPENISA__NR_ipc			(OPENISA__NR_Linux + 117)
#define OPENISA__NR_fsync			(OPENISA__NR_Linux + 118)
#define OPENISA__NR_sigreturn			(OPENISA__NR_Linux + 119)
#define OPENISA__NR_clone			(OPENISA__NR_Linux + 120)
#define OPENISA__NR_setdomainname		(OPENISA__NR_Linux + 121)
#define OPENISA__NR_uname			(OPENISA__NR_Linux + 122)
#define OPENISA__NR_modify_ldt			(OPENISA__NR_Linux + 123)
#define OPENISA__NR_adjtimex			(OPENISA__NR_Linux + 124)
#define OPENISA__NR_mprotect			(OPENISA__NR_Linux + 125)
#define OPENISA__NR_sigprocmask		(OPENISA__NR_Linux + 126)
#define OPENISA__NR_create_module		(OPENISA__NR_Linux + 127)
#define OPENISA__NR_init_module		(OPENISA__NR_Linux + 128)
#define OPENISA__NR_delete_module		(OPENISA__NR_Linux + 129)
#define OPENISA__NR_get_kernel_syms		(OPENISA__NR_Linux + 130)
#define OPENISA__NR_quotactl			(OPENISA__NR_Linux + 131)
#define OPENISA__NR_getpgid			(OPENISA__NR_Linux + 132)
#define OPENISA__NR_fchdir			(OPENISA__NR_Linux + 133)
#define OPENISA__NR_bdflush			(OPENISA__NR_Linux + 134)
#define OPENISA__NR_sysfs			(OPENISA__NR_Linux + 135)
#define OPENISA__NR_personality		(OPENISA__NR_Linux + 136)
#define OPENISA__NR_afs_syscall		(OPENISA__NR_Linux + 137) /* Syscall for Andrew File System */
#define OPENISA__NR_setfsuid			(OPENISA__NR_Linux + 138)
#define OPENISA__NR_setfsgid			(OPENISA__NR_Linux + 139)
#define OPENISA__NR__llseek			(OPENISA__NR_Linux + 140)
#define OPENISA__NR_getdents			(OPENISA__NR_Linux + 141)
#define OPENISA__NR__newselect			(OPENISA__NR_Linux + 142)
#define OPENISA__NR_flock			(OPENISA__NR_Linux + 143)
#define OPENISA__NR_msync			(OPENISA__NR_Linux + 144)
#define OPENISA__NR_readv			(OPENISA__NR_Linux + 145)
#define OPENISA__NR_writev			(OPENISA__NR_Linux + 146)
#define OPENISA__NR_cacheflush			(OPENISA__NR_Linux + 147)
#define OPENISA__NR_cachectl			(OPENISA__NR_Linux + 148)
#define OPENISA__NR_sysopenisa			(OPENISA__NR_Linux + 149)
#define OPENISA__NR_unused150			(OPENISA__NR_Linux + 150)
#define OPENISA__NR_getsid			(OPENISA__NR_Linux + 151)
#define OPENISA__NR_fdatasync			(OPENISA__NR_Linux + 152)
#define OPENISA__NR__sysctl			(OPENISA__NR_Linux + 153)
#define OPENISA__NR_mlock			(OPENISA__NR_Linux + 154)
#define OPENISA__NR_munlock			(OPENISA__NR_Linux + 155)
#define OPENISA__NR_mlockall			(OPENISA__NR_Linux + 156)
#define OPENISA__NR_munlockall			(OPENISA__NR_Linux + 157)
#define OPENISA__NR_sched_setparam		(OPENISA__NR_Linux + 158)
#define OPENISA__NR_sched_getparam		(OPENISA__NR_Linux + 159)
#define OPENISA__NR_sched_setscheduler		(OPENISA__NR_Linux + 160)
#define OPENISA__NR_sched_getscheduler		(OPENISA__NR_Linux + 161)
#define OPENISA__NR_sched_yield		(OPENISA__NR_Linux + 162)
#define OPENISA__NR_sched_get_priority_max	(OPENISA__NR_Linux + 163)
#define OPENISA__NR_sched_get_priority_min	(OPENISA__NR_Linux + 164)
#define OPENISA__NR_sched_rr_get_interval	(OPENISA__NR_Linux + 165)
#define OPENISA__NR_nanosleep			(OPENISA__NR_Linux + 166)
#define OPENISA__NR_mremap			(OPENISA__NR_Linux + 167)
#define OPENISA__NR_accept			(OPENISA__NR_Linux + 168)
#define OPENISA__NR_bind			(OPENISA__NR_Linux + 169)
#define OPENISA__NR_connect			(OPENISA__NR_Linux + 170)
#define OPENISA__NR_getpeername		(OPENISA__NR_Linux + 171)
#define OPENISA__NR_getsockname		(OPENISA__NR_Linux + 172)
#define OPENISA__NR_getsockopt			(OPENISA__NR_Linux + 173)
#define OPENISA__NR_listen			(OPENISA__NR_Linux + 174)
#define OPENISA__NR_recv			(OPENISA__NR_Linux + 175)
#define OPENISA__NR_recvfrom			(OPENISA__NR_Linux + 176)
#define OPENISA__NR_recvmsg			(OPENISA__NR_Linux + 177)
#define OPENISA__NR_send			(OPENISA__NR_Linux + 178)
#define OPENISA__NR_sendmsg			(OPENISA__NR_Linux + 179)
#define OPENISA__NR_sendto			(OPENISA__NR_Linux + 180)
#define OPENISA__NR_setsockopt			(OPENISA__NR_Linux + 181)
#define OPENISA__NR_shutdown			(OPENISA__NR_Linux + 182)
#define OPENISA__NR_socket			(OPENISA__NR_Linux + 183)
#define OPENISA__NR_socketpair			(OPENISA__NR_Linux + 184)
#define OPENISA__NR_setresuid			(OPENISA__NR_Linux + 185)
#define OPENISA__NR_getresuid			(OPENISA__NR_Linux + 186)
#define OPENISA__NR_query_module		(OPENISA__NR_Linux + 187)
#define OPENISA__NR_poll			(OPENISA__NR_Linux + 188)
#define OPENISA__NR_nfsservctl			(OPENISA__NR_Linux + 189)
#define OPENISA__NR_setresgid			(OPENISA__NR_Linux + 190)
#define OPENISA__NR_getresgid			(OPENISA__NR_Linux + 191)
#define OPENISA__NR_prctl			(OPENISA__NR_Linux + 192)
#define OPENISA__NR_rt_sigreturn		(OPENISA__NR_Linux + 193)
#define OPENISA__NR_rt_sigaction		(OPENISA__NR_Linux + 194)
#define OPENISA__NR_rt_sigprocmask		(OPENISA__NR_Linux + 195)
#define OPENISA__NR_rt_sigpending		(OPENISA__NR_Linux + 196)
#define OPENISA__NR_rt_sigtimedwait		(OPENISA__NR_Linux + 197)
#define OPENISA__NR_rt_sigqueueinfo		(OPENISA__NR_Linux + 198)
#define OPENISA__NR_rt_sigsuspend		(OPENISA__NR_Linux + 199)
#define OPENISA__NR_pread64			(OPENISA__NR_Linux + 200)
#define OPENISA__NR_pwrite64			(OPENISA__NR_Linux + 201)
#define OPENISA__NR_chown			(OPENISA__NR_Linux + 202)
#define OPENISA__NR_getcwd			(OPENISA__NR_Linux + 203)
#define OPENISA__NR_capget			(OPENISA__NR_Linux + 204)
#define OPENISA__NR_capset			(OPENISA__NR_Linux + 205)
#define OPENISA__NR_sigaltstack		(OPENISA__NR_Linux + 206)
#define OPENISA__NR_sendfile			(OPENISA__NR_Linux + 207)
#define OPENISA__NR_getpmsg			(OPENISA__NR_Linux + 208)
#define OPENISA__NR_putpmsg			(OPENISA__NR_Linux + 209)
#define OPENISA__NR_mmap2			(OPENISA__NR_Linux + 210)
#define OPENISA__NR_truncate64			(OPENISA__NR_Linux + 211)
#define OPENISA__NR_ftruncate64		(OPENISA__NR_Linux + 212)
#define OPENISA__NR_stat64			(OPENISA__NR_Linux + 213)
#define OPENISA__NR_lstat64			(OPENISA__NR_Linux + 214)
#define OPENISA__NR_fstat64			(OPENISA__NR_Linux + 215)
#define OPENISA__NR_pivot_root			(OPENISA__NR_Linux + 216)
#define OPENISA__NR_mincore			(OPENISA__NR_Linux + 217)
#define OPENISA__NR_madvise			(OPENISA__NR_Linux + 218)
#define OPENISA__NR_getdents64			(OPENISA__NR_Linux + 219)
#define OPENISA__NR_fcntl64			(OPENISA__NR_Linux + 220)
#define OPENISA__NR_reserved221		(OPENISA__NR_Linux + 221)
#define OPENISA__NR_gettid			(OPENISA__NR_Linux + 222)
#define OPENISA__NR_readahead			(OPENISA__NR_Linux + 223)
#define OPENISA__NR_setxattr			(OPENISA__NR_Linux + 224)
#define OPENISA__NR_lsetxattr			(OPENISA__NR_Linux + 225)
#define OPENISA__NR_fsetxattr			(OPENISA__NR_Linux + 226)
#define OPENISA__NR_getxattr			(OPENISA__NR_Linux + 227)
#define OPENISA__NR_lgetxattr			(OPENISA__NR_Linux + 228)
#define OPENISA__NR_fgetxattr			(OPENISA__NR_Linux + 229)
#define OPENISA__NR_listxattr			(OPENISA__NR_Linux + 230)
#define OPENISA__NR_llistxattr			(OPENISA__NR_Linux + 231)
#define OPENISA__NR_flistxattr			(OPENISA__NR_Linux + 232)
#define OPENISA__NR_removexattr		(OPENISA__NR_Linux + 233)
#define OPENISA__NR_lremovexattr		(OPENISA__NR_Linux + 234)
#define OPENISA__NR_fremovexattr		(OPENISA__NR_Linux + 235)
#define OPENISA__NR_tkill			(OPENISA__NR_Linux + 236)
#define OPENISA__NR_sendfile64			(OPENISA__NR_Linux + 237)
#define OPENISA__NR_futex			(OPENISA__NR_Linux + 238)
#define OPENISA__NR_sched_setaffinity		(OPENISA__NR_Linux + 239)
#define OPENISA__NR_sched_getaffinity		(OPENISA__NR_Linux + 240)
#define OPENISA__NR_io_setup			(OPENISA__NR_Linux + 241)
#define OPENISA__NR_io_destroy			(OPENISA__NR_Linux + 242)
#define OPENISA__NR_io_getevents		(OPENISA__NR_Linux + 243)
#define OPENISA__NR_io_submit			(OPENISA__NR_Linux + 244)
#define OPENISA__NR_io_cancel			(OPENISA__NR_Linux + 245)
#define OPENISA__NR_exit_group			(OPENISA__NR_Linux + 246)
#define OPENISA__NR_lookup_dcookie		(OPENISA__NR_Linux + 247)
#define OPENISA__NR_epoll_create		(OPENISA__NR_Linux + 248)
#define OPENISA__NR_epoll_ctl			(OPENISA__NR_Linux + 249)
#define OPENISA__NR_epoll_wait			(OPENISA__NR_Linux + 250)
#define OPENISA__NR_remap_file_pages		(OPENISA__NR_Linux + 251)
#define OPENISA__NR_set_tid_address		(OPENISA__NR_Linux + 252)
#define OPENISA__NR_restart_syscall		(OPENISA__NR_Linux + 253)
#define OPENISA__NR_fadvise64			(OPENISA__NR_Linux + 254)
#define OPENISA__NR_statfs64			(OPENISA__NR_Linux + 255)
#define OPENISA__NR_fstatfs64			(OPENISA__NR_Linux + 256)
#define OPENISA__NR_timer_create		(OPENISA__NR_Linux + 257)
#define OPENISA__NR_timer_settime		(OPENISA__NR_Linux + 258)
#define OPENISA__NR_timer_gettime		(OPENISA__NR_Linux + 259)
#define OPENISA__NR_timer_getoverrun		(OPENISA__NR_Linux + 260)
#define OPENISA__NR_timer_delete		(OPENISA__NR_Linux + 261)
#define OPENISA__NR_clock_settime		(OPENISA__NR_Linux + 262)
#define OPENISA__NR_clock_gettime		(OPENISA__NR_Linux + 263)
#define OPENISA__NR_clock_getres		(OPENISA__NR_Linux + 264)
#define OPENISA__NR_clock_nanosleep		(OPENISA__NR_Linux + 265)
#define OPENISA__NR_tgkill			(OPENISA__NR_Linux + 266)
#define OPENISA__NR_utimes			(OPENISA__NR_Linux + 267)
#define OPENISA__NR_mbind			(OPENISA__NR_Linux + 268)
#define OPENISA__NR_get_mempolicy		(OPENISA__NR_Linux + 269)
#define OPENISA__NR_set_mempolicy		(OPENISA__NR_Linux + 270)
#define OPENISA__NR_mq_open			(OPENISA__NR_Linux + 271)
#define OPENISA__NR_mq_unlink			(OPENISA__NR_Linux + 272)
#define OPENISA__NR_mq_timedsend		(OPENISA__NR_Linux + 273)
#define OPENISA__NR_mq_timedreceive		(OPENISA__NR_Linux + 274)
#define OPENISA__NR_mq_notify			(OPENISA__NR_Linux + 275)
#define OPENISA__NR_mq_getsetattr		(OPENISA__NR_Linux + 276)
#define OPENISA__NR_vserver			(OPENISA__NR_Linux + 277)
#define OPENISA__NR_waitid			(OPENISA__NR_Linux + 278)
/* #define OPENISA__NR_sys_setaltroot		(OPENISA__NR_Linux + 279) */
#define OPENISA__NR_add_key			(OPENISA__NR_Linux + 280)
#define OPENISA__NR_request_key		(OPENISA__NR_Linux + 281)
#define OPENISA__NR_keyctl			(OPENISA__NR_Linux + 282)
#define OPENISA__NR_set_thread_area		(OPENISA__NR_Linux + 283)
#define OPENISA__NR_inotify_init		(OPENISA__NR_Linux + 284)
#define OPENISA__NR_inotify_add_watch		(OPENISA__NR_Linux + 285)
#define OPENISA__NR_inotify_rm_watch		(OPENISA__NR_Linux + 286)
#define OPENISA__NR_migrate_pages		(OPENISA__NR_Linux + 287)
#define OPENISA__NR_openat			(OPENISA__NR_Linux + 288)
#define OPENISA__NR_mkdirat			(OPENISA__NR_Linux + 289)
#define OPENISA__NR_mknodat			(OPENISA__NR_Linux + 290)
#define OPENISA__NR_fchownat			(OPENISA__NR_Linux + 291)
#define OPENISA__NR_futimesat			(OPENISA__NR_Linux + 292)
#define OPENISA__NR_fstatat64			(OPENISA__NR_Linux + 293)
#define OPENISA__NR_unlinkat			(OPENISA__NR_Linux + 294)
#define OPENISA__NR_renameat			(OPENISA__NR_Linux + 295)
#define OPENISA__NR_linkat			(OPENISA__NR_Linux + 296)
#define OPENISA__NR_symlinkat			(OPENISA__NR_Linux + 297)
#define OPENISA__NR_readlinkat			(OPENISA__NR_Linux + 298)
#define OPENISA__NR_fchmodat			(OPENISA__NR_Linux + 299)
#define OPENISA__NR_faccessat			(OPENISA__NR_Linux + 300)
#define OPENISA__NR_pselect6			(OPENISA__NR_Linux + 301)
#define OPENISA__NR_ppoll			(OPENISA__NR_Linux + 302)
#define OPENISA__NR_unshare			(OPENISA__NR_Linux + 303)
#define OPENISA__NR_splice			(OPENISA__NR_Linux + 304)
#define OPENISA__NR_sync_file_range		(OPENISA__NR_Linux + 305)
#define OPENISA__NR_tee			(OPENISA__NR_Linux + 306)
#define OPENISA__NR_vmsplice			(OPENISA__NR_Linux + 307)
#define OPENISA__NR_move_pages			(OPENISA__NR_Linux + 308)
#define OPENISA__NR_set_robust_list		(OPENISA__NR_Linux + 309)
#define OPENISA__NR_get_robust_list		(OPENISA__NR_Linux + 310)
#define OPENISA__NR_kexec_load			(OPENISA__NR_Linux + 311)
#define OPENISA__NR_getcpu			(OPENISA__NR_Linux + 312)
#define OPENISA__NR_epoll_pwait		(OPENISA__NR_Linux + 313)
#define OPENISA__NR_ioprio_set			(OPENISA__NR_Linux + 314)
#define OPENISA__NR_ioprio_get			(OPENISA__NR_Linux + 315)
#define OPENISA__NR_utimensat			(OPENISA__NR_Linux + 316)
#define OPENISA__NR_signalfd			(OPENISA__NR_Linux + 317)
#define OPENISA__NR_timerfd			(OPENISA__NR_Linux + 318)
#define OPENISA__NR_eventfd			(OPENISA__NR_Linux + 319)
#define OPENISA__NR_fallocate			(OPENISA__NR_Linux + 320)
#define OPENISA__NR_timerfd_create		(OPENISA__NR_Linux + 321)
#define OPENISA__NR_timerfd_gettime		(OPENISA__NR_Linux + 322)
#define OPENISA__NR_timerfd_settime		(OPENISA__NR_Linux + 323)
#define OPENISA__NR_signalfd4			(OPENISA__NR_Linux + 324)
#define OPENISA__NR_eventfd2			(OPENISA__NR_Linux + 325)
#define OPENISA__NR_epoll_create1		(OPENISA__NR_Linux + 326)
#define OPENISA__NR_dup3			(OPENISA__NR_Linux + 327)
#define OPENISA__NR_pipe2			(OPENISA__NR_Linux + 328)
#define OPENISA__NR_inotify_init1		(OPENISA__NR_Linux + 329)
#define OPENISA__NR_preadv			(OPENISA__NR_Linux + 330)
#define OPENISA__NR_pwritev			(OPENISA__NR_Linux + 331)
#define OPENISA__NR_rt_tgsigqueueinfo		(OPENISA__NR_Linux + 332)
#define OPENISA__NR_perf_event_open		(OPENISA__NR_Linux + 333)
#define OPENISA__NR_accept4			(OPENISA__NR_Linux + 334)
#define OPENISA__NR_recvmmsg			(OPENISA__NR_Linux + 335)
#define OPENISA__NR_fanotify_init		(OPENISA__NR_Linux + 336)
#define OPENISA__NR_fanotify_mark		(OPENISA__NR_Linux + 337)
#define OPENISA__NR_prlimit64			(OPENISA__NR_Linux + 338)
#define OPENISA__NR_name_to_handle_at		(OPENISA__NR_Linux + 339)
#define OPENISA__NR_open_by_handle_at		(OPENISA__NR_Linux + 340)
#define OPENISA__NR_clock_adjtime		(OPENISA__NR_Linux + 341)
#define OPENISA__NR_syncfs			(OPENISA__NR_Linux + 342)
#define OPENISA__NR_sendmmsg			(OPENISA__NR_Linux + 343)
#define OPENISA__NR_setns			(OPENISA__NR_Linux + 344)
#define OPENISA__NR_process_vm_readv		(OPENISA__NR_Linux + 345)
#define OPENISA__NR_process_vm_writev		(OPENISA__NR_Linux + 346)
#define OPENISA__NR_kcmp			(OPENISA__NR_Linux + 347)
#define OPENISA__NR_finit_module		(OPENISA__NR_Linux + 348)

/*
 * Offset of the last Linux o32 flavoured syscall
 */
#define OPENISA__NR_Linux_syscalls		348


int *openisa_syscall::get_syscall_table() {
  static int syscall_table[] = {
    OPENISA__NR_restart_syscall,
    OPENISA__NR_exit,
    OPENISA__NR_fork,
    OPENISA__NR_read,
    OPENISA__NR_write,
    OPENISA__NR_open,
    OPENISA__NR_close,
    OPENISA__NR_creat,
    OPENISA__NR_time,
    OPENISA__NR_lseek,
    OPENISA__NR_getpid,
    OPENISA__NR_access,
    OPENISA__NR_kill,
    OPENISA__NR_dup,
    OPENISA__NR_times,
    OPENISA__NR_brk,
    OPENISA__NR_mmap,
    OPENISA__NR_munmap,
    OPENISA__NR_stat,
    OPENISA__NR_lstat,
    OPENISA__NR_fstat,
    OPENISA__NR_uname,
    OPENISA__NR__llseek,
    OPENISA__NR_readv,
    OPENISA__NR_writev,
    OPENISA__NR_mmap2,
    OPENISA__NR_stat64,
    OPENISA__NR_lstat64,
    OPENISA__NR_fstat64,
    OPENISA__NR_getuid,
    OPENISA__NR_getgid,
    OPENISA__NR_geteuid,
    OPENISA__NR_getegid,
    OPENISA__NR_fcntl64,
    OPENISA__NR_exit_group,
    OPENISA__NR_socketcall,
    OPENISA__NR_gettimeofday,
    OPENISA__NR_settimeofday,
    OPENISA__NR_clock_gettime
  };
  return syscall_table;
}
