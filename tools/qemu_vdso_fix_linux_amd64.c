/* Workaround qemu-x86_64 not supporting either of vdso/vsyscall and old glibc
 * versions unconditionally using vsyscall when vdso is not available (no
 * syscall fallback). */

/* Compile:
 * clang -O2 -shared -o tools/qemu_vdso_fix_linux_amd64.so tools/qemu_vdso_fix_linux_amd64.c
 */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <sys/syscall.h>
#include <sys/time.h>
#include <unistd.h>

int gettimeofday(struct timeval* tv, struct timezone* tz)
{
  return syscall(SYS_gettimeofday, tv, tz);
}
