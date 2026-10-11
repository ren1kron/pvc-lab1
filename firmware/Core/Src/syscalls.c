/* No host I/O exists in this firmware. Explicit newlib hooks prevent nosys
 * linker warnings; no UART, semihosting, file system or blocking I/O is used. */
#include <errno.h>
int _close(int fd) { (void)fd; errno = EBADF; return -1; }
int _lseek(int fd, int offset, int whence)
{ (void)fd; (void)offset; (void)whence; errno = ESPIPE; return -1; }
int _read(int fd, char *buffer, int length)
{ (void)fd; (void)buffer; (void)length; errno = ENOSYS; return -1; }
int _write(int fd, char *buffer, int length)
{ (void)fd; (void)buffer; (void)length; errno = ENOSYS; return -1; }
