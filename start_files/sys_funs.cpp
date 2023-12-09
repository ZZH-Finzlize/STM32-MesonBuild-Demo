#include <cerrno>

#define UNUSED_PARAM \
    (void) fd;       \
    (void) buffer;   \
    (void) size

#ifdef __cplusplus
extern "C" {
#endif

int _write(int fd, char *buffer, int size)
{
    UNUSED_PARAM;
    return -ENOSYS;
}

int _read(int fd, char *buffer, int size)
{
    UNUSED_PARAM;
    return -ENOSYS;
}

int _exit(int fd, char *buffer, int size)
{
    UNUSED_PARAM;
    return -ENOSYS;
}

int _getpid(int fd, char *buffer, int size)
{
    UNUSED_PARAM;
    return -ENOSYS;
}

int _sbrk(int fd, char *buffer, int size)
{
    UNUSED_PARAM;
    return -ENOSYS;
}

int _kill(int fd, char *buffer, int size)
{
    UNUSED_PARAM;
    return -ENOSYS;
}

#ifdef __cplusplus
}
#endif
